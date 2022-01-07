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
#include "pifus_socket.h"
#include "pifus_constants.h"

struct pifus_state state;

/**
 * @brief Calls shmem_open with next available app id
 * @return fd for the shared memory
 */
int create_app_shm_region(void) {
    char* shm_name;
    uint8_t app_number = 0;

    int fd;
    while (true) {
        asprintf(&shm_name, "%s%u", SHM_APP_NAME_PREFIX, app_number);

        fd = shm_open(shm_name, O_RDWR | O_CREAT | O_EXCL, SHM_MODE);

        if (fd >= 0) {
            state.app_number = app_number;

            printf("pifus: created shmem with name %s\n", shm_name);
            return fd;
        } else {
            if (errno == EEXIST) {
                printf(
                    "pifus: shmem with name %s already exists, trying "
                    "next...\n",
                    shm_name);
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
 */
void map_app_shm_region(int fd) {
    int err = ftruncate(fd, SHM_APP_SIZE);
    if (err < 0) {
        printf("pifus: ftruncate failed with code %i\n", errno);
        exit(1);
    }

    state.app_shm_ptr = (int*)mmap(NULL, SHM_APP_SIZE, PROT_READ | PROT_WRITE,
                                   MAP_SHARED, fd, 0);

    if (state.app_shm_ptr == MAP_FAILED) {
        printf("pifus: mmap failed with code %i\n", *(state.app_shm_ptr));
        exit(1);
    }
}

void pifus_initialize(void) {
    int app_shmem_fd = create_app_shm_region();
    map_app_shm_region(app_shmem_fd);
}