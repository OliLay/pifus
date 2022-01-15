/* standard includes */
#include <stdio.h>

/* local includes */
#include "writer.h"
#include "pifus.h"
#include "pifus_socket.h"

int main(int argc, char *argv[])
{
    printf("Starting pifus_writer...\n");

    pifus_initialize();

    struct pifus_socket* socket = pifus_socket();
    struct pifus_socket* socket1 = pifus_socket();

    const struct pifus_operation op = {.opcode = 88};
    enqueue_operation(socket, op);

    const struct pifus_operation op1 = {.opcode = 101};
    enqueue_operation(socket1, op1);

    struct pifus_operation get_op;
    pifus_ring_buffer_pop(&socket->squeue, &get_op);
    struct pifus_operation get_op1;
    pifus_ring_buffer_pop(&socket1->squeue, &get_op1);

    printf("get from squeue0: %u\n", get_op.opcode);
    printf("get from squeue1: %u\n", get_op1.opcode);

    /* TODO: writer logic */

    pifus_exit();

    return 0;
}