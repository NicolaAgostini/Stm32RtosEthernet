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

//per Mbed TLS
int TLS_NetworkRead(Network* n, unsigned char* buffer, int len, int timeout_ms);
int TLS_NetworkWrite(Network* n, unsigned char* buffer, int len, int timeout_ms);
int TLS_NetworkConnect(Network* n, const char* addr, int port);

void my_debug(void *ctx, int level, const char *file, int line, const char *str) ;
void TLS_NetworkDisconnect(Network* n);


#endif
