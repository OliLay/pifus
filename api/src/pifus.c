#define _GNU_SOURCE

/* standard includes */
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* shmem includes */
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

/* local includes */
#include "pifus.h"
#include "pifus_constants.h"
#include "pifus_socket.h"
#include "pifus_shmem.h"

char* app_shm_name = NULL;
struct pifus_app* app_state = NULL;

int create_app_shm_region(void);
struct pifus_app* map_app_region(int fd);

/**
 * @brief Calls shmem_open with next available app id
 * @return fd for the shared memory
 */
int create_app_shm_region(void) {
    app_index_t app_number = 0;

    int fd;
    while (true) {
        if (asprintf(&app_shm_name, "%s%u", SHM_APP_NAME_PREFIX, app_number) < 0) {
            printf("pifus: error when calling asprintf\n");
        }

        fd = shm_open_region(app_shm_name, true);

        if (fd >= 0) {
            return fd;
        } else {
            if (errno == EEXIST) {
                printf(
                    "pifus: shmem with name %s already exists, trying "
                    "next...\n",
                    app_shm_name);
                app_number++;
            } else {
                printf("pifus: errno %i when calling shm_open\n", errno);
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
struct pifus_app* map_app_region(int fd) {
    return (struct pifus_app*)shm_map_region(fd, SHM_APP_SIZE, true);
}

void pifus_initialize(void) {
    int app_shmem_fd = create_app_shm_region();
    app_state = map_app_region(app_shmem_fd);

    printf("pifus: Initialized!\n");
}

void pifus_exit(void) { 
    pifus_socket_exit_all();
    shm_unlink_region(app_shm_name);
}

// TODO: pifus_shutdown