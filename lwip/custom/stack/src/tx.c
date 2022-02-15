#define _GNU_SOURCE
#include "tx.h"

/* std */
#include "errno.h"
#include "pthread.h"
#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"

/* lwip */
#include "lwip/tcp.h"

/* pifus */
#include "pifus_constants.h"
#include "pifus_ring_buffer.h"
#include "pifus_shmem.h"
#include "pifus_socket.h"
#include "stack.h"
#include "utils/futex.h"
#include "utils/log.h"

pthread_t tx_thread;
static struct futex_waitv waitvs[TX_MAX_FUTEXES_PER_THREAD];
/** shadow variables of futexes **/
futex_t app_futexes[MAX_APP_AMOUNT];
futex_t socket_futexes[MAX_APP_AMOUNT][MAX_SOCKETS_PER_APP];

/** app_local_highest_socket_number[#app] -> highest active socket number */
socket_index_t app_local_highest_socket_number[MAX_APP_AMOUNT];
/** futex nr to app_index **/
app_index_t app_from_futex_nr[MAX_APP_AMOUNT];
/** futex nr to pifus_socket_identifier **/
struct pifus_socket *socket_from_futex_nr[MAX_SOCKETS_PER_APP];

/**
 * @brief Maps new sockets (if any) for the app with the given index.
 *
 * @param app_index The index of the app to check.
 */
void map_new_sockets(app_index_t app_index) {
  socket_index_t current_highest_index =
      app_local_highest_socket_number[app_index];
  socket_index_t app_highest_index = app_ptrs[app_index]->highest_socket_number;

  if (current_highest_index < app_highest_index) {
    char *shm_name;
    for (socket_index_t socket_index = current_highest_index + 1;
         socket_index <= app_highest_index; socket_index++) {
      asprintf(&shm_name, "%s%u%s%u", SHM_APP_NAME_PREFIX, app_index,
               SHM_SOCKET_NAME_PREFIX, socket_index);

      int fd = shm_open_region(shm_name, false);

      if (fd < 0) {
        pifus_log("pifus_tx: failed to map '%s' with errno %i!\n", shm_name,
                  errno);
      } else {
        struct pifus_socket *socket =
            shm_map_region(fd, SHM_SOCKET_SIZE, false);
        socket_ptrs[app_index][socket_index] = socket;

        socket->identifier.app_index = app_index;
        socket->identifier.socket_index = socket_index;

        pifus_operation_result_ring_buffer_create(&socket->cqueue, CQUEUE_SIZE);
        pifus_write_queue_create(&socket->write_queue, WRITE_QUEUE_SIZE);
        pifus_recv_queue_create(&socket->recv_queue, RECV_QUEUE_SIZE);
        pifus_byte_buffer_create(&socket->recv_buffer, RECV_BUFFER_SIZE);

        pifus_log("pifus_tx: Mapped (%u/%u)\n", app_index, socket_index);

        if (socket->protocol == PROTOCOL_TCP) {
          if (socket->pcb.tcp != NULL) {
            /** this has to be done if we have a listening socket, so that
             * tcp_arg is set even though no operation has been queued from
             * that socket yet but we can still receive data.
             * NOTE: theoretically, this is not save (due to not being called
             * from the stack thread), but tcp_arg only sets a single variable,
             * which can not be set at the same time from the main thread.) */
            tcp_arg(socket->pcb.tcp, socket);
            pifus_log("pifus_tx: (out of order) set arg for socket (%u/%u)\n",
                      app_index, socket_index);
          }
        }

        app_local_highest_socket_number[app_index] = socket_index;
      }

      free(shm_name);
    }
  }
}

/**
 * @brief Searches for new app shmem regions (starting at next_app_number) and
 * maps them.
 */
void scan_for_app_regions(void) {
  char *app_shm_name = NULL;

  for (app_index_t i = next_app_number; i < MAX_APP_AMOUNT; i++) {
    asprintf(&app_shm_name, "%s%u", SHM_APP_NAME_PREFIX, next_app_number);

    int fd = shm_open_region(app_shm_name, false);

    if (fd >= 0) {
      app_ptrs[next_app_number] =
          (struct pifus_app *)shm_map_region(fd, SHM_APP_SIZE, false);
      next_app_number++;
    } else {
      return;
    }

    free(app_shm_name);
  }
}

/**
 * @brief Handles event of new socket creation.
 *
 * @param app_index The app_index where new socket(s) have been created.
 */
void handle_new_sockets(app_index_t app_index) {
  pifus_log("pifus_tx: new socket(s) detected for app %u\n", app_index);

  // refresh shadow variable of futex
  app_futexes[app_index] = app_ptrs[app_index]->highest_socket_number;

  map_new_sockets(app_index);
}

bool has_socket_max_parallel_ops(struct pifus_socket *socket) {
  return (socket->enqueued_ops - socket->dequeued_ops >=
          TX_MAX_PARALLEL_OPS_PER_SOCKET);
}

/**
 * @brief Handles event of a 'put' in the squeue of a particular socket.
 *
 * @param socket Ptr to the socket
 * squeue.
 */
void handle_squeue_change(struct pifus_socket *socket) {
  app_index_t app_index = socket->identifier.app_index;
  socket_index_t socket_index = socket->identifier.socket_index;

  futex_t old_shadow_value = socket_futexes[app_index][socket_index];
  // get diff so we know how much has been inserted into the squeue
  size_t shadow_actual_difference = socket->squeue_futex - old_shadow_value;

  while (shadow_actual_difference > 0 && !has_socket_max_parallel_ops(socket)) {
    struct pifus_operation *op;
    if (pifus_operation_ring_buffer_peek(&socket->squeue, socket->squeue_buffer,
                                         &op)) {
      struct pifus_internal_operation internal_op;
      internal_op.operation = *op;
      internal_op.socket = socket;
      if (pifus_tx_ring_buffer_put(&tx_queue.ring_buffer,
                                   tx_queue.tx_queue_buffer, internal_op)) {
        pifus_operation_ring_buffer_erase_first(&socket->squeue);
        shadow_actual_difference--;

        // refresh shadow variable of futex
        socket_futexes[app_index][socket_index]++;
        socket->enqueued_ops++;
      } else {
        pifus_log("pifus_tx: Could not put() into tx_queue. Is it full?\n");
        return;
      }
    } else {
      pifus_debug_log("pifus_tx: Could not get() from squeue.\n");
      return;
    }
  }
}

/**
 * @brief Fills the waitv structure with both app and socket futexes, so that
 * afterwards futex_waitv can be called to wait for the futexes.
 *
 * @return uint8_t The amount of futexes to wait for.
 */
uint8_t fill_waitv(void) {
  uint8_t current_amount_futexes = 0;
  for (app_index_t app_index = 0; app_index < next_app_number; app_index++) {
    struct pifus_app *current_app_ptr = app_ptrs[app_index];

    if (current_app_ptr != NULL) {
      if (current_app_ptr->highest_socket_number != app_futexes[app_index]) {
        handle_new_sockets(app_index);
      }

      waitvs[current_amount_futexes].uaddr =
          (uintptr_t)&current_app_ptr->highest_socket_number;
      waitvs[current_amount_futexes].flags = FUTEX_32;
      waitvs[current_amount_futexes].val = app_futexes[app_index];
      waitvs[current_amount_futexes].__reserved = 0;
      app_from_futex_nr[current_amount_futexes] = app_index;

      current_amount_futexes++;

      for (socket_index_t socket_index = 1;
           socket_index <= app_ptrs[app_index]->highest_socket_number;
           socket_index++) {
        struct pifus_socket *current_socket_ptr =
            socket_ptrs[app_index][socket_index];

        if (current_socket_ptr != NULL) {
          if (current_socket_ptr->squeue_futex !=
              socket_futexes[app_index][socket_index]) {
            current_socket_ptr->identifier.app_index = app_index;
            current_socket_ptr->identifier.socket_index = socket_index;
            handle_squeue_change(current_socket_ptr);
          }

          waitvs[current_amount_futexes].uaddr =
              (uintptr_t)&current_socket_ptr->squeue_futex;
          waitvs[current_amount_futexes].flags = FUTEX_32;
          waitvs[current_amount_futexes].val =
              socket_futexes[app_index][socket_index];
          waitvs[current_amount_futexes].__reserved = 0;

          socket_from_futex_nr[current_amount_futexes] = current_socket_ptr;

          current_amount_futexes++;
        }
      }
    }
  }

  return current_amount_futexes;
}

/**
 * @brief This loop essentially does three major things:
 * - scan for new app regions every iteration
 * - futex_waitv on all app futexes (if triggered -> add new sockets)
 * - futex_waitv on all socket squeue futexes (if triggered -> dequeue op)
 *
 * @param arg unused
 * @return void* no return value
 */
void *tx_thread_loop(void *arg) {
  while (true) {
    // TODO: this spins if we have no futexes to wait on (so only until
    // first app connected)
    scan_for_app_regions();

    uint8_t amount_futexes = fill_waitv();
    if (amount_futexes > 0) {
      // TODO: use tv_nsec to allow MSEC constant, needs time arithmetic
      struct timespec timespec;
      clock_gettime(CLOCK_MONOTONIC, &timespec);
      timespec.tv_sec += TX_WAIT_TIMEOUT_SEC;
      timespec.tv_nsec = 0;

      int ret_code =
          futex_waitv(waitvs, amount_futexes, 0, &timespec, CLOCK_MONOTONIC);
      if (ret_code >= 0) {
        struct futex_waitv signaled_wait = waitvs[ret_code];

        if (socket_from_futex_nr[ret_code] == NULL) {
          handle_new_sockets(app_from_futex_nr[ret_code]);
        } else {
          handle_squeue_change(socket_from_futex_nr[ret_code]);
        }

        /* clean up previous mappings */
        memset(app_from_futex_nr, 0, sizeof(app_from_futex_nr));
        memset(socket_from_futex_nr, 0, sizeof(socket_from_futex_nr));
      }
    }
  }
}

void start_tx_thread(void) {
  int ret = pthread_create(&tx_thread, NULL, tx_thread_loop, NULL);

  if (ret < 0) {
    pifus_log("pifus_tx: Could not start TX thread due to %i!\n", errno);
  } else {
    pifus_debug_log("pifus_tx: Started TX thread!\n");
  }
}
