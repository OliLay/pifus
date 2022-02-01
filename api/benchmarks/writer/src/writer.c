#define _GNU_SOURCE

/* standard includes */
#include <stdio.h>
#include <unistd.h>
#include <string.h>

/* local includes */
#include "pifus.h"
#include "pifus_socket.h"
#include "pifus_ip.h"
#include "writer.h"

void print_result(struct pifus_operation_result* result) {
  printf("Result returned: opcode %s, result code %u \n",
           operation_str(result->code), result->result_code);
}

int main(int argc, char *argv[]) {
  printf("Starting pifus_writer...\n");

  pifus_initialize();

  struct pifus_socket *socket = pifus_socket(PROTOCOL_TCP);
  struct pifus_socket *socket1 = pifus_socket(PROTOCOL_UDP);

  pifus_socket_bind(socket, PIFUS_IPV4_ADDR, 50113);

  /* TODO: writer logic */

  struct pifus_operation_result operation_result;
  pifus_socket_wait(socket, &operation_result);
  print_result(&operation_result);

  struct pifus_ip_addr remote_addr;
  ip_addr_from_string("192.168.1.201", &remote_addr);
  pifus_socket_connect(socket, remote_addr, 11337);
  pifus_socket_wait(socket, &operation_result);
  print_result(&operation_result);

  char *data = "Sepp ist ein großartiges Katzenwesen.\n";
  pifus_socket_write(socket, data, strlen(data));
  pifus_socket_wait(socket, &operation_result);
  print_result(&operation_result);

  sleep(1);
  
  int i = 0;
  while (true) {
    char *loop_data;
    asprintf(&loop_data, "%s%i%s", "Sepp ist ein großartiges Katzenwesen. #", i, "\n");
    
    pifus_socket_write(socket, loop_data, strlen(loop_data));
    pifus_socket_wait(socket, &operation_result);
    print_result(&operation_result);

   // sleep(1);
    i++;
  }

  /**
   *  TODO: think about callback mechanism, e.g.
   *    - call pifus_initialize(&callback_func)
   *    - callback_func takes a socket*, and a operation_result
   *    - "RX thread" runs an blocks with futex_waitv, and as soon as a result
   *is available, callback is called
   *    - NOTE: for this, sockets have to be held internally, and shadow
   *variables of futexes
   *    - NOTE: wait (poll) mode should still be available!
   **/

  pifus_exit();

  return 0;
}