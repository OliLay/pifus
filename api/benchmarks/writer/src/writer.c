/* standard includes */
#include <stdio.h>
#include <unistd.h>

/* local includes */
#include "pifus.h"
#include "pifus_socket.h"
#include "writer.h"

int main(int argc, char *argv[]) {
  printf("Starting pifus_writer...\n");

  pifus_initialize();

  struct pifus_socket *socket = pifus_socket(PROTOCOL_TCP);
  struct pifus_socket *socket1 = pifus_socket(PROTOCOL_UDP);

  pifus_socket_bind(socket, PIFUS_IPV4_ADDR, 13337);

  /* TODO: writer logic */

  struct pifus_operation_result operation_result;
  while (true) {
    /**
     *  TODO: think about callback mechanism, e.g.
     *    - call pifus_initialize(&callback_func)
     *    - callback_func takes a socket*, and a operation_result
     *    - "RX thread" runs an blocks with futex_waitv, and as soon as a result is available, callback is called
     *    - NOTE: for this, sockets have to be held internally, and shadow variables of futexes
     *    - NOTE: wait (poll) mode should still be available!
     **/
  
    pifus_socket_wait(socket, &operation_result);

    printf("Result returned: opcode %s, result code %u \n",
           operation_str(operation_result.code), operation_result.result_code);
  }

  pifus_exit();

  return 0;
}