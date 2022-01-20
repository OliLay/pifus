#ifndef PIFUS_OPERATION_H
#define PIFUS_OPERATION_H

#include "stdint.h"

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

struct pifus_operation {
    enum operation_code op;
    // TODO: arguments? e.g. connect
};

const char* operation_str(enum operation_code operation_code);

#endif