#define _GNU_SOURCE
#include "discovery.h"

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
#include "prio_thread.h"
#include "stack.h"
#include "utils/futex.h"
#include "utils/log.h"

pthread_t discovery_thread;
static struct futex_waitv waitvs[MAX_APP_AMOUNT];
/** shadow variables of futexes **/
futex_t app_futexes[MAX_APP_AMOUNT];
/** app_local_highest_socket_number[#app] -> highest active socket number */
socket_index_t app_local_highest_socket_number[MAX_APP_AMOUNT];
/** futex nr to app_index **/
app_index_t app_from_futex_nr[MAX_APP_AMOUNT];
/** prio threads data structues **/
struct pifus_new_socket_queue high_prio_new_socket_queue;
struct pifus_new_socket_queue medium_prio_new_socket_queue;
struct pifus_new_socket_queue low_prio_new_socket_queue;

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
        pifus_log("pifus_discovery: failed to map '%s' with errno %i!\n",
                  shm_name, errno);
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

        pifus_log("pifus_discovery: Mapped (%u/%u)\n", app_index, socket_index);

        if (socket->protocol == PROTOCOL_TCP) {
          if (socket->pcb.tcp != NULL) {
            /** this has to be done if we have a listening socket, so that
             * tcp_arg is set even though no operation has been queued from
             * that socket yet but we can still receive data.
             * NOTE: theoretically, this is not save (due to not being called
             * from the stack thread), but tcp_arg only sets a single variable,
             * which can not be set at the same time from the main thread.) */
            tcp_arg(socket->pcb.tcp, socket);
            pifus_log(
                "pifus_discovery: (out of order) set arg for socket (%u/%u)\n",
                app_index, socket_index);
          }
        }

        app_local_highest_socket_number[app_index] = socket_index;

        struct pifus_new_socket_queue *new_socket_queue = NULL;
        if (socket->priority == PRIORITY_HIGH) {
          new_socket_queue = &high_prio_new_socket_queue;
        } else if (socket->priority == PRIORITY_MEDIUM) {
          new_socket_queue = &medium_prio_new_socket_queue;
        } else {
          new_socket_queue = &low_prio_new_socket_queue;
        }

        pifus_socket_identifier_queue_put(&new_socket_queue->socket_queue,
                                          new_socket_queue->socket_queue_buffer,
                                          socket->identifier);
        new_socket_queue->socket_queue_futex++;
        futex_wake(&new_socket_queue->socket_queue_futex);
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
  pifus_log("pifus_discovery: new socket(s) detected for app %u\n", app_index);

  // refresh shadow variable of futex
  app_futexes[app_index] = app_ptrs[app_index]->highest_socket_number;

  map_new_sockets(app_index);
}

/**
 * @brief Fills the waitv structure with both app futexes, so that
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
    }
  }

  return current_amount_futexes;
}

void start_prio_threads(void) {
  start_prio_thread(&tx_queue.high_prio_queue, tx_queue.high_prio_queue_buffer,
                    &high_prio_new_socket_queue);
  start_prio_thread(&tx_queue.medium_prio_queue,
                    tx_queue.medium_prio_queue_buffer,
                    &medium_prio_new_socket_queue);
  start_prio_thread(&tx_queue.low_prio_queue, tx_queue.low_prio_queue_buffer,
                    &low_prio_new_socket_queue);
}

/**
 * @brief This loop essentially does two major things:
 * - scan for new app regions every iteration
 * - futex_waitv on all app futexes (if triggered -> notify corresponding prio
 * thread)
 *
 * @param arg unused
 * @return void* no return value
 */
void *discovery_thread_loop(void *arg) {
  start_prio_threads();

  while (true) {
    // TODO: this spins if we have no futexes to wait on (so only until
    // first app connected)
    scan_for_app_regions();

    uint8_t amount_futexes = fill_waitv();
    if (amount_futexes > 0) {
      // TODO: use tv_nsec to allow MSEC constant, needs time arithmetic
      struct timespec timespec;
      clock_gettime(CLOCK_MONOTONIC, &timespec);
      timespec.tv_sec += DISCOVERY_WAIT_TIMEOUT_SEC;
      timespec.tv_nsec = 0;

      int ret_code =
          futex_waitv(waitvs, amount_futexes, 0, &timespec, CLOCK_MONOTONIC);
      if (ret_code >= 0) {
        handle_new_sockets(app_from_futex_nr[ret_code]);

        /* clean up previous mapping */
        memset(app_from_futex_nr, 0, sizeof(app_from_futex_nr));
      }
    }
  }
}

void start_discovery_thread(void) {
  pifus_socket_identifier_queue_create(&high_prio_new_socket_queue.socket_queue,
                                       DISCOVERY_MAX_NEW_SOCKETS);
  pifus_socket_identifier_queue_create(
      &medium_prio_new_socket_queue.socket_queue, DISCOVERY_MAX_NEW_SOCKETS);
  pifus_socket_identifier_queue_create(&low_prio_new_socket_queue.socket_queue,
                                       DISCOVERY_MAX_NEW_SOCKETS);

  int ret = pthread_create(&discovery_thread, NULL, discovery_thread_loop, NULL);

  if (ret < 0) {
    pifus_log("pifus_discovery: Could not start Discovery thread due to %i!\n",
              errno);
  } else {
    pifus_log("pifus_discovery: Started Discovery thread!\n");
  }
}
