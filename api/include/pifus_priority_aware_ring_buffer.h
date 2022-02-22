#ifndef PIFUS_PRIORITY_AWARE_RING_BUFFER
#define PIFUS_PRIORITY_AWARE_RING_BUFFER

#include "pifus_constants.h"
#include "pifus_ring_buffer.h"
#include "stdbool.h"
#include "utils/futex.h"

struct pifus_priority_aware_ring_buffer {
  struct pifus_internal_operation_ring_buffer low_prio_queue;
  struct pifus_internal_operation low_prio_queue_buffer[TX_QUEUE_SIZE];
  futex_t low_prio_queue_futex;

  struct pifus_internal_operation_ring_buffer medium_prio_queue;
  struct pifus_internal_operation medium_prio_queue_buffer[TX_QUEUE_SIZE];
  futex_t medium_prio_queue_futex;

  struct pifus_internal_operation_ring_buffer high_prio_queue;
  struct pifus_internal_operation high_prio_queue_buffer[TX_QUEUE_SIZE];
  futex_t high_prio_queue_futex;

  struct pifus_internal_operation_ring_buffer *last_peek;
};

void pifus_priority_aware_ring_buffer_create(
    struct pifus_priority_aware_ring_buffer *ring_buffer);

bool pifus_priority_aware_ring_buffer_peek(
    struct pifus_priority_aware_ring_buffer *ring_buffer,
    struct pifus_internal_operation **element);

bool pifus_priority_aware_ring_buffer_erase_first(
    struct pifus_priority_aware_ring_buffer *ring_buffer);

bool pifus_priority_aware_ring_buffer_find(
    struct pifus_priority_aware_ring_buffer *ring_buffer,
    struct pifus_internal_operation **element,
    bool (*satisfies)(struct pifus_internal_operation *));

#endif
