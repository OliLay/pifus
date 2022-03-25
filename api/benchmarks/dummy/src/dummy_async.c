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
#include "csv_writer.h"

volatile size_t total_txed;
size_t warmup_count = 10000;

void parse_args(int argc, char *argv[], enum pifus_priority *prio) {
    if (argc < 2) {
        printf("At least prio is required!\n");
        exit(1);
    }

    char* output_prefix = NULL;
    int opt;
    while ((opt = getopt(argc, argv, "l:o:")) != -1) {
        switch (opt) {
            case 'l':
                *prio = str_to_prio(optarg);
                break;
            case 'o':
                output_prefix = optarg;
                break;
            default:    
                fprintf(stderr, "Usage: %s [-l, -o]\n", argv[0]);
                exit(1);
        }
    }

    if (output_prefix == NULL) {
        output_prefix = "dummy";
    }

    if (!asprintf(&tx_filename, "%s_tx.txt", output_prefix)) {
        exit(1);
    }
    if (!asprintf(&txed_filename, "%s_txed.txt", output_prefix)) {
        exit(1);
    }
}

void callback_func(struct pifus_socket *socket,
                   enum pifus_operation_code op_code) {
    total_txed++;
    if (total_txed > warmup_count) {
        struct timeval tp;
        gettimeofday(&tp, NULL);
        long int us = tp.tv_sec * 1000000 + tp.tv_usec;
        txed_stamps[txed_index] = us;
        txed_index++;
    }

    struct pifus_operation_result result;
    pifus_socket_pop_result(socket, &result);
}

int main(int argc, char *argv[]) {
    csv_writer_init();
    printf("Starting pifus_dummy...\n");

    enum pifus_priority prio = PRIORITY_MEDIUM;
    parse_args(argc, argv, &prio);

    pifus_initialize(&callback_func);

    struct pifus_socket *socket = pifus_socket(PROTOCOL_TCP, prio);

    struct timeval tp;
    size_t total_tx = 0;

    while (true) {
        while (total_txed + 5 < total_tx) {}

        gettimeofday(&tp, NULL);
        if (pifus_socket_nop(socket)) {
            total_tx++;

            if (total_tx > warmup_count) {
                long int us = tp.tv_sec * 1000000 + tp.tv_usec;
                tx_stamps[tx_index] = us;
                tx_index++;
            }
        }
    }

    pifus_exit();
    return 0;
}