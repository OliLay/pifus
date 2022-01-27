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
 * Similar struct as lwIP ip4_addr defined in ip4_addr.h
 */
struct pifus_ip4_addr {
  uint32_t addr;
};

/**
 * Similar struct as lwIP ip6_addr defined in ip6_addr.h
 */
struct pifus_ip6_addr {
  uint32_t addr[4];
};

/**
 * Similar struct as lwIP ip_addr defined in ip_addr.h
 */
struct pifus_ip_addr {
  union {
    struct pifus_ip6_addr ip6;
    struct pifus_ip4_addr ip4;
  } u_addr;
  uint8_t type;
};

#endif