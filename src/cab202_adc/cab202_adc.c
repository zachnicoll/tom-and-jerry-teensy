/*
**	cab202_adc.c
**
**	ADC access functions for the CAB202 TeensyPewPew.
**
**	Author: original: unknown.
**			this version: Lawrence Buckingham, October 2017.
*/

#include "cab202_adc.h"

/*
**	Initialize and enable ADC with pre-scaler 128.
**
**	Assuming CPU speed is 8MHz, sets the ADC clock to a frequency of
**	8000000/128 = 62500Hz. Normal conversion takes 13 ADC clock cycles,
**	or 0.000208 seconds. The first conversion will be slower, due to
**	need to initialise ADC circuit.
*/
void adc_init() {
	// ADC Enable and pre-scaler of 128: ref table 24-5 in datasheet
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

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
uint16_t adc_read(uint8_t channel) {
	// Select AVcc voltage reference and pin combination.
	// Low 5 bits of channel spec go in ADMUX(MUX4:0)
	// 5th bit of channel spec goes in ADCSRB(MUX5).
	ADMUX = (channel & ((1 << 5) - 1)) | (1 << REFS0);
	ADCSRB = (channel & (1 << 5));

	// Start single conversion by setting ADSC bit in ADCSRA
	ADCSRA |= (1 << ADSC);

	// Wait for ADSC bit to clear, signalling conversion complete.
	while ( ADCSRA & (1 << ADSC) ) {}

	// Result now available.
	return ADC;
}

