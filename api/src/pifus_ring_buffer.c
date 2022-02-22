/**
 * @file
 * @brief Lock-free ring buffer
 */
/*****************************************************************************
 * Last updated on  2021-03-02
 *
 *                    Q u a n t u m  L e a P s
 *                    ------------------------
 *                    Modern Embedded Software
 *
 * Copyright (C) 2021 Quantum Leaps, LLC. All rights reserved.
 *
 * This software is licensed under the following open source MIT license:
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Contact information:
 * <www.state-machine.com>
 * <info@state-machine.com>
 */
#include "pifus_ring_buffer.h"

#include <stdbool.h>
#include <stdint.h>

/*..........................................................................*/
#define RING_BUFFER_FN_CREATE(NAME)                                            \
  void NAME##_create(struct NAME *ring_buffer, uint16_t buffer_length) {       \
    ring_buffer->end = buffer_length;                                          \
    ring_buffer->head = 0U;                                                    \
    ring_buffer->tail = 0U;                                                    \
  }

/*..........................................................................*/
#define RING_BUFFER_FN_PUT(NAME, ELEMENT_TYPE)                                 \
  bool NAME##_put(struct NAME *const ring_buffer, ELEMENT_TYPE *buf,           \
                  ELEMENT_TYPE const el) {                                     \
    uint16_t head = ring_buffer->head + 1U;                                    \
    if (head == ring_buffer->end) {                                            \
      head = 0U;                                                               \
    }                                                                          \
    if (head != ring_buffer->tail) { /* buffer NOT full? */                    \
      buf[ring_buffer->head] = el;                                             \
      ring_buffer->head = head; /* update the head to a *valid* index */       \
      return true;              /* element placed in the buffer */             \
    } else {                                                                   \
      return false; /* element NOT placed in the buffer */                     \
    }                                                                          \
  }
/*..........................................................................*/

#define RING_BUFFER_FN_GET(NAME, ELEMENT_TYPE)                                 \
  bool NAME##_get(struct NAME *const ring_buffer, ELEMENT_TYPE *buf,           \
                  ELEMENT_TYPE *pel) {                                         \
    uint16_t tail = ring_buffer->tail;                                         \
    if (ring_buffer->head != tail) { /* ring buffer NOT empty? */              \
      *pel = buf[tail];                                                        \
      ++tail;                                                                  \
      if (tail == ring_buffer->end) {                                          \
        tail = 0U;                                                             \
      }                                                                        \
      ring_buffer->tail = tail; /* update the tail to a *valid* index */       \
      return true;                                                             \
    } else {                                                                   \
      return false;                                                            \
    }                                                                          \
  }

#define RING_BUFFER_FN_PEEK(NAME, ELEMENT_TYPE)                                \
  bool NAME##_peek(struct NAME *const ring_buffer, ELEMENT_TYPE *buf,          \
                   ELEMENT_TYPE **pel) {                                       \
    uint16_t tail = ring_buffer->tail;                                         \
    if (ring_buffer->head != tail) { /* ring buffer NOT empty? */              \
      *pel = &buf[tail];                                                       \
      return true;                                                             \
    } else {                                                                   \
      return false;                                                            \
    }                                                                          \
  }

#define RING_BUFFER_FN_ERASE_FIRST(NAME)                                       \
  bool NAME##_erase_first(struct NAME *const ring_buffer) {                    \
    uint16_t tail = ring_buffer->tail;                                         \
    if (ring_buffer->head != tail) { /* ring buffer NOT empty? */              \
      ++tail;                                                                  \
      if (tail == ring_buffer->end) {                                          \
        tail = 0U;                                                             \
      }                                                                        \
      ring_buffer->tail = tail; /* update the tail to a *valid* index */       \
      return true;                                                             \
    } else {                                                                   \
      return false;                                                            \
    }                                                                          \
  }

#define RING_BUFFER_FN_ERASE_FIND(NAME, ELEMENT_TYPE)                          \
  bool NAME##_find(struct NAME *const ring_buffer, ELEMENT_TYPE *buf,          \
                   ELEMENT_TYPE **pel, bool (*satisfies)(ELEMENT_TYPE *)) {    \
    uint16_t tail = ring_buffer->tail;                                         \
    while (ring_buffer->head != tail) { /* ring buffer NOT empty? */           \
      if (satisfies(&buf[tail])) {                                             \
        *pel = &buf[tail];                                                     \
        return true;                                                           \
      } else {                                                                 \
        tail++;                                                                \
        if (tail == ring_buffer->end) {                                        \
          tail = 0U;                                                           \
        }                                                                      \
      }                                                                        \
    }                                                                          \
    return false;                                                              \
  }

#define RING_BUFFER_FN_IS_FULL(NAME)                                           \
  bool NAME##_is_full(struct NAME *const ring_buffer) {                        \
    uint16_t head = ring_buffer->head + 1U;                                    \
    if (head == ring_buffer->end) {                                            \
      head = 0U;                                                               \
    }                                                                          \
    return head == ring_buffer->tail;                                          \
  }
  
#define RING_BUFFER_FN_PEEK_INDEX(NAME, ELEMENT_TYPE)                          \
  bool NAME##_peek_index(struct NAME *const ring_buffer, ELEMENT_TYPE *buf,    \
                         uint16_t index, ELEMENT_TYPE **pel) {                 \
    uint16_t tail = ring_buffer->tail;                                         \
    uint16_t current_index = 0;                                                \
    while (ring_buffer->head != tail) { /* ring buffer NOT empty? */           \
      if (current_index == index) {                                            \
        *pel = &buf[tail];                                                     \
        return true;                                                           \
      } else {                                                                 \
        tail++;                                                                \
        current_index++;                                                       \
        if (tail == ring_buffer->end) {                                        \
          tail = 0U;                                                           \
        }                                                                      \
      }                                                                        \
    }                                                                          \
    return false;                                                              \
  }

#define RING_BUFFER_SOURCE_DEFS(NAME, ELEMENT_TYPE)                            \
  RING_BUFFER_FN_CREATE(NAME)                                                  \
  RING_BUFFER_FN_GET(NAME, ELEMENT_TYPE)                                       \
  RING_BUFFER_FN_PEEK(NAME, ELEMENT_TYPE)                                      \
  RING_BUFFER_FN_PUT(NAME, ELEMENT_TYPE)                                       \
  RING_BUFFER_FN_ERASE_FIRST(NAME)                                             \
  RING_BUFFER_FN_IS_FULL(NAME)                                                 \
  RING_BUFFER_FN_ERASE_FIND(NAME, ELEMENT_TYPE)                                \
  RING_BUFFER_FN_PEEK_INDEX(NAME, ELEMENT_TYPE)

RING_BUFFER_SOURCE_DEFS(pifus_operation_ring_buffer, struct pifus_operation)
RING_BUFFER_SOURCE_DEFS(pifus_operation_result_ring_buffer,
                        struct pifus_operation_result)
RING_BUFFER_SOURCE_DEFS(pifus_internal_operation_ring_buffer, struct pifus_internal_operation)
RING_BUFFER_SOURCE_DEFS(pifus_write_queue, struct pifus_write_queue_entry)
RING_BUFFER_SOURCE_DEFS(pifus_recv_queue, struct pifus_recv_queue_entry)