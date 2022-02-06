#include "bind.h"

/* pifus */
#include "utils/log.h"

/* lwIP */
#include "lwip/tcp.h"

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