/*
 * NetworkInterface.c
 *
 *  Created on: 22 gen 2026
 *      Author: n.agostini
 */
#include "stm32f7xx_hal.h"
#include "NetworkInterface.h"
#include "lwip/sockets.h"
#include "lwip/errno.h"
#include "lwip/netif.h"
#include "lwip/ip_addr.h"
#include <stdio.h>
#include "cmsis_os.h"   // per osDelay

//per Mbed tls
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/net_sockets.h"

extern struct netif gnetif;  // rete LwIP generata da CubeMX

extern RNG_HandleTypeDef hrng; // Dichiarata nel main.c

//varibili per Mbed TLS
static mbedtls_ssl_context ssl;
static mbedtls_ssl_config conf;
static mbedtls_entropy_context entropy;
static mbedtls_ctr_drbg_context ctr_drbg;
static int is_tls_initialized = 0; // Flag per evitare doppie init

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
/**
 * Funzione di callback richiesta da MbedTLS per raccogliere entropia
 * dall'hardware RNG della STM32.
 */
int stm32_rng_poll(void *data, unsigned char *output, size_t len, size_t *olen)
{
    uint32_t random_val;
    size_t i = 0;

    while (i < len) {
        // Genera un numero casuale a 32 bit
        if (HAL_RNG_GenerateRandomNumber(&hrng, &random_val) == HAL_OK) {
            // Copia i byte necessari (massimo 4 alla volta)
            size_t to_copy = (len - i > 4) ? 4 : (len - i);
            memcpy(output + i, &random_val, to_copy);
            i += to_copy;
        } else {
            // Se l'hardware fallisce, restituiamo un errore
            return -1;
        }
    }
    *olen = len;
    return 0;
}


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
    //tv_rcv.tv_sec = 0;
    //tv_rcv.tv_usec = 200000; // 200ms

    //test per TLS
    tv_rcv.tv_sec = 1;
    tv_rcv.tv_usec = 0;

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


//funzioni con Mbed TLS

int TLS_NetworkRead(Network *n, unsigned char *buffer, int len, int timeout_ms) {
    if (n->my_socket < 0) return -1;

    unsigned char spy_buf[50]; // Bastano i primi byte per capire il frame TLS
	int spy_ret = lwip_recv(n->my_socket, spy_buf, sizeof(spy_buf), MSG_PEEK | MSG_DONTWAIT);

	if (spy_ret > 0) {
		printf(">>> [RAW ENCRYPTED ON WIRE] (%d bytes): ", spy_ret);
		for (int i = 0; i < spy_ret; i++) {
			printf("%02X ", spy_buf[i]);
		}
		printf("\n");
	}

    // Imposta timeout
    int effective_timeout = (timeout_ms < 500) ? 500 : timeout_ms;
    struct timeval tv;
    tv.tv_sec = effective_timeout / 1000;
    tv.tv_usec = (effective_timeout % 1000) * 1000;
    lwip_setsockopt(n->my_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    // --- LOG DATI CIFRATI (OPZIONALE MA UTILE) ---
    // Se vuoi vedere cosa arriva dal socket prima di MbedTLS,
    // dovresti farlo dentro la callback 'mbedtls_net_recv',
    // ma per ora concentriamoci sull'uscita di ssl_read.

    int ret = mbedtls_ssl_read(&ssl, buffer, len);

    if (ret > 0) {
    	/*
        printf(">>> TLS DECRYPTED (%d bytes): ", ret);
        for (int i = 0; i < ret; i++) {
            printf("%02X ", buffer[i]);
        }
        printf("\n");
        */
        return ret;
    }

    if (ret == 0) {
        printf("!!! TLS Connection closed by peer\n");
        return -1;
    }

    if (ret == MBEDTLS_ERR_NET_RECV_FAILED || ret == MBEDTLS_ERR_SSL_TIMEOUT || ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
        return 0;
    }
    if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY)
    {
		printf("!!! Il Broker ha inviato Close Notify (chiusura pulita TLS)\n");
		return -1;
    }

	if (ret == 0 || ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY)
	{
		// disconnessione broker
		printf("!!! Connessione chiusa dal Broker (FIN ricevuto) Ret: %d\n",ret);
		return -1;
	}

    // --- QUI AVVIENE L'ERRORE ---
    printf("!!! Errore decrittazione/lettura: -0x%04X\n", -ret);

    // Vediamo se il socket LwIP ha qualcosa da dire
    int so_error;
    socklen_t err_len = sizeof(so_error);
    lwip_getsockopt(n->my_socket, SOL_SOCKET, SO_ERROR, &so_error, &err_len);
    printf("!!! LwIP errno: %d, Socket SO_ERROR: %d\n", errno, so_error);

    return -1;
}

int TLS_NetworkWrite(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
    if (n->my_socket < 0) return -1;

    struct timeval tv;
    tv.tv_sec  = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    lwip_setsockopt(n->my_socket, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    /* scrivo attraverso il motore SSL: cripta i dati e li invia al socket */
    int ret = mbedtls_ssl_write(&ssl, buffer, len);

    if (ret > 0) {
        return ret;
    }

    if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
        return 0;
    }

    printf("MbedTLS Write Error: -0x%04X\n", -ret);
    return -1;
}


int TLS_NetworkConnect(Network* n, const char* addr, int port)
{
    int ret;
    const char *pers = "stm32_mqtt";

    // connessione TCP standard (riutilizzo la funzione NetworkConnect !!!)
    if (NetworkConnect(n, addr, port) != 0) return -1;

    // setup MbedTLS
    if (!is_tls_initialized) {
        mbedtls_ssl_init(&ssl);
        mbedtls_ssl_config_init(&conf);
        mbedtls_ctr_drbg_init(&ctr_drbg);
        mbedtls_entropy_init(&entropy);
        is_tls_initialized = 1;

        /* --- AGGIUNTA FONDAMENTALE --- */
		// Registriamo la nostra funzione stm32_rng_poll come sorgente "Strong"
		ret = mbedtls_entropy_add_source(&entropy, stm32_rng_poll, NULL,
										 32, MBEDTLS_ENTROPY_SOURCE_STRONG);

        //debug verboso
        //mbedtls_ssl_conf_dbg(&conf, my_debug, stdout);
        //mbedtls_debug_set_threshold(4); // 4 è il massimo
    }

    if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                        (const unsigned char *)pers, strlen(pers))) != 0)
    {
		printf("Errore Seeding: -0x%04X\n", -ret);
		return -1;
    }

    if ((ret = mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_CLIENT,
                                           MBEDTLS_SSL_TRANSPORT_STREAM,
                                           MBEDTLS_SSL_PRESET_DEFAULT)) != 0) return -1;

    // MODALITÀ INSECURE
    mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_NONE);
    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);

    if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0) return -1;

    // link  del'ID del socket di LwIP al motore TLS
    mbedtls_ssl_set_bio(&ssl, &n->my_socket, mbedtls_net_send, mbedtls_net_recv, NULL);

    // handshake: la negoziazione delle chiavi AES
    printf("Inizio Handshake TLS...\n");
    while ((ret = mbedtls_ssl_handshake(&ssl)) != 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            printf("Fallimento Handshake: -0x%04X\n", -ret);
            return -1;
        }
    }

    // cambio i puntatori a funzione
    n->mqttread = TLS_NetworkRead;
    n->mqttwrite = TLS_NetworkWrite;

    printf("Tunnel TLS 1.2 Pronto!\n");
    return 0;
}

void TLS_NetworkDisconnect(Network* n)
{
    // chiudi la sessione TLS in modo pulito (invia il pacchetto close_notify)
    mbedtls_ssl_close_notify(&ssl);

    // libera le risorse interne (buffer di cifratura)
    mbedtls_ssl_free(&ssl);
    mbedtls_ssl_config_free(&conf);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);

    is_tls_initialized = 0; // resettato per la prossima connessione

    // chiusura del socket fisico (LwIP)
    NetworkDisconnect(n);
}


void my_debug(void *ctx, int level, const char *file, int line, const char *str)
{
    printf("%s", str);
}

