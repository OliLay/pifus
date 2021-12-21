#include "stdio.h"

#include "lwip/opt.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"

static struct tcp_pcb *pcb;

int main(int argc, char *argv[])
{
    // TODO: may have to include defaultnetif.c in srces and initialize interface like in test.c
    // maybe do this in one file, that is the same for all my example applications!
    printf("Sepp!\n");

    return 0;
}