#ifndef LWIP_LISTEN_H
#define LWIP_LISTEN_H

/* pifus */
#include "pifus_shmem.h"

/* lwIP */
#include "lwip/tcp.h"

struct pifus_operation_result
tx_tcp_listen(struct pifus_internal_operation *internal_op);

#endif