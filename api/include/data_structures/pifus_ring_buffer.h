#ifndef PIFUS_RING_BUFFER_H
#define PIFUS_RING_BUFFER_H

/* std */
#include <stdbool.h>
/* local */
#include "data_structures/ext/ring_buf.h"

// TODO: think about deleting and direct usage of RingBuf.

struct pifus_ring_buffer {
    RingBuf* internal_ring_buffer;
};

void pifus_ring_buffer_create(struct pifus_ring_buffer* ring_buffer,
                              struct pifus_operation* array,
                              uint8_t buffer_length);

bool pifus_ring_buffer_put(struct pifus_ring_buffer* ring_buffer,
                           struct pifus_operation const element);

struct pifus_operation* pifus_ring_buffer_get(
    struct pifus_ring_buffer* ring_buffer);

#endif /* PIFUS_RING_BUFFER_H */