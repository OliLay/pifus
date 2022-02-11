#include "recv.h"

/* std */
#include "string.h"

/* pifus */
#include "pifus_byte_buffer.h"
#include "pifus_shmem.h"
#include "stack.h"
#include "utils/log.h"

uint16_t pop_from_buffer(struct pifus_socket *socket,
                         struct pifus_memory_block *recv_block,
                         struct pifus_recv_queue_entry *recv_queue_entry,
                         uint16_t len_needed_recv) {
  uint16_t len_bytes_from_recv_buffer = pifus_byte_buffer_pop(
      &socket->recv_buffer, socket->recv_buffer_array, len_needed_recv,
      (uint8_t *)shm_data_get_data_ptr(recv_block) +
          recv_queue_entry->data_offset);

  if (len_bytes_from_recv_buffer > 0 &&
      recv_queue_entry->size > len_bytes_from_recv_buffer) {
    recv_queue_entry->size -= len_bytes_from_recv_buffer;
    recv_queue_entry->data_offset += len_bytes_from_recv_buffer;
  }

  return len_bytes_from_recv_buffer;
}

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
    uint16_t total_data_ackable = 0;

    bool recv_buffer_empty = pifus_byte_buffer_is_empty(&socket->recv_buffer);

    while (pifus_recv_queue_peek(&socket->recv_queue, socket->recv_queue_buffer,
                                 &recv_queue_entry) &&
           (len_data_available > 0 || !recv_buffer_empty)) {

      struct pifus_memory_block *recv_block =
          shm_data_get_block_ptr(app, recv_queue_entry->recv_block_offset);
      uint16_t len_needed_recv = recv_queue_entry->size;

      if (!recv_buffer_empty) {
        uint16_t len_bytes_from_recv_buffer = pop_from_buffer(
            socket, recv_block, recv_queue_entry, len_needed_recv);

        if (len_bytes_from_recv_buffer > 0) {
          len_needed_recv -= len_bytes_from_recv_buffer;
        }
      } else {
        recv_buffer_empty = true;
      }

      bool recv_op_fulfilled = true;
      printf("Need further %u bytes for the buffer, %u bytes available!\n",
             len_needed_recv, len_data_available);
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

        // TODO: doesn't copy that 1 byte for some reason...
        if (amount_to_copy == 1) {
              printf("Copied %u bytes into entry with offset %lu and size %u\n",
               amount_to_copy, recv_queue_entry->data_offset,
               amount_to_copy);
        }
      
        len_data_available -= amount_to_copy;
        len_needed_recv -= amount_to_copy;
        total_data_ackable += amount_to_copy;

        if (pbuf_completely_consumed) {
          offset_inside_next_pbuf = 0;
          next_pbuf = next_pbuf->next;
        } else {
          offset_inside_next_pbuf += amount_to_copy;
        }

        if (!recv_op_fulfilled) {
          recv_queue_entry->size -= amount_to_copy;
          recv_queue_entry->data_offset += amount_to_copy;
        }
      }

      if (recv_op_fulfilled) {
        printf("op fulfilled through normal loop!\n");
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
      }
    }

    if (len_data_available > 0) {
      pifus_debug_log(
          "pifus: no recv() call any more, but recv'd %u more bytes. "
          "Buffering!\n",
          len_data_available);

      while (next_pbuf != NULL && len_data_available > 0) {
        uint16_t len_next_pbuf = next_pbuf->len - offset_inside_next_pbuf;

        uint16_t bytes_copied = pifus_byte_buffer_add(
            &socket->recv_buffer, socket->recv_buffer_array,
            next_pbuf->payload + offset_inside_next_pbuf, len_next_pbuf);

        len_data_available -= bytes_copied;
        total_data_ackable += bytes_copied;

        if (bytes_copied < len_next_pbuf) {
          // buffer full
          pifus_log("pifus: recv buffer full. Discarding additional "
                    "data. (%u bytes)\n",
                    len_data_available);
          break;
        }

        next_pbuf = next_pbuf->next;
      }
    }

    tcp_recved(tpcb, total_data_ackable);
  } else {
    pifus_log("pifus: err in tcp_recv_callback %i\n", err);
    // err
  }

  pbuf_free(p);
  return ERR_OK;
}

struct pifus_operation_result
tx_tcp_recv(struct pifus_internal_operation *internal_op) {
  struct pifus_recv_data *recv_data = &internal_op->operation.data.recv;
  struct pifus_socket *socket = internal_op->socket;
  struct tcp_pcb *pcb = socket->pcb.tcp;

  struct pifus_recv_queue_entry recv_queue_entry;
  recv_queue_entry.size = recv_data->size;
  recv_queue_entry.recv_block_offset = recv_data->recv_block_offset;
  recv_queue_entry.data_offset = 0;

  struct pifus_operation_result operation_result;

  if (!pifus_byte_buffer_is_empty(&socket->recv_buffer)) {
    struct pifus_app *app = app_ptrs[socket->identifier.app_index];
    struct pifus_memory_block *recv_block =
        shm_data_get_block_ptr(app, recv_queue_entry.recv_block_offset);

    uint16_t len_bytes_from_recv_buffer = pop_from_buffer(
        socket, recv_block, &recv_queue_entry, recv_queue_entry.size);

    if (len_bytes_from_recv_buffer > 0 &&
        recv_queue_entry.size == len_bytes_from_recv_buffer) {
      printf("op fulfilled through pre checking!\n");

      // filled completely from buffer, can directly return
      operation_result.code = TCP_RECV;
      operation_result.result_code = PIFUS_OK;
      operation_result.data.recv.recv_block_offset =
          recv_queue_entry.recv_block_offset;
      operation_result.data.recv.size = recv_queue_entry.size;
      // to be set on client side
      operation_result.data.recv.memory_block_ptr = NULL;
      return operation_result;
    }
  }

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

  tcp_recv(pcb, &tcp_recv_callback);

  return operation_result;
}