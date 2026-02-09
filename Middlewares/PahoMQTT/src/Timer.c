/*
 * Timer.c
 *
 *  Created on: 22 gen 2026
 *      Author: n.agostini
 */

#include "Timer.h"
#include "stm32f7xx_hal.h"  // per HAL_GetTick()

void TimerInit(Timer* t)
{
    t->end_time = 0;
}

char TimerIsExpired(Timer* t)
{
    uint32_t now = HAL_GetTick();
    return ((int32_t)(t->end_time - now) <= 0);
}

void TimerCountdownMS(Timer* t, unsigned int ms)
{
    t->end_time = HAL_GetTick() + ms;
}

int TimerLeftMS(Timer* t)
{
    int diff = t->end_time - HAL_GetTick();
    return (diff > 0) ? diff : 0;
}

//timer in millisecond
void TimerCountdown(Timer* t, unsigned int timeout)
{
    t->end_time = HAL_GetTick() + timeout;
}
