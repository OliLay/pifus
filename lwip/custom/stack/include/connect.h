#ifndef LWIP_CONNECT_H
#define LWIP_CONNECT_H

/* pifus */
#include "pifus_shmem.h"

/* lwIP */
#include "lwip/tcp.h"

err_t tcp_connected_callback(void *arg, struct tcp_pcb *tpcb, err_t err);

struct pifus_operation_result
tx_tcp_connect(struct pifus_internal_operation *internal_op);

#endif