#define _GNU_SOURCE

/* std includes */
#include <stdio.h>
#include <stdlib.h>

/* local includes */
#include "pifus.h"
#include "pifus_constants.h"
#include "pifus_shmem.h"
#include "pifus_socket.h"
#include "data_structures/pifus_ring_buffer.h"
#include "data_structures/ext/ring_buf.h"

typedef struct pifus_ring_buffer pifus_squeue;
typedef struct pifus_ring_buffer pifus_cqueue;

int* map_socket_region(void) {
    state.highest_socket_number++;
    char* shm_name;

    asprintf(&shm_name, "%s%s%u", SHM_APP_NAME_PREFIX, SHM_SOCKET_NAME_PREFIX,
             state.highest_socket_number);

    int fd = shm_create_region(shm_name);

    return shm_map_region(fd, SHM_SOCKET_SIZE);
}

void allocate_squeue(struct pifus_socket* socket) {
    struct pifus_ring_buffer* squeue = malloc(sizeof(struct pifus_ring_buffer));
    
    // squeue is the first thing in socket shmem
    squeue->internal_ring_buffer = (RingBuf*) socket->shmem_ptr;

    // set new ptrs accordingly
    socket->squeue_ptr = socket->shmem_ptr;
    socket->squeue_buffer_ptr = socket->squeue_ptr + sizeof(RingBuf);

    // map ptrs
    RingBuf* squeue_ptr = (RingBuf*) socket->squeue_ptr;
    struct pifus_operation* squeue_buffer_ptr = (struct pifus_operation*) socket->squeue_buffer_ptr;

    pifus_ring_buffer_create(squeue, squeue_buffer_ptr, SQUEUE_SIZE);
}

struct pifus_socket* socket(void) {
    struct pifus_socket* socket = malloc(sizeof(struct pifus_socket));

    socket->shmem_ptr = map_socket_region();

    allocate_squeue(socket);
    /**
     * TODO:
     * - DONE: Alloc shmem area (appX/socketY) with latest socket # + 1
     * - Alloc cqueue (NOT DONE), squeue (DONE) inside shmem
     * - Store notification inside app shmem for stack-side (e.g. in a lock-free
     *queue) with ptr to socket's shmem
     * - Return internal pifus_socket ptr for further usage
     **/

    return socket;
}