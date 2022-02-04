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
    struct pifus_memory_block *current_block, size_t desired_size);
void shm_data_delete_block(struct pifus_memory_block *block);
struct pifus_memory_block *
shm_data_get_next_block_ptr(struct pifus_app *app,
                            struct pifus_memory_block *block);
int64_t shm_data_get_offset(struct pifus_app *app,
                            struct pifus_memory_block *block);
bool shm_data_is_ptr_out_of_range(struct pifus_app *app, void *ptr);
void print_memory(struct pifus_app *app);

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

// TODO: remove after debugging.
void print_memory(struct pifus_app *app) {
  struct pifus_memory_block *next_block = shm_data_get_block_ptr(app, 0);

  while (next_block != NULL && next_block->size > 0) {
    printf("|| free %i offset %li size %lu ", next_block->free,
           shm_data_get_offset(app, next_block), next_block->size);
    next_block = shm_data_get_next_block_ptr(app, next_block);
  }
  printf("\n");
}

struct pifus_memory_block *shm_data_next_suitable_block(
    struct pifus_app *app, struct pifus_memory_block *previous_block,
    struct pifus_memory_block *current_block, size_t desired_size) {

  bool block_exists = current_block->size > 0;

  if (!block_exists) {
    // append to end
    if (shm_data_is_ptr_out_of_range(
            app,
            (uint8_t *)shm_data_get_data_ptr(current_block) + desired_size)) {
      return NULL;
    }

    current_block->size = desired_size;

    if (previous_block == NULL) {
      current_block->prev_block_offset = -1;
    } else {
      current_block->prev_block_offset =
          shm_data_get_offset(app, previous_block);
    }

    return current_block;
  }

  struct pifus_memory_block *next_block =
      shm_data_get_next_block_ptr(app, current_block);

  bool block_exactly_that_size = block_exists &&
                                 current_block->size == desired_size &&
                                 current_block->free;
  if (block_exactly_that_size) {
    // reuse, we do not have to change the block, it was already there, just
    // change its content
    return current_block;
  }

  bool block_large_enough =
      block_exists && current_block->size > desired_size &&
      current_block->size - desired_size > sizeof(struct pifus_memory_block) &&
      current_block->free;

  if (block_large_enough) {
    // split block into two blocks.
    // First one has desired size, second one the difference
    size_t pre_split_size = current_block->size;

    current_block->size = desired_size;
    struct pifus_memory_block *new_next_block =
        shm_data_get_next_block_ptr(app, current_block);

    new_next_block->free = true;
    new_next_block->size =
        pre_split_size - desired_size - sizeof(struct pifus_memory_block);

    new_next_block->prev_block_offset = shm_data_get_offset(app, current_block);

    if (next_block != NULL && next_block->size > 0) {
      next_block->prev_block_offset = shm_data_get_offset(app, new_next_block);
    }

    return current_block;
  }

  if (next_block != NULL) {
    return shm_data_next_suitable_block(app, current_block, next_block,
                                        desired_size);
  } else {
    return NULL;
  }
}

bool shm_data_allocate(struct pifus_app *app, size_t size, int64_t *ptr_offset,
                       struct pifus_memory_block **block) {
  struct pifus_memory_block *start_block =
      (struct pifus_memory_block *)(app + 1);

  struct pifus_memory_block *allocating_block =
      shm_data_next_suitable_block(app, NULL, start_block, size);

  if (allocating_block == NULL) {
    return false;
  }

  allocating_block->free = false;

  *ptr_offset = shm_data_get_offset(app, allocating_block);
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
  bool out_of_range = (uint8_t *)ptr > (uint8_t *)app + SHM_APP_SIZE;

  if (out_of_range) {
    pifus_log("pifus_shm: End of memory region reached! Begin %p, Region size "
              "%u, Ptr %p\n",
              app, SHM_APP_SIZE, ptr);
  }

  return out_of_range;
}

int64_t shm_data_get_offset(struct pifus_app *app,
                            struct pifus_memory_block *block) {
  // TODO: why -8?
  return (uint8_t *)block - (uint8_t *)app + sizeof(struct pifus_app) - 8;
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

void shm_data_free(struct pifus_app *app, struct pifus_memory_block *block) {
  struct pifus_memory_block *next_block =
      shm_data_get_next_block_ptr(app, block);

  bool exists_previous_block = block->prev_block_offset >= 0;
  bool exists_next_block = next_block != NULL && next_block->size > 0;

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
        shm_data_get_block_ptr(app, block->prev_block_offset);

    if (previous_block->free) {
      if (exists_next_block) {
        next_block->prev_block_offset = block->prev_block_offset;
      }
      // merge with previous block
      new_merged_block = previous_block;
      new_merged_block->size += sizeof(struct pifus_memory_block) + block->size;

      pifus_debug_log("pifus_shm: Freeing, merging with previous block "
                      "(offset: %u). New (merged) block now has size %u.\n",
                      block->prev_block_offset, new_merged_block->size);
    }
  }

  if (exists_next_block && next_block->free) {
    // merge with next block
    new_merged_block->size +=
        sizeof(struct pifus_memory_block) + next_block->size;

    pifus_debug_log("pifus_shm: Freeing, merging with next block.\n");
  }

  memset(new_merged_block + 1, 0x00, new_merged_block->size);
  new_merged_block->free = true;
}

struct pifus_memory_block *shm_data_get_block_ptr(struct pifus_app *app,
                                                  int64_t block_offset) {
  return (struct pifus_memory_block *)((uint8_t *)app +
                                       sizeof(struct pifus_app) + block_offset);
}

void *shm_data_get_data_ptr(struct pifus_memory_block *memory_block) {
  return memory_block + 1;
}