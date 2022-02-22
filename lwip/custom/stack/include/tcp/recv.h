#ifndef LWIP_RECV_H
#define LWIP_RECV_H

/* pifus */
#include "pifus_shmem.h"

/* lwIP */
#include "lwip/tcp.h"

err_t tcp_recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p,
                        err_t err);

struct pifus_operation_result
tx_tcp_recv(struct pifus_internal_operation *internal_op);

#endif