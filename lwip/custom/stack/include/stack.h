#ifndef LWIP_STACK_H
#define LWIP_STACK_H

#include "pifus_priority_aware_ring_buffer.h"
#include "pifus_ring_buffer.h"
#include "utils/futex.h"

extern struct pifus_app *app_ptrs[MAX_APP_AMOUNT];
extern struct pifus_socket *socket_ptrs[MAX_APP_AMOUNT][MAX_SOCKETS_PER_APP];
extern struct tcp_pcb *socket_tcp_pcbs[MAX_APP_AMOUNT][MAX_SOCKETS_PER_APP];

extern futex_t socket_futexes[MAX_APP_AMOUNT][MAX_SOCKETS_PER_APP];

extern app_index_t next_app_number;
extern struct pifus_priority_aware_ring_buffer tx_queue;

/**
 * @brief Called every time the lwIP main loop processes, i.e. after checking
 * timeouts and polling the driver.
 */
void lwip_loop_iteration(void);

/**
 * Should be called when there are new operations enqueued in the stack. (TX side)
 */
void signal_tx_interrupt(void);

/**
 * @brief Called when lwIP stack init is complete.
 */
void lwip_init_complete(void);

void enqueue_in_cqueue(struct pifus_socket *socket,
                       struct pifus_operation_result *operation_result);

#endif /* LWIP_STACK_H */
