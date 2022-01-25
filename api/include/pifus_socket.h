#ifndef PIFUS_SOCKET_H
#define PIFUS_SOCKET_H

#include "data_structures/pifus_operation_ring_buffer.h"
#include "pifus_constants.h"
#include "pifus_shmem.h"

/**
 * @brief Creates a new pifus socket.
 *
 * @return Pointer to a pifus_socket
 */
struct pifus_socket *pifus_socket(enum protocol protocol);

void pifus_socket_bind(struct pifus_socket *socket, enum ip_type ip_type,
                       uint16_t port);

void pifus_socket_poll(struct pifus_socket *socket,
                       struct pifus_operation_result *operation_result);

void pifus_socket_exit_all(void);

#endif /* PIFUS_SOCKET_H */
