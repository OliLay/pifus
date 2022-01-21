#ifndef LWIP_STACK_H
#define LWIP_STACK_H

/** Uniquely identifies a socket. */
struct pifus_socket_identifier
{
    app_index_t app_index;
    socket_index_t socket_index;
};

/**
 * Internal representation of an operation.
 * Contains the operation and information about the socket.
 */
struct internal_pifus_operation {
    struct pifus_operation operation;
    struct pifus_socket_identifier socket_identifier;
};

struct pifus_tx_queue {
   // struct pifus_ring_buffer ring_buffer;
    struct internal_pifus_operation tx_queue_buffer[TX_QUEUE_SIZE];
};


extern struct pifus_app *app_ptrs[MAX_APP_AMOUNT];
extern struct pifus_socket *socket_ptrs[MAX_APP_AMOUNT][MAX_SOCKETS_PER_APP];
extern app_index_t next_app_number;
extern struct pifus_tx_queue tx_queue;

/**
 * @brief Called every time the lwIP main loop processes, i.e. after checking
 * timeouts and polling the driver.
 */
void lwip_loop_iteration(void);

/**
 * @brief Called when lwIP stack init is complete.
 */
void lwip_init_complete(void);

#endif /* LWIP_STACK_H */
