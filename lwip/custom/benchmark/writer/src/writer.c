#define _GNU_SOURCE

/* std incluces */
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

void writer_handle_error(const char *error_str) {
  printf("Error occurred: %s\n", error_str);
  exit(1);
}

void writer_send_now(void *arg) {
  LWIP_UNUSED_ARG(arg);
  err_t result;

  char *loop_data;
  asprintf(&loop_data, "Group42 ist fast coronafrei!#%i", current_number);
  result = tcp_write(pcb, loop_data, strlen(loop_data), 0);

  if (result == ERR_OK) {
    current_number++;
    if (current_number > 9) {
      current_number = 0;
    }
    printf("Sent data: '%s'\n", loop_data);
    result = tcp_output(pcb);

    // create one-shot timer again!
    sys_timeout(10, writer_send_now, NULL);

    if (result != ERR_OK) {
      writer_handle_error("writer_send_now: tcp_output result");
    }
  } else {
    // writer_handle_error("writer_send_now: tcp_write result");

    // create one-shot timer again!
    sys_timeout(10, writer_send_now, NULL);
  }
}

err_t writer_connected_callback(void *arg, struct tcp_pcb *tpcb, err_t err) {
  LWIP_UNUSED_ARG(arg);
  LWIP_UNUSED_ARG(tpcb);
  LWIP_UNUSED_ARG(err);

  printf("Connected!\n");

  sys_timeout(10, writer_send_now, NULL);

  return ERR_OK;
}

void writer_init(void) {
  printf("Init writer...\n");

  pcb = tcp_new_ip_type(IPADDR_TYPE_V4);

  if (pcb == NULL) {
    writer_handle_error("writer_init: pcb == NULL");
  } else {
    err_t result;
    result = tcp_bind(pcb, IPADDR_TYPE_V4, 50113);

    if (result == ERR_OK) {
      result = tcp_connect(pcb, reader_addr, 11337, &writer_connected_callback);

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

  reader_addr = (ip_addr_t *)malloc(sizeof(ip_addr_t));
  ipaddr_aton("192.168.1.201", reader_addr);

  run_lwip(&writer_init, NULL, "192.168.1.200", "192.168.1.1", "255.255.255.0");

  return 0;
}