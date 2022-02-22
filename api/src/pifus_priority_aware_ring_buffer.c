#include "pifus_priority_aware_ring_buffer.h"
#include "pifus_shmem.h"

#include "stdio.h"

void pifus_priority_aware_ring_buffer_create(
    struct pifus_priority_aware_ring_buffer *ring_buffer) {
  pifus_internal_operation_ring_buffer_create(&ring_buffer->high_prio_queue,
                                              TX_QUEUE_SIZE);
  pifus_internal_operation_ring_buffer_create(&ring_buffer->medium_prio_queue,
                                              TX_QUEUE_SIZE);
  pifus_internal_operation_ring_buffer_create(&ring_buffer->low_prio_queue,
                                              TX_QUEUE_SIZE);
}

bool pifus_priority_aware_ring_buffer_peek(
    struct pifus_priority_aware_ring_buffer *ring_buffer,
    struct pifus_internal_operation **element) {

  if (pifus_internal_operation_ring_buffer_peek(
          &ring_buffer->high_prio_queue, ring_buffer->high_prio_queue_buffer,
          element)) {
    ring_buffer->last_peek = &ring_buffer->high_prio_queue;
    return true;
  }

  if (pifus_internal_operation_ring_buffer_peek(
          &ring_buffer->medium_prio_queue,
          ring_buffer->medium_prio_queue_buffer, element)) {
    ring_buffer->last_peek = &ring_buffer->medium_prio_queue;
    return true;
  }

  if (pifus_internal_operation_ring_buffer_peek(
          &ring_buffer->low_prio_queue, ring_buffer->low_prio_queue_buffer,
          element)) {
    ring_buffer->last_peek = &ring_buffer->low_prio_queue;
    return true;
  }

  return false;
}

bool pifus_priority_aware_ring_buffer_erase_first(
    struct pifus_priority_aware_ring_buffer *ring_buffer) {
  return pifus_internal_operation_ring_buffer_erase_first(
      ring_buffer->last_peek);
}

bool pifus_priority_aware_ring_buffer_find(
    struct pifus_priority_aware_ring_buffer *ring_buffer,
    struct pifus_internal_operation **element,
    bool (*satisfies)(struct pifus_internal_operation *)) {

  if (pifus_internal_operation_ring_buffer_find(
          &ring_buffer->high_prio_queue, ring_buffer->high_prio_queue_buffer,
          element, satisfies)) {
    return true;
  }

  if (pifus_internal_operation_ring_buffer_find(
          &ring_buffer->medium_prio_queue,
          ring_buffer->medium_prio_queue_buffer, element, satisfies)) {
    return true;
  }

  if (pifus_internal_operation_ring_buffer_find(
          &ring_buffer->low_prio_queue, ring_buffer->low_prio_queue_buffer,
          element, satisfies)) {
    return true;
  }

  return false;
}