#ifndef LWIP_STACK_H
#define LWIP_STACK_H

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
