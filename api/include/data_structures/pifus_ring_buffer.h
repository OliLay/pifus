#ifndef PIFUS_RING_BUFFER_H
#define PIFUS_RING_BUFFER_H

/* std */
#include <stdbool.h>
/* local */
#include "data_structures/ext/ring_buf.h"

struct pifus_ring_buffer
{
    RingBuf internal_ring_buffer;
};

void pifus_ring_buffer_create(struct pifus_ring_buffer *ring_buffer,
                              uint8_t buffer_length);

bool pifus_ring_buffer_put(struct pifus_ring_buffer *ring_buffer,
                           struct pifus_operation *array,
                           struct pifus_operation const element);

bool pifus_ring_buffer_pop(struct pifus_ring_buffer *ring_buffer,
                           struct pifus_operation *array,
                           struct pifus_operation *element);

#endif /* PIFUS_RING_BUFFER_H */