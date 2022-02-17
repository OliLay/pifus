#define _GNU_SOURCE

/* standard includes */
#include "pthread.h"
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* shmem includes */
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

/* local includes */
#include "pifus.h"
#include "pifus_constants.h"
#include "pifus_shmem.h"
#include "pifus_socket.h"
#include "utils/log.h"

pthread_t callback_thread;
pifus_callback callback;
extern futex_t socket_futexes[MAX_SOCKETS_PER_APP];
static struct futex_waitv waitvs[MAX_SOCKETS_PER_APP];
/** futex nr to socket_index **/
socket_index_t socket_from_futex_nr[MAX_SOCKETS_PER_APP];

char *app_shm_name = NULL;
struct pifus_app *app_state = NULL;

int create_app_shm_region(void);
struct pifus_app *map_app_region(int fd);
void start_callback_thread(void);
void *callback_thread_loop(void *arg);
uint8_t fill_sockets_waitv(void);

/**
 * @brief Calls shmem_open with next available app id
 * @return fd for the shared memory
 */
int create_app_shm_region(void) {
  app_index_t app_number = 0;

  int fd;
  while (true) {
    if (asprintf(&app_shm_name, "%s%u", SHM_APP_NAME_PREFIX, app_number) < 0) {
      pifus_log("pifus: error when calling asprintf\n");
    }

    fd = shm_open_region(app_shm_name, true);

    if (fd >= 0) {
      return fd;
    } else {
      if (errno == EEXIST) {
        pifus_debug_log("pifus: shmem with name %s already exists, trying "
                        "next...\n",
                        app_shm_name);
        app_number++;
      } else {
        pifus_log("pifus: errno %i when calling shm_open\n", errno);
      }
    }
  }
}

/**
 * @brief Maps the app's shared memory into the process memory space.
 *
 * @param fd The fd of the app's shared memory.
 * @return Pointer to pifus_app struct.
 */
struct pifus_app *map_app_region(int fd) {
  return (struct pifus_app *)shm_map_region(fd, SHM_APP_SIZE, true);
}

void pifus_initialize(pifus_callback cb) {
  int app_shmem_fd = create_app_shm_region();
  app_state = map_app_region(app_shmem_fd);

  if (cb != NULL) {
    callback = cb;
    pifus_log("Using callback-mode, starting callback thread.\n");
    start_callback_thread();
  }

  pifus_log("pifus: Initialized!\n");
}

void pifus_exit(void) {
  pifus_socket_exit_all();
  shm_unlink_region(app_shm_name);
}

uint8_t fill_sockets_waitv(void) {
  uint8_t current_amount_futexes = 0;
  for (socket_index_t socket_index = 1;
       socket_index <= app_state->highest_socket_number; socket_index++) {
    struct pifus_socket *current_socket_ptr = sockets[socket_index];

    if (current_socket_ptr != NULL) {
      if (current_socket_ptr->cqueue_futex != socket_futexes[socket_index]) {
        callback(current_socket_ptr);
      }

      waitvs[current_amount_futexes].uaddr =
          (uintptr_t)&current_socket_ptr->cqueue_futex;
      waitvs[current_amount_futexes].flags = FUTEX_32;
      waitvs[current_amount_futexes].val = socket_futexes[socket_index];
      waitvs[current_amount_futexes].__reserved = 0;
      socket_from_futex_nr[current_amount_futexes] = socket_index;

      current_amount_futexes++;
    }
  }

  return current_amount_futexes;
}

void *callback_thread_loop(void *arg) {
  PIFUS_UNUSED_ARG(arg);
  while (true) {
    uint8_t amount_futexes = fill_sockets_waitv();
    if (amount_futexes > 0) {
      int ret_code = futex_waitv(waitvs, amount_futexes, 0, NULL, 0);

      if (ret_code >= 0) {
        struct pifus_socket* socket = sockets[socket_from_futex_nr[ret_code]];

        // TODO: need to increase shadow variables :)
        callback(socket);

        /* clean up previous mappings */
        memset(socket_from_futex_nr, 0, sizeof(socket_from_futex_nr));
      }
    }
  }
}

void start_callback_thread(void) {
  int ret = pthread_create(&callback_thread, NULL, callback_thread_loop, NULL);

  if (ret < 0) {
    pifus_log("pifus: Could not start callback thread due to %i!\n", errno);
  } else {
    pifus_debug_log("pifus: Started callback thread!\n");
  }
}