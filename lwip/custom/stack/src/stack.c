/* std incluces */
#include "stdio.h"
#include "string.h"

/* lwIP includes */
#include "lwip/debug.h"
#include "lwip/ip_addr.h"
#include "lwip/mem.h"
#include "lwip/opt.h"
#include "lwip/stats.h"
#include "lwip/sys.h"
#include "lwip/tcp.h"
#include "lwip/timeouts.h"

/* local includes */
#include "init.h"
#include "stack.h"

void lwip_loop_iteration(void) { printf("lwip loop callback\n"); }

void lwip_init_complete(void) { printf("lwip init complete.\n"); }

int main(int argc, char *argv[]) {
    LWIP_UNUSED_ARG(argc);
    LWIP_UNUSED_ARG(argv);

    printf("Starting up pifus...\n");

    ip_addr_t *stack_addr = (ip_addr_t *)malloc(sizeof(ip_addr_t));
    ipaddr_aton("192.168.1.201", stack_addr);

    run_lwip(&lwip_init_complete, &lwip_loop_iteration, "192.168.1.200",
             "192.168.1.1", "255.255.255.0");

    return 0;
}