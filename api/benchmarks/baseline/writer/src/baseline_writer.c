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

struct socket_wrapper {
    struct pifus_socket *socket;
    char *tx_filename;
    char *txed_filename;

    volatile size_t total_sent;
    volatile size_t total_dequeued;
};

struct socket_wrapper wrappers[1024];
uint16_t number_of_sockets;
uint16_t port;
char *reader_ip;

size_t warmup_count = 10000;

void parse_args(int argc, char *argv[]) {
    if (argc < 4) {
        printf("At least IP, port and number of sockets are required!\n");
        exit(1);
    }

    char *output_prefix = NULL;
    int opt;
    while ((opt = getopt(argc, argv, "c:o:p:")) != -1) {
        switch (opt) {
            case 'p':
                port = atoi(optarg);
                break;
            case 'c':
                number_of_sockets = atoi(optarg);
                break;
            case 'o':
                output_prefix = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s [-c, -p, -o]\n", argv[0]);
                exit(1);
        }
    }

    if (output_prefix == NULL) {
        output_prefix = "baseline_writer";
    }

    if (number_of_sockets == 0) {
        printf("Number of sockets has to be specified!\n");
        exit(1);
    }

    if (port == 0) {
        printf("Port has to be specified!\n");
        exit(1);
    }

    reader_ip = argv[optind];

    for (int i = 0; i < number_of_sockets; i++) {
        if (!asprintf(&wrappers[i].tx_filename, "%s_%i_tx.txt", output_prefix,
                      i)) {
            exit(1);
        }
        if (!asprintf(&wrappers[i].txed_filename, "%s_%i_txed.txt",
                      output_prefix, i)) {
            exit(1);
        }
    }
}

void write_csv(char *filename, long value) {
    FILE *file = fopen(filename, "a");
    fprintf(file, "%li\n", value);
    fclose(file);
}

void callback_func(struct pifus_socket *socket,
                   enum pifus_operation_code op_code) {
    if (op_code == TCP_WRITE) {
        struct timeval tp;
        gettimeofday(&tp, NULL);
        long int us = tp.tv_sec * 1000000 + tp.tv_usec;

        struct pifus_operation_result result;
        pifus_socket_pop_result(socket, &result);

        if (result.result_code != PIFUS_OK) {
            printf("This should not happen, write did not return OK!\n");
            exit(1);
        }

        write_csv(wrappers[socket->identifier.socket_index - 1].txed_filename,
                  us);
        wrappers[socket->identifier.socket_index - 1].total_dequeued++;
    }
}

void* socket_thread(void *arg) {
    int i = *(int *)arg;
    struct pifus_socket *socket = pifus_socket(PROTOCOL_TCP, PRIORITY_HIGH);
    printf("Thread with %i has socket id %i\n", i, socket->identifier.socket_index);

    wrappers[i].socket = socket;

    pifus_socket_bind(socket, PIFUS_IPV4_ADDR, 0);

    struct pifus_operation_result operation_result;
    pifus_socket_wait(socket, &operation_result);

    struct pifus_ip_addr remote_addr;
    ip_addr_from_string(reader_ip, &remote_addr);
    pifus_socket_connect(socket, remote_addr, port);
    pifus_socket_wait(socket, &operation_result);

    struct timeval tp;
    while (true) {
        if (!(wrappers[i].total_dequeued + WRITE_QUEUE_SIZE - 1 <=
              wrappers[i].total_sent)) {
            // throttle, else benchmark gets falsified (as we're building a
            // too large queue)

            char *loop_data =
                "Predictable interface for a user space IP stack!#0";

            gettimeofday(&tp, NULL);
            if (pifus_socket_write(wrappers[i].socket, loop_data,
                                   strlen(loop_data))) {
                long int us = tp.tv_sec * 1000000 + tp.tv_usec;
                write_csv(wrappers[i].tx_filename, us);

                wrappers[i].total_sent++;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    printf("Starting pifus_baseline_writer...\n");

    parse_args(argc, argv);

    pifus_initialize(&callback_func);

    for (int i = 0; i < number_of_sockets; i++) {
        pthread_t thread;
        int *arg = (int *)malloc(sizeof(int));
        *arg = i;
        int err = pthread_create(&thread, NULL, &socket_thread, arg);

        usleep(50000);
    }

    printf("Looping!\n");

    while (true) {
    };

    pifus_exit();
    return 0;
}