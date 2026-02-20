/*
 * utils.h
 *
 *  Created on: 18 feb 2026
 *      Author: n.agostini
 */

#ifndef INC_UTILS_H_
#define INC_UTILS_H_

#include <stdint.h>

void set_rtc_from_sntp(uint32_t sec);
void start_sntp_service(void);


#endif /* INC_UTILS_H_ */
