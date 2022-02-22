#define _GNU_SOURCE

/* standard includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* local includes */
#include "pifus.h"
#include "pifus_ip.h"
#include "pifus_socket.h"
#include "writer.h"

void print_result(struct pifus_operation_result *result) {
  printf("Result returned: opcode %s, result code %u \n",
         operation_str(result->code), result->result_code);
}

int main(int argc, char *argv[]) {
  printf("Starting pifus_writer...\n");

  pifus_initialize(NULL);

  struct pifus_socket *socket = pifus_socket(PROTOCOL_TCP, PRIORITY_MEDIUM);
  pifus_socket_bind(socket, PIFUS_IPV4_ADDR, 50113);

  struct pifus_operation_result operation_result;
  pifus_socket_wait(socket, &operation_result);
  print_result(&operation_result);

  struct pifus_ip_addr remote_addr;
  ip_addr_from_string("192.168.1.201", &remote_addr);
  pifus_socket_connect(socket, remote_addr, 11337);
  pifus_socket_wait(socket, &operation_result);
  print_result(&operation_result);
  int total_sent = 0;
  while (true) {
    int sent = 0;
    while (sent < 10) {
      char *loop_data;
      asprintf(&loop_data,
               "Predictable interface for a user space IP stack!#%i", sent);

      if (pifus_socket_write(socket, loop_data, strlen(loop_data))) {
        printf("Wrote '%s'\n", loop_data);
        sent++;
        total_sent++;
      }

      free(loop_data);
    }
    printf("Total sent: %i\n", total_sent);

    while (pifus_socket_pop_result(socket, &operation_result)) {
      print_result(&operation_result);
    }
  }

  pifus_exit();

  return 0;
}