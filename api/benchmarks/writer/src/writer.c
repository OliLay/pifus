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

    pifus_socket_bind(socket, PIFUS_IPADDR_TYPE_V4, 13337);

    /* TODO: writer logic */

    while (true) {
        
    }

    pifus_exit();

    return 0;
}