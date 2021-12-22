#ifndef LWIP_INIT_H
#define LWIP_INIT_H

void (*init_finished_callback)(void);

void app_init_lwip(void (*app_callback)(void));

#endif
