/*
 * NetworkInterface.h
 *
 *  Created on: 22 gen 2026
 *      Author: n.agostini
 */

#ifndef NETWORK_INTERFACE_H
#define NETWORK_INTERFACE_H

#include <stdint.h>

typedef struct Network Network;  // forward declaration

typedef struct Network {
    int my_socket;
    int (*mqttread)(Network*, unsigned char*, int, int);   // funzione di lettura
    int (*mqttwrite)(Network*, unsigned char*, int, int);  // funzione di scrittura
} Network;

/* prototipi delle funzioni */
void NetworkInit(Network* n);
int NetworkConnect(Network* n, const char* addr, int port);
int NetworkDisconnect(Network* n);
int NetworkRead(Network* n, unsigned char* buffer, int len, int timeout_ms);
int NetworkWrite(Network* n, unsigned char* buffer, int len, int timeout_ms);

#endif
