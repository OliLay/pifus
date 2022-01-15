/* std */
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

/* shmem includes */
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

/* local includes */
#include "pifus_constants.h"
#include "pifus_shmem.h"

int shm_create_region(char* shm_name) {
    int fd = shm_open(shm_name, O_RDWR | O_CREAT | O_EXCL, SHM_MODE);

    if (fd >= 0) {
        printf("pifus: created shmem with name %s\n", shm_name);
    }

    return fd;
}

void* shm_map_region(int fd, size_t size) {
    int err = ftruncate(fd, size);
    if (err < 0) {
        printf("pifus: ftruncate failed with code %i\n", errno);
        exit(1);
    }

    void* shmem_ptr = (int*)mmap(NULL, size, PROT_READ | PROT_WRITE,
                                MAP_SHARED, fd, 0);

    if (shmem_ptr == MAP_FAILED) {
        printf("pifus: mmap failed with code %i\n", errno);
        exit(1);
    }

    memset(shmem_ptr, 0x00, size);

    return shmem_ptr;
}

void shm_unlink_region(char* shm_name) {
    shm_unlink(shm_name);
}