#ifndef PIFUS_OPERATION_H
#define PIFUS_OPERATION_H

#include "stdint.h"
#include "stdbool.h"

#define OPERATIONS  C(TCP_BIND, 0)\
                    C(TCP_LISTEN, 1)\
                    C(TCP_ACCEPT, 2)\
                    C(TCP_CONNECT, 3)\
                    C(TCP_WRITE, 4)\
                    C(TCP_RECV, 5)\
                    C(TCP_CLOSE, 6)\
                    C(TCP_ABORT, 7)\
                    C(UDP_BIND, 100)\
                    C(UDP_CONNECT, 101)\
                    C(UDP_DISCONNECT, 102)\
                    C(UDP_SEND, 103)\
                    C(UDP_RECV, 104)
#define C(k, v) k = v,
enum operation_code {
    OPERATIONS
};
#undef C

/**
 * Same struct as lwIP lwip_ip_addr_type defined in ip_addr.h
 */
enum ip_type {
  /** IPv4 */
  PIFUS_IPADDR_TYPE_V4 =   0U,
  /** IPv6 */
  PIFUS_IPADDR_TYPE_V6 =   6U,
  /** IPv4+IPv6 ("dual-stack") */
  PIFUS_IPADDR_TYPE_ANY = 46U
};

/**
 * Data needed for the bind operation.
 */
struct pifus_bind_data {
    enum ip_type ip_type;
    uint16_t port;
};

struct pifus_operation {
    enum operation_code op;

    union {
        struct pifus_bind_data bind;
    } data;
};

const char* operation_str(enum operation_code operation_code);

bool is_tcp_operation(struct pifus_operation* operation);

#endif