#ifndef PIFUS_LOG_H
#define PIFUS_LOG_H

#ifndef PIFUS_UNUSED_ARG
#define PIFUS_UNUSED_ARG(x) (void)x
#endif /* PIFUS_UNUSED_ARG */

void pifus_log(const char *fmt, ...);

void pifus_debug_log(const char *fmt, ...);

#endif