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

struct pifus_memory_block *
shm_data_next_suitable_block(struct pifus_memory_block *start_block,
                             struct pifus_memory_block *current_block,
                             size_t size);

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

struct pifus_memory_block *
shm_data_next_suitable_block(struct pifus_memory_block *start_block,
                             struct pifus_memory_block *current_block,
                             size_t size) {

  bool block_exists = current_block->size > 0;
  bool block_suitable = (current_block->free && current_block->size >= size);

  if (!block_exists || block_suitable) {
    return current_block;
  } else {
    struct pifus_memory_block *next_block =
        (struct pifus_memory_block *)((uint8_t *)current_block +
                                      sizeof(struct pifus_memory_block) +
                                      current_block->size);

    if ((uint8_t *)next_block + sizeof(struct pifus_memory_block) + size >
        (uint8_t *)start_block + SHM_APP_SIZE) {
      pifus_log("pifus_shm: End of memory region reached, no suitable block "
                "found for size %u\n!",
                size);
      // end of memory region, no suitable block found.
      return NULL;
    }

    return shm_data_next_suitable_block(start_block, next_block, size);
  }
}

bool shm_data_allocate(struct pifus_app *app_region, size_t size,
                       ptrdiff_t *ptr_offset,
                       struct pifus_memory_block **block) {
  struct pifus_memory_block *start_block =
      (struct pifus_memory_block *)(app_region + 1);
  struct pifus_memory_block *allocating_block =
      shm_data_next_suitable_block(start_block, start_block, size);

  if (allocating_block == NULL) {
    return false;
  }

  allocating_block->free = false;
  allocating_block->size = size;
  *ptr_offset = (uint8_t *)allocating_block - (uint8_t *)start_block;
  *block = allocating_block;

  pifus_debug_log("pifus_shm: Allocated block with size %u and offset %u "
                  "inside app data!\n",
                  size, *ptr_offset);

  return true;
}

void shm_data_free(struct pifus_memory_block *block) {

  memset(block + 1, 0x00, block->size);
  block->free = true;
}

struct pifus_memory_block *shm_data_get_block_ptr(struct pifus_app *app_region,
                                                  ptrdiff_t block_offset) {
  return (struct pifus_memory_block *)((uint8_t *)app_region +
                                       sizeof(struct pifus_app) + block_offset);
}

void *shm_data_get_data_ptr(struct pifus_memory_block *memory_block) {
  return memory_block + 1;
}