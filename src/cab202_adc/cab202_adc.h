/*
**	cab202_adc.h
**
**	ADC access functions for the CAB202 TeensyPewPew.
**
**	Author: original: unknown.
**			this version: Lawrence Buckingham, October 2017.
*/

#pragma once

#include <stdint.h>
#include <avr/io.h>

/*
**	Initialize and enable ADC with pre-scaler 128.
**
**	Assuming CPU speed is 8MHz, sets the ADC clock to a frequency of
**	8000000/128 = 62500Hz. Normal conversion takes 13 ADC clock cycles,
**	or 0.000208 seconds. The first conversion will be slower, due to
**	need to initialise ADC circuit.
*/
void adc_init();

/*
**	Do single conversion to read value of designated ADC pin combination.
**
**	Input:
**	channel - A 6-bit value which specifies the ADC input channel and gain
**			  selection. Refer table 24-4 in datasheet.
**
**	On TeensyPewPew, channel should be 0, 1, or 4.
**	0 = Pot0
**	1 = Pot1
**	4 = Broken-out Pin F4.
*/
uint16_t adc_read(uint8_t channel);
