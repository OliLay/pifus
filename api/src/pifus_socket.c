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

struct pifus_socket *map_socket_region(void);
void allocate_structures(struct pifus_socket *socket);
void notify_new_socket(void);
void notify_new_squeue_operation(struct pifus_socket *socket);

struct pifus_socket *map_socket_region(void)
{
    app_state->highest_socket_number++;
    char *shm_name;

    if (asprintf(&shm_name, "%s%s%u", app_shm_name, SHM_SOCKET_NAME_PREFIX,
                 app_state->highest_socket_number) < 0)
    {
        printf("pifus: error when calling asprintf\n");
    };

    int fd = shm_open_region(shm_name, true);

    free(shm_name);
    return (struct pifus_socket *)shm_map_region(fd, SHM_SOCKET_SIZE, true);
}

void allocate_structures(struct pifus_socket *socket)
{
    pifus_ring_buffer_create(&socket->squeue, SQUEUE_SIZE);
    pifus_ring_buffer_create(&socket->cqueue, CQUEUE_SIZE);
}

void notify_new_socket(void)
{
    int retcode = futex_wake(&app_state->highest_socket_number);

    if (retcode < 0)
    {
        printf(
            "pifus: Could not wake futex for notifying about new socket due to "
            "%i.\n",
            errno);
    }
}

struct pifus_socket *pifus_socket(enum protocol protocol)
{
    struct pifus_socket *socket = map_socket_region();

    socket->protocol = protocol;
    allocate_structures(socket);
    notify_new_socket();

    return socket;
}

void notify_new_squeue_operation(struct pifus_socket *socket)
{
    socket->squeue_futex++;
    futex_wake(&socket->squeue_futex);
}

// TODO: remove from header, should be only internal function, that is called by e.g. recv, send
void enqueue_operation(struct pifus_socket *socket,
                       struct pifus_operation const op)
{
    pifus_ring_buffer_put(&socket->squeue, socket->squeue_buffer, op);
    notify_new_squeue_operation(socket);
}

void pifus_socket_exit_all(void)
{
    if (app_state->highest_socket_number > 0)
    {
        for (socket_index_t i = 0; i <= app_state->highest_socket_number; i++)
        {
            char *socket_shm_name;
            if (asprintf(&socket_shm_name, "%s%s%u", app_shm_name,
                         SHM_SOCKET_NAME_PREFIX, i) < 0)
            {
                printf("pifus: error when calling asprintf\n");
                continue;
            }
            shm_unlink_region(socket_shm_name);

            free(socket_shm_name);
        }
    }
}