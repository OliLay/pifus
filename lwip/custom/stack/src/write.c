#include "write.h"

/* pifus */
#include "stack.h"
#include "utils/log.h"

/**
 * True when the send buffer is full, else false.
 */
bool send_buffer_full = false;

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

    struct pifus_write_queue_entry write_queue_entry;
    write_queue_entry.size = block->size;
    write_queue_entry.write_block_offset = write_data->block_offset;

    if (pifus_write_queue_put(&socket->write_queue, socket->write_queue_buffer,
                              write_queue_entry)) {
      operation_result.result_code = PIFUS_ASYNC;
    } else {
      operation_result.result_code = PIFUS_ERR;
      pifus_log("pifus: Could not store write length in write_queue_buffer as "
                "it is full!\n");
    }

    tcp_sent(pcb, &tcp_sent_callback);
  } else {
    if (result == ERR_MEM) {
      send_buffer_full = true;
      operation_result.result_code = PIFUS_TRY_AGAIN;
    } else {
      pifus_log("pifus: could not tcp_write as err is '%i'\n", result);
      operation_result.result_code = PIFUS_ERR;
    }
  }

  return operation_result;
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
    operation_result.data.write.block_offset =
        write_queue_entry->write_block_offset;

    pifus_write_queue_erase_first(&socket->write_queue);

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
