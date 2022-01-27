#include "pifus_ip.h"

#include <string.h>

void ip_addr_from_string(char *str, struct pifus_ip_addr* pifus_ip_addr) {
  strncpy(pifus_ip_addr->value, str, 45);
}