#ifndef PIFUS_OPERATION_H
#define PIFUS_OPERATION_H

#include "stdint.h"

enum operation_code {
    TCP_BIND = 0,
    TCP_LISTEN = 1,
    TCP_ACCEPT = 2,
    TCP_CONNECT = 3,
    TCP_WRITE = 4,
    TCP_RECV = 5,
    TCP_CLOSE = 6,
    TCP_ABORT = 7,
    UDP_BIND = 100,
    UDP_CONNECT = 101,
    UDP_DISCONNECT = 102,
    UDP_SEND = 103,
    UDP_RECV = 104,
};

struct pifus_operation {
    enum operation_code op;
    // TODO: arguments? e.g. connect
};

#endif