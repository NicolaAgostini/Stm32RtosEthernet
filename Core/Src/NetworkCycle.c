/*
 * NetworkCycle.h
 *
 *  Created on: 23 gen 2026
 *      Author: n.agostini
 */

#include "lwip/netif.h"      // struttura netif, netif_is_up()
#include "lwip/ip_addr.h"     // ip4_addr_t e funzioni ip4addr_ntoa()
#include "lwip/ip4_addr.h"    // macro netif_ip4_addr()
#include "ethernetif.h"       // per gnetif

extern struct netif gnetif;

void Network_Cycle(void)
{

    sys_check_timeouts();

}
