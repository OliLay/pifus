/* std incluces */
#include "errno.h"
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

/* pifus */
#include "pifus_shmem.h"
#include "utils/futex.h"

/* local includes */
#include "init.h"
#include "stack.h"
#include "tx.h"

/**
 * app_ptrs[#app] -> ptr to shmem app region
 */
struct pifus_app *app_ptrs[MAX_APP_AMOUNT];
/**
 * next app number to be assigned
 */
app_index_t next_app_number = 0;
/**
 * socket_ptrs[#app][#socket] -> ptr to shmem socket region of #app and
 * corresponding #socket of that app
 */
struct pifus_socket *socket_ptrs[MAX_APP_AMOUNT][MAX_SOCKETS_PER_APP];
/**
 * TX operations from all sockets.
 */
struct pifus_tx_queue tx_queue;

void lwip_loop_iteration(void) {
    /** TODO:
     * - [TODO] distribute RX packets
     * - [IN PROGRESS] TX handling (from TX thread via queue)
     */

    struct pifus_internal_operation tx_op;
    while (pifus_tx_ring_buffer_get(&tx_queue.ring_buffer,
                                    tx_queue.tx_queue_buffer, &tx_op)) {
        printf("pifus: Operation received from app%u/socket%u: %s\n",
               tx_op.socket_identifier.app_index,
               tx_op.socket_identifier.socket_index,
               operation_str(tx_op.operation.op));

        /**
         * TODO:
         *  - take action depending on op (e.g. call send, recv)
         *  - for some operation types, we need a look up the operation
         * afterwards, e.g. for recv
         *      - we dequeue TX op here and call recv()
         *      - recv_callback() is called later on
         *      - we then have to look up the recv operation again to find
         *  - for other operation types, such as write, we can directly execute
         * the action and insert something into cqueue
         */
    }
}

void lwip_init_complete(void) {
    printf("pifus: lwip init complete.\n");

    pifus_tx_ring_buffer_create(&tx_queue.ring_buffer, TX_QUEUE_SIZE);
    start_tx_thread();
}

int main(int argc, char *argv[]) {
    LWIP_UNUSED_ARG(argc);
    LWIP_UNUSED_ARG(argv);

    printf("pifus: Starting up...\n");

    ip_addr_t *stack_addr = (ip_addr_t *)malloc(sizeof(ip_addr_t));
    ipaddr_aton("192.168.1.201", stack_addr);

    run_lwip(&lwip_init_complete, &lwip_loop_iteration, "192.168.1.200",
             "192.168.1.1", "255.255.255.0");

    return 0;
}