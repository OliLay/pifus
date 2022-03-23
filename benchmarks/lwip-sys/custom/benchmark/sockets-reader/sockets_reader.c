#define _GNU_SOURCE

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "init_sys.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"

void start(void);
void thread_loop(void *arg);

void thread_loop(void *arg) {
  int sock = lwip_socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_len = sizeof(addr);
  addr.sin_family = AF_INET;
  addr.sin_port = PP_HTONS(11337);
  addr.sin_addr.s_addr = INADDR_ANY;

  if (lwip_bind(sock, (struct sockaddr *)&addr, sizeof(addr)) != ERR_OK) {
    perror("lwip_bind failed!\n");
  }
  printf("Bound.\n");

  if (lwip_listen(sock, 0) != ERR_OK) {
    perror("lwip_listen failed!\n");
  }
  printf("Listen.\n");

  int socket_fds[1024] = {};
  socket_fds[0] = sock;
  int highest_sock_index = 0;
  fd_set readset;
  struct sockaddr_in remote;

  while (1) {
    FD_ZERO(&readset);
    for (int i = 0; i <= highest_sock_index; i++) {
      FD_SET(socket_fds[i], &readset);
    }

    int result =
        lwip_select(highest_sock_index + 1, &readset, NULL, NULL, NULL);

    if (result > 0) {
      if (FD_ISSET(sock, &readset)) {
        int size = sizeof(remote);
        int new_sock = lwip_accept(sock, (struct sockaddr *)&remote, &size);
        printf("Accepted new connection!\n");
        highest_sock_index++;
        socket_fds[highest_sock_index] = new_sock;
      }

      for (int i = 1; i <= highest_sock_index; i++) {
        if (FD_ISSET(socket_fds[i], &readset)) {
          int *data = malloc(50);
          int read_bytes = lwip_recv(socket_fds[i], data, 50, 0);
          free(data);
        }
      }
    }
  }
}

void start(void) {
  sys_thread_new("socket_thread", &thread_loop, NULL, DEFAULT_THREAD_STACKSIZE,
                 DEFAULT_THREAD_PRIO);
}

int main(int argc, char *argv[]) {
  char *ip = "192.168.1.200";
  if (argc > 1) {
    ip = argv[1];
  }

  run_lwip_sys(&start, ip, "192.168.1.1", "255.255.255.0");

  return 0;
}
