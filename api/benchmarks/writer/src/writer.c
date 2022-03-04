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

void parse_args(int argc, char *argv[], char **reader_ip, uint16_t *port,
                enum pifus_priority *prio) {
  if (argc < 3) {
    printf("At least IP and port are required!\n");
    exit(1);
  }

  int opt;
  while ((opt = getopt(argc, argv, "i:p:l:")) != -1) {
    switch (opt) {
    case 'p':
      *port = atoi(optarg);
      break;
    case 'l':
      *prio = str_to_prio(optarg);
      break;
    default:
      fprintf(stderr, "Usage: %s [-pl] [IP]\n", argv[0]);
      exit(1);
    }
  }

  if (port == 0) {
    printf("Port is required!\n");
    exit(1);
  }

  *reader_ip = argv[optind];

  printf("Using reader IP '%s:%u' and prio '%s'\n", *reader_ip, *port,
         prio_str(*prio));
}

int main(int argc, char *argv[]) {
  printf("Starting pifus_writer...\n");

  char *reader_ip;
  uint16_t port;
  enum pifus_priority prio = PRIORITY_MEDIUM;
  parse_args(argc, argv, &reader_ip, &port, &prio);

  pifus_initialize(NULL);

  struct pifus_socket *socket = pifus_socket(PROTOCOL_TCP, prio);
  pifus_socket_bind(socket, PIFUS_IPV4_ADDR, 0);

  struct pifus_operation_result operation_result;
  pifus_socket_wait(socket, &operation_result);
  print_result(&operation_result);

  struct pifus_ip_addr remote_addr;
  ip_addr_from_string(reader_ip, &remote_addr);
  pifus_socket_connect(socket, remote_addr, port);
  pifus_socket_wait(socket, &operation_result);
  print_result(&operation_result);
  
  size_t total_sent = 0;
  while (true) {
    int sent = 0;
    while (sent < 10) {
      char *loop_data;
      if (!asprintf(&loop_data,
                    "Predictable interface for a user space IP stack!#%i",
                    sent)) {
        exit(1);
      }

      if (pifus_socket_write(socket, loop_data, strlen(loop_data))) {
        printf("Wrote '%s'\n", loop_data);
        sent++;
        total_sent++;
      }

      free(loop_data);
    }
    printf("Total sent: %lu\n", total_sent);

    while (pifus_socket_pop_result(socket, &operation_result)) {
      print_result(&operation_result);
    }
  }

  pifus_exit();

  return 0;
}