/*
 * saveFlashCycle.h
 *
 *  Created on: 9 feb 2026
 *      Author: n.agostini
 */

#ifndef INC_SAVEFLASHCYCLE_H_
#define INC_SAVEFLASHCYCLE_H_

#include "cmsis_os.h" //per osEvent
#include "stm32f7xx_hal.h" // necessario per HAL_GPIO_ReadPin / WritePin
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>



void StartButtonTask(osEvent*);
uint8_t Flash_Read(uint32_t*);


#endif /* INC_SAVEFLASHCYCLE_H_ */
