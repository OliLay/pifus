#ifndef LWIP_READER_H
#define LWIP_READER_H

#include "lwip/ip_addr.h"

void reader_handle_error(const char* error_str);

err_t reader_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);

err_t reader_accept(void *arg, struct tcp_pcb *newpcb, err_t err);

void reader_init(void);

#endif /* LWIP_READER_H */
