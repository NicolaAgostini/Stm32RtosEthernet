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

extern struct netif gnetif;

Network network;
MQTTClient client;
unsigned char sendbuf[128], readbuf[128];

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
}

void sendWatchdog(uint32_t counter)
{
	printf("COUNTER: %lu\r\n", counter);
	// Prepara il messaggio
	MQTTMessage pubMessage;
	char payloadBuffer[64];
	snprintf(payloadBuffer, sizeof(payloadBuffer), "WatchDog da STM32! Conteggio: %lu", counter);

	pubMessage.qos = QOS0;          // Qualità del servizio (0, 1 o 2)
	pubMessage.retained = 0;        // Il broker non deve conservare il messaggio
	pubMessage.payload = payloadBuffer;   // Il contenuto
	pubMessage.payloadlen = strlen(payloadBuffer);

	// Pubblica sul topic "stm32/dati"
	if (MQTTPublish(&client, "machine/watchDog", &pubMessage) != MQTT_SUCCESS) {
		printf("Errore durante il Publish\n");
	} else {
		printf("Messaggio inviato: %s\n", payloadBuffer);
	}
}

void MQTT_Cycle(void)
{
	mqtt_state_t currentState = MQTT_STATE_INIT;
	uint32_t lastWatchdogTick = 0;
	uint32_t counter = 0;
	int rc;

	for (;;) {
		switch (currentState) {

			case MQTT_STATE_INIT:
				// Aspettiamo che l'interfaccia LwIP sia pronta
				if (netif_is_up(&gnetif) && netif_is_link_up(&gnetif)) {
					NetworkInit(&network);
					currentState = MQTT_STATE_CONNECT_NETWORK;
				} else {
					printf("In attesa del link Ethernet...\n");
					osDelay(1000);
				}
				break;

			case MQTT_STATE_CONNECT_NETWORK:
				printf("Connessione al socket TCP (%s:%d)...\n", MQTT_BROKER_IP, MQTT_BROKER_PORT);
				if (NetworkConnect(&network, MQTT_BROKER_IP, MQTT_BROKER_PORT) == 0) {
					// Inizializziamo il client solo dopo che il socket è ok
					MQTTClientInit(&client, &network, 1000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));
					currentState = MQTT_STATE_CONNECT_BROKER;
				} else {
					printf("Errore NetworkConnect. Riprovo...\n");
					osDelay(3000); // Aspetta prima di riprovare
				}
				break;

			case MQTT_STATE_CONNECT_BROKER:
				{
					MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
					connectData.MQTTVersion = 3;
					connectData.clientID.cstring = "STM32F756ZG";
					connectData.keepAliveInterval = 60; // Secondi

					printf("Invio pacchetto MQTT CONNECT...\n");
					if ((rc = MQTTConnect(&client, &connectData)) == 0) {
						printf("MQTT Connesso!\n");
						MQTTSubscribe(&client, "test/topic", QOS0, messageArrived);
						lastWatchdogTick = osKernelSysTick(); // Reset timer
						currentState = MQTT_STATE_RUNNING;
					} else {
						printf("MQTTConnect fallito (rc=%d). Riconnessione socket...\n", rc);
						lwip_close(network.my_socket); // Chiudi il socket vecchio
						currentState = MQTT_STATE_CONNECT_NETWORK;
						osDelay(2000);
					}
				}
				break;

			case MQTT_STATE_RUNNING:
				// Gestione messaggi in arrivo e Keep-Alive

				rc = MQTTYield(&client, 10);
				//printf("MQTT_STATE_RUNNING\n");
				if (rc != MQTT_SUCCESS) {
					printf("Connessione persa (Yield error)...\n");
					currentState = MQTT_STATE_ERROR;
					break;
				}

				// tempo del Watchdog (ogni 5 secondi)
				uint32_t currentTick = osKernelSysTick();
				if ((currentTick - lastWatchdogTick) >= 5000) {
					sendWatchdog(counter++);
					lastWatchdogTick = currentTick;

					if (counter > (1UL << 31)) counter = 0;
				}

				osDelay(100); // Piccolo rilascio CPU
				break;

			case MQTT_STATE_ERROR:
				// Pulizia in caso di errore
				printf("Reset connessione in corso...\n");
				lwip_close(network.my_socket);
				network.my_socket = -1;
				client.isconnected = 0;
				osDelay(2000);
				currentState = MQTT_STATE_INIT;
				break;
		}
	}
}
