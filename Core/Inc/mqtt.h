/*
 * mqtt.h
 *
 *  Created on: 21 gen 2026
 *      Author: n.agostini
 */

#ifndef MQTT_H
#define MQTT_H

#include "MQTTClient.h"
#include "NetworkInterface.h"
extern osSemaphoreId mqtt_mutexHandle;

void MQTT_Cycle(void);
void messageArrived(MessageData* data);
void sendWatchdog(uint32_t counter);

#endif
