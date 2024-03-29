/* std includes */
#include "stdbool.h"
#include "stdio.h"
#include "time.h"

/* lwIP core includes */
#include "lwip/api.h"
#include "lwip/debug.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/opt.h"
#include "lwip/stats.h"
#include "lwip/sys.h"
#include "lwip/tcp.h"
#include "lwip/tcpip.h"
#include "lwip/timeouts.h"
#include "lwip/udp.h"

/* lwIP netif includes */
#include "lwip/etharp.h"
#include "netif/ethernet.h"

/* local includes */
#include "default_netif.h"
#include "init.h"

static void status_callback(struct netif *state_netif) {
  if (netif_is_up(state_netif)) {
    printf("status_callback==UP, local interface IP is %s\n",
           ip4addr_ntoa(netif_ip4_addr(state_netif)));
  } else {
    printf("status_callback==DOWN\n");
  }
}

static void link_callback(struct netif *state_netif) {
  if (netif_is_link_up(state_netif)) {
    printf("link_callback==UP\n");
  } else {
    printf("link_callback==DOWN\n");
  }
}

static void init_iface(ip4_addr_t *ipaddr, ip4_addr_t *netmask,
                       ip4_addr_t *gw) {
  printf("Starting lwIP, local interface IP is %s\n", ip4addr_ntoa(ipaddr));

  /* init randomizer again (seed per thread) */
  srand((unsigned int)time(NULL));

  init_default_netif(ipaddr, netmask, gw);

  netif_set_status_callback(netif_default, status_callback);
  netif_set_link_callback(netif_default, link_callback);

  netif_set_up(netif_default);
}

static void loop(void) {
  printf("lwIP: Starting main loop...\n");

  while (true) {
    sys_check_timeouts();

    netif_poll_all();
    default_netif_poll();

    if (loop_callback != NULL) {
      loop_callback();
    }
  }

  default_netif_shutdown();
}

/**
 * @brief Initializes lwIP and runs main loop. Does _not_ return.
 *
 * @param app_callback Callback that is invoked when stack initialization is
 * finished.
 */
void run_lwip(void (*_init_finished_callback)(void),
              void (*_loop_callback)(void), const char *ip_addr_str,
              const char *gateay_addr_str, const char *netmask_str) {
  init_finished_callback = _init_finished_callback;
  loop_callback = _loop_callback;

  setvbuf(stdout, NULL, _IONBF, 0);

  lwip_init();

  ip4_addr_t *ip_addr = (ip4_addr_t *)malloc(sizeof(ip4_addr_t));
  ip4_addr_t *gateway_addr = (ip4_addr_t *)malloc(sizeof(ip4_addr_t));
  ip4_addr_t *netmask = (ip4_addr_t *)malloc(sizeof(ip4_addr_t));

  ip4addr_aton(ip_addr_str, ip_addr);
  ip4addr_aton(gateay_addr_str, gateway_addr);
  ip4addr_aton(netmask_str, netmask);

  init_iface(ip_addr, netmask, gateway_addr);

  init_finished_callback();

  loop();
}