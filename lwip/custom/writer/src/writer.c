/* std incluces */
#include "stdio.h"

/* lwIP includes */
#include "lwip/opt.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"

/* local includes */
#include "init.h"
#include "ping.h"

/* PCB for this connection */
static struct tcp_pcb *pcb;


void init_finished(void);
void init_finished(void)
{   
    printf("Sepp!\n");

    ip_addr_t* ping_target = (ip_addr_t *)malloc(sizeof(ip_addr_t));
   /*ip_addr_set_zero (ping_target);*/

    /* TODO: make configurable */
     ipaddr_aton ("192.168.1.1", ping_target);
  /*ping_init(&netif_default->gw);*/
    ping_init(ping_target);
    //exit(0);
}

int main(int argc, char *argv[])
{
    app_init_lwip(&init_finished);

    return 0;
}