#ifndef PIFUS_IP_H
#define PIFUS_IP_H

#include "stdbool.h"
#include "stdint.h"

/**
 * Similar struct as lwIP lwip_ip_addr_type defined in ip_addr.h
 */
enum pifus_ip_type {
  /** IPv4 */
  PIFUS_IPV4_ADDR = 0U,
  /** IPv6 */
  PIFUS_IPV6_ADDR = 6U,
  /** IPv4+IPv6 ("dual-stack") */
  PIFUS_IPVX_ADDR = 46U
};


/**
 * IP addr in string form.
 */
struct pifus_ip_addr {
  char value[45];
};

void ip_addr_from_string(char *str, struct pifus_ip_addr* pifus_ip_addr);

#endif