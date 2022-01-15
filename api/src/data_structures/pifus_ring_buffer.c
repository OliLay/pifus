/* std lib */
#include "stdlib.h"

/* local includes */
#include "data_structures/pifus_ring_buffer.h"

void pifus_ring_buffer_create(struct pifus_ring_buffer* ring_buffer,
                              struct pifus_operation* array,
                              uint8_t buffer_length) {
    RingBuf_ctor(&ring_buffer->internal_ring_buffer, array, buffer_length);
}

bool pifus_ring_buffer_put(struct pifus_ring_buffer* ring_buffer,
                           struct pifus_operation const element) {
    return RingBuf_put(&ring_buffer->internal_ring_buffer, element);
}

bool pifus_ring_buffer_pop(struct pifus_ring_buffer* ring_buffer,
                           struct pifus_operation* element) {
    return RingBuf_get(&ring_buffer->internal_ring_buffer, element);
}