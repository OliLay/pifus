/* std includes */
#include "stdio.h"
#include "stdbool.h"
#include "time.h"

/* lwIP core includes */
#include "lwip/opt.h"

#include "lwip/sys.h"
#include "lwip/timeouts.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/init.h"
#include "lwip/tcpip.h"
#include "lwip/netif.h"
#include "lwip/api.h"

#include "lwip/tcp.h"
#include "lwip/udp.h"

/* lwIP netif includes */
#include "lwip/etharp.h"
#include "netif/ethernet.h"

/* local includes */
#include "init.h"
#include "default_netif.h"

static void
status_callback(struct netif *state_netif)
{
    if (netif_is_up(state_netif))
    {
        printf("status_callback==UP, local interface IP is %s\n", ip4addr_ntoa(netif_ip4_addr(state_netif)));
    }
    else
    {
        printf("status_callback==DOWN\n");
    }
}

static void
link_callback(struct netif *state_netif)
{
    if (netif_is_link_up(state_netif))
    {
        printf("link_callback==UP\n");
    }
    else
    {
        printf("link_callback==DOWN\n");
    }
}

static void init_iface(void)
{
    ip4_addr_t ipaddr, netmask, gw;
    ip4_addr_set_zero(&gw);
    ip4_addr_set_zero(&ipaddr);
    ip4_addr_set_zero(&netmask);

    // TODO: make configurable
    IP4_ADDR(&ipaddr, 192,168,1,200);
    IP4_ADDR(&gw, 192,168,1,1);
    IP4_ADDR(&netmask, 255,255,255,0);
    printf("Starting lwIP, local interface IP is %s\n", ip4addr_ntoa(&ipaddr));

    init_default_netif(&ipaddr, &netmask, &gw);

    netif_set_status_callback(netif_default, status_callback);
    netif_set_link_callback(netif_default, link_callback);

    netif_set_up(netif_default);
}

static void init_callback(void *arg)
{ /* remove compiler warning */
    sys_sem_t *init_sem;
    LWIP_ASSERT("arg != NULL", arg != NULL);
    init_sem = (sys_sem_t *)arg;

    /* init randomizer again (seed per thread) */
    srand((unsigned int)time(NULL));

    /* init network interfaces */
    init_iface();

    /* init is complete, notify app */
    init_finished_callback();

    sys_sem_signal(init_sem);
}

/* MAIN LOOP for driver update */
static void loop(void)
{
    printf("Starting loop...\n");

    while (true)
    {
        default_netif_poll();
    }

    default_netif_shutdown();
}

void run_lwip(void (*app_callback)(void))
{
    init_finished_callback = app_callback;

    setvbuf(stdout, NULL, _IONBF, 0);
    
    err_t err;
    sys_sem_t init_sem;

    err = sys_sem_new(&init_sem, 0);
    LWIP_ASSERT("failed to create init_sem", err == ERR_OK);
    LWIP_UNUSED_ARG(err);
    tcpip_init(init_callback, &init_sem);

    sys_sem_wait(&init_sem);
    sys_sem_free(&init_sem);

    loop();
}