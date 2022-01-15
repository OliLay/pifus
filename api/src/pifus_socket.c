#define _GNU_SOURCE

/* std includes */
#include <stdio.h>
#include <stdlib.h>

/* local includes */
#include "pifus.h"
#include "pifus_constants.h"
#include "pifus_shmem.h"
#include "pifus_socket.h"

struct pifus_socket* map_socket_region(void) {
    app_state->highest_socket_number++;
    char* shm_name;

    asprintf(&shm_name, "%s%s%lu", app_shm_name, SHM_SOCKET_NAME_PREFIX,
             app_state->highest_socket_number);
    
    int fd = shm_create_region(shm_name);

    return shm_map_region(fd, SHM_SOCKET_SIZE);
}

void allocate_structures(struct pifus_socket* socket) {
    pifus_ring_buffer_create(&socket->squeue, socket->squeue_buffer,
                             SQUEUE_SIZE);
    pifus_ring_buffer_create(&socket->cqueue, socket->cqueue_buffer,
                             CQUEUE_SIZE);
}

struct pifus_socket* pifus_socket(void) {
    struct pifus_socket* socket = map_socket_region();

    allocate_structures(socket);
    /**
     * TODO:
     * - Store notification inside app shmem for stack-side (e.g. in a lock-free
     *queue) with ptr to socket's shmem, futex!
     **/

    return socket;
}

// TODO: remove from header
void enqueue_operation(struct pifus_socket* socket,
                       struct pifus_operation const op) {
    pifus_ring_buffer_put(&socket->squeue, op);
}

void pifus_socket_exit_all(void) {
    if (app_state->highest_socket_number > 0) {
        for (uint64_t i = 0; i <= app_state->highest_socket_number; i++) {
            char* socket_shm_name;
            asprintf(&socket_shm_name, "%s%s%lu", app_shm_name, SHM_SOCKET_NAME_PREFIX, i);
            shm_unlink_region(socket_shm_name);
        }
    }
}