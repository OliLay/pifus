#define _GNU_SOURCE

/* std incluces */
#include "stdio.h"
#include "string.h"
#include "errno.h"

/* lwIP includes */
#include "lwip/debug.h"
#include "lwip/ip_addr.h"
#include "lwip/mem.h"
#include "lwip/opt.h"
#include "lwip/stats.h"
#include "lwip/sys.h"
#include "lwip/tcp.h"
#include "lwip/timeouts.h"

/* pifus */
#include "pifus_shmem.h"
#include "utils/futex.h"

/* local includes */
#include "init.h"
#include "stack.h"
#include "tx.h"

/**
 * app_ptrs[#app] -> ptr to shmem app region
 */
struct pifus_app *app_ptrs[MAX_APP_AMOUNT];
/**
 * app_local_highest_socket_number[#app] -> highest active socket number
 */
socket_index_t app_local_highest_socket_number[MAX_APP_AMOUNT];
/**
 * next app number to be assigned
 */
app_index_t next_app_number = 0;
/**
 * socket_ptrs[#app][#socket] -> ptr to shmem socket region of #app and corresponding #socket of that app
 */
struct pifus_socket *socket_ptrs[MAX_APP_AMOUNT][MAX_SOCKETS_PER_APP];

/**
 * @brief Maps new sockets (if any) for the app with the given index. 
 * 
 * @param app_index The index of the app to check.
 */
void map_new_sockets(app_index_t app_index)
{
    socket_index_t current_highest_index = app_local_highest_socket_number[app_index];
    socket_index_t app_highest_index = app_ptrs[app_index]->highest_socket_number;

    if (current_highest_index < app_highest_index)
    {
        char *shm_name;
        for (socket_index_t i = current_highest_index + 1; i <= app_highest_index; i++)
        {
            asprintf(&shm_name, "%s%u%s%u", SHM_APP_NAME_PREFIX, app_index, SHM_SOCKET_NAME_PREFIX, i);

            int fd = shm_open_region(shm_name, false);

            if (fd < 0)
            {
                printf("pifus: failed to map '%s' with errno %i\n", shm_name, errno);
            }

            socket_ptrs[app_index][i] = (struct pifus_socket *)shm_map_region(fd, SHM_SOCKET_SIZE, false);

            printf("pifus: Mapped socket %u for app %u\n", i, app_index);
        }
    }
}

/**
 * @brief Searches for new app shmem regions (starting at next_app_number) and maps them.
 */
void scan_for_app_regions(void)
{
    char *app_shm_name = NULL;

    for (app_index_t i = next_app_number; i < MAX_APP_AMOUNT; i++)
    {
        asprintf(&app_shm_name, "%s%u", SHM_APP_NAME_PREFIX, next_app_number);

        int fd = shm_open_region(app_shm_name, false);

        if (fd >= 0)
        {
            app_ptrs[next_app_number] = (struct pifus_app *)shm_map_region(fd, SHM_APP_SIZE, false);

            // do this once initially, rest is done by waking up via futexes
            map_new_sockets(next_app_number);

            next_app_number++;
        }
        else
        {
            return;
        }
    }
}

void lwip_loop_iteration(void)
{
    /** TODO:
     * - [DONE] functionality for socket add
     * - distribute RX packets
     * - TX handling (from TX thread via queue)
     */
}

void lwip_init_complete(void)
{
    printf("pifus: lwip init complete.\n");

    start_tx_thread();
    /** TODO:
     * - TX thread start
     **/
}

int main(int argc, char *argv[])
{
    LWIP_UNUSED_ARG(argc);
    LWIP_UNUSED_ARG(argv);

    printf("pifus: Starting up...\n");

    ip_addr_t *stack_addr = (ip_addr_t *)malloc(sizeof(ip_addr_t));
    ipaddr_aton("192.168.1.201", stack_addr);

    run_lwip(&lwip_init_complete, &lwip_loop_iteration, "192.168.1.200",
             "192.168.1.1", "255.255.255.0");

    return 0;
}