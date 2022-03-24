#define _GNU_SOURCE

/* standard includes */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* local includes */
#include "pifus.h"
#include "pifus_ip.h"
#include "pifus_socket.h"

void print_result(struct pifus_operation_result *result) {
  printf("Result returned: opcode %s, result code %u \n",
         operation_str(result->code), result->result_code);
}

volatile uint16_t max_accepted_socket;

struct socket_wrapper {
  struct pifus_socket *volatile socket;
  volatile int64_t enqueued;
  volatile int64_t dequeued;
};

struct socket_wrapper accepted_sockets[1024];

void callback_func(struct pifus_socket *socket,
                   enum pifus_operation_code op_code) {
  if (op_code == TCP_RECV) {
    struct pifus_operation_result operation_result;
    pifus_socket_pop_result(socket, &operation_result);

    accepted_sockets[socket->identifier.socket_index - 1].dequeued++;

    if (operation_result.result_code != PIFUS_OK) {
      printf("recv() returned != OK, shouldn't happen!\n");
      exit(1);
    }
    pifus_free(&operation_result);
  } else if (op_code == TCP_ACCEPT) {
    struct pifus_operation_result operation_result;
    pifus_socket_pop_result(socket, &operation_result);

    printf("Accepted new connection!\n");

    accepted_sockets[max_accepted_socket + 1].socket =
        operation_result.data.accept.socket;
    max_accepted_socket++;
  }
}

int main(int argc, char *argv[]) {
  printf("Starting pifus_baseline_reader...\n");

  pifus_initialize(&callback_func);

  struct pifus_socket *socket = pifus_socket(PROTOCOL_TCP, PRIORITY_HIGH);

  pifus_socket_bind(socket, PIFUS_IPV4_ADDR, 11337);

  struct pifus_operation_result operation_result;
  pifus_socket_wait(socket, &operation_result);
  print_result(&operation_result);

  pifus_socket_listen(socket, 10);
  pifus_socket_wait(socket, &operation_result);
  print_result(&operation_result);

  pifus_socket_accept(socket, PRIORITY_HIGH);

  while (true) {
    for (int i = 1; i <= max_accepted_socket; i++) {
      if (accepted_sockets[i].enqueued - accepted_sockets[i].dequeued < 20) {
        pifus_socket_recv(accepted_sockets[i].socket, 50);
        accepted_sockets[i].enqueued++;
      }
    }
  }

  pifus_exit();

  return 0;
}