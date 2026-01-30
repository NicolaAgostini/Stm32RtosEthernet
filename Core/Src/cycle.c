/*
 * cycle.c
 *
 *  Created on: 21 gen 2026
 *      Author: n.agostini
 */

#include "cycle.h"
#include <stdbool.h>
#include <stdio.h>




static bool echoBtnPC9 = false;
static bool BtnPC9_ActState = false;
static bool LedPC8_actState = false;

static uint32_t startTime, delta, delta_green, startTime_green;
static uint32_t filterTimeBtn = 100;

void Cycle_HandleInput(void)
{

    BtnPC9_ActState = (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_9) == GPIO_PIN_RESET);
	delta = HAL_GetTick() - startTime;
	if (!echoBtnPC9 && BtnPC9_ActState && delta >= filterTimeBtn) //fronte salita pulsante premuto con pull down a LOW e controllo filtro per evitare rimbalzi
	{
		if (LedPC8_actState) //se led acceso allora spegnilo
		{
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET); //comanda uscita led
			osDelay(10); //evita debouncing NB: usare osDelay() perchè sysTick è già usato dal kernel per scheduling
			LedPC8_actState = false;
			printf("Spengo!\r\n");

		}
		else//se led spento allora accendilo
		{
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET); //comanda uscita led
			osDelay(10); //evita debouncing
			LedPC8_actState = true;
			printf("Accendo!\r\n");
		}

		startTime = HAL_GetTick();
	}

    echoBtnPC9 = BtnPC9_ActState;

    //led green ok toggle
    delta_green = HAL_GetTick() - startTime_green;
    if (delta_green > 5000)
    {
    	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_10);
    	startTime_green = HAL_GetTick();
    }

}
