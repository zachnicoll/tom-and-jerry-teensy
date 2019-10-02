/*
** Nokia 5110 LCD logical model.
**
**	Enumerated types and macros to support learning about the LCD.
**
**	Lawrence Buckingham, 10 Sep 2017.
**	(C) Queensland University of Technology, Brisbane, Australia.
*/

#pragma once
#include <stdint.h>

#if ! defined(LCD_X)
#define LCD_X 84
#endif

#if ! defined(LCD_Y)
#define LCD_Y 48
#endif

// Issue a command to the Nokia5110.
#define LCD_CMD( op_code, args ) lcd_write(0,(op_code)|(args))

// Send a byte of pixel data to the Nokia5110.
#define LCD_DATA( args )         lcd_write(1,(args))

// Valid op_codes for Nokia5110.
typedef enum lcd_op_code_t {
	lcd_nop              = 0,
	lcd_set_function     = (1 << 5),

	// When lcd_instr_basic has been selected, these op_codes are available.
	lcd_set_display_mode = (1 << 3),
	lcd_set_y_addr       = (1 << 6),
	lcd_set_x_addr       = (1 << 7),

	// When lcd_instr_extended has been selected, these op_codes are available.
	lcd_set_temp_coeff   = (1 << 2),
	lcd_set_bias         = (1 << 4),
	lcd_set_contrast     = (1 << 7),
} lcd_op_code_t;

// Arguments for lcd_set_function
typedef enum lcd_power_mode_t {
	lcd_chip_active = 0,
	lcd_power_down  = (1 << 2),
} lcd_power_mode_t;

// Arguments for lcd_set_function
typedef enum lcd_addressing_mode_t {
	lcd_addr_horizontal = 0,
	lcd_addr_vertical   = (1 << 1),
} lcd_addressing_mode_t;

// Arguments for lcd_set_function
typedef enum lcd_instruction_mode_t {
	lcd_instr_basic    = 0,
	lcd_instr_extended = 1
} lcd_instruction_mode_t;

// Arguments for lcd_set_display_mode
typedef enum lcd_display_mode_t {
	lcd_display_all_off = (0b000), // decimal 0
	lcd_display_all_on  = (0b001), // decimal 1
	lcd_display_normal  = (0b100), // decimal 4
	lcd_display_inverse = (0b101), // decimal 5 
} lcd_display_mode_t;

// Arguments for lcd_set_y_addr: numeric values from 0 to 5.
// Arguments for lcd_set_x_addr: numeric values from 0 to 83.
// Arguments for lcd_set_temp_coeff: numeric values from 0 to 3.
// Arguments for lcd_set_bias: numeric values from 0 to 7. Ref Data Sheet.
// Arguments for lcd_set_contrast: numeric VOP values from 0 to 127.

/*
**	A logical model of the LCD state machine.
**
**	!!! This is not for use in Teensy programs      !!!
**	!!! But it can be used to implement an emulator !!!
**
*/

typedef struct Nokia5110_t {
	// PowerDown mode
	lcd_power_mode_t powerMode;

	// Instruction set
	lcd_instruction_mode_t instructionSet;

	// LCD contrast (V_{OP} setting. Valid values 0 .. 0x7f
	uint8_t contrast;

	// LCD Bias. Valid values 0 .. 7
	uint8_t bias;

	// LCD Temperature Coefficient. Valid values 0.. 0x03. 
	uint8_t temperatureCoefficient;

	// Display mode.
	lcd_display_mode_t displayMode;

	// Addressing mode (cursor direction control)
	lcd_addressing_mode_t addressing;

	// Current horizontal position of data cursor (0..83).
	// When a data byte is received, the byte is inserted at pixels[x][y],
	// after which x and/or y are updated according to the addressing mode.
	uint8_t x;

	// Current vertical position of data cursor (0..5)
	// When a data byte is received, the byte is inserted at pixels[x][y],
	// after which x and/or y are updated according to the addressing mode.
	uint8_t y;

	// Pixel data 2D array
	uint8_t pixels[LCD_X][LCD_Y / 8];

} Nokia5110_t;

