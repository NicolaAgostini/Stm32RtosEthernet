/*
 * time_syscall.c
 *
 *  Created on: 2 mar 2026
 *      Author: n.agostini
 */

#include <sys/time.h>
#include <time.h>
#include "main.h"

// hrtc del main
extern RTC_HandleTypeDef hrtc;

/**
 * @brief Funzione epr ritornare l'ora corrente del sistema
 * * @param tv Puntatore alla struttura timeval da riempire
 * @param tzvp Puntatore alla timezone (NULL)
 * @return int 0 in caso di successo
 */
int _gettimeofday(struct timeval *tv, void *tzvp) {
    if (tv == NULL) {
        return -1;
    }

    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};
    struct tm t = {0};

    /* lettura dell'hardware RTC */

    if (HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK) return -1;
    if (HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK) return -1;

    /* conversione nel formato standard POSIX (struct tm) */
    t.tm_sec   = sTime.Seconds;
    t.tm_min   = sTime.Minutes;
    t.tm_hour  = sTime.Hours;
    t.tm_mday  = sDate.Date;
    t.tm_mon   = sDate.Month - 1;      // Mesi da 0 a 11 in C standard
    t.tm_year  = sDate.Year + 100;
    t.tm_isdst = -1;                  // Ora legale non specificata

    /* conversione in secondi Unix (Epoch) */
    /* mktime calcola i secondi trascorsi dal 1 Gennaio 1970 */
    tv->tv_sec = mktime(&t);
    tv->tv_usec = 0; // L'RTC standard non ha risoluzione in microsecondi

    return 0;
}
