#ifndef LWIP_PING_H
#define LWIP_PING_H

#include "lwip/ip_addr.h"

/**
 * PING_USE_SOCKETS: Set to 1 to use sockets, otherwise the raw api is used
 */
#ifndef PING_USE_SOCKETS
#define PING_USE_SOCKETS  0
#endif

void ping_init(void);

#if !PING_USE_SOCKETS
void ping_send_now(void);
#endif /* !PING_USE_SOCKETS */

#endif /* LWIP_PING_H */
