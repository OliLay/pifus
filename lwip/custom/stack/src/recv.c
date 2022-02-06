#include "recv.h"

/* pifus */
#include "stack.h"
#include "utils/log.h"

err_t tcp_recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p,
                        err_t err) {
  struct pifus_socket *socket = arg;
  pifus_debug_log("pifus: tcp_recv_callback called for (%u/%u)!\n",
                  socket->identifier.app_index,
                  socket->identifier.socket_index);

  if (err = ERR_OK) {

  } else {
  }

  return ERR_OK;
  // TODO
}

struct pifus_operation_result
tx_tcp_recv(struct pifus_internal_operation *internal_op) {
  struct pifus_recv_data *recv_data = &internal_op->operation.data.recv;
  struct pifus_socket *socket = internal_op->socket;
  struct tcp_pcb *pcb = socket->pcb.tcp;

  tcp_recv(pcb, &tcp_recv_callback);

  struct pifus_recv_queue_entry recv_queue_entry;
  recv_queue_entry.size = recv_data->size;
  recv_queue_entry.recv_block_offset = recv_data->recv_block_offset;

  struct pifus_operation_result operation_result;

  if (pifus_recv_queue_put(&socket->recv_queue, socket->recv_queue_buffer,
                           recv_queue_entry)) {
    operation_result.result_code = PIFUS_ASYNC;
  } else {
    operation_result.result_code = PIFUS_ERR;
    pifus_log(
        "pifus: Could not store recv length/offset in recv_queue_buffer as "
        "it is full!\n");
  }
  // TODO
  operation_result.data.recv = internal_op->operation.data.recv;

  return operation_result;

  // TODO
}