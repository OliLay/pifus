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
#include "data_structures/pifus_tx_ring_buffer.h"

#include <stdbool.h>
#include <stdint.h>

/*..........................................................................*/
void pifus_tx_ring_buffer_create(struct pifus_tx_ring_buffer* ring_buffer,
                                 buf_index_type buffer_length) {
    ring_buffer->end = buffer_length;
    ring_buffer->head = 0U;
    ring_buffer->tail = 0U;
}
/*..........................................................................*/
bool pifus_tx_ring_buffer_put(struct pifus_tx_ring_buffer* const ring_buffer,
                              struct pifus_internal_operation* buf,
                              struct pifus_internal_operation const el) {
    buf_index_type head = ring_buffer->head + 1U;
    if (head == ring_buffer->end) {
        head = 0U;
    }
    if (head != ring_buffer->tail) { /* buffer NOT full? */
        buf[ring_buffer->head] = el;
        ring_buffer->head = head; /* update the head to a *valid* index */
        return true;              /* element placed in the buffer */
    } else {
        return false; /* element NOT placed in the buffer */
    }
}
/*..........................................................................*/
bool pifus_tx_ring_buffer_get(struct pifus_tx_ring_buffer* const ring_buffer,
                              struct pifus_internal_operation* buf,
                              struct pifus_internal_operation* pel) {
    buf_index_type tail = ring_buffer->tail;
    if (ring_buffer->head != tail) { /* ring buffer NOT empty? */
        *pel = buf[tail];
        ++tail;
        if (tail == ring_buffer->end) {
            tail = 0U;
        }
        ring_buffer->tail = tail; /* update the tail to a *valid* index */
        return true;
    } else {
        return false;
    }
}
