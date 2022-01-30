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

struct pifus_socket *map_socket_region(void);
void enqueue_operation(struct pifus_socket *socket,
                       struct pifus_operation const op);
void free_write_buffers(struct pifus_operation_result *operation_result);

struct pifus_socket *map_socket_region(void) {
  app_state->highest_socket_number++;
  char *shm_name;

  if (asprintf(&shm_name, "%s%s%u", app_shm_name, SHM_SOCKET_NAME_PREFIX,
               app_state->highest_socket_number) < 0) {
    pifus_log("pifus: error when calling asprintf\n");
  };

  int fd = shm_open_region(shm_name, true);

  free(shm_name);
  return (struct pifus_socket *)shm_map_region(fd, SHM_SOCKET_SIZE, true);
}

void free_write_buffers(struct pifus_operation_result *operation_result) {
  if (operation_result->code == TCP_WRITE ||
      operation_result->code == UDP_SEND) {
    struct pifus_memory_block *block = shm_data_get_block_ptr(
        app_state, operation_result->data.write.block_offset);
    shm_data_free(block);
  }
}

void enqueue_operation(struct pifus_socket *socket,
                       struct pifus_operation const op) {
  pifus_operation_ring_buffer_put(&socket->squeue, socket->squeue_buffer, op);

  socket->squeue_futex++;
  futex_wake(&socket->squeue_futex);
}

struct pifus_socket *pifus_socket(enum protocol protocol) {
  struct pifus_socket *socket = map_socket_region();

  socket->protocol = protocol;
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

void pifus_socket_bind(struct pifus_socket *socket, enum pifus_ip_type ip_type,
                       uint16_t port) {
  struct pifus_operation bind_operation;

  if (socket->protocol == PROTOCOL_TCP) {
    bind_operation.code = TCP_BIND;
  } else {
    bind_operation.code = UDP_BIND;
  }

  bind_operation.data.bind.ip_type = ip_type;
  bind_operation.data.bind.port = port;

  enqueue_operation(socket, bind_operation);
}

void pifus_socket_connect(struct pifus_socket *socket,
                          struct pifus_ip_addr ip_addr, uint16_t port) {
  struct pifus_operation connect_operation;

  if (socket->protocol == PROTOCOL_TCP) {
    connect_operation.code = TCP_CONNECT;
  } else {
    connect_operation.code = UDP_CONNECT;
  }

  connect_operation.data.connect.ip_addr = ip_addr;
  connect_operation.data.connect.port = port;

  enqueue_operation(socket, connect_operation);
}

bool pifus_socket_write(struct pifus_socket *socket, void *data, size_t size) {
  struct pifus_operation write_operation;

  if (socket->protocol == PROTOCOL_TCP) {
    write_operation.code = TCP_WRITE;
  } else {
    write_operation.code = UDP_SEND;
  }

  ptrdiff_t block_offset;
  struct pifus_memory_block *block = NULL;

  if (!shm_data_allocate(app_state, size, &block_offset, &block)) {
    pifus_log("pifus: Could not allocate memory for write()!\n");
    return false;
  }

  memcpy(shm_data_get_data_ptr(block), data, size);

  write_operation.data.write.block_offset = block_offset;
  write_operation.data.write.size = size;

  enqueue_operation(socket, write_operation);

  return true;
}

void pifus_socket_wait(struct pifus_socket *socket,
                       struct pifus_operation_result *operation_result) {
  if (pifus_operation_result_ring_buffer_get(
          &socket->cqueue, socket->cqueue_buffer, operation_result)) {

    free_write_buffers(operation_result);
    return;
  }

  while (true) {
    if (futex_wait(&socket->cqueue_futex, socket->cqueue_futex) < 0) {
      pifus_debug_log("pifus: futex_wait in socket_poll returned %s\n",
                      strerror(errno));
    } else {
      if (pifus_operation_result_ring_buffer_get(
              &socket->cqueue, socket->cqueue_buffer, operation_result)) {
        free_write_buffers(operation_result);

        return;
      }
    }
  }
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