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

uint32_t next_app_number = 0;
struct pifus_app *app_ptrs[MAX_APP_AMOUNT];

/**
 * @brief Searches for new app shmem regions (starting at next_app_number) and maps them.
 */
void scan_for_app_regions(void)
{
    char *app_shm_name = NULL;

    for (uint32_t i = next_app_number; i < MAX_APP_AMOUNT; i++)
    {
        asprintf(&app_shm_name, "%s%u", SHM_APP_NAME_PREFIX, next_app_number);

        int fd = shm_open_region(app_shm_name, false);

        if (fd >= 0)
        {
            app_ptrs[next_app_number] = (struct pifus_app *)shm_map_region(fd, SHM_APP_SIZE, false);
            next_app_number++;

            printf("pfius: Found new region with shmem name %s\n", app_shm_name);
        }
        else
        {
            return;
        }
    }
}

void lwip_loop_iteration(void)
{
    scan_for_app_regions();

    if (app_ptrs[0] != NULL) {
        printf("app0 highest_socket_number %u\n", app_ptrs[0]->highest_socket_number);
    }

    /** TODO:
     * distribute RX packets
     * 
     * consume TX packets (from TX thread via queue)
     * 
     */
}

void lwip_init_complete(void)
{
    printf("pifus: lwip init complete.\n");

    scan_for_app_regions();

    /** TODO:
     * Shmem init, TX thread start
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