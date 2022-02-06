#include "connect.h"

/* pifus */
#include "stack.h"
#include "utils/log.h"


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