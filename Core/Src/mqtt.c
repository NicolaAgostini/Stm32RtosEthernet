/*
 * mqtt.c
 *
 *  Created on: 21 gen 2026
 *      Author: n.agostini
 */
#include "MQTTClient.h"
#include "NetworkInterface.h"

Network network;
MQTTClient client;
unsigned char sendbuf[128], readbuf[128];

void MQTT_Cycle(void)
{
	NetworkInit(&network);

		while (NetworkConnect(&network, "192.168.250.124", 1883) != 0) {
			printf("Not connected!\r\n");
			osDelay(1000); // retry
		}


		MQTTClientInit(&client, &network, 1000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));

		MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
		connectData.MQTTVersion = 3;
		connectData.clientID.cstring = "STM32F756ZG";

		if (MQTTConnect(&client, &connectData) != 0) {
			printf("MQTT Connect failed\n");
		}

		for (;;) { //infinite loop
			printf("TASK MQTT in run\n");
			MQTTYield(&client, 100);
			osDelay(1000);
		}
		osDelay(1000); // 1 sec
}
