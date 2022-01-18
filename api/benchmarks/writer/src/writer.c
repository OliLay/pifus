/* standard includes */
#include <stdio.h>
#include <unistd.h>

/* local includes */
#include "writer.h"
#include "pifus.h"
#include "pifus_socket.h"

int main(int argc, char *argv[])
{
    printf("Starting pifus_writer...\n");

    pifus_initialize();

    struct pifus_socket* socket = pifus_socket(PROTOCOL_UDP);
    struct pifus_socket* socket1 = pifus_socket(PROTOCOL_TCP);

    const struct pifus_operation op = {.op = UDP_CONNECT};
    enqueue_operation(socket, op);

    const struct pifus_operation op1 = {.op = TCP_ABORT};
    enqueue_operation(socket1, op1);

    /* TODO: writer logic */

    sleep(5);

    pifus_exit();

    return 0;
}