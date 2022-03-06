#define _GNU_SOURCE

/* standard includes */
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "pthread.h"

/* shmem includes */
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

/* local includes */
#include "pifus.h"
#include "pifus_constants.h"
#include "pifus_shmem.h"
#include "pifus_socket.h"
#include "utils/log.h"

pthread_t callback_thread;
pifus_callback callback;
futex_t socket_futexes[MAX_SOCKETS_PER_APP];
static struct futex_waitv waitvs[MAX_SOCKETS_PER_APP];
/** futex nr to socket_index **/
socket_index_t socket_from_futex_nr[MAX_SOCKETS_PER_APP];

char *app_shm_name = NULL;
struct pifus_app *app_state = NULL;

int create_app_shm_region(void);
struct pifus_app *map_app_region(int fd);
void start_callback_thread(void);
void *callback_thread_loop(void *arg);
uint8_t fill_sockets_waitv(void);
void handle_new_cqueue_entry(struct pifus_socket *socket);

/**
 * @brief Calls shmem_open with next available app id
 * @return fd for the shared memory
 */
int create_app_shm_region(void) {
    app_index_t app_number = 0;

    int fd;
    while (true) {
        if (asprintf(&app_shm_name, "%s%u", SHM_APP_NAME_PREFIX, app_number) <
            0) {
            pifus_log("pifus: error when calling asprintf\n");
        }

        fd = shm_open_region(app_shm_name, true);

        if (fd >= 0) {
            return fd;
        } else {
            if (errno == EEXIST) {
                pifus_debug_log(
                    "pifus: shmem with name %s already exists, trying "
                    "next...\n",
                    app_shm_name);
                app_number++;
            } else {
                pifus_log("pifus: errno %i when calling shm_open\n", errno);
            }
        }
    }
}

/**
 * @brief Maps the app's shared memory into the process memory space.
 *
 * @param fd The fd of the app's shared memory.
 * @return Pointer to pifus_app struct.
 */
struct pifus_app *map_app_region(int fd) {
    return (struct pifus_app *)shm_map_region(fd, SHM_APP_SIZE, true);
}

void pifus_initialize(pifus_callback cb) {
    int app_shmem_fd = create_app_shm_region();
    app_state = map_app_region(app_shmem_fd);

    if (cb != NULL) {
        callback = cb;
        pifus_log("Using callback-mode, starting callback thread.\n");
        start_callback_thread();
    }

    pifus_log("pifus: Initialized!\n");
}

void pifus_exit(void) {
    pifus_socket_exit_all();
    shm_unlink_region(app_shm_name);
}

void handle_new_cqueue_entry(struct pifus_socket *socket) {
    if (socket == NULL) {
        // may be NULL already (after closing)
        return;
    }

    size_t shadow_actual_difference =
        socket->cqueue_futex - socket_futexes[socket->identifier.socket_index];
    socket_futexes[socket->identifier.socket_index] = socket->cqueue_futex;

    for (size_t i = 0; i < shadow_actual_difference; i++) {
        enum pifus_operation_code *next_op_code = NULL;

        if (pifus_socket_peek_result_code(socket, &next_op_code)) {
            callback(socket, *next_op_code);
        } else {
            pifus_log(
                "Not calling callback, as result has been dequeued already by "
                "user.\n");
        }
    }
}

uint8_t fill_sockets_waitv(void) {
    pthread_mutex_lock(&sockets_mutex);
    uint8_t current_amount_futexes = 0;
    for (socket_index_t socket_index = 1;
         socket_index <= app_state->highest_socket_number; socket_index++) {
        struct pifus_socket *current_socket_ptr = sockets[socket_index];

        if (current_socket_ptr != NULL) {
            if (current_socket_ptr->cqueue_futex !=
                socket_futexes[socket_index]) {
                handle_new_cqueue_entry(current_socket_ptr);
            }

            waitvs[current_amount_futexes].uaddr =
                (uintptr_t)&current_socket_ptr->cqueue_futex;
            waitvs[current_amount_futexes].flags = FUTEX_32;
            waitvs[current_amount_futexes].val = socket_futexes[socket_index];
            waitvs[current_amount_futexes].__reserved = 0;
            socket_from_futex_nr[current_amount_futexes] = socket_index;

            current_amount_futexes++;
        }
    }

    pthread_mutex_unlock(&sockets_mutex);
    return current_amount_futexes;
}

void *callback_thread_loop(void *arg) {
    PIFUS_UNUSED_ARG(arg);
    while (true) {
        uint8_t amount_futexes = fill_sockets_waitv();
        if (amount_futexes > 0) {
            struct timespec timespec;
            clock_gettime(CLOCK_MONOTONIC, &timespec);
            timespec.tv_sec += 1;
            int ret_code = futex_waitv(waitvs, amount_futexes, 0, &timespec,
                                       CLOCK_MONOTONIC);

            if (ret_code >= 0) {
                socket_index_t socket_index = socket_from_futex_nr[ret_code];
                struct pifus_socket *socket = sockets[socket_index];

                handle_new_cqueue_entry(socket);

                /* clean up previous mappings */
                memset(socket_from_futex_nr, 0, sizeof(socket_from_futex_nr));
            }
        }
    }
}

void start_callback_thread(void) {
    int ret =
        pthread_create(&callback_thread, NULL, callback_thread_loop, NULL);

    if (ret < 0) {
        pifus_log("pifus: Could not start callback thread due to %i!\n", errno);
        exit(1);
    } else {
        pifus_debug_log("pifus: Started callback thread!\n");
    }
}