#ifndef LWIP_TX_H
#define LWIP_TX_H

void *tx_thread_loop(void *arg);

void start_tx_thread(void);

#endif