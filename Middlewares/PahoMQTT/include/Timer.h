/*
 * Timer.h
 *
 *  Created on: 22 gen 2026
 *      Author: n.agostini
 */

#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

/* struttura Timer usata da Paho MQTT */
typedef struct Timer {
    uint32_t end_time; // tick in ms a cui scade il timer
} Timer;

/* funzioni per inizializzare e gestire timer */
void TimerInit(Timer* t);
char TimerIsExpired(Timer* t);
void TimerCountdownMS(Timer* t, unsigned int ms);
int TimerLeftMS(Timer* t);
void TimerCountdown(Timer* t, unsigned int timeout);

#endif

