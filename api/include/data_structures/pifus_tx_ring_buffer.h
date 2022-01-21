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
#ifndef PIFUS_TX_QUEUE_H
#define PIFUS_TX_QUEUE_H

#include <stdbool.h>
#include "data_structures/pifus_operation.h"
#include "pifus_shmem.h"

/** Uniquely identifies a socket. */
struct pifus_socket_identifier {
    app_index_t app_index;
    socket_index_t socket_index;
};

/**
 * Internal representation of an operation.
 * Contains the operation and information about the socket.
 */
struct internal_pifus_operation {
    struct pifus_operation operation;
    struct pifus_socket_identifier socket_identifier;
};

struct pifus_tx_ring_buffer {
    buf_index_type end;  /*!< offset of the end of the ring buffer */
    buf_index_type head; /*!< offset to where next byte will be inserted */
    buf_index_type tail; /*!< offset of where next byte will be extracted */
};

void pifus_tx_ring_buffer_create(struct pifus_tx_ring_buffer* ring_buffer,
                           uint8_t buffer_length);

bool pifus_tx_ring_buffer_get(struct pifus_tx_ring_buffer* const ring_buffer,
                        struct internal_pifus_operation* buf, struct internal_pifus_operation* pel);

bool pifus_tx_ring_buffer_put(struct pifus_tx_ring_buffer* const ring_buffer,
                        struct internal_pifus_operation* buf, struct internal_pifus_operation const el);
#endif /* PIFUS_TX_QUEUE_H */
