/*
 * NetworkInterface.c
 *
 *  Created on: 22 gen 2026
 *      Author: n.agostini
 */
#include "NetworkInterface.h"
#include "lwip/sockets.h"
#include "lwip/errno.h"
#include "lwip/netif.h"
#include "lwip/ip_addr.h"
#include <stdio.h>
#include "cmsis_os.h"   // per osDelay

extern struct netif gnetif;  // rete LwIP generata da CubeMX

void NetworkInit(Network* n)
{
    n->my_socket = -1;
    n->mqttread  = NetworkRead;
    n->mqttwrite = NetworkWrite;
}

// Funzione helper: controlla se la rete è pronta
static int Network_IsReady(void)
{
    return (netif_is_up(&gnetif) &&
            !ip4_addr_isany_val(*netif_ip4_addr(&gnetif)));
}

/*
int NetworkConnect(Network* n, const char* addr, int port)
{
    struct sockaddr_in serverAddr;

    if (!Network_IsReady()) {
        printf("Network not ready, retrying...\n");
        return -1;
    }

    if (Network_IsReady()) {
        printf("Network is ready -> IP: %s\n", ip4addr_ntoa(netif_ip4_addr(&gnetif)));
    } else {
        printf("Network not ready\n");
    }

    n->my_socket = lwip_socket(AF_INET, SOCK_STREAM, 0);
    if (n->my_socket < 0) {
        printf("Socket creation failed, errno=%d\n", errno);
        return -1;
    }

    // NB: IMPOSTA IL TIMEOUT (100ms) altrimenti MQTYeld rimane in attesa in ricezione del publish e non invia..
	// Se LWIP_SO_RCVTIMEO è 1 su lwipopts.h
	//#define LWIP_SO_RCVTIMEO                1
	//#define LWIP_SO_SNDTIMEO                1

	//int timeout = 100; // In millisecondi per LwIP
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 100000; // 100ms
	if (lwip_setsockopt(n->my_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
		printf("ERRORE: SO_RCVTIMEO non supportato! Controlla lwipopts.h\n");
	}

	// 1. IMPOSTA IL SOCKET COME NON-BLOCKING
	int flags = lwip_fcntl(n->my_socket, F_GETFL, 0);
	lwip_fcntl(n->my_socket, F_SETFL, flags | O_NONBLOCK);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(addr);

    int ret = lwip_connect(n->my_socket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    //int ret = -1;
    if (ret < 0) {
        if (errno == EINPROGRESS) {
            // Connessione in corso → OK, torniamo subito e proveremo più tardi
            printf("Connect in progress...\n");
            return 0;
        } else {
            printf("Connect failed, ret=%d errno=%d\n", ret, errno); //Se errore 104 significa che il broker rifiuta la connessione (mosquitto -> impostare allow_anonymus true)
            lwip_close(n->my_socket);
            n->my_socket = -1;
            osDelay(500); // piccola pausa prima di ritentare
            return -1;
        }
    }
    /*
    // Trasformo in modalità bloccante Da questo momento in poi, le funzioni di lettura/scrittura torneranno immediatamente
	int flags = lwip_fcntl(n->my_socket, F_GETFL, 0);
	if (flags < 0 || lwip_fcntl(n->my_socket, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		printf("Errore nell'impostazione O_NONBLOCK\n");
		// Opzionale: decidere se chiudere o proseguire. Di solito si prosegue.
	}

    printf("MQTT Broker connected immediately!\n");
    return 0;
}
*/

int NetworkConnect(Network *n, const char *addr, int port) {
    struct sockaddr_in serverAddr;
    int ret;

    if (!Network_IsReady()) {
        printf("Network not ready, retrying...\n");
        return -1;
    }

    printf("Network is ready -> IP: %s\n", ip4addr_ntoa(netif_ip4_addr(&gnetif)));

    // creazione Socket
    n->my_socket = lwip_socket(AF_INET, SOCK_STREAM, 0);
    if (n->my_socket < 0) {
        printf("Socket creation failed, errno=%d\n", errno);
        return -1;
    }

    // Socket  NON-BLOCKING per  connect
    int flags = lwip_fcntl(n->my_socket, F_GETFL, 0);
    lwip_fcntl(n->my_socket, F_SETFL, flags | O_NONBLOCK);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(addr);

    printf("Connecting to broker %s:%d...\n", addr, port);
    ret = lwip_connect(n->my_socket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

    if (ret < 0) {
        if (errno == EINPROGRESS) {
            // Usa select() per gestire il timeout della connessione
            fd_set fdset;
            struct timeval tv_conn;
            FD_ZERO(&fdset);
            FD_SET(n->my_socket, &fdset);

            tv_conn.tv_sec = 3;  // <--- Timeout di 3 secondi
            tv_conn.tv_usec = 0;

            // aspetto che socket scrivibile = connessione riuscita
            ret = lwip_select(n->my_socket + 1, NULL, &fdset, NULL, &tv_conn);

            if (ret > 0) {
                // se c'è errore nella conenssione socket
                int so_error;
                socklen_t len = sizeof(so_error);
                lwip_getsockopt(n->my_socket, SOL_SOCKET, SO_ERROR, &so_error, &len);
                if (so_error == 0) {
                    ret = 0; // Successo!
                } else {
                    printf("Connect error via getsockopt: %d\n", so_error);
                    ret = -1;
                }
            } else {
                printf("Connection timeout (Broker unreachable)\n");
                ret = -1;
            }
        } else {
            printf("Immediate connect error: %d\n", errno);
            ret = -1;
        }
    }

    // socket torna in modalità BLOCKING (Necessario per Paho MQTT standard)
    lwip_fcntl(n->my_socket, F_SETFL, flags);

    if (ret != 0) {
        lwip_close(n->my_socket);
        n->my_socket = -1;
        return -1;
    }

    // timeout RCV per MQTTYield (per evitare deadlock)
    struct timeval tv_rcv;
    tv_rcv.tv_sec = 0;
    tv_rcv.tv_usec = 200000; // 200ms
    if (lwip_setsockopt(n->my_socket, SOL_SOCKET, SO_RCVTIMEO, &tv_rcv, sizeof(tv_rcv)) < 0) {
        printf("Warning: Could not set SO_RCVTIMEO\n");
    }

    printf("MQTT Broker connected successfully!\n");
    return 0;
}

int NetworkDisconnect(Network* n)
{
    if (n->my_socket >= 0) {
        lwip_close(n->my_socket);
        n->my_socket = -1;
    }
    return 0;
}

int NetworkRead(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
    if (n->my_socket < 0) return -1;

    struct timeval tv;
    tv.tv_sec  = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    lwip_setsockopt(n->my_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    int ret = lwip_recv(n->my_socket, buffer, len, 0);
    if (ret < 0)
    {
    	int err = errno;
        if (errno == EWOULDBLOCK || errno == EAGAIN)
        {
        	return 0;
        }
        else
		{
        	printf("Socket Recv Error: %d\n", err);
			return -1;
		}
    }

    //printf("LwIP ha ricevuto %d bytes. Primo byte: 0x%02X\n", ret, buffer[0]);
    return ret;
}

int NetworkWrite(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
    if (n->my_socket < 0) return -1;

    struct timeval tv;
    tv.tv_sec  = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    lwip_setsockopt(n->my_socket, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    int ret = lwip_send(n->my_socket, buffer, len, 0);
    if (ret < 0) return -1;
    return ret;
}
