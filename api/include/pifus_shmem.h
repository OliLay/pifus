#ifndef PIFUS_SHMEM_H
#define PIFUS_SHMEM_H

#include "pifus_constants.h"
#include "data_structures/pifus_ring_buffer.h"
#include "data_structures/ext/ring_buf.h"
#include "utils/futex.h"

typedef struct pifus_ring_buffer pifus_squeue;
typedef struct pifus_ring_buffer pifus_cqueue;

typedef uint32_t app_index_t;
typedef uint32_t socket_index_t;

enum protocol {
    PROTOCOL_TCP = 0,
    PROTOCOL_UDP = 1
};

/**
 * @brief Shmem layout of socket region.
 */
struct pifus_socket
{
    enum protocol protocol;
    /* squeue */
    futex_t squeue_futex;
    pifus_squeue squeue;
    struct pifus_operation squeue_buffer[SQUEUE_SIZE];
    /* cqueue */
    futex_t cqueue_futex;
    pifus_cqueue cqueue;
    struct pifus_operation cqueue_buffer[CQUEUE_SIZE];
};

/**
 * @brief Shmem layout of app region.
 */
struct pifus_app
{
    /* Gives index of highest socket. Used as futex for wake-up when new socket
     * is created */
    socket_index_t highest_socket_number;
};

/**
 * @brief Tries to open a shm region with the given name.
 * 
 * @param shm_name The name of the shm region.
 * @param create If the region should be created.
 * @return int the fd
 */
int shm_open_region(char *shm_name, bool create);

/**
 * @brief Maps the shared memory into the process memory space.
 *
 * @param fd The fd of the app's shared memory.
 * @param size Size of the shared mem region.
 * @param create If this is the first try of mapping the region.
 * @return void ptr to the mapped region
 */
void *shm_map_region(int fd, size_t size, bool create);

void shm_unlink_region(char *shm_name);

#endif /* PIFUS_SHMEM_H */
