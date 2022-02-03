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
 * TX operations from all sockets.
 */
struct pifus_tx_queue tx_queue;

void enqueue_in_cqueue(struct pifus_socket *socket,
                       struct pifus_operation_result *operation_result) {

  if (pifus_operation_result_ring_buffer_put(
          &socket->cqueue, socket->cqueue_buffer, *operation_result)) {
    pifus_debug_log("pifus: Enqueuing into cqueue for (%u/%u)\n",
                    socket->identifier.app_index,
                    socket->identifier.socket_index);

    socket->cqueue_futex++;
    futex_wake(&socket->cqueue_futex);
  } else {
    pifus_log("pifus: Could not put into cqueue for (%u/%u). Is it full?\n",
              socket->identifier.app_index, socket->identifier.socket_index);
  }
}

err_t tcp_connected_callback(void *arg, struct tcp_pcb *tpcb, err_t err) {
  struct pifus_socket *socket = arg;
  pifus_debug_log("pifus: tcp_connected_callback called for (%u/%u)!\n",
                  socket->identifier.app_index,
                  socket->identifier.socket_index);

  struct pifus_operation_result operation_result;
  operation_result.code = TCP_CONNECT;
  operation_result.result_code = PIFUS_OK;

  enqueue_in_cqueue(socket, &operation_result);

  return ERR_OK;
}

err_t tcp_sent_callback(void *arg, struct tcp_pcb *tpcb, u16_t len) {
  struct pifus_socket *socket = arg;
  pifus_debug_log("pifus: tcp_sent_callback called for (%u/%u)!\n",
                  socket->identifier.app_index,
                  socket->identifier.socket_index);

  struct pifus_write_queue_entry *write_queue_entry;
  if (!pifus_write_queue_peek(&socket->write_queue, socket->write_queue_buffer,
                              &write_queue_entry)) {
    pifus_log(
        "pifus: Could not dequeue write_queue_entry from write_queue in sent "
        "callback! Can't notify app that write was successful.\n");
  }

  if (write_queue_entry->size <= len) {
    struct pifus_operation_result operation_result;
    operation_result.code = TCP_WRITE;
    operation_result.result_code = PIFUS_OK;
    operation_result.data.write.block_offset = write_queue_entry->write_block_offset;

    // TODO: just for dequeing. maybe add erase_first() method which just
    // removes first entry
    pifus_write_queue_get(&socket->write_queue, socket->write_queue_buffer,
                          write_queue_entry);

    enqueue_in_cqueue(socket, &operation_result);

    if (write_queue_entry->size < len) {
      // we need to consider further entries in the queue
      return tcp_sent_callback(arg, tpcb, len - write_queue_entry->size);
    }
  } else {
    // update len of first element in queue
    write_queue_entry->size = write_queue_entry->size - len;
  }

  return ERR_OK;
}

void tcp_err_callback(void *arg, err_t err) {
  struct pifus_socket *socket = arg;
  pifus_log("pifus: tcp_err_callback called for socket %u in app %u!\n",
            socket->identifier.socket_index, socket->identifier.app_index);

  // this was free'd by lwIP automatically, before err_callback is called
  socket->pcb.tcp = NULL;

  struct pifus_operation_result operation_result;
  operation_result.code = CONNECTION_LOST;
  operation_result.result_code = PIFUS_ERR;

  enqueue_in_cqueue(socket, &operation_result);
}

struct pifus_operation_result
tx_tcp_write(struct pifus_internal_operation *internal_op) {
  struct pifus_write_data *write_data = &internal_op->operation.data.write;
  struct pifus_socket *socket = internal_op->socket;

  struct pifus_app *app = app_ptrs[socket->identifier.app_index];
  struct pifus_memory_block *block =
      shm_data_get_block_ptr(app, write_data->block_offset);

  void *data_ptr = shm_data_get_data_ptr(block);

  struct tcp_pcb *pcb = socket->pcb.tcp;
  err_t result = tcp_write(pcb, data_ptr, block->size, 0);

  struct pifus_operation_result operation_result;
  operation_result.data.write = internal_op->operation.data.write;
  if (result == ERR_OK) {
    pifus_debug_log("pifus: PIFUS -- *async* --> lwIP tcp_write succeeded!\n");
    operation_result.result_code = PIFUS_ASYNC;

    struct pifus_write_queue_entry write_queue_entry;
    write_queue_entry.size = block->size;
    write_queue_entry.write_block_offset = write_data->block_offset;

    if (!pifus_write_queue_put(&socket->write_queue, socket->write_queue_buffer,
                               write_queue_entry)) {
      pifus_log("pifus: Could not store write length in write_queue_buffer as "
                "it is full!\n");
    }

    tcp_sent(pcb, &tcp_sent_callback);
  } else {
    pifus_log("pifus: could not tcp_write as err is '%i'\n",
              result);
    operation_result.result_code = PIFUS_ERR;

    // TODO: maybe think of queueing for later and returning to squeue
  }

  return operation_result;
}

struct pifus_operation_result
tx_tcp_connect(struct pifus_internal_operation *internal_op) {
  struct pifus_connect_data *connect_data =
      &internal_op->operation.data.connect;
  struct tcp_pcb *pcb = internal_op->socket->pcb.tcp;

  ip_addr_t reader_addr;
  ipaddr_aton(connect_data->ip_addr.value, &reader_addr);

  err_t result = tcp_connect(pcb, &reader_addr, connect_data->port,
                             &tcp_connected_callback);

  struct pifus_operation_result operation_result;
  if (result == ERR_OK) {
    pifus_debug_log(
        "pifus: PIFUS -- *async* --> lwIP tcp_connect succeeded!\n");
    operation_result.result_code = PIFUS_ASYNC;
  } else {
    pifus_log("pifus: could not tcp_connect!\n");
    operation_result.result_code = PIFUS_ERR;
  }

  return operation_result;
}

struct pifus_operation_result
tx_tcp_bind(struct pifus_internal_operation *internal_op) {
  err_t result;
  struct pifus_bind_data *bind_data = &internal_op->operation.data.bind;
  struct tcp_pcb *pcb = internal_op->socket->pcb.tcp;
  const ip_addr_t *ip_addr_type;

  if (bind_data->ip_type == PIFUS_IPV4_ADDR) {
    ip_addr_type = IP4_ADDR_ANY;
  } else if (bind_data->ip_type == PIFUS_IPV6_ADDR) {
    ip_addr_type = IP6_ADDR_ANY;
  } else if (bind_data->ip_type == PIFUS_IPVX_ADDR) {
    ip_addr_type = IP_ANY_TYPE;
  } else {
    pifus_log("pifus: unknown ip_type in bind()\n");
  }

  result = tcp_bind(pcb, ip_addr_type, bind_data->port);

  struct pifus_operation_result operation_result;
  if (result == ERR_OK) {
    pifus_debug_log("pifus: PIFUS -> lwIP tcp_bind succeeded!\n");
    operation_result.result_code = PIFUS_OK;
  } else {
    pifus_log("pifus: could not tcp_bind!\n");
    operation_result.result_code = PIFUS_ERR;
  }

  return operation_result;
}

struct pifus_operation_result
process_tx_op(struct pifus_internal_operation *internal_op) {
  struct pifus_operation_result operation_result;

  if (is_tcp_operation(&internal_op->operation)) {
    if (internal_op->socket->pcb.tcp == NULL) {
      // this has to happen in the main lwIP thread, as calling tcp_* funcs is
      // not safe in the TX thread
      internal_op->socket->pcb.tcp = tcp_new();
      tcp_arg(internal_op->socket->pcb.tcp, internal_op->socket);
    }

    switch (internal_op->operation.code) {
    case TCP_BIND:
      operation_result = tx_tcp_bind(internal_op);
      break;
    case TCP_CONNECT:
      operation_result = tx_tcp_connect(internal_op);
      break;
    case TCP_WRITE:
      operation_result = tx_tcp_write(internal_op);
      break;
    default:
      pifus_log("pifus: TX op code %u is not known!\n",
                internal_op->operation.code);
    }
  } else {
    pifus_log("pifus: UDP TX not implemented yet! bye bye \n");
    exit(1);
    // TODO: UDP TX handling
  }

  // set the opcode to the according one that was TX'ed
  operation_result.code = internal_op->operation.code;
  return operation_result;
}

void lwip_loop_iteration(void) {
  /** TODO:
   * - [TODO] distribute RX packets
   * - [IN PROGRESS] TX handling (from TX thread via queue)
   */

  struct pifus_internal_operation tx_op;
  while (pifus_tx_ring_buffer_get(&tx_queue.ring_buffer,
                                  tx_queue.tx_queue_buffer, &tx_op)) {
    const app_index_t app_index = tx_op.socket->identifier.app_index;
    const socket_index_t socket_index = tx_op.socket->identifier.socket_index;

    pifus_log("pifus: Operation received from app%u/socket%u: %s\n", app_index,
              socket_index, operation_str(tx_op.operation.code));

    struct pifus_operation_result operation_result = process_tx_op(&tx_op);

    if (operation_result.result_code == PIFUS_ASYNC) {
      pifus_debug_log("pifus: Async operation triggered in lwIP, not enqueing "
                      "into cqueue for %u in app %u\n",
                      app_index, socket_index);
    } else {
      enqueue_in_cqueue(tx_op.socket, &operation_result);
    }

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