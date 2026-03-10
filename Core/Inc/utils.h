/*
 * utils.h
 *
 *  Created on: 18 feb 2026
 *      Author: n.agostini
 */

#ifndef INC_UTILS_H_
#define INC_UTILS_H_

#include <stdint.h>
#include "cmsis_os.h"



void set_rtc_from_sntp(uint32_t sec);
void start_sntp_service(void);
int get_system_time_string(uint8_t *, uint8_t *, uint8_t *, uint8_t *, uint8_t *, uint8_t *);
void set_rtc_manual(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);

#endif /* INC_UTILS_H_ */
