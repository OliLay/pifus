#include "listen.h"

/* pifus */
#include "utils/log.h"

struct pifus_operation_result
tx_tcp_listen(struct pifus_internal_operation *internal_op) {
  struct pifus_listen_data *listen_data = &internal_op->operation.data.listen;
  struct tcp_pcb *pcb = internal_op->socket->pcb.tcp;

  pcb = tcp_listen_with_backlog(pcb, listen_data->backlog_size);

  struct pifus_operation_result operation_result;
  if (pcb == NULL) {
    pifus_log("pifus: could not tcp_listen!\n");
    operation_result.result_code = PIFUS_ERR;
  } else {
    pifus_debug_log("pifus: PIFUS -> lwIP tcp_connect succeeded!\n");
    internal_op->socket->pcb.tcp = pcb;
    operation_result.result_code = PIFUS_OK;
  }

  return operation_result;
}