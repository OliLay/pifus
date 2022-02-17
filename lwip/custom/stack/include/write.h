#ifndef LWIP_WRITE_H
#define LWIP_WRITE_H

/* pifus */
#include "pifus_shmem.h"

/* lwIP */
#include "lwip/tcp.h"

err_t tcp_sent_callback(void *arg, struct tcp_pcb *tpcb, u16_t len);

struct pifus_operation_result
tx_tcp_write(struct pifus_internal_operation *internal_op);

#endif