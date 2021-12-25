#ifndef LWIP_WRITER_H
#define LWIP_WRITER_H

#include "lwip/ip_addr.h"

ip_addr_t *reader_addr = NULL;

void writer_handle_error(const char* error_str);

err_t writer_connected_callback(void *arg, struct tcp_pcb *tpcb, err_t err);

void writer_init(void);

void writer_send_now(void *arg);

#endif /* LWIP_WRITER_H */