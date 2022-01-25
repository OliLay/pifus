#ifndef PIFUS_SOCKET_H
#define PIFUS_SOCKET_H

#include "pifus_ring_buffer.h"
#include "pifus_constants.h"
#include "pifus_shmem.h"

/**
 * @brief Creates a new pifus socket.
 *
 * @return Pointer to a pifus_socket
 */
struct pifus_socket *pifus_socket(enum protocol protocol);

/**
 * @brief Binds a socket to a certain port and address type (IPv4/6 or any of them).
 * 
 * @param socket The socket that should be bound.
 * @param ip_type The IP address type that should be used.
 * @param port The port that should be used.
 */
void pifus_socket_bind(struct pifus_socket *socket, enum ip_type ip_type,
                       uint16_t port);

/**
 * @brief Waits for a operation inside a socket to complete.
 * 
 * @param socket The socket that is relevant.
 * @param operation_result Pointer to the struct where the result should be stored in.
 */
void pifus_socket_wait(struct pifus_socket *socket,
                       struct pifus_operation_result *operation_result);

void pifus_socket_exit_all(void);

#endif /* PIFUS_SOCKET_H */
