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
	//printf("Task network 2!\r\n");
	//sys_check_timeouts();        // timer LwIP
	//printf("IP AAAAAAA: %s\n", ip4addr_ntoa(netif_ip4_addr(&gnetif)));
    sys_check_timeouts();

}
