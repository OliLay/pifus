#define _GNU_SOURCE

/* std incluces */
#include "stdbool.h"
#include "stdio.h"
#include "string.h"

/* lwIP includes */
#include "lwip/debug.h"
#include "lwip/ip_addr.h"
#include "lwip/mem.h"
#include "lwip/opt.h"
#include "lwip/stats.h"
#include "lwip/sys.h"
#include "lwip/tcp.h"
#include "lwip/timeouts.h"

/* local includes */
#include "init.h"
#include "writer.h"

/* PCB for this connection */
static struct tcp_pcb *pcb;
int current_number = 0;

char *tx_filename;
char *txed_filename;

size_t total_tx;
size_t total_txed;
size_t warmup_count = 10000;

bool connected = false;

void writer_handle_error(const char *error_str) {
    // printf("Error occurred: %s\n", error_str);
    // exit(1);
}

void write_csv(char *filename, long value) {
    FILE *file = fopen(filename, "a");
    fprintf(file, "%li\n", value);
    fclose(file);
}

u16_t leftover_len = 0;
err_t tcp_sent_callback(void *arg, struct tcp_pcb *tpcb, u16_t len) {
    len += leftover_len;
    while (len >= 50) {
        total_txed++;
        if (total_txed > warmup_count) {
            struct timeval tp;
            gettimeofday(&tp, NULL);
            long int us = tp.tv_sec * 1000000 + tp.tv_usec;
            write_csv(txed_filename, us);
        }

        len -= 50;
    }

    leftover_len = len;

    return ERR_OK;
}

void writer_loop_callback(void) {
    if (!connected) {
        return;
    }

    while (!(total_txed + 5 - 1 <= total_tx)) {
        // throttle, else benchmark gets falsified (as we're building a too
        // large queue)
        if (!writer_send_now()) {
            return;
        }
    }
}

bool writer_send_now(void) {
    err_t result;

    char *loop_data;
    if (!asprintf(&loop_data,
                  "Predictable interface for a user space IP stack!#%i",
                  current_number)) {
        exit(1);
    }
    struct timeval tp;
    gettimeofday(&tp, NULL);
    result = tcp_write(pcb, loop_data, strlen(loop_data), 0);

    if (result == ERR_OK) {
        total_tx++;
        current_number++;
        if (current_number > 9) {
            current_number = 0;
        }
        result = tcp_output(pcb);

        if (total_tx > warmup_count) {
            long int us = tp.tv_sec * 1000000 + tp.tv_usec;
            write_csv(tx_filename, us);
        }

        if (result != ERR_OK) {
            writer_handle_error("writer_send_now: tcp_output result");
        }
    } else {
        //  writer_handle_error("writer_send_now: tcp_write result");
        return false;
    }

    return true;
}

err_t writer_connected_callback(void *arg, struct tcp_pcb *tpcb, err_t err) {
    LWIP_UNUSED_ARG(arg);
    LWIP_UNUSED_ARG(tpcb);
    LWIP_UNUSED_ARG(err);

    printf("Connected!\n");

    connected = true;
    return ERR_OK;
}

void writer_init(void) {
    printf("Init writer...\n");

    pcb = tcp_new_ip_type(IPADDR_TYPE_V4);
    tcp_nagle_disable(pcb);

    if (pcb == NULL) {
        writer_handle_error("writer_init: pcb == NULL");
    } else {
        err_t result;
        result = tcp_bind(pcb, IPADDR_TYPE_V4, 50113);

        if (result == ERR_OK) {
            result = tcp_connect(pcb, reader_addr, 11337,
                                 &writer_connected_callback);
            tcp_sent(pcb, &tcp_sent_callback);

            if (result != ERR_OK) {
                writer_handle_error("writer_init: tcp_connect result");
            }
        } else {
            writer_handle_error("writer_init: tcp_bind result");
        }
    }
}

int main(int argc, char *argv[]) {
    LWIP_UNUSED_ARG(argc);
    LWIP_UNUSED_ARG(argv);

    char *output_prefix = "lwip";

    int opt;
    while ((opt = getopt(argc, argv, "o:")) != -1) {
        switch (opt) {
            case 'o':
                output_prefix = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s [-o]\n", argv[0]);
                exit(1);
        }
    }

    if (!asprintf(&tx_filename, "%s_tx.txt", output_prefix)) {
        exit(1);
    }
    if (!asprintf(&txed_filename, "%s_txed.txt", output_prefix)) {
        exit(1);
    }

    reader_addr = (ip_addr_t *)malloc(sizeof(ip_addr_t));
    ipaddr_aton("192.168.1.201", reader_addr);

    run_lwip(&writer_init, &writer_loop_callback, "192.168.1.200",
             "192.168.1.1", "255.255.255.0");

    return 0;
}