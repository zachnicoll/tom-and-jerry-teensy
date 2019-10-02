/*
 *  CAB202 Teensy Library: 'cab202_teensy'
 *	graphics.h
 *
 *	B.Talbot, September 2015
 *  L.Buckingham, September 2017
 *	Queensland University of Technology
 */
#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#include <stdint.h>

#include "ascii_font.h"
#include "lcd.h"

/*
 *  Size of the screen_buffer, measured in bytes. There are LCD_X
 *	columns, and LCD_Y rows of pixels. Pixels are packed vertically
 *	into bytes, with 8 pixels in each byte.
 */
#define LCD_BUFFER_SIZE (LCD_X * (LCD_Y / 8))

/**
 *	Enumerated type to define colours. We have two colours:
 *	FG_COLOUR - foreground.
 *	BG_COLOUR - background.
 */
typedef enum colour_t {
	BG_COLOUR = 0,
	FG_COLOUR = 1
} colour_t;

/*
 *  Array of bytes used as screen buffer.
 *  (accessible from any file that includes graphics.h)
 */
extern uint8_t screen_buffer[LCD_BUFFER_SIZE];

/*
 *  Copy the contents of the screen buffer to the LCD.
 *	This is the only function that interfaces with the LCD hardware
 *  (sends entire current buffer to LCD screen)
 */
void show_screen(void);

/*
 * Clear the screen buffer (all pixels set to BG_COLOUR).
 */
void clear_screen(void);

/**
 *	Draw (or erase) a designated pixel in the screen buffer.
 *
 *	Parameters:
 *		x - The horizontal position of the pixel. The left edge of the screen
 *			is at x=0; the right edge is at (LCD_X-1).
 *		y - The vertical position of the pixel. The top edge of the screen is
 *			at y=0; the bottom edge is at (LCD_Y-1).
 *		colour - The colour, FG_COLOUR or BG_COLOUR.
 */
void draw_pixel(int x, int y, colour_t colour);

/**
 *	Draw a line in the screen buffer.
 *
 *	Parameters:
 *		x1 - The horizontal position of the start point of the line.
 *		y1 - The vertical position of the start point of the line.
 *		x2 - The horizontal position of the end point of the line.
 *		y2 - The vertical position of the end point of the line.
 *		colour - The colour, FG_COLOUR or BG_COLOUR.
 */
void draw_line(int x1, int y1, int x2, int y2, colour_t colour);

/**
 *	Render one of the printable ASCII characters into the screen buffer.
 *
 *	Parameters:
 *		x - The horizontal position of the top-left corner of the glyph.
 *		y - The vertical position of the top-left corner of the glyph.
 *		character - The (ASCII code of the) character to render. Valid values
 *			range from 0x20 == 32 == 'SPACE' to 0x7f == 127 == 'BACKSPACE'.
 *		colour - The colour, FG_COLOUR or BG_COLOUR. If colour is BG_COLOUR,
 *			the character is rendered as an inverse video block.
 */
void draw_char(int top_left_x, int top_left_y, char character, colour_t colour);

/**
 *	Render a string of printable ASCII characters into the screen buffer.
 *
 *	Parameters:
 *		x - The horizontal position of the top-left corner of the displayed
 *			text.
 *		y - The vertical position of the top-left corner of the displayed
 *			text.
 *		text - A string to render. Valid values for each element range from 
 *			0x20 == 32 to 0x7f == 127.
 *		colour - The colour, FG_COLOUR or BG_COLOUR. If colour is BG_COLOUR,
 *			the text is rendered as an inverse video block.
 */
void draw_string(int top_left_x, int top_left_y, char *text, colour_t colour);

#endif /* GRAPHICS_H_ */
