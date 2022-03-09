#ifndef PIFUS_SOCKET_H
#define PIFUS_SOCKET_H

#include "pifus_constants.h"
#include "pifus_ip.h"
#include "pifus_ring_buffer.h"
#include "pifus_shmem.h"
#include "stddef.h"

extern pthread_mutex_t sockets_mutex;
extern struct pifus_socket *sockets[MAX_SOCKETS_PER_APP];

/**
 * @brief Creates a new pifus socket.
 *
 * @return Pointer to a pifus_socket
 */
struct pifus_socket *pifus_socket(enum pifus_protocol protocol,
                                  enum pifus_priority priority);

/**
 * @brief Binds a socket to a certain port and address type (IPv4/6 or any of
 * them).
 *
 * @param socket The socket that should be bound.
 * @param ip_type The IP address type that should be used.
 * @param port The port that should be used.
 * @return true upon success, false upon error.
 */
bool pifus_socket_bind(struct pifus_socket *socket, enum pifus_ip_type ip_type,
                       uint16_t port);

/**
 * @brief Connects a socket to another host.
 *
 * @param socket The socket that should used.
 * @param ip_addr The IP address that should be connected to.
 * @param port The port that should be connected to.
 * @return true upon success, false upon error.
 */
bool pifus_socket_connect(struct pifus_socket *socket,
                          struct pifus_ip_addr ip_addr, uint16_t port);

/**
 * @brief Write to a socket.
 *
 * @param socket The socket that should written on.
 * @param data Pointer to the data.
 * @param size Size of the data.
 * @return true upon success, false upon error.
 */
bool pifus_socket_write(struct pifus_socket *socket, void *data, size_t size);

/**
 * @brief Receive from a socket.
 *
 * @param socket The socket that should be received on.
 * @param size Size that should be read.
 * @return true upon success, false upon error.
 */
bool pifus_socket_recv(struct pifus_socket *socket, size_t size);

/**
 * @brief Listen on a socket.
 *
 * @param socket The socket that should be listened on.
 * @param backlog maximum length to which the queue of pending connections may
 * grow
 * @return true upon success, false upon error.
 */
bool pifus_socket_listen(struct pifus_socket *socket, uint8_t backlog_size);

/**
 * @brief Accept connection on a socket.
 *
 * @param socket The socket where a connection should be accepted on.
 * grow
 * @param accepted_sockets_priority What kind of priority the accepted sockets
 * should have.
 * @return true upon success, false upon error.
 */
bool pifus_socket_accept(struct pifus_socket *socket,
                         enum pifus_priority accepted_sockets_priority);

/**
 * @brief Close a socket.
 * NOTE: after this is returned successfully (in cqueue),
 * no further operations can be issued on the socket.
 *
 * @param socket The socket that should be closed.
 * @return true upon success, false upon error.
 */
bool pifus_socket_close(struct pifus_socket *socket);

/**
 * @brief Insert a NOP operation. This operation has no effect and is just used
 * for benchmarking purposes.
 *
 * @param socket The socket that do a NOP.
 * @return true upon success, false upon error.
 */
bool pifus_socket_nop(struct pifus_socket *socket);

/**
 * @brief Dequeues the latest result received from the stack.
 *
 * @param socket The socket which the result should be related to.
 * @param operation_result Output argument
 * @return true If there was a new result and it was written into the output arg
 * @return false If there was no new result
 */
bool pifus_socket_pop_result(struct pifus_socket *socket,
                             struct pifus_operation_result *operation_result);

/**
 * @brief Gets the op code of the next result.
 *
 * @param socket The socket that the result should belong to.
 * @param code Out: Pointer to a pointer, pointing to opcode on success
 * @return true Success
 * @return false No success (e.g. nothing in queue)
 */
bool pifus_socket_peek_result_code(struct pifus_socket *socket,
                                   enum pifus_operation_code **code);

/**
 * @brief Waits for a operation inside a socket to complete.
 *
 * @param socket The socket that is relevant.
 * @param operation_result Pointer to the struct where the result should be
 * stored in.
 */
void pifus_socket_wait(struct pifus_socket *socket,
                       struct pifus_operation_result *operation_result);

/**
 * @brief Frees the data belonging to the operation result.
 *
 */
void pifus_free(struct pifus_operation_result *operation_result);

void pifus_socket_exit_all(void);

#endif /* PIFUS_SOCKET_H */
