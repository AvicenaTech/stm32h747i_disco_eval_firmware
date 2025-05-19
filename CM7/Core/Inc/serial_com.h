/*
 * serial_com.h
 *
 *  Created on: Mar 11, 2025
 *      Author: ndondlinger
 */

#ifndef INC_SERIAL_COM_H_
#define INC_SERIAL_COM_H_

#include "stm32h7xx.h"

void USB_Serial_Init(void);
void USB_Serial_TransmitStr(char*);
void USB_Serial_TransmitErrorStr(char*);
void USB_Serial_RxHandler(uint8_t*, uint32_t);

#endif /* INC_SERIAL_COM_H_ */
