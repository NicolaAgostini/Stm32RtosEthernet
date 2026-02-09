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

extern struct netif gnetif;

Network network;
MQTTClient client;
__attribute__((aligned(4))) unsigned char sendbuf[1024];
__attribute__((aligned(4))) unsigned char readbuf[1024];

osSemaphoreId mqtt_mutexHandle;

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
	if (client.isconnected == 0 || network.my_socket < 0)
	{
	    //printf("Salto Publish: client non connesso\n");
	    return;
	}
	printf("COUNTER: %lu\r\n", counter);
	// Prepara il messaggio
	MQTTMessage pubMessage;
	static char payloadBuffer[128];
	snprintf(payloadBuffer, sizeof(payloadBuffer), "WatchDog da STM32! Conteggio: %lu", counter);

	pubMessage.qos = QOS0;          // Qualità del servizio (0, 1 o 2)
	pubMessage.retained = 0;        // Il broker non deve conservare il messaggio
	pubMessage.payload = payloadBuffer;   // Il contenuto
	pubMessage.payloadlen = strlen(payloadBuffer);

	/*
	if (osSemaphoreWait(mqtt_mutexHandle, 100) == osOK)
	{
		if (client.isconnected)
		{
			MQTTPublish(&client, "machine/watchDog", &pubMessage);
		}
		osSemaphoreRelease(mqtt_mutexHandle);
	}
	*/
	MQTTPublish(&client, "machine/watchDog", &pubMessage);
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
					MQTTClientInit(&client, &network, 30000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));
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
					connectData.keepAliveInterval = 60000; // millisecondi!!!

					printf("Invio pacchetto MQTT CONNECT...\n");
					//qui metto le subscribe!
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
				//osSemaphoreWait(mqtt_mutexHandle, osWaitForever);
				//printf("countdown: %d\r\n", (int)client.keepAliveInterval);
				rc = MQTTYield(&client, 10);
				//osSemaphoreRelease(mqtt_mutexHandle);
				//printf("MQTT_STATE_RUNNING\n");
				if (rc != MQTT_SUCCESS) {
					printf("Connessione persa (Yield error)...%d\n", rc);
					currentState = MQTT_STATE_ERROR;
					break;
				}

				// tempo del Watchdog (ogni 5 secondi)
				uint32_t currentTick = osKernelSysTick();
				if ((currentTick - lastWatchdogTick) >= 5000)
				{
					sendWatchdog(counter++);

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
					lwip_close(network.my_socket);
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
