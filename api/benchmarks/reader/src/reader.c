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
#include "reader.h"

void print_result(struct pifus_operation_result *result) {
  printf("Result returned: opcode %s, result code %u \n",
         operation_str(result->code), result->result_code);
}

int main(int argc, char *argv[]) {
  printf("Starting pifus_reader...\n");

  pifus_initialize();

  struct pifus_socket *socket = pifus_socket(PROTOCOL_TCP);

  pifus_socket_bind(socket, PIFUS_IPV4_ADDR, 11337);

  struct pifus_operation_result operation_result;
  pifus_socket_wait(socket, &operation_result);
  print_result(&operation_result);

  pifus_socket_listen(socket, 10);
  pifus_socket_wait(socket, &operation_result);
  print_result(&operation_result);

  pifus_socket_accept(socket);
  pifus_socket_wait(socket, &operation_result);
  print_result(&operation_result);

  struct pifus_socket *accepted_socket = operation_result.data.accept.socket;

  while (true) {
    if (pifus_socket_recv(accepted_socket, 15)) {
      pifus_socket_wait(accepted_socket, &operation_result);
      print_result(&operation_result);

      // TODO: make this more user friendly (retrieving data ptr)
      if (operation_result.result_code == PIFUS_OK) {
        struct pifus_memory_block *block =
            operation_result.data.recv.memory_block_ptr;
        char* data = shm_data_get_data_ptr(block);

        fwrite(data, sizeof(char), block->size, stdout);
        printf("\n");

        shm_data_free(app_state, block);
      }
    }
  }

  pifus_exit();

  return 0;
}