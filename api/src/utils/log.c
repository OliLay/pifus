#include "utils/log.h"

#ifndef PIFUS_UNUSED_ARG
#define PIFUS_UNUSED_ARG(x) (void)x
#endif /* PIFUS_UNUSED_ARG */

/* std */
#include <stdarg.h>
#include <stdio.h>

/* pifus */
#include "pifus_constants.h"

void pifus_log(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
}

void pifus_debug_log(const char *fmt, ...) {
#ifdef PIFUS_DEBUG
  pifus_log(fmt);
#else
  // to avoid compiler warning
  PIFUS_UNUSED_ARG(fmt);
#endif
}