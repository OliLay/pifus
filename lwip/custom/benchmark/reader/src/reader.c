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
#include "reader.h"

/* PCB for this connection */
static struct tcp_pcb *pcb;

void reader_handle_error(const char *error_str) {
  printf("Error occurred: %s\n", error_str);
  exit(1);
}

err_t reader_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
  if (err == ERR_OK) {
    if (p == NULL) {
      printf("Remote closed connection!\n");
      return ERR_OK;
    } else {
      struct pbuf *next_pbuf = p;

      printf("Received packet: %iB total length, %iB pbuf length. Payload: \n",
             next_pbuf->tot_len, next_pbuf->len);

      while (next_pbuf != NULL) {
        char str[next_pbuf->len];
        memcpy(str, next_pbuf->payload, next_pbuf->len);

        fwrite(str, sizeof(char), next_pbuf->len, stdout);
        tcp_recved(tpcb, next_pbuf->len);

        if (next_pbuf->next == NULL) {
            printf("--- End of pbuf chain!\n");
        }

        next_pbuf = next_pbuf->next;
      }
    }
  } else {
    reader_handle_error("reader_recv: err param");
  }

  if (p != NULL) {
    pbuf_free(p);
  }

  return ERR_OK;
}

err_t reader_accept(void *arg, struct tcp_pcb *newpcb, err_t err) {
  if (err == ERR_OK) {
    printf("Accepted new connection!\n");

    pcb = newpcb;
    tcp_recv(pcb, &reader_recv);

    return ERR_OK;
  } else {
    reader_handle_error("reader_accept: err param");
  }
}

void reader_init(void) {
  printf("Init reader...\n");

  pcb = tcp_new_ip_type(IPADDR_TYPE_V4);

  if (pcb == NULL) {
    reader_handle_error("reader_init: pcb == NULL");
  } else {
    err_t result;
    result = tcp_bind(pcb, IPADDR_TYPE_V4, 11337);

    if (result == ERR_OK) {
      pcb = tcp_listen(pcb);

      if (pcb == NULL) {
        reader_handle_error("reader_init: tcp_connect pcb == NULL");
      } else {
        tcp_accept(pcb, &reader_accept);
      }
    } else {
      reader_handle_error("reader_init: tcp_bind result");
    }
  }
}

int main(int argc, char *argv[]) {
  LWIP_UNUSED_ARG(argc);
  LWIP_UNUSED_ARG(argv);

  run_lwip(&reader_init, NULL, "192.168.1.200", "192.168.1.1", "255.255.255.0");

  return 0;
}