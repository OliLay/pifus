#include "accept.h"

/* pifus */
#include "stack.h"
#include "utils/log.h"

err_t tcp_accepted_callback(void *arg, struct tcp_pcb *newpcb, err_t err) {
  struct pifus_socket *socket = arg;
  pifus_debug_log("pifus: tcp_accepted_callback called for (%u/%u)!\n",
                  socket->identifier.app_index,
                  socket->identifier.socket_index);

  struct pifus_operation_result operation_result;
  operation_result.code = TCP_ACCEPT;
  operation_result.result_code = PIFUS_OK;

  operation_result.data.accept.socket = NULL;
  operation_result.data.accept.pcb = newpcb;

  enqueue_in_cqueue(socket, &operation_result);

  return ERR_OK;
}

struct pifus_operation_result
tx_tcp_accept(struct pifus_internal_operation *internal_op) {
  struct tcp_pcb *pcb = internal_op->socket->pcb.tcp;

  tcp_accept(pcb, &tcp_accepted_callback);

  struct pifus_operation_result operation_result;
  pifus_debug_log("pifus: PIFUS -- *async* --> lwIP tcp_accept succeeded!\n");
  operation_result.result_code = PIFUS_ASYNC;

  return operation_result;
}