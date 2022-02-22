#ifndef LWIP_ACCEPT_H
#define LWIP_ACCEPT_H

/* pifus */
#include "pifus_shmem.h"

/* lwIP */
#include "lwip/tcp.h"

err_t tcp_accepted_callback(void *arg, struct tcp_pcb *newpcb, err_t err);

struct pifus_operation_result
tx_tcp_accept(struct pifus_internal_operation *internal_op);

#endif