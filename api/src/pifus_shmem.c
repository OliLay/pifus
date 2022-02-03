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

struct pifus_memory_block *shm_data_next_suitable_block(
    struct pifus_app *app, struct pifus_memory_block *previous_block,
    struct pifus_memory_block *current_block, size_t size,
    struct pifus_memory_block **previous_block_out);
void shm_data_delete_block(struct pifus_memory_block *block);
struct pifus_memory_block *
shm_data_get_next_block_ptr(struct pifus_app *app,
                            struct pifus_memory_block *block);
bool shm_data_is_ptr_out_of_range(struct pifus_app *app, void *ptr);

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

  void *shmem_ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

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

struct pifus_memory_block *shm_data_next_suitable_block(
    struct pifus_app *app, struct pifus_memory_block *previous_block,
    struct pifus_memory_block *current_block, size_t size,
    struct pifus_memory_block **previous_block_out) {

  bool block_exists = current_block->size > 0;
  bool block_suitable = (current_block->free && current_block->size >= size);

  if (!block_exists || block_suitable) {
    *previous_block_out = previous_block;
    return current_block;
  } else {
    struct pifus_memory_block *next_block =
        (struct pifus_memory_block *)((uint8_t *)current_block +
                                      sizeof(struct pifus_memory_block) +
                                      current_block->size);

    void *next_block_end_ptr =
        (uint8_t *)next_block + sizeof(struct pifus_memory_block) + size;
    if (shm_data_is_ptr_out_of_range(app, next_block_end_ptr)) {
      // end of memory region, no suitable block found.

      pifus_log("pifus_shm: End of memory region reached, no suitable block "
                "found for size %u!\n",
                size);
      return NULL;
    }

    return shm_data_next_suitable_block(app, current_block, next_block, size,
                                        previous_block_out);
  }
}

bool shm_data_allocate(struct pifus_app *app_region, size_t size,
                       ptrdiff_t *ptr_offset,
                       struct pifus_memory_block **block) {
  struct pifus_memory_block *start_block =
      (struct pifus_memory_block *)(app_region + 1);

  struct pifus_memory_block *previous_block = NULL;
  struct pifus_memory_block *allocating_block = shm_data_next_suitable_block(
      app_region, NULL, start_block, size, &previous_block);

  if (allocating_block == NULL) {
    return false;
  }

  allocating_block->free = false;
  allocating_block->size = size;

  if (previous_block != NULL) {
    allocating_block->prev_block_offset =
        (uint8_t *)previous_block - (uint8_t *)start_block;
  } else {
    allocating_block->prev_block_offset = 0;
  }
  *ptr_offset = (uint8_t *)allocating_block - (uint8_t *)start_block;
  *block = allocating_block;

  pifus_debug_log("pifus_shm: Allocated block with size %u and offset %u "
                  "inside app data!\n",
                  size, *ptr_offset);

  return true;
}

void shm_data_delete_block(struct pifus_memory_block *block) {
  memset(block, 0x00, sizeof(struct pifus_memory_block) + block->size);
}

bool shm_data_is_ptr_out_of_range(struct pifus_app *app, void *ptr) {
  return (uint8_t *)ptr > (uint8_t *)app + SHM_APP_SIZE;
}

struct pifus_memory_block *
shm_data_get_next_block_ptr(struct pifus_app *app,
                            struct pifus_memory_block *block) {
  struct pifus_memory_block *next_block =
      (struct pifus_memory_block *)((uint8_t *)block +
                                    sizeof(struct pifus_memory_block) +
                                    block->size);

  if (shm_data_is_ptr_out_of_range(app, next_block)) {
    return NULL;
  } else {
    return next_block;
  }
}

void shm_data_free(struct pifus_app *app_region,
                   struct pifus_memory_block *block) {
  struct pifus_memory_block *next_block =
      shm_data_get_next_block_ptr(app_region, block);

  bool exists_previous_block = block->prev_block_offset != 0;
  bool exists_next_block = next_block != NULL && next_block->size != 0;

  if (!exists_previous_block && !exists_next_block) {
    // delete if no previous and next block
    shm_data_delete_block(block);

    pifus_debug_log("pifus_shm: Freeing, no previous block and no next block, "
                    "deleting block.\n");
    return;
  }

  struct pifus_memory_block *new_merged_block = block;
  if (exists_previous_block) {
    // previous block exists
    struct pifus_memory_block *previous_block =
        shm_data_get_block_ptr(app_region, block->prev_block_offset);

    if (previous_block->free) {
      // merge with previous block
      new_merged_block = previous_block;
      new_merged_block->size += sizeof(struct pifus_memory_block) + block->size;
      shm_data_delete_block(previous_block);

      pifus_debug_log("pifus_shm: Freeing, merging with previous block.\n");
    }
  }

  if (exists_next_block && next_block->free) {
    // merge with next block
    new_merged_block->size +=
        sizeof(struct pifus_memory_block) + next_block->size;
    shm_data_delete_block(next_block);

    pifus_debug_log("pifus_shm: Freeing, merging with next block.\n");
  }

  memset(new_merged_block + 1, 0x00, new_merged_block->size);
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