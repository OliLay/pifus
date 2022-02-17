#define _GNU_SOURCE

/* std includes */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* local includes */
#include "pifus.h"
#include "pifus_constants.h"
#include "pifus_shmem.h"
#include "pifus_socket.h"
#include "utils/futex.h"
#include "utils/log.h"

struct pifus_socket *sockets[MAX_SOCKETS_PER_APP];

struct pifus_socket *map_socket_region(void);
bool is_queue_full(struct pifus_socket *socket);
bool enqueue_operation(struct pifus_socket *socket,
                       struct pifus_operation const op);
bool dequeue_operation(struct pifus_socket *socket,
                       struct pifus_operation_result *operation_result);
void free_write_buffers(struct pifus_app *app, uint64_t block_offset);
struct pifus_socket *pifus_socket_with_tcp_pcb(enum protocol protocol,
                                               void *tcp_pcb);
void unlink_socket(struct pifus_socket *socket);

bool is_queue_full(struct pifus_socket *socket) {
  return pifus_operation_ring_buffer_is_full(&socket->squeue);
}

struct pifus_socket *map_socket_region(void) {
  app_state->highest_socket_number++;
  char *shm_name;

  if (asprintf(&shm_name, "%s%s%u", app_shm_name, SHM_SOCKET_NAME_PREFIX,
               app_state->highest_socket_number) < 0) {
    pifus_log("pifus: error when calling asprintf\n");
  };

  int fd = shm_open_region(shm_name, true);

  free(shm_name);

  struct pifus_socket *socket =
      (struct pifus_socket *)shm_map_region(fd, SHM_SOCKET_SIZE, true);
  sockets[app_state->highest_socket_number] = socket;

  return socket;
}

bool enqueue_operation(struct pifus_socket *socket,
                       struct pifus_operation const op) {
  if (!pifus_operation_ring_buffer_put(&socket->squeue, socket->squeue_buffer,
                                       op)) {
    pifus_log("pifus: Could not enqueue to squeue, maybe its full?\n");
    return false;
  }

  socket->squeue_futex++;
  futex_wake(&socket->squeue_futex);
  return true;
}

struct pifus_socket *pifus_socket(enum protocol protocol) {
  return pifus_socket_with_tcp_pcb(protocol, NULL);
}

struct pifus_socket *pifus_socket_with_tcp_pcb(enum protocol protocol,
                                               void *tcp_pcb) {
  struct pifus_socket *socket = map_socket_region();

  socket->protocol = protocol;

  if (tcp_pcb != NULL) {
    pifus_debug_log("pifus: Created new socket with given PCB!\n");
    socket->pcb.tcp = tcp_pcb;
  }

  pifus_operation_ring_buffer_create(&socket->squeue, SQUEUE_SIZE);

  int retcode = futex_wake(&app_state->highest_socket_number);
  if (retcode < 0) {
    pifus_log(
        "pifus: Could not wake futex for notifying about new socket due to "
        "%i.\n",
        errno);
  }

  return socket;
}

bool pifus_socket_bind(struct pifus_socket *socket, enum pifus_ip_type ip_type,
                       uint16_t port) {
  if (is_queue_full(socket)) {
    return false;
  }
  struct pifus_operation bind_operation;

  if (socket->protocol == PROTOCOL_TCP) {
    bind_operation.code = TCP_BIND;
  } else {
    bind_operation.code = UDP_BIND;
  }

  bind_operation.data.bind.ip_type = ip_type;
  bind_operation.data.bind.port = port;

  return enqueue_operation(socket, bind_operation);
}

bool pifus_socket_connect(struct pifus_socket *socket,
                          struct pifus_ip_addr ip_addr, uint16_t port) {
  if (is_queue_full(socket)) {
    return false;
  }

  struct pifus_operation connect_operation;

  if (socket->protocol == PROTOCOL_TCP) {
    connect_operation.code = TCP_CONNECT;
  } else {
    connect_operation.code = UDP_CONNECT;
  }

  connect_operation.data.connect.ip_addr = ip_addr;
  connect_operation.data.connect.port = port;

  return enqueue_operation(socket, connect_operation);
}

bool pifus_socket_write(struct pifus_socket *socket, void *data, size_t size) {
  if (is_queue_full(socket)) {
    return false;
  }

  struct pifus_operation write_operation;
  if (socket->protocol == PROTOCOL_TCP) {
    write_operation.code = TCP_WRITE;
  } else {
    write_operation.code = UDP_SEND;
  }

  block_offset_t block_offset;
  struct pifus_memory_block *block = NULL;

  if (!shm_data_allocate(app_state, size, &block_offset, &block)) {
    pifus_log("pifus: Could not allocate memory for write()!\n");
    return false;
  }

  memcpy(shm_data_get_data_ptr(block), data, size);

  write_operation.data.write.block_offset = block_offset;

  return enqueue_operation(socket, write_operation);
}

void free_write_buffers(struct pifus_app *app, uint64_t block_offset) {
  struct pifus_memory_block *block = shm_data_get_block_ptr(app, block_offset);
  shm_data_free(app, block);
}

bool pifus_socket_recv(struct pifus_socket *socket, size_t size) {
  if (is_queue_full(socket)) {
    return false;
  }

  struct pifus_operation recv_operation;
  if (socket->protocol == PROTOCOL_TCP) {
    recv_operation.code = TCP_RECV;
  } else {
    recv_operation.code = UDP_RECV;
  }

  block_offset_t block_offset;
  struct pifus_memory_block *block = NULL;

  if (!shm_data_allocate(app_state, size, &block_offset, &block)) {
    pifus_log("pifus: Could not allocate memory for recv()!\n");
    return false;
  }

  recv_operation.data.recv.recv_block_offset = block_offset;
  recv_operation.data.recv.size = size;

  return enqueue_operation(socket, recv_operation);
}

bool pifus_socket_listen(struct pifus_socket *socket, uint8_t backlog_size) {
  if (is_queue_full(socket)) {
    return false;
  }

  struct pifus_operation listen_operation;
  if (socket->protocol == PROTOCOL_TCP) {
    listen_operation.code = TCP_LISTEN;
  } else {
    // UDP has no listen
    return false;
  }

  listen_operation.data.listen.backlog_size = backlog_size;

  return enqueue_operation(socket, listen_operation);
}

bool pifus_socket_accept(struct pifus_socket *socket) {
  if (is_queue_full(socket)) {
    return false;
  }

  struct pifus_operation accept_operation;
  if (socket->protocol == PROTOCOL_TCP) {
    accept_operation.code = TCP_ACCEPT;
  } else {
    // UDP has no accept
    return false;
  }

  return enqueue_operation(socket, accept_operation);
}

bool pifus_socket_close(struct pifus_socket *socket) {
  if (is_queue_full(socket)) {
    return false;
  }

  struct pifus_operation close_operation;
  if (socket->protocol == PROTOCOL_TCP) {
    close_operation.code = TCP_CLOSE;
  } else {
    close_operation.code = UDP_DISCONNECT;
  }

  if (!enqueue_operation(socket, close_operation)) {
    return false;
  }

  return true;
}

bool dequeue_operation(struct pifus_socket *socket,
                       struct pifus_operation_result *operation_result) {

  bool success = pifus_operation_result_ring_buffer_get(
      &socket->cqueue, socket->cqueue_buffer, operation_result);

  if (success) {
    if (operation_result->code == TCP_WRITE) {
      free_write_buffers(app_state, operation_result->data.write.block_offset);
    }

    if (operation_result->code == TCP_ACCEPT &&
        operation_result->result_code == PIFUS_OK) {
      operation_result->data.accept.socket = pifus_socket_with_tcp_pcb(
          PROTOCOL_TCP, operation_result->data.accept.pcb);
    }

    if (operation_result->code == TCP_RECV ||
        operation_result->code == UDP_RECV) {
      if (operation_result->result_code == PIFUS_OK) {
        operation_result->data.recv.memory_block_ptr = shm_data_get_block_ptr(
            app_state, operation_result->data.recv.recv_block_offset);
      }
    }

    if (operation_result->code == TCP_CLOSE ||
        operation_result->code == UDP_DISCONNECT) {
      unlink_socket(socket);
      sockets[socket->identifier.socket_index] = NULL;
    }

    pifus_debug_log("Dequeued from cqueue\n");
  }

  return success;
}

bool pifus_socket_pop_result(struct pifus_socket *socket,
                             struct pifus_operation_result *operation_result) {
  return dequeue_operation(socket, operation_result);
}

bool pifus_socket_peek_result_code(struct pifus_socket *socket,
                                   enum pifus_operation_code **code) {
  struct pifus_operation_result* result = NULL;
  bool success = pifus_operation_result_ring_buffer_peek(
      &socket->cqueue, socket->cqueue_buffer, &result);
  
  if (success) {
    *code = &result->code;
    return true;
  } else {
    return false;
  }
}

void pifus_socket_wait(struct pifus_socket *socket,
                       struct pifus_operation_result *operation_result) {
  if (dequeue_operation(socket, operation_result)) {
    return;
  }

  while (true) {
    /* can only use futexes + wait when in poll-mode,
    because in callback mode, we already use futex_waitv to
    invoke the callback */
    if (callback == NULL &&
        futex_wait(&socket->cqueue_futex, socket->cqueue_futex) < 0) {
      pifus_log("pifus: futex_wait in socket_poll returned %s\n",
                strerror(errno));
    }

    if (dequeue_operation(socket, operation_result)) {
      return;
    }
  }
}

void unlink_socket(struct pifus_socket *socket) {
  char *socket_shm_name;
  if (asprintf(&socket_shm_name, "%s%s%u", app_shm_name, SHM_SOCKET_NAME_PREFIX,
               socket->identifier.socket_index) < 0) {
    pifus_log("pifus: error when calling asprintf\n");
    return;
  }
  shm_unlink_region(socket_shm_name);
  free(socket_shm_name);
}

void pifus_socket_exit_all(void) {
  if (app_state->highest_socket_number > 0) {
    for (socket_index_t i = 0; i <= app_state->highest_socket_number; i++) {
      char *socket_shm_name;
      if (asprintf(&socket_shm_name, "%s%s%u", app_shm_name,
                   SHM_SOCKET_NAME_PREFIX, i) < 0) {
        pifus_log("pifus: error when calling asprintf\n");
        continue;
      }
      shm_unlink_region(socket_shm_name);

      free(socket_shm_name);
    }
  }
}