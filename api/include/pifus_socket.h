#ifndef PIFUS_SOCKET_H
#define PIFUS_SOCKET_H

/**
 * @brief Internal, client-side datastructure of a socket.
 * Holds pointers to squeue and cqueue.
 */
struct pifus_socket {
    int* shmem_ptr;
    int* squeue_ptr;
    int* squeue_buffer_ptr;
    int* cqueue_ptr;
    int* cqueue_buffer_ptr;
};

/**
 * @brief Creates a new pifus socket.
 * 
 * @return Pointer to a pifus_socket 
 */
struct pifus_socket* socket(void);

#endif /* PIFUS_SOCKET_H */
