/**
 * @file
 * @brief Lock-free ring buffer
 */
/*****************************************************************************
 * Last updated on  2021-02-12
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
#ifndef PIFUS_RING_BUFFER
#define PIFUS_RING_BUFFER

#include <stdbool.h>
#include <stddef.h>

#include "pifus_operation.h"

#define RING_BUFFER_STRUCT(NAME)                                               \
  struct NAME {                                                                \
    uint16_t end;                                                              \
    uint16_t head;                                                             \
    uint16_t tail;                                                             \
  };

#define RING_BUFFER_CREATE(NAME)                                               \
  void NAME##_create(struct NAME *ring_buffer, uint16_t buffer_length);

#define RING_BUFFER_GET(NAME, ELEMENT_TYPE)                                    \
  bool NAME##_get(struct NAME *const ring_buffer, ELEMENT_TYPE *buf,           \
                  ELEMENT_TYPE *pel);

#define RING_BUFFER_PEEK(NAME, ELEMENT_TYPE)                                   \
  bool NAME##_peek(struct NAME *const ring_buffer, ELEMENT_TYPE *buf,          \
                   ELEMENT_TYPE **pel);

#define RING_BUFFER_PEEK_INDEX(NAME, ELEMENT_TYPE)                             \
  bool NAME##_peek_index(struct NAME *const ring_buffer, ELEMENT_TYPE *buf,    \
                         uint16_t index, ELEMENT_TYPE **pel);

#define RING_BUFFER_ERASE_FIRST(NAME)                                          \
  bool NAME##_erase_first(struct NAME *const ring_buffer);

#define RING_BUFFER_PUT(NAME, ELEMENT_TYPE)                                    \
  bool NAME##_put(struct NAME *const ring_buffer, ELEMENT_TYPE *buf,           \
                  ELEMENT_TYPE const pel);

#define RING_BUFFER_FIND(NAME, ELEMENT_TYPE)                                   \
  bool NAME##_find(struct NAME *const ring_buffer, ELEMENT_TYPE *buf,          \
                   ELEMENT_TYPE **pel, bool (*satisfies)(ELEMENT_TYPE *));

#define RING_BUFFER_IS_FULL(NAME)                                              \
  bool NAME##_is_full(struct NAME *const ring_buffer);

#define RING_BUFFER_HEADER_DEFS(NAME, ELEMENT_TYPE)                            \
  RING_BUFFER_STRUCT(NAME)                                                     \
  RING_BUFFER_CREATE(NAME)                                                     \
  RING_BUFFER_GET(NAME, ELEMENT_TYPE)                                          \
  RING_BUFFER_PEEK(NAME, ELEMENT_TYPE)                                         \
  RING_BUFFER_ERASE_FIRST(NAME)                                                \
  RING_BUFFER_PUT(NAME, ELEMENT_TYPE)                                          \
  RING_BUFFER_IS_FULL(NAME)                                                    \
  RING_BUFFER_FIND(NAME, ELEMENT_TYPE)                                         \
  RING_BUFFER_PEEK_INDEX(NAME, ELEMENT_TYPE)

RING_BUFFER_HEADER_DEFS(pifus_operation_ring_buffer, struct pifus_operation)
RING_BUFFER_HEADER_DEFS(pifus_operation_result_ring_buffer,
                        struct pifus_operation_result)
RING_BUFFER_HEADER_DEFS(pifus_tx_ring_buffer, struct pifus_internal_operation)
RING_BUFFER_HEADER_DEFS(pifus_write_queue, struct pifus_write_queue_entry)
RING_BUFFER_HEADER_DEFS(pifus_recv_queue, struct pifus_recv_queue_entry)

#endif /* PIFUS_RING_BUFFER */
