#ifndef LWIP_PRIO_THREAD_H
#define LWIP_PRIO_THREAD_H

#include "pifus_ring_buffer.h"
#include "discovery.h"
#include "utils/futex.h"

void start_prio_thread(struct pifus_internal_operation_ring_buffer *tx_queue,
                       struct pifus_internal_operation *tx_queue_buffer,
                       struct pifus_new_socket_queue *new_socket_queue);

#endif