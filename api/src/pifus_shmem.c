/* std */
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* shmem includes */
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

/* local includes */
#include "pifus_constants.h"
#include "pifus_shmem.h"
#include "utils/log.h"

int shm_open_region(char *shm_name, bool create) {
  int flags = O_RDWR | O_EXCL;

  if (create) {
    flags |= O_CREAT;
  }

  int fd = shm_open(shm_name, flags, SHM_MODE);

  if (fd >= 0) {
    pifus_debug_log("pifus_shm: opened shmem with name %s\n", shm_name);
  }

  return fd;
}

void *shm_map_region(int fd, size_t size, bool create) {
  int err = ftruncate(fd, size);
  if (err < 0) {
    pifus_log("pifus_shm: ftruncate failed with code %i\n", errno);
    exit(1);
  }

  void *shmem_ptr =
      (int *)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  if (shmem_ptr == MAP_FAILED) {
    pifus_log("pifus_shm: mmap failed with code %i\n", errno);
    exit(1);
  }

  if (create) {
    memset(shmem_ptr, 0x00, size);
  }

  return shmem_ptr;
}

void shm_unlink_region(char *shm_name) { shm_unlink(shm_name); }