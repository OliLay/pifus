#ifndef PIFUS_OPERATION_H
#define PIFUS_OPERATION_H

#include "stdbool.h"
#include "stdint.h"

#include "pifus_identifiers.h"

#define OPERATIONS                                                             \
  C(TCP_BIND, 0)                                                               \
  C(TCP_LISTEN, 1)                                                             \
  C(TCP_ACCEPT, 2)                                                             \
  C(TCP_CONNECT, 3)                                                            \
  C(TCP_WRITE, 4)                                                              \
  C(TCP_RECV, 5)                                                               \
  C(TCP_CLOSE, 6)                                                              \
  C(TCP_ABORT, 7)                                                              \
  C(UDP_BIND, 100)                                                             \
  C(UDP_CONNECT, 101)                                                          \
  C(UDP_DISCONNECT, 102)                                                       \
  C(UDP_SEND, 103)                                                             \
  C(UDP_RECV, 104)
#define C(k, v) k = v,
enum pifus_operation_code { OPERATIONS };
#undef C

/**
 * Similar struct as lwIP lwip_ip_addr_type defined in ip_addr.h
 */
enum ip_type {
  /** IPv4 */
  PIFUS_IPV4_ADDR = 0U,
  /** IPv6 */
  PIFUS_IPV6_ADDR = 6U,
  /** IPv4+IPv6 ("dual-stack") */
  PIFUS_IPVX_ADDR = 46U
};

/**
 * Data needed for the bind operation.
 */
struct pifus_bind_data {
  enum ip_type ip_type;
  uint16_t port;
};

struct pifus_operation {
  enum pifus_operation_code code;

  union {
    struct pifus_bind_data bind;
  } data;
};

/**
 * Internal representation of an operation.
 * Contains the operation and information about the socket.
 */
struct pifus_internal_operation {
  struct pifus_operation operation;
  struct pifus_socket_identifier socket_identifier;

  union {
    /** void pointers because this header file is also used on client side which
     * does not have access to lwIP */
    void *tcp;
    void *udp;
  } pcb;
};

enum pifus_result_code { PIFUS_OK = 0, PIFUS_ERR = 1 };

struct pifus_operation_result {
  enum pifus_operation_code code;
  enum pifus_result_code result_code;

  // TODO: insert into union for additional data being returned!
  // union {
  //
  // } data;
};

const char *operation_str(enum pifus_operation_code operation_code);

bool is_tcp_operation(struct pifus_operation *operation);

#endif