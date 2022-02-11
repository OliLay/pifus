#include "pifus_byte_buffer.h"

#include "stdio.h"
#include "utils/log.h"
#include <string.h>

void pifus_byte_buffer_create(struct pifus_byte_buffer *byte_buffer,
                              uint16_t buffer_length) {
  byte_buffer->tail = 0;
  byte_buffer->end = buffer_length;
  byte_buffer->head = 0;
}

uint16_t pifus_byte_buffer_add(struct pifus_byte_buffer *byte_buffer,
                               uint8_t *buffer, uint8_t *to_add,
                               uint16_t to_add_length) {
  uint16_t really_added_length = 0;
  uint16_t head = byte_buffer->head + 1;
  bool overstepped = false;
  while (really_added_length < to_add_length) {
    if (head == byte_buffer->end) {
      head = 0;
      overstepped = true;
    }

    if (head == byte_buffer->tail) {
      break;
    } else {
      head++;
      really_added_length++;
    }
  }

  if (really_added_length > 0) {
    if (overstepped) {
      uint16_t end_copy_length = byte_buffer->end - byte_buffer->head - 1;
      if (end_copy_length > 0) {
        memcpy(&buffer[byte_buffer->head + 1], to_add, end_copy_length);
      }
      memcpy(buffer, to_add + end_copy_length,
             really_added_length - end_copy_length);
    } else {
      memcpy(&buffer[byte_buffer->head + 1], to_add, really_added_length);
    }

    // keep invariant! (head points to last used item!)
    head--;
    byte_buffer->head = head;
  }

  return really_added_length;
}

uint16_t pifus_byte_buffer_pop(struct pifus_byte_buffer *byte_buffer,
                               uint8_t *buffer, uint16_t length,
                               uint8_t *output_buffer) {
  uint16_t tail = byte_buffer->tail;
  bool overstepped = false;
  uint16_t popped_length = 0;
  while (popped_length < length) {
    if (byte_buffer->head == tail) {
      // reached end
      break;
    } else {
      popped_length++;
      ++tail;

      if (tail == byte_buffer->end) {
        tail = 0;
        overstepped = true;
      }
    }
  }

  if (popped_length > 0) {
    if (overstepped) {
      uint16_t end_copy_length = byte_buffer->end - byte_buffer->tail - 1;
      if (end_copy_length > 0) {
        memcpy(output_buffer, &buffer[byte_buffer->tail + 1], end_copy_length);
      }
      memcpy(output_buffer + end_copy_length, buffer,
             popped_length - end_copy_length);
    } else {
      memcpy(output_buffer, &buffer[byte_buffer->tail + 1], popped_length);
    }

    byte_buffer->tail = tail;
  }

  return popped_length;
}

uint16_t pifus_byte_buffer_is_empty(struct pifus_byte_buffer *byte_buffer) {
  return byte_buffer->head == byte_buffer->tail;
}