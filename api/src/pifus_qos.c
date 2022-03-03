#include "pifus_qos.h"
#include <string.h>

const char *prio_str(enum pifus_priority prio) {
  const char *str;
  if (prio == PRIORITY_HIGH) {
    str = "PRIORITY_HIGH";
  } else if (prio == PRIORITY_MEDIUM) {
    str = "PRIORITY_MEDIUM";
  } else if (prio == PRIORITY_LOW) {
    str = "PRIORITY_LOW";
  } else {
    str = "UNKNOWN PRIORITY";
  }
  return str;
}

enum pifus_priority str_to_prio(char *str) {
  if (strcmp("LOW", str) == 0) {
    return PRIORITY_LOW;
  } else if (strcmp("HIGH", str) == 0) {
    return PRIORITY_HIGH;
  } else {
    return PRIORITY_MEDIUM;
  }
}