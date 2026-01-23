/*
 * cycle.h
 *
 *  Created on: 21 gen 2026
 *      Author: n.agostini
 */

#ifndef INC_CYCLE_H_
#define INC_CYCLE_H_

#include "stm32f7xx_hal.h" // necessario per HAL_GPIO_ReadPin / WritePin
#include <stdint.h>

/* funzione pubblica da chiamare nel task */
void Cycle_HandleInput(void);

#endif /* INC_CYCLE_H_ */
