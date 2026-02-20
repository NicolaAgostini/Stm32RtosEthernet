/*
 * saveFlashCycle.c
 *
 *  Created on: 9 feb 2026
 *      Author: n.agostini
 */
#include "saveFlashCycle.h"

#include "stm32f7xx_hal.h"
#include <stdio.h>

#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x080C0000) /* Base @ of Sector 7, 256 Kbytes */
#define FLASH_SECTOR_SIZE (256 * 1024) // 256KB per il settore 7

extern osMessageQId flashQueueHandle;


static bool BtnPC11_ActState = false;
static bool echoBtnPC11 = false;
static uint32_t startTime, delta;
static uint32_t filterTimeBtn = 100;
static uint32_t counterToSave = 0;



void StartButtonTask(osEvent* event)
{
	BtnPC11_ActState = (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_11) == GPIO_PIN_RESET);
	delta = HAL_GetTick() - startTime;
	if (!echoBtnPC11 && BtnPC11_ActState && delta >= filterTimeBtn) //fronte salita pulsante premuto con pull down a LOW e controllo filtro per evitare rimbalzi
	{
		// preleva l'ultimo valore disponibile dalla coda
		// timeout di 0 perché serve l'ultimo valore già presente
		*event = osMessageGet(flashQueueHandle, 0);

		if (event->status == osEventMessage)
		{
			counterToSave = event->value.v;

			printf("Pulsante! Salvo valore %lu in Flash...\n", counterToSave);

			//uint32_t counter = Flash_Read();
			//printf("Flash: Valore che viene sovrascritto: %lu ----\r\n", counter);

			// funzione di scrittura Flash
			vTaskSuspendAll(); // Ferma lo scheduler di FreeRTOS

			Flash_Write(counterToSave);

			//scrive la data e ora
			//recupera data e ora
			uint8_t hour;
			uint8_t min;
			uint8_t sec;
			uint8_t day;
			uint8_t month;
			uint8_t year;

			get_system_time_string(&hour, &min, &sec, &day, &month, &year);

			Flash_Write((uint32_t)year);
			Flash_Write((uint32_t)month);
			Flash_Write((uint32_t)day);

			Flash_Write((uint32_t)sec);
			Flash_Write((uint32_t)min);
			Flash_Write((uint32_t)hour);

			xTaskResumeAll();  // Riparte tutto

		}

		startTime = HAL_GetTick();
	}

	echoBtnPC11 = BtnPC11_ActState;
}


void Flash_Write(uint32_t dataToSave) {
    uint32_t address = ADDR_FLASH_SECTOR_7;
    uint32_t current_val;
    uint8_t found = 0;

    HAL_FLASH_Unlock();

    // cerca il primo spazio libero (4 byte alla volta della dimensione di uint32_t)
    // se si utilizza struct assicurarsi di saltare multipli di 4 byte (word 32 bit)
    for (uint32_t i = 0; i < FLASH_SECTOR_SIZE; i += 4) {
        address = ADDR_FLASH_SECTOR_7 + i;
        current_val = *(__IO uint32_t*)address;

        if (current_val == 0xFFFFFFFF) {
            found = 1; // Trovato spazio vuoto!
            break;
        }
    }

    // se il settore è pieno, dobbiamo cancellarlo e ricominciare dall'inizio
    if (!found) {
        printf("Settore pieno! Cancellazione necessaria...\n");
        FLASH_EraseInitTypeDef EraseInitStruct;
        uint32_t SectorError = 0;

        EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
        EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
        EraseInitStruct.Sector = FLASH_SECTOR_7;
        EraseInitStruct.NbSectors = 1;

        if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK) {
            printf("Errore Erase!\n");
            HAL_FLASH_Lock();
            return;
        }
        address = ADDR_FLASH_SECTOR_7; // Ricomincia dal primo indirizzo
    }

    // scrivi il dato
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, (uint64_t)dataToSave) == HAL_OK) {
        printf("Dato %02d salvato all'indirizzo: 0x%08X\n",dataToSave, (unsigned int)address);
    }

    HAL_FLASH_Lock();
}

/*
// leggere il valore all'accensione, cerca l'ultimo valore scritto ovvero quello prima di 0xFFFFFFFF che è la memoria non ancora scritta, tutta a 1
uint32_t Flash_Read(void) {
    uint32_t last_valid_val = 0xFFFFFFFF;
    uint32_t address = ADDR_FLASH_SECTOR_7;

    for (uint32_t i = 0; i < FLASH_SECTOR_SIZE; i += 4)
    {
        uint32_t current_val = *(__IO uint32_t*)(ADDR_FLASH_SECTOR_7 + i);

        if (current_val == 0xFFFFFFFF)
        {
            break; // siamo alla fine dei dati scritti
        }
        last_valid_val = current_val;
    }
    return last_valid_val;
}
*/

uint8_t Flash_Read(uint32_t *valuesArray) {
    uint32_t last_valid_val = 0xFFFFFFFF;
    uint32_t address = ADDR_FLASH_SECTOR_7;
    uint8_t valueOK = -1;
    for (uint32_t i = 0; i < FLASH_SECTOR_SIZE; i += 4)
    {
        uint32_t current_val = *(__IO uint32_t*)(ADDR_FLASH_SECTOR_7 + i);

        if( i >= 28) //ci sono 7 elementi (da 0...a 24 perché 28 è 0xFFFF), inserisci l'array con gli ultimi 7 elementi mantenendolo aggiornato
        {
        	valuesArray[0] = *(__IO uint32_t*)(ADDR_FLASH_SECTOR_7 + i - (28));
        	printf("Dato %02d recuperato all'indirizzo: 0x%08X\n",valuesArray[0], (unsigned int)(ADDR_FLASH_SECTOR_7 + i - (28)));

        	valuesArray[1] = *(__IO uint32_t*)(ADDR_FLASH_SECTOR_7 + i - (24));
        	printf("Dato %02d recuperato all'indirizzo: 0x%08X\n",valuesArray[1], (unsigned int)(ADDR_FLASH_SECTOR_7 + i - (24)));

        	valuesArray[2] = *(__IO uint32_t*)(ADDR_FLASH_SECTOR_7 + i - (20));
        	printf("Dato %02d recuperato all'indirizzo: 0x%08X\n",valuesArray[2], (unsigned int)(ADDR_FLASH_SECTOR_7 + i - (20)));

        	valuesArray[3] = *(__IO uint32_t*)(ADDR_FLASH_SECTOR_7 + i - (16));
        	printf("Dato %02d recuperato all'indirizzo: 0x%08X\n",valuesArray[3], (unsigned int)(ADDR_FLASH_SECTOR_7 + i - (16)));

        	valuesArray[4] = *(__IO uint32_t*)(ADDR_FLASH_SECTOR_7 + i - (12));
        	printf("Dato %02d recuperato all'indirizzo: 0x%08X\n",valuesArray[4], (unsigned int)(ADDR_FLASH_SECTOR_7 + i - (12)));

        	valuesArray[5] = *(__IO uint32_t*)(ADDR_FLASH_SECTOR_7 + i - (8));
        	printf("Dato %02d recuperato all'indirizzo: 0x%08X\n",valuesArray[5], (unsigned int)(ADDR_FLASH_SECTOR_7 + i - (8)));

        	valuesArray[6] = *(__IO uint32_t*)(ADDR_FLASH_SECTOR_7 + i - (4));
        	printf("Dato %02d recuperato all'indirizzo: 0x%08X\n",valuesArray[6], (unsigned int)(ADDR_FLASH_SECTOR_7 + i - (4)));

        	valueOK = 0;
        }

        if (current_val == 0xFFFFFFFF)
        {
        	if( i < 24)
        	{
            	valueOK = -1;
        	}
            break; // siamo alla fine dei dati scritti
        }
        last_valid_val = current_val;
    }
    return valueOK;
}

