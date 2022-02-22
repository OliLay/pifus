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

int current_expected_number = 0;

void print_result(struct pifus_operation_result *result) {
  printf("Result returned: opcode %s, result code %u \n",
         operation_str(result->code), result->result_code);
}

void callback_func(struct pifus_socket *socket,
                   enum pifus_operation_code op_code) {
  printf("Callback %s received from sock %u\n", operation_str(op_code),
         socket->identifier.socket_index);

  if (op_code == TCP_RECV) {
    struct pifus_operation_result operation_result;
    pifus_socket_pop_result(socket, &operation_result);

    if (operation_result.result_code == PIFUS_OK) {
      struct pifus_memory_block *block =
          operation_result.data.recv.memory_block_ptr;
      char *data = shm_data_get_data_ptr(block);
      char number = data[block->size - 1] - '0';

      fwrite(data, sizeof(char), block->size, stdout);
      printf("\n");

      if (number != current_expected_number) {
        printf("ERROR: Expected %i but got %i instead!\n",
               current_expected_number, number);

        current_expected_number = number;
        exit(1);
      }

      shm_data_free(app_state, block);

      if (current_expected_number < 9) {
        current_expected_number++;
      } else {
        current_expected_number = 0;
      }

      pifus_socket_recv(socket, 50);
    }
  } else if (op_code == TCP_ACCEPT) {
    struct pifus_operation_result operation_result;
    pifus_socket_pop_result(socket, &operation_result);

    if (operation_result.result_code == PIFUS_OK) {
      struct pifus_socket *accepted_socket =
          operation_result.data.accept.socket;
      pifus_socket_recv(accepted_socket, 50);
    }
  }
}

int main(int argc, char *argv[]) {
  printf("Starting pifus_reader...\n");

  pifus_initialize(&callback_func);

  struct pifus_socket *to_be_closed_socket =
      pifus_socket(PROTOCOL_TCP, PRIORITY_HIGH);
  struct pifus_socket *socket = pifus_socket(PROTOCOL_TCP, PRIORITY_HIGH);

  pifus_socket_bind(socket, PIFUS_IPV4_ADDR, 11337);

  struct pifus_operation_result operation_result;
  pifus_socket_wait(socket, &operation_result);
  print_result(&operation_result);

  pifus_socket_listen(socket, 10);
  pifus_socket_wait(socket, &operation_result);
  print_result(&operation_result);

  pifus_socket_close(to_be_closed_socket);
  pifus_socket_wait(to_be_closed_socket, &operation_result);

  pifus_socket_accept(socket, PRIORITY_HIGH);

  // rest is done via callback :)
  while (true) {
    sleep(10);
  }

  pifus_exit();

  return 0;
}