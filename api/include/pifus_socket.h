#ifndef PIFUS_SOCKET_H
#define PIFUS_SOCKET_H

#include "data_structures/pifus_ring_buffer.h"
#include "data_structures/ext/ring_buf.h"
#include "pifus_constants.h"

typedef struct pifus_ring_buffer pifus_squeue;
typedef struct pifus_ring_buffer pifus_cqueue;

enum protocol {
    PROTOCOL_TCP = 0,
    PROTOCOL_UDP = 1
};

/**
 * @brief Internal, client-side datastructure of a socket.
 * Holds pointers to squeue and cqueue.
 */
struct pifus_socket {
    enum protocol protocol;
    pifus_squeue squeue;
    struct pifus_operation squeue_buffer[SQUEUE_SIZE];
    pifus_cqueue cqueue;
    struct pifus_operation cqueue_buffer[CQUEUE_SIZE];
};

/**
 * @brief Creates a new pifus socket.
 * 
 * @return Pointer to a pifus_socket 
 */
struct pifus_socket* pifus_socket(enum protocol protocol);

void enqueue_operation(struct pifus_socket* socket, struct pifus_operation const op);

void pifus_socket_exit_all(void);

#endif /* PIFUS_SOCKET_H */
