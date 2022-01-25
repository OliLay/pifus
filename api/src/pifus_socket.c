#define _GNU_SOURCE

/* std includes */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

/* local includes */
#include "pifus.h"
#include "pifus_constants.h"
#include "pifus_shmem.h"
#include "pifus_socket.h"
#include "utils/futex.h"
#include "utils/log.h"

struct pifus_socket *map_socket_region(void);
void allocate_structures(struct pifus_socket *socket);
void enqueue_operation(struct pifus_socket *socket,
                       struct pifus_operation const op);

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

void allocate_structures(struct pifus_socket *socket) {
  pifus_operation_ring_buffer_create(&socket->squeue, SQUEUE_SIZE);
  pifus_operation_result_ring_buffer_create(&socket->cqueue, CQUEUE_SIZE);
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
  allocate_structures(socket);

  int retcode = futex_wake(&app_state->highest_socket_number);
  if (retcode < 0) {
    pifus_log(
        "pifus: Could not wake futex for notifying about new socket due to "
        "%i.\n",
        errno);
  }

  return socket;
}

void pifus_socket_bind(struct pifus_socket *socket, enum ip_type ip_type,
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

void pifus_socket_poll(struct pifus_socket *socket,
                       struct pifus_operation_result *operation_result) {
  while (!pifus_operation_result_ring_buffer_get(
      &socket->cqueue, socket->cqueue_buffer, operation_result)) {
    // spins
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