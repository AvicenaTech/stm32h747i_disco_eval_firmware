/*
 * fake_function_calls.h
 *
 *  Created on: Mar 17, 2025
 *      Author: ndondlinger
 */

#ifndef INC_FAKE_FUNCTION_CALLS_H_
#define INC_FAKE_FUNCTION_CALLS_H_

void Fake_ASIC_Read(uint16_t, uint16_t*, uint8_t);
void Fake_ASIC_Write(uint16_t, uint16_t*, uint8_t);
uint8_t Fake_ADC_Read(uint8_t, float*);
void Fake_ASIC_OemCapture(uint8_t, uint8_t, uint8_t*);

#endif /* INC_FAKE_FUNCTION_CALLS_H_ */
