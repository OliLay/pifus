#ifndef LWIP_DISCOVERY_H
#define LWIP_DISCOVERY_H

#include "pifus_constants.h"
#include "pifus_identifiers.h"
#include "pifus_ring_buffer.h"
#include "utils/futex.h"

struct pifus_new_socket_queue {
  struct pifus_socket_identifier_queue socket_queue;
  struct pifus_socket_identifier socket_queue_buffer[DISCOVERY_MAX_NEW_SOCKETS];
  futex_t socket_queue_futex;
};

void *discovery_thread_loop(void *arg);

void start_discovery_thread(void);

#endif