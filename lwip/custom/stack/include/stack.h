#ifndef LWIP_STACK_H
#define LWIP_STACK_H

extern struct pifus_app *app_ptrs[MAX_APP_AMOUNT];
extern struct pifus_socket *socket_ptrs[MAX_APP_AMOUNT][MAX_SOCKETS_PER_APP];
extern app_index_t next_app_number;

/* TODO: docu */
void map_new_sockets(app_index_t app_index);
void scan_for_app_regions(void);

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
