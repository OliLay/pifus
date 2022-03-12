#define _GNU_SOURCE

/* standard includes */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

/* local includes */
#include "pifus.h"
#include "pifus_ip.h"
#include "pifus_socket.h"

char *high_tx_filename;
char *high_txed_filename;
char *medium_tx_filename;
char *medium_txed_filename;
char *low_tx_filename;
char *low_txed_filename;

volatile size_t total_txed;
volatile size_t total_tx;
size_t warmup_count = 10000;

volatile size_t high_txed;
volatile size_t medium_txed;
volatile size_t low_txed;
volatile size_t high_tx = 0;
volatile size_t medium_tx = 0;
volatile size_t low_tx = 0;

void parse_args(int argc, char *argv[]) {
  if (argc < 2) {
    printf("At least prio is required!\n");
    exit(1);
  }

  char *output_prefix = NULL;
  int opt;
  while ((opt = getopt(argc, argv, "o:")) != -1) {
    switch (opt) {
    case 'o':
      output_prefix = optarg;
      break;
    default:
      fprintf(stderr, "Usage: %s [-l, -o]\n", argv[0]);
      exit(1);
    }
  }

  if (output_prefix == NULL) {
    output_prefix = "dummy_multiple";
  }

  if (!asprintf(&high_tx_filename, "%s_HIGH_tx.txt", output_prefix)) {
    exit(1);
  }
  if (!asprintf(&high_txed_filename, "%s_HIGH_txed.txt", output_prefix)) {
    exit(1);
  }
  if (!asprintf(&medium_tx_filename, "%s_MEDIUM_tx.txt", output_prefix)) {
    exit(1);
  }
  if (!asprintf(&medium_txed_filename, "%s_MEDIUM_txed.txt", output_prefix)) {
    exit(1);
  }
  if (!asprintf(&low_tx_filename, "%s_LOW_tx.txt", output_prefix)) {
    exit(1);
  }
  if (!asprintf(&low_txed_filename, "%s_LOW_txed.txt", output_prefix)) {
    exit(1);
  }
}

void write_csv(char *filename, long value) {
  FILE *file = fopen(filename, "a");
  fprintf(file, "%li\n", value);
  fclose(file);
}

void callback_func(struct pifus_socket *socket,
                   enum pifus_operation_code op_code) {
  total_txed++;
  // if (total_txed > warmup_count) {
  struct timeval tp;
  gettimeofday(&tp, NULL);
  long int us = tp.tv_sec * 1000000 + tp.tv_usec;

  if (socket->priority == PRIORITY_HIGH) {
    write_csv(high_txed_filename, us);
    high_txed++;
  } else if (socket->priority == PRIORITY_MEDIUM) {
    write_csv(medium_txed_filename, us);
    medium_txed++;
  } else {
    write_csv(low_txed_filename, us);
    low_txed++;
  }
  // }

  struct pifus_operation_result result;
  pifus_socket_pop_result(socket, &result);
}

void enqueue(struct pifus_socket *socket) {
  struct timeval tp;
  gettimeofday(&tp, NULL);
  if (pifus_socket_nop(socket)) {
    total_tx++;

    // if (total_tx > warmup_count) {
    long int us = tp.tv_sec * 1000000 + tp.tv_usec;

    if (socket->priority == PRIORITY_HIGH) {
      write_csv(high_tx_filename, us);
      high_tx++;
    } else if (socket->priority == PRIORITY_MEDIUM) {
      write_csv(medium_tx_filename, us);
      medium_tx++;
    } else {
      write_csv(low_tx_filename, us);
      low_tx++;
    }
    // }
  }
}

int main(int argc, char *argv[]) {
  printf("Starting pifus_dummy...\n");

  parse_args(argc, argv);

  pifus_initialize(&callback_func);

  struct pifus_socket *high_socket = pifus_socket(PROTOCOL_TCP, PRIORITY_HIGH);
  struct pifus_socket *medium_socket =
      pifus_socket(PROTOCOL_TCP, PRIORITY_MEDIUM);
  struct pifus_socket *low_socket = pifus_socket(PROTOCOL_TCP, PRIORITY_LOW);

  while (true) {
    enqueue(high_socket);
    enqueue(medium_socket);
    enqueue(low_socket);
  }

  pifus_exit();
  return 0;
}