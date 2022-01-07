#ifndef PIFUS_SOCKET_H
#define PIFUS_SOCKET_H

/**
 * @brief Internal, client-side datastructure of a socket.
 * Holds pointers to squeue and cqueue.
 */
struct pifus_socket {
    // TODO
};

/**
 * @brief Creates a new pifus socket.
 * 
 * @return Pointer to a pifus_socket 
 */
struct pifus_socket* socket(void);

#endif /* PIFUS_SOCKET_H */
