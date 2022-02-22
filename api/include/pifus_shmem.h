#ifndef PIFUS_SHMEM_H
#define PIFUS_SHMEM_H

#include "stddef.h"
#include "stdint.h"

#include "pifus_byte_buffer.h"
#include "pifus_constants.h"
#include "pifus_identifiers.h"
#include "pifus_operation.h"
#include "pifus_ring_buffer.h"
#include "pifus_qos.h"
#include "utils/futex.h"

enum pifus_protocol { PROTOCOL_TCP = 0, PROTOCOL_UDP = 1 };

/**
 * @brief Shmem layout of socket region.
 */
struct pifus_socket {
  enum pifus_protocol protocol;
  struct pifus_socket_identifier identifier;
  enum pifus_priority priority;

  /* squeue */
  futex_t squeue_futex;
  struct pifus_operation_ring_buffer squeue;
  struct pifus_operation squeue_buffer[SQUEUE_SIZE];

  /* cqueue */
  futex_t cqueue_futex;
  struct pifus_operation_result_ring_buffer cqueue;
  struct pifus_operation_result cqueue_buffer[CQUEUE_SIZE];

  /* async operations lookup */
  struct pifus_write_queue write_queue;
  struct pifus_write_queue_entry write_queue_buffer[WRITE_QUEUE_SIZE];
  struct pifus_recv_queue recv_queue;
  struct pifus_recv_queue_entry recv_queue_buffer[RECV_QUEUE_SIZE];

  /* recv */
  uint16_t unread_data_offset;

  /* recv buffer */
  struct pifus_byte_buffer recv_buffer;
  uint8_t recv_buffer_array[RECV_BUFFER_SIZE];

  /* accept */
  enum pifus_priority accepted_sockets_priority;

  /* stats (for rate limiting) */
  uint64_t enqueued_ops;
  uint64_t dequeued_ops;

  /* ATTENTION: pointers below this note are only valid for stack side */
  /* internal lwIP mappings */
  union {
    /** void pointers because this header file is also used on client side which
     * does not have access to lwIP. */
    void *tcp;
    void *udp;
  } pcb;
};

/**
 * @brief Shmem layout of app region.
 */
struct pifus_app {
  /* Gives index of highest socket. Used as futex for wake-up when new
   * socket is created */
  socket_index_t highest_socket_number;
};

/**
 * @brief Describes a memory block inside the app's data region.
 */
struct pifus_memory_block {
  bool free;
  size_t size;
  /**
   * Block offset (absolute) of the previous block.
   * -1 if there is no previous block.
   */
  block_offset_t prev_block_offset;
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
 * @param fd The fd of the app's shared memory
 * @param size Size of the shared mem region.
 * @param create If this is the first try of mapping the region.
 * @return void ptr to the mapped region
 */
void *shm_map_region(int fd, size_t size, bool create);

void shm_unlink_region(char *shm_name);

/**
 * @brief Allocates memory in the app's data region block.
 *
 * @param app_region Pointer to the app region.
 * @param size The desired size of the block.
 * @param ptr_offset Output parameter: offset to the begin of the app's data
 * region block.
 * @param block Output parameter: ptr to block (application specific valid
 * address)
 * @return true If allocation was successful.
 * @return false If allocation was _not_ successful.
 */
bool shm_data_allocate(struct pifus_app *app_region, size_t size,
                       block_offset_t *ptr_offset,
                       struct pifus_memory_block **block);

/**
 * @brief Frees memory in the app's data region block.
 *
 * @param app_region The app region ptr.
 * @param block The block to free.
 */
void shm_data_free(struct pifus_app *app_region,
                   struct pifus_memory_block *block);

/**
 * @brief Convenience function for converting a block offset to a pointer.
 *
 * @param app_region The app region ptr.
 * @param block_offset The pointer offset of the block
 * @return Ptr to memory block.
 */
struct pifus_memory_block *shm_data_get_block_ptr(struct pifus_app *app_region,
                                                  block_offset_t block_offset);

/**
 * @brief Convenience function for getting the data ptr of a
 * pifus_memory_block.
 *
 * @param memory_block The memory_block ptr.
 * @return void ptr to data.
 */
void *shm_data_get_data_ptr(struct pifus_memory_block *memory_block);

#endif /* PIFUS_SHMEM_H */
