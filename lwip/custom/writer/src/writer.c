/* std incluces */
#include "stdio.h"

/* lwIP includes */
#include "lwip/opt.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"

/* local includes */
#include "init.h"

/* PCB for this connection */
static struct tcp_pcb *pcb;


void init_finished(void);
void init_finished(void)
{   
    printf("Sepp!\n");
    //exit(0);
}

int main(int argc, char *argv[])
{
    app_init_lwip(&init_finished);

    return 0;
}