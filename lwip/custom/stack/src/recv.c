#include "recv.h"

/* std */
#include "string.h"

/* pifus */
#include "pifus_shmem.h"
#include "stack.h"
#include "utils/log.h"

err_t tcp_recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p,
                        err_t err) {
  struct pifus_socket *socket = arg;
  pifus_debug_log("pifus: tcp_recv_callback called for (%u/%u)!\n",
                  socket->identifier.app_index,
                  socket->identifier.socket_index);

  uint16_t len_data_available = 0;
  if (err == ERR_OK) {
    if (p == NULL) {
      pifus_log("Connection closed for (%u/%u)! (tcp_recv_callback)\n",
                socket->identifier.app_index, socket->identifier.socket_index);
      return ERR_OK;
    }
    struct pifus_app *app = app_ptrs[socket->identifier.app_index];

    len_data_available = p->tot_len;
    struct pbuf *next_pbuf = p;
    uint16_t offset_inside_next_pbuf = 0;
    struct pifus_recv_queue_entry *recv_queue_entry;

    while (pifus_recv_queue_peek(&socket->recv_queue, socket->recv_queue_buffer,
                                 &recv_queue_entry) &&
           len_data_available > 0) {

      struct pifus_memory_block *recv_block =
          shm_data_get_block_ptr(app, recv_queue_entry->recv_block_offset);
      uint16_t len_needed_recv = recv_queue_entry->size;

      bool recv_op_fulfilled = true;
      if (len_needed_recv > len_data_available) {
        recv_op_fulfilled = false;
      }

      while (next_pbuf != NULL && len_needed_recv > 0) {
        uint16_t amount_to_copy;
        uint16_t len_next_pbuf = next_pbuf->len - offset_inside_next_pbuf;
        bool pbuf_completely_consumed = true;

        // essentially min(len_needed_recv, len_next_pbuf)
        if (len_next_pbuf > len_needed_recv) {
          amount_to_copy = len_needed_recv;
          pbuf_completely_consumed = false;
        } else {
          amount_to_copy = len_next_pbuf;
        }

        // copy into shared memory
        memcpy((uint8_t *)shm_data_get_data_ptr(recv_block) +
                   recv_queue_entry->data_offset,
               next_pbuf->payload + offset_inside_next_pbuf, amount_to_copy);

        len_data_available -= amount_to_copy;
        len_needed_recv -= amount_to_copy;

        if (pbuf_completely_consumed) {
          offset_inside_next_pbuf = 0;
          next_pbuf = next_pbuf->next;
        } else {
          offset_inside_next_pbuf += amount_to_copy;
        }
      }

      // TODO: maybe move down and not per recv() cqueue entry, but overall
      tcp_recved(tpcb, len_needed_recv);

      if (recv_op_fulfilled) {
        struct pifus_operation_result operation_result;
        operation_result.code = TCP_RECV;
        operation_result.result_code = PIFUS_OK;
        operation_result.data.recv.recv_block_offset =
            recv_queue_entry->recv_block_offset;
        operation_result.data.recv.size = recv_queue_entry->size;

        // to be set on client side
        operation_result.data.recv.memory_block_ptr = NULL;

        pifus_recv_queue_erase_first(&socket->recv_queue);
        enqueue_in_cqueue(socket, &operation_result);
      } else {
        recv_queue_entry->size -= len_data_available;
        recv_queue_entry->data_offset = len_data_available;
      }
    }
  } else {
    pifus_log("pifus: err in tcp_recv_callback %i\n", err);
    // err
  }

  if (len_data_available > 0) {
    pifus_log("pifus: no recv() call any more, but recv'd %u more bytes...\n",
              len_data_available);
    /* TODO: handle case, when no squeue entry is there any more, but we have
     * data left here.
     * 2 options:
     *   - need to save it somehow here (copy), as pbuf chain is freed
     * afterwards.
     *   - free it. it will not be ACK'ed and therefore be retransmitted. (not
     * a good option imho)
     * */
  }

  pbuf_free(p);
  return ERR_OK;
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
  recv_queue_entry.data_offset = 0;

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

  operation_result.data.recv = internal_op->operation.data.recv;

  return operation_result;
}