#include "prio_thread.h"

/* pifus */
#include "pifus_ring_buffer.h"
#include "pifus_shmem.h"
#include "stack.h"
#include "utils/futex.h"
#include "utils/log.h"

/* third party */
#include "list/linked_list.h"

/* std */
#include "errno.h"
#include "pthread.h"
#include "stdlib.h"
typedef struct pifus_socket *socket_ptr_t;

struct Foo {
  socket_ptr_t *socket_ptr;
};

struct prio_thread_arguments {
  struct pifus_internal_operation_ring_buffer *tx_queue;
  struct pifus_internal_operation *tx_queue_buffer;

  struct pifus_socket_identifier_queue *socket_queue;
  struct pifus_socket_identifier *socket_queue_buffer;
  futex_t *socket_queue_futex;
};

bool has_socket_max_parallel_ops(struct pifus_socket *socket) {
  return (socket->enqueued_ops - socket->dequeued_ops >=
          TX_MAX_PARALLEL_OPS_PER_SOCKET);
}

/**
 * @brief Handles event of a 'put' in the squeue of a particular socket.
 */
void handle_squeue_change(struct pifus_socket *socket,
                          struct prio_thread_arguments *args) {
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

      if (pifus_internal_operation_ring_buffer_put(
              args->tx_queue, args->tx_queue_buffer, internal_op)) {
        pifus_operation_ring_buffer_erase_first(&socket->squeue);
        shadow_actual_difference--;

        // refresh shadow variable of futex
        socket_futexes[app_index][socket_index]++;

        socket->enqueued_ops++;
      } else {
        pifus_log("pifus_prio: Could not put() into tx_queue. Is it full?\n");
        return;
      }
    } else {
      pifus_debug_log("pifus_prio: Could not get() from squeue.\n");
      return;
    }
  }
}

uint8_t fill_prio_waitv(struct futex_waitv *waitvs,
                        struct pifus_socket *socket_from_futex_nr[],
                        List *socket_ptr_list,
                        struct prio_thread_arguments *args) {
  uint8_t current_amount_futexes = 0;

  /* iterate over responsible sockets */
  void *socket_ptr_ptr;

  bool removable_socket_inside_list = false;
  ListFirst(socket_ptr_list, false);
  while (ListNext(socket_ptr_list, &socket_ptr_ptr)) {
    struct pifus_socket *socket = *((socket_ptr_t *)socket_ptr_ptr);
    if (socket == NULL) {
      removable_socket_inside_list = true;
    } else {
      app_index_t app_index = socket->identifier.app_index;
      socket_index_t socket_index = socket->identifier.socket_index;

      if (socket->squeue_futex != socket_futexes[app_index][socket_index]) {
        handle_squeue_change(socket, args);
      }

      waitvs[current_amount_futexes].uaddr = (uintptr_t)&socket->squeue_futex;
      waitvs[current_amount_futexes].flags = FUTEX_32;
      waitvs[current_amount_futexes].val =
          socket_futexes[app_index][socket_index];
      waitvs[current_amount_futexes].__reserved = 0;

      socket_from_futex_nr[current_amount_futexes] = socket;

      current_amount_futexes++;
    }
  }

  // keep list consistent! Can't remove while iterating, so do it one-by-one
  while (removable_socket_inside_list) {
    uint16_t current_index = 0;

    ListFirst(socket_ptr_list, false);
    while (ListNext(socket_ptr_list, &socket_ptr_ptr)) {
      struct pifus_socket *socket = *((socket_ptr_t *)socket_ptr_ptr);
      if (socket == NULL) {
        ListRemove(socket_ptr_list, current_index);
        break;
      }
      current_index++;
    }
    removable_socket_inside_list = false;
  }

  return current_amount_futexes;
}

void *prio_thread_loop(void *arg) {
  struct prio_thread_arguments *args = arg;

  List *list = ListInit();
  static struct futex_waitv waitvs[TX_MAX_FUTEXES_PER_THREAD];
  /** futex nr to pifus_socket* **/
  struct pifus_socket *socket_from_futex_nr[TX_MAX_FUTEXES_PER_THREAD];
  futex_t new_socket_queue_futex_shadow = 0;

  while (true) {
    /* add new sockets */
    struct pifus_socket_identifier new_socket;
    while (pifus_socket_identifier_queue_get(
        args->socket_queue, args->socket_queue_buffer, &new_socket)) {

      // ptr to a socket_ptr (entry in buffer)
      socket_ptr_t *socket_ptr =
          &socket_ptrs[new_socket.app_index][new_socket.socket_index];

      ListPushFront(list, socket_ptr);
      new_socket_queue_futex_shadow++;
    }

    // futexes from sockets squeue
    uint8_t amount_futexes =
        fill_prio_waitv(waitvs, socket_from_futex_nr, list, args);
    // futex from new_socket_queue
    waitvs[amount_futexes].uaddr = (uintptr_t)args->socket_queue_futex;
    waitvs[amount_futexes].flags = FUTEX_32;
    waitvs[amount_futexes].val = new_socket_queue_futex_shadow;
    waitvs[amount_futexes].__reserved = 0;
    amount_futexes++;

    int ret_code = futex_waitv(waitvs, amount_futexes, 0, NULL, 0);
    if (ret_code >= 0) {
      if (socket_from_futex_nr[ret_code] == NULL) {
        pifus_debug_log(
            "pifus_prio: Received new_socket_queue notification!\n");
        // must be new_socket_queue notification, just run through so we pop
        // next iteration
      } else {
        handle_squeue_change(socket_from_futex_nr[ret_code], args);
      }

      /* clean up previous mapping */
      memset(socket_from_futex_nr, 0, sizeof(socket_from_futex_nr));
    }
  }
}

void start_prio_thread(struct pifus_internal_operation_ring_buffer *tx_queue,
                       struct pifus_internal_operation *tx_queue_buffer,
                       struct pifus_new_socket_queue *new_socket_queue) {
  pthread_t prio_thread;

  // copy onto heap
  struct prio_thread_arguments *args =
      malloc(sizeof(struct prio_thread_arguments));
  args->tx_queue = tx_queue;
  args->tx_queue_buffer = tx_queue_buffer;
  args->socket_queue = &new_socket_queue->socket_queue;
  args->socket_queue_buffer = new_socket_queue->socket_queue_buffer;
  args->socket_queue_futex = &new_socket_queue->socket_queue_futex;

  int ret = pthread_create(&prio_thread, NULL, prio_thread_loop, args);
  if (ret < 0) {
    pifus_log("pifus_prio: Could not start prio thread due to %i!\n", errno);
  } else {
    pifus_log("pifus_prio: Started prio thread!\n");
  }
}
