/*
 * mqtt.c
 *
 *  Created on: 21 gen 2026
 *      Author: n.agostini
 */
#include "MQTTClient.h"
#include "NetworkInterface.h"
#include "config.h"
#include "lwip/netif.h"
#include "cmsis_os.h"

#include "queue.h"

#include "lwip/apps/sntp.h"
#include "utils.h"
#include "main.h"
#include <string.h>
#include "lwip/dns.h"
#include <sys/time.h>
#include <time.h>

extern struct netif gnetif;
extern osMessageQId flashQueueHandle;
extern RTC_HandleTypeDef hrtc; // Recuperiamo l'handle dell'RTC definito nel main.c
extern int _gettimeofday(struct timeval *tv, void *tzvp);


Network network;
MQTTClient client;
__attribute__((aligned(4))) unsigned char sendbuf[2048];
__attribute__((aligned(4))) unsigned char readbuf[2048];

int intFromBroker = 0; //data from broker to test TLS connection MQTT

//osSemaphoreId mqtt_mutexHandle;

typedef enum {
    MQTT_STATE_INIT,
    MQTT_STATE_CONNECT_NETWORK,
    MQTT_STATE_CONNECT_BROKER,
    MQTT_STATE_RUNNING,
    MQTT_STATE_ERROR
} mqtt_state_t;




// callback per messaggio arrivato
void messageArrived(MessageData* data)
{

    printf("MQTT RX topic: %.*s\r\n",
           data->topicName->lenstring.len,
           data->topicName->lenstring.data);

    printf("MQTT RX payload: %.*s\r\n",
           data->message->payloadlen,
           (char*)data->message->payload);

    //qui imposto il valore nella variabile arrivata dal Broker
    char payload_string[16];
    int len = (data->message->payloadlen > 15) ? 15 : data->message->payloadlen;
    memcpy(payload_string, data->message->payload, len);
    payload_string[len] = '\0';
    //converto stringa che arriva dal payload del broker in intero
    intFromBroker = atoi(payload_string);

    printf("Messaggio ricevuto sul topic: %.*s -> Convertito in int: %d\n",
                data->message->payloadlen, (char*)data->message->payload, intFromBroker);

}

void sendWatchdog(uint32_t counter)
{
	if (client.isconnected == 0 || network.my_socket < 0)
	{
	    //printf("Salto Publish: client non connesso\n");
	    return;
	}
	// read timestamp
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN); // NB: va sempre letta dopo l'ora!

	printf("Hour: [%02d:%02d:%02d] Date:[%02d/%02d/%04d] COUNTER: %lu\r\n", sTime.Hours, sTime.Minutes, sTime.Seconds,
			sDate.Date, sDate.Month, sDate.Year+2000, counter);

	printf("COUNTER: %lu\r\n", counter);

	// Prepara il messaggio
	MQTTMessage pubMessage;
	static char payloadBuffer[128];
	snprintf(payloadBuffer, sizeof(payloadBuffer), "WatchDog da STM32!, Ora: %02d:%02d:%02d Data: %02d:%02d:%04d Conteggio: %lu, "
			"Data from broker %d",sTime.Hours, sTime.Minutes, sTime.Seconds, sDate.Date, sDate.Month, sDate.Year+2000, counter,
			intFromBroker);



	pubMessage.qos = QOS0;          // Qualità del servizio (0, 1 o 2)
	pubMessage.retained = 0;        // Il broker non deve conservare il messaggio
	pubMessage.payload = payloadBuffer;   // Il contenuto
	pubMessage.payloadlen = strlen(payloadBuffer);

	MQTTPublish(&client, "machine/watchDog", &pubMessage);
}



//STATE MACHINE PER GESTIONE MQTT

void MQTT_Cycle(void)
{
	mqtt_state_t currentState = MQTT_STATE_INIT;
	uint32_t lastWatchdogTick = 0;
	uint32_t counter = 0;
	int rc;
	int sntp_initialized = 0;

	for (;;) {
		switch (currentState) {

			case MQTT_STATE_INIT:
				// Aspettiamo che l'interfaccia LwIP sia pronta
				if (netif_is_up(&gnetif) && netif_is_link_up(&gnetif)) {

					NetworkInit(&network);

					// Configure Server DNS (Google)
					ip_addr_t dns_primary;
					IP4_ADDR(&dns_primary, 8, 8, 8, 8);
					dns_setserver(0, &dns_primary);

					// Configure Server DNS (Cloudflare)
					ip_addr_t dns_secondary;
					IP4_ADDR(&dns_secondary, 1, 1, 1, 1);
					dns_setserver(1, &dns_secondary);

					//start SNTP service
					if (!sntp_initialized)
					{
					    start_sntp_service();
					    sntp_initialized = 1;
					}


					currentState = MQTT_STATE_CONNECT_NETWORK;
				} else {
					printf("In attesa del link Ethernet...\n");
					printf("Stato Netif: UP=%d, LINK=%d\n", netif_is_up(&gnetif), netif_is_link_up(&gnetif));
					osDelay(1000);
				}
				break;

			case MQTT_STATE_CONNECT_NETWORK:
				printf("Connessione TLS (Porta 8883)...\n");
				// uso la funzione che ingloba TCP e TLS
				if (osSemaphoreWait(sntpSyncSemHandle, 30000))
				{
					printf("Semaforo rilasciato, ora impostata correttamente \n");
					if (TLS_NetworkConnect(&network, MQTT_BROKER_IP, 8883) == 0)
					{
						MQTTClientInit(&client, &network, 30000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));
						currentState = MQTT_STATE_CONNECT_BROKER;
					} else {
						printf("Errore TLS. Riprovo...\n");
						osDelay(3000);
					}
				}
				else
				{
					printf("MQTT Task: Timeout SNTP! Riprovo...\n");
					// Opzionale: riavvia sntp o torna a init
					currentState = MQTT_STATE_INIT;
				}

				/*
				if (TLS_NetworkConnect(&network, MQTT_BROKER_IP, 8883) == 0)
				{
					MQTTClientInit(&client, &network, 30000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));
					currentState = MQTT_STATE_CONNECT_BROKER;
				}
				else
				{
					printf("Errore TLS. Riprovo...\n");
					osDelay(3000);
				}
				*/

				break;

			case MQTT_STATE_CONNECT_BROKER:
				{
					//TEST PER LEGGERE ORA //
					struct timeval tv;
					_gettimeofday(&tv, NULL); // Chiamiamo la funzione

					// Convertiamo il timestamp in una stringa leggibile
					printf("--- VERIFICA ORA DI SISTEMA ---\n");
					printf("Timestamp Unix: %ld\n", (long)tv.tv_sec);
					printf("Data/Ora: %s", ctime(&tv.tv_sec));
					printf("-------------------------------\n");
					//TEST PER LEGGERE ORA //

					MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
					connectData.MQTTVersion = 3;
					connectData.clientID.cstring = "STM32F756ZG";
					connectData.keepAliveInterval = 60000; // millisecondi!!!
					//connectData.keepAliveInterval = 60;

					// credenziali per collegamento TLS
					connectData.username.cstring = "stm32RAM";
					connectData.password.cstring = "ram2026";

					printf("Invio pacchetto MQTT CONNECT su tunnel TLS cifrato...\n");
					//qui metto le subscribe!
					if ((rc = MQTTConnect(&client, &connectData)) == 0) {
						printf("MQTT Connesso (TLS + auth)!\n");
						//commento per test
						MQTTSubscribe(&client, "test/topic", QOS0, messageArrived);
						lastWatchdogTick = osKernelSysTick(); // Reset timer
						currentState = MQTT_STATE_RUNNING;
					} else {
						printf("MQTTConnect fallito (rc=%d). Riconnessione socket...\n", rc);
						//lwip_close(network.my_socket); // Chiudi il socket vecchio (solo TCP)
						TLS_NetworkDisconnect(&network); //con Mbed TLS
						currentState = MQTT_STATE_CONNECT_NETWORK;
						osDelay(2000);
					}
				}
				break;

			case MQTT_STATE_RUNNING:
				// Gestione messaggi in arrivo e Keep-Alive
				//osSemaphoreWait(mqtt_mutexHandle, osWaitForever);
				//printf("countdown: %d\r\n", (int)client.keepAliveInterval);
				rc = MQTTYield(&client, 10);
				//osSemaphoreRelease(mqtt_mutexHandle);
				//printf("MQTT_STATE_RUNNING\n");
				if (rc != MQTT_SUCCESS) {
					printf("Connessione persa (Yield error)...%d\n", rc);
					printf("Errore Yield: %d, Socket Status: %d\n", rc, network.my_socket);
					currentState = MQTT_STATE_ERROR;
					break;
				}

				// tempo del Watchdog (ogni 5 secondi)
				uint32_t currentTick = osKernelSysTick();
				if ((currentTick - lastWatchdogTick) >= 5000)
				{
					//commento per test
					sendWatchdog(counter++);

					xQueueOverwrite(flashQueueHandle, &counter); //per sovrascrivere se la coda è piena (se nessuno legge e svuota)

					lastWatchdogTick = currentTick;

					if (counter > (1UL << 31)) counter = 0;
				}

				osDelay(100); // Piccolo rilascio CPU
				break;

			case MQTT_STATE_ERROR:
				// Pulizia in caso di errore
				printf("Reset connessione in corso...\n");
				//osSemaphoreWait(mqtt_mutexHandle, osWaitForever);
				if (network.my_socket >= 0)
				{
					//lwip_close(network.my_socket); //con TCP solamente
					TLS_NetworkDisconnect(&network); //con Mbed TLS
					network.my_socket = -1;
				}
				client.isconnected = 0;
				//osSemaphoreRelease(mqtt_mutexHandle);
				osDelay(2000);
				currentState = MQTT_STATE_INIT;

				break;
		}
	}
}
