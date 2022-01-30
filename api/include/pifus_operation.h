#ifndef PIFUS_OPERATION_H
#define PIFUS_OPERATION_H

#include "stdbool.h"
#include "stddef.h"
#include "stdint.h"

#include "pifus_identifiers.h"
#include "pifus_ip.h"

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
  C(UDP_RECV, 104)                                                             \
  C(CONNECTION_LOST, 200)
#define C(k, v) k = v,
enum pifus_operation_code { OPERATIONS };
#undef C

/**
 * Data needed for the bind operation.
 */
struct pifus_bind_data {
  enum pifus_ip_type ip_type;
  uint16_t port;
};

/**
 * Data needed for the connect operation.
 */
struct pifus_connect_data {
  struct pifus_ip_addr ip_addr;
  uint16_t port;
};

/**
 * Data needed for the write operation.
 */
struct pifus_write_data {
  ptrdiff_t block_offset;
  size_t size;
};

struct pifus_operation {
  enum pifus_operation_code code;

  union {
    struct pifus_bind_data bind;
    struct pifus_connect_data connect;
    struct pifus_write_data write;
  } data;
};

/**
 * Internal representation of an operation.
 * Contains the operation and a pointer to the socket.
 */
struct pifus_internal_operation {
  struct pifus_operation operation;
  struct pifus_socket *socket;
};

enum pifus_result_code { PIFUS_OK = 0, PIFUS_ERR = 1, PIFUS_ASYNC = 2 };

struct pifus_operation_result {
  enum pifus_operation_code code;
  enum pifus_result_code result_code;

  union {
    struct pifus_write_data write;
  } data;
};

const char *operation_str(enum pifus_operation_code operation_code);

bool is_tcp_operation(struct pifus_operation *operation);

#endif