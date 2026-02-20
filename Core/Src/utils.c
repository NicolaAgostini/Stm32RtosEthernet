/*
 * utils.c
 *
 *  Created on: 18 feb 2026
 *      Author: n.agostini
 */


#include "utils.h"
#include "stm32f7xx_hal.h"
#include <time.h>
#include <stdio.h>
#include <stdint.h>

#include "lwip/apps/sntp.h"
#include "lwip/apps/sntp_opts.h"


// Recuperiamo l'handle dell'RTC definito nel main.c
extern RTC_HandleTypeDef hrtc;

void set_rtc_from_sntp(uint32_t sec) {
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};
    struct tm *timeinfo;
    time_t rawtime = (time_t)sec;

    // Offset Italia (UTC+1)
    rawtime += 3600;

    timeinfo = gmtime(&rawtime);

    // Impostazione ORA
    sTime.Hours   = timeinfo->tm_hour;
    sTime.Minutes = timeinfo->tm_min;
    sTime.Seconds = timeinfo->tm_sec;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;
    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

    // Impostazione DATA
    sDate.WeekDay = (timeinfo->tm_wday == 0) ? RTC_WEEKDAY_SUNDAY : timeinfo->tm_wday;
    sDate.Month   = timeinfo->tm_mon + 1;
    sDate.Date    = timeinfo->tm_mday;
    sDate.Year    = timeinfo->tm_year - 100;
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    printf("SNTP [Utils]: Ora aggiornata correttamente.\r\n");
}

// Utility extra per leggere l'ora velocemente nei log
void print_system_time_string(char* buffer) {
    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
    sprintf(buffer, "%02d:%02d:%02d", sTime.Hours, sTime.Minutes, sTime.Seconds);
}

// Utility extra per recuperare data e ora
int get_system_time_string(uint8_t *hours, uint8_t *min, uint8_t *sec, uint8_t *day, uint8_t *month, uint8_t *year)
{
    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
    if( hours != NULL && min != NULL && sec != NULL && day != NULL && month != NULL && year != NULL)
    {
        *hours = sTime.Hours;
        *min = sTime.Minutes;
        *sec = sTime.Seconds;

        *day = sDate.Date;
        *month = sDate.Month;
        *year = sDate.Year;

        return 0;
    }
    else
    {
    	return -1;
    }


}


void start_sntp_service(void)
{

	sntp_setoperatingmode(SNTP_OPMODE_POLL);

	// define server for time
	sntp_setservername(0, "pool.ntp.org");


	// start service
	sntp_init();

	printf("SNTP: Servizio avviato, attesa sincronizzazione...\r\n");

}

void set_rtc_manual(uint8_t hours, uint8_t minutes, uint8_t sec, uint8_t day, uint8_t month, uint8_t year) {
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    // Impostazione ORA
    sTime.Hours   = hours;
    sTime.Minutes = minutes;
    sTime.Seconds = sec;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;
    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

    // Impostazione DATA
    sDate.Date    = day;
    sDate.Month   = month;
    sDate.Year    = year;
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    printf("Ora aggiornata manualmente= %02d:%02d:%02d  Data:%02d:%02d:%04d \r\n", hours, minutes, sec, day, month, ((uint32_t)year)+2000);
}





