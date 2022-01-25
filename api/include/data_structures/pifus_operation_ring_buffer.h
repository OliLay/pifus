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
#ifndef PIFUS_OPERATION_RING_BUF_H
#define PIFUS_OPERATION_RING_BUF_H

#include <stdbool.h>

#include "data_structures/pifus_operation.h"

typedef struct pifus_operation ring_buf_elem;
typedef uint16_t buf_index_type;

struct pifus_operation_ring_buffer {
  buf_index_type end;  /*!< offset of the end of the ring buffer */
  buf_index_type head; /*!< offset to where next byte will be inserted */
  buf_index_type tail; /*!< offset of where next byte will be extracted */
};

void pifus_operation_ring_buffer_create(
    struct pifus_operation_ring_buffer *ring_buffer,
    buf_index_type buffer_length);

bool pifus_operation_ring_buffer_get(
    struct pifus_operation_ring_buffer *const ring_buffer, ring_buf_elem *buf,
    ring_buf_elem *pel);

bool pifus_operation_ring_buffer_put(
    struct pifus_operation_ring_buffer *const ring_buffer, ring_buf_elem *buf,
    ring_buf_elem const el);
#endif /* PIFUS_OPERATION_RING_BUF_H */
