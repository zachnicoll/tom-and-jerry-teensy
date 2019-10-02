#include <stdint.h>
#include <stdio.h>
#include <avr/io.h> 
#include <avr/interrupt.h>
#include <util/delay.h>
#include <cpu_speed.h>

#include <graphics.h>
#include <macros.h>
#include "lcd_model.h"

void start_screen(void){
    int pressed = 0;
    while(pressed == 0){
        if(BIT_IS_SET(PINF, 6)){
            pressed = 1;
        }

        draw_string(5,0, "Zachary Nicoll", FG_COLOUR);
        draw_string(5,10, "n10214453", FG_COLOUR);
        draw_string(10,30, "Tom And Jerry", FG_COLOUR);
        draw_string(5,40, "-On the Teensy-", FG_COLOUR);
        show_screen();
    }
}


void setup(void){
    // Setup LCD Display
    set_clock_speed(CPU_8MHz);
	lcd_init(LCD_DEFAULT_CONTRAST);
	lcd_clear();

    // Setup Data Direction Register
    
    // Joystick - Input
    CLEAR_BIT(DDRD, 2); // Up Joystick
    CLEAR_BIT(DDRB, 2); // Left Joystick
    CLEAR_BIT(DDRB, 8); // Down Joystick
    CLEAR_BIT(DDRD, 1);// Right Joystick
    CLEAR_BIT(DDRB, 1); // Centre Joystick

    // Tactile Buttons - Input
    CLEAR_BIT(DDRF, 5); // SW2 (Left Button)
    CLEAR_BIT(DDRF, 6); // SW3 (Right Button)

    // Thumbwheels - Input
    CLEAR_BIT(DDRF, 1); // Left Pot
    CLEAR_BIT(DDRF, 2); // Right Pot

    // LEDs - Output
    SET_BIT(DDRB, 2);// Left LED
    SET_BIT(DDRB, 3); // Right LED

    start_screen();

    show_screen();
}

void process(void){


    show_screen();
}

int main(void){
    setup();

    for( ;; ){
        process();
    }
}