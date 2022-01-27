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
#include "init.h"
#include "pifus_shmem.h"
#include "tx.h"
#include "utils/futex.h"
#include "utils/log.h"

/* local includes */
#include "stack.h"

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
 * Socket tcp_pcbs pointers (representing the lwIP connection)
 */
struct tcp_pcb *socket_tcp_pcbs[MAX_APP_AMOUNT][MAX_SOCKETS_PER_APP];
/**
 * TX operations from all sockets.
 */
struct pifus_tx_queue tx_queue;

void enqueue_in_cqueue(struct pifus_socket *socket,
                       struct pifus_operation_result *operation_result) {
  pifus_operation_result_ring_buffer_put(&socket->cqueue, socket->cqueue_buffer,
                                         *operation_result);

  socket->cqueue_futex++;
  futex_wake(&socket->cqueue_futex);
}

struct pifus_operation_result
tx_tcp_bind(struct pifus_internal_operation *internal_op) {
  err_t result;
  struct pifus_bind_data *bind_data = &internal_op->operation.data.bind;
  struct tcp_pcb *pcb = internal_op->pcb.tcp;

  if (bind_data->ip_type == PIFUS_IPV4_ADDR) {
    result = tcp_bind(pcb, IP4_ADDR_ANY, bind_data->port);
  } else if (bind_data->ip_type == PIFUS_IPV6_ADDR) {
    result = tcp_bind(pcb, IP6_ADDR_ANY, bind_data->port);
  } else if (bind_data->ip_type == PIFUS_IPVX_ADDR) {
    result = tcp_bind(pcb, IP_ANY_TYPE, bind_data->port);
  } else {
    pifus_log("pifus: unknown ip_type in bind()\n");
  }

  struct pifus_operation_result operation_result;
  if (result == ERR_OK) {
    pifus_debug_log("pifus: PIFUS -> lwIP tcp_bind suceeded!\n");
    operation_result.result_code = PIFUS_OK;
  } else {
    pifus_log("pifus: could not tcp_bind!\n");
    operation_result.result_code = PIFUS_ERR;
  }

  return operation_result;
}

struct pifus_operation_result
process_tx_op(struct pifus_internal_operation *internal_op) {
  if (is_tcp_operation(&internal_op->operation)) {
    internal_op->pcb.tcp =
        socket_tcp_pcbs[internal_op->socket_identifier.app_index]
                       [internal_op->socket_identifier.socket_index];

    struct pifus_operation_result operation_result;
    switch (internal_op->operation.code) {
    case TCP_BIND:
      operation_result = tx_tcp_bind(internal_op);
      break;
    default:
      pifus_log("pifus: TX op code %u is not known!\n",
                internal_op->operation.code);
    }

    // set the opcode to the according one that was TX'ed
    operation_result.code = internal_op->operation.code;
    return operation_result;
  } else {
    pifus_log("pifus: UDP TX not implemented yet! bye bye \n");
    exit(1);
    // TODO: UDP TX handling
  }
}

void lwip_loop_iteration(void) {
  /** TODO:
   * - [TODO] distribute RX packets
   * - [IN PROGRESS] TX handling (from TX thread via queue)
   */

  struct pifus_internal_operation tx_op;
  while (pifus_tx_ring_buffer_get(&tx_queue.ring_buffer,
                                  tx_queue.tx_queue_buffer, &tx_op)) {
    app_index_t app_index = tx_op.socket_identifier.app_index;
    socket_index_t socket_index = tx_op.socket_identifier.socket_index;

    pifus_log("pifus: Operation received from app%u/socket%u: %s\n", app_index,
              socket_index, operation_str(tx_op.operation.code));

    struct pifus_operation_result operation_result = process_tx_op(&tx_op);

    struct pifus_socket *socket = socket_ptrs[app_index][socket_index];

    pifus_debug_log("pifus: Enqueuing into cqueue from socket %u in app %u\n",
                    app_index, socket_index);
    enqueue_in_cqueue(socket, &operation_result);

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
  pifus_debug_log("pifus: lwip init complete.\n");

  pifus_tx_ring_buffer_create(&tx_queue.ring_buffer, TX_QUEUE_SIZE);
  start_tx_thread();
}

int main(int argc, char *argv[]) {
  LWIP_UNUSED_ARG(argc);
  LWIP_UNUSED_ARG(argv);

  pifus_log("pifus: Starting up...\n");

  ip_addr_t *stack_addr = (ip_addr_t *)malloc(sizeof(ip_addr_t));
  ipaddr_aton("192.168.1.201", stack_addr);

  run_lwip(&lwip_init_complete, &lwip_loop_iteration, "192.168.1.200",
           "192.168.1.1", "255.255.255.0");

  return 0;
}