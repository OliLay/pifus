#define _GNU_SOURCE

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "init_sys.h"
#include "lwip/api.h"
#include "lwip/sys.h"
#include "lwip/tcp.h"

void parse_args(int argc, char *argv[]);
void write_csv(char *filename, long value);
void do_work(void);
void socket_thread(void *arg);
void start(void);

struct socket_wrapper {
    struct netconn *conn;
    char *tx_filename;
    char *txed_filename;
    volatile size_t total_dequeued;
};

struct socket_wrapper wrappers[1024];
uint16_t number_of_sockets;
uint16_t port;
char *reader_ip;

size_t warmup_count = 10000;

void callback(struct netconn *conn, enum netconn_evt evt, u16_t len);

void parse_args(int argc, char *argv[]) {
    if (argc < 4) {
        printf("At port and number of sockets are required!\n");
        exit(1);
    }

    const char *output_prefix = NULL;
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
        output_prefix = "baseline_lwip_writer";
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

void callback(struct netconn *conn, enum netconn_evt evt, u16_t len) {
    if (evt == NETCONN_EVT_SENDPLUS) {
        while (len >= 50) {
            struct timeval tp;
            gettimeofday(&tp, NULL);
            long int us = tp.tv_sec * 1000000 + tp.tv_usec;
            write_csv(wrappers[conn->socket_num].txed_filename, us);

            len -= 50;
            wrappers[conn->socket_num].total_dequeued++;
        }
    }
}

void socket_thread(void *arg) {
    int i = *(int *)arg;
    
    ip_addr_t *reader_addr = (ip_addr_t *)malloc(sizeof(ip_addr_t));
    ipaddr_aton("192.168.1.201", reader_addr);

    struct netconn *conn = netconn_new_with_callback(NETCONN_TCP, &callback);

    err_t err = netconn_bind(conn, NULL, 0);
    if (err != ERR_OK) {
        printf("Could not bind, err is %i, i is %i\n", err, i);
        exit(1);
    }

    err = netconn_connect(conn, reader_addr, port);
    while (err != ERR_OK) {
        err = netconn_connect(conn, reader_addr, port);
    }

    // added so we can easily map to the wrappers
    conn->socket_num = i;

    wrappers[i].conn = conn;
    tcp_nagle_disable(conn->pcb.tcp);

    struct timeval tp;
    size_t total_sent = 0;
    while (true) {
        const char *loop_data =
            "Predictable interface for a user space IP stack!#0";

        while (wrappers[i].total_dequeued + 5 - 1 <= total_sent) {
            // throttle, else benchmark gets falsified (as we're building a too
            // large queue)
        }

        gettimeofday(&tp, NULL);
        if (netconn_write(wrappers[i].conn, loop_data, strlen(loop_data), 0) ==
            ERR_OK) {
            long int us = tp.tv_sec * 1000000 + tp.tv_usec;
            write_csv(wrappers[i].tx_filename, us);
            total_sent++;
        }
    }
}

void start(void) {
    for (int i = 0; i < number_of_sockets; i++) {
        int *arg = (int *)malloc(sizeof(int));
        *arg = i;
        sys_thread_new("socket_thread", socket_thread, arg,
                       DEFAULT_THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);
    }
}

int main(int argc, char *argv[]) {
    parse_args(argc, argv);

    run_lwip_sys(&start, "192.168.1.200", "192.168.1.1", "255.255.255.0");

    return 0;
}
