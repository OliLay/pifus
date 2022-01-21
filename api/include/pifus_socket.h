#ifndef PIFUS_SOCKET_H
#define PIFUS_SOCKET_H

#include "data_structures/pifus_operation_ring_buffer.h"
#include "pifus_shmem.h"
#include "pifus_constants.h"

/**
 * @brief Creates a new pifus socket.
 * 
 * @return Pointer to a pifus_socket 
 */
struct pifus_socket* pifus_socket(enum protocol protocol);

void enqueue_operation(struct pifus_socket* socket, struct pifus_operation const op);

void pifus_socket_exit_all(void);

#endif /* PIFUS_SOCKET_H */
