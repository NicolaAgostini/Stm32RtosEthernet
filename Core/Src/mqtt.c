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

void sendMessage(void)
{
	// Prepara il messaggio
	MQTTMessage pubMessage;
	char *payload = "Saluti da STM32!";

	pubMessage.qos = QOS0;          // Qualità del servizio (0, 1 o 2)
	pubMessage.retained = 0;        // Il broker non deve conservare il messaggio
	pubMessage.payload = payload;   // Il contenuto
	pubMessage.payloadlen = strlen(payload);

	// Pubblica sul topic "stm32/dati"
	if (MQTTPublish(&client, "machine/data", &pubMessage) != MQTT_SUCCESS) {
		printf("Errore durante il Publish\n");
	} else {
		printf("Messaggio inviato: %s\n", payload);
	}
}

void MQTT_Cycle(void)
{
    while (!netif_is_up(&gnetif)) {
        printf("Waiting for network up...\r\n");
        osDelay(500);
    }

	NetworkInit(&network);

	while (NetworkConnect(&network, MQTT_BROKER_IP, MQTT_BROKER_PORT) != 0) {
		printf("Not connected!\r\n");
		osDelay(1000); // retry
	}
	printf("NetworkConnect success!\r\n");

	MQTTClientInit(&client, &network, 1000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));

	MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
	connectData.MQTTVersion = 3;
	connectData.clientID.cstring = "STM32F756ZG";

	int rc_mqtt = MQTTConnect(&client, &connectData);
	    if (rc_mqtt != 0) {
	        printf("MQTTConnect failed! rc=%d\n", rc_mqtt);
	    } else {
	        printf("MQTT connected!\r\n");

	        // Subscribe
	        if (MQTTSubscribe(&client, "test/topic", QOS0, messageArrived) != MQTT_SUCCESS) {
	            printf("Subscribe failed\r\n");
	        } else {
	            printf("Subscribed to test/topic\r\n");
	        }
	    }

	for (;;) { //infinite loop
		//printf("TASK MQTT in run\n");
		MQTTYield(&client, 100);
		osDelay(100);
	}
	osDelay(1000); // 1 sec
}
