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

struct pifus_socket* map_socket_region(void) {
    app_state->highest_socket_number++;
    char* shm_name;

    asprintf(&shm_name, "%s%s%u", app_shm_name, SHM_SOCKET_NAME_PREFIX,
             app_state->highest_socket_number);

    int fd = shm_open_region(shm_name, true);

    return shm_map_region(fd, SHM_SOCKET_SIZE, true);
}

void allocate_structures(struct pifus_socket* socket) {
    pifus_ring_buffer_create(&socket->squeue, socket->squeue_buffer,
                             SQUEUE_SIZE);
    pifus_ring_buffer_create(&socket->cqueue, socket->cqueue_buffer,
                             CQUEUE_SIZE);
}

void notify_new_socket(void) {
    // TODO: should be nonblocking!
    int retcode = futex_wake(&app_state->highest_socket_number);

    if (retcode < 0) {
        printf(
            "pifus: Could not wake futex for notifying about new socket due to "
            "%i.\n",
            errno);
    }
}

struct pifus_socket* pifus_socket(enum protocol protocol) {
    struct pifus_socket* socket = map_socket_region();

    socket->protocol = protocol;
    allocate_structures(socket);
    notify_new_socket();

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
            asprintf(&socket_shm_name, "%s%s%lu", app_shm_name,
                     SHM_SOCKET_NAME_PREFIX, i);
            shm_unlink_region(socket_shm_name);
        }
    }
}