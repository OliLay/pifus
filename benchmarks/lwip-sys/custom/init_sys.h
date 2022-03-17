#ifndef LWIP_INIT_H
#define LWIP_INIT_H

void (*init_finished_callback)(void);
void (*loop_callback)(void);

void run_lwip_sys(void (*app_callback)(void),
              const char *ip_addr_str, const char *gateay_addr_str,
              const char *netmask_str);

#endif
