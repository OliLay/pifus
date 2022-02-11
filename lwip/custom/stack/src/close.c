#include "close.h"

/* pifus */
#include "stack.h"
#include "utils/log.h"

/* lwIP */
#include "lwip/tcp.h"

struct pifus_operation_result
tx_tcp_close(struct pifus_internal_operation *internal_op) {
  err_t result;
  struct tcp_pcb *pcb = internal_op->socket->pcb.tcp;
  result = tcp_close(pcb);

  struct pifus_operation_result operation_result;
  if (result == ERR_OK) {
    operation_result.result_code = PIFUS_OK;

    app_index_t app_index = internal_op->socket->identifier.app_index;
    socket_index_t socket_index = internal_op->socket->identifier.socket_index;
    // do not further use this socket. (e.g. for futex wait)
    socket_ptrs[app_index][socket_index] = NULL;

    pifus_log("pifus: Closed socket (%u/%u)!\n", app_index, socket_index);
  } else {
    pifus_log("pifus: could not tcp_close!\n");
    operation_result.result_code = PIFUS_ERR;
  }

  return operation_result;
}