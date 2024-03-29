/* std incluces */
#include "errno.h"
#include "stdio.h"
#include "string.h"
#include "sys/eventfd.h"

/* lwIP includes */
#include "lwip/debug.h"
#include "lwip/ip_addr.h"
#include "lwip/mem.h"
#include "lwip/opt.h"
#include "lwip/stats.h"
#include "lwip/sys.h"
#include "lwip/tcp.h"
#include "lwip/timeouts.h"
#include "netif/tapif.h"

/* pifus */
#include "pifus_shmem.h"
#include "utils/futex.h"
#include "utils/log.h"

/* local includes */
#include "discovery.h"
#include "init.h"
#include "stack.h"
#include "tcp/accept.h"
#include "tcp/bind.h"
#include "tcp/close.h"
#include "tcp/connect.h"
#include "tcp/listen.h"
#include "tcp/recv.h"
#include "tcp/write.h"

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
 * Socket futex shadows.
 */
futex_t socket_futexes[MAX_APP_AMOUNT][MAX_SOCKETS_PER_APP];
/**
 * TX operations from all sockets.
 */
struct pifus_priority_aware_ring_buffer tx_queue;
/**
 * How many times the stack iterated with a full SNDBUFFER.
 */
uint8_t full_sndbuf_iterations = 0;

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
process_tx_op(struct pifus_internal_operation *internal_op) {
  struct pifus_operation_result operation_result;

  if (internal_op->operation.code == NOP || internal_op->operation.code == CLIENT_NOP) {
    operation_result.result_code = PIFUS_OK;
  } else {
    if (is_tcp_operation(&internal_op->operation)) {
      if (internal_op->socket->pcb.tcp == NULL) {
        // this has to happen in the main lwIP thread, as calling tcp_* funcs is
        // not safe in the TX thread
        struct tcp_pcb * pcb = tcp_new();
        internal_op->socket->pcb.tcp = pcb;

        // TODO: check if this has an impact on the other benchmarks
        tcp_nagle_disable(pcb);
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
      case TCP_RECV:
        operation_result = tx_tcp_recv(internal_op);
        break;
      case TCP_LISTEN:
        operation_result = tx_tcp_listen(internal_op);
        break;
      case TCP_ACCEPT:
        operation_result = tx_tcp_accept(internal_op);
        break;
      case TCP_CLOSE:
        operation_result = tx_tcp_close(internal_op);
        break;
      default:
        pifus_log("pifus: TX op code %u is not known!\n",
                  internal_op->operation.code);
      }
    } else {
      pifus_log("pifus: UDP TX not implemented yet! bye bye \n");
      exit(1);
    }
  }

  // set the opcode to the according one that was TX'ed
  operation_result.code = internal_op->operation.code;
  return operation_result;
}

bool is_recv_op(struct pifus_internal_operation *operation) {
  return operation->operation.code == TCP_RECV;
}

bool handle_operation(struct pifus_internal_operation *tx_op, bool recv_scan,
                      uint8_t *dequeued_ops) {
  const app_index_t app_index = tx_op->socket->identifier.app_index;
  const socket_index_t socket_index = tx_op->socket->identifier.socket_index;

  pifus_debug_log("pifus: Operation received from app%u/socket%u: %s\n",
                  app_index, socket_index,
                  operation_str(tx_op->operation.code));

  struct pifus_operation_result operation_result = process_tx_op(tx_op);

  if (operation_result.code == NOP && !recv_scan) {
    pifus_priority_aware_ring_buffer_erase_first(&tx_queue);
    return true;
  }

  if (operation_result.result_code == PIFUS_TRY_AGAIN) {
    if (full_sndbuf_iterations >= MAX_FULL_SND_BUF_ITERATIONS) {
      pifus_log("Scanning tx_queue for recv ops to prevent deadlock, as SNDBUF "
                "was full for %u iterations.\n",
                full_sndbuf_iterations);
      struct pifus_internal_operation *recv_op = NULL;
      if (pifus_priority_aware_ring_buffer_find(&tx_queue, &recv_op,
                                                &is_recv_op)) {
        return handle_operation(recv_op, true, dequeued_ops);
      } else {
        return false;
      }
    }

    full_sndbuf_iterations++;

    return false;
  }

  if (operation_result.result_code != PIFUS_ASYNC) {
    // ASYNC ops are not enqueued into cqueue, only after they have finished
    enqueue_in_cqueue(tx_op->socket, &operation_result);
  }

  tx_op->socket->dequeued_ops++;

  if (recv_scan) {
    // we took an op further down, replace it with NOP as we can not
    // pop from the middle of the queue
    tx_op->operation.code = NOP;
  } else {
    full_sndbuf_iterations = 0;
    pifus_priority_aware_ring_buffer_erase_first(&tx_queue);
    (*dequeued_ops)++;
  }

  return true;
}

void lwip_loop_iteration(void) {
  uint8_t dequeued_operations = 0;

  struct pifus_internal_operation *tx_op;
  while (dequeued_operations < MAX_DEQUEUES_PER_ITERATION &&
         pifus_priority_aware_ring_buffer_peek(&tx_queue, &tx_op)) {

    if (!handle_operation(tx_op, false, &dequeued_operations)) {
      return;
    }
  }
}

void lwip_init_complete(void) {
  pifus_debug_log("pifus: lwip init complete.\n");

  pifus = true;
  tx_fd = eventfd(0, EFD_NONBLOCK);

  pifus_priority_aware_ring_buffer_create(&tx_queue);
  start_discovery_thread();
}

int main(int argc, char *argv[]) {
  LWIP_UNUSED_ARG(argc);
  LWIP_UNUSED_ARG(argv);

  pifus_log("pifus: Starting up...\n");

  run_lwip(&lwip_init_complete, &lwip_loop_iteration, "192.168.1.200",
           "192.168.1.1", "255.255.255.0");

  return 0;
}

void signal_tx_interrupt(void) { eventfd_write(tx_fd, 1); }