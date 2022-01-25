#ifndef LWIP_STACK_H
#define LWIP_STACK_H

#include "data_structures/pifus_tx_ring_buffer.h"

struct pifus_tx_queue {
    struct pifus_tx_ring_buffer ring_buffer;
    struct pifus_internal_operation tx_queue_buffer[TX_QUEUE_SIZE];
};

extern struct pifus_app *app_ptrs[MAX_APP_AMOUNT];
extern struct pifus_socket *socket_ptrs[MAX_APP_AMOUNT][MAX_SOCKETS_PER_APP];
extern struct tcp_pcb *socket_tcp_pcbs[MAX_APP_AMOUNT][MAX_SOCKETS_PER_APP];

extern app_index_t next_app_number;
extern struct pifus_tx_queue tx_queue;

/**
 * @brief Called every time the lwIP main loop processes, i.e. after checking
 * timeouts and polling the driver.
 */
void lwip_loop_iteration(void);

/**
 * @brief Called when lwIP stack init is complete.
 */
void lwip_init_complete(void);

#endif /* LWIP_STACK_H */
