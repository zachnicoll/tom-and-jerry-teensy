#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <cpu_speed.h>

#include <graphics.h>
#include <macros.h>
#include "lcd_model.h"

// Contant Vars
#define STATUS_BAR_HEIGHT 10
#define NUM_SWITCHES 7

// Global Vars
int current_level = 1;

struct player
{
    int lives, score, x, y, fireworks;
} tom, jerry;

struct wall
{
    int x1, y1, x2, y2;
};

volatile uint8_t state_counts[7];
volatile uint8_t switch_states[7];
// [SW1, SW2, SWA, SWB, SWC, SWD, SWCENTER]

void start_screen(void)
{
    int pressed = 0;
    while (pressed == 0)
    {
        if (BIT_IS_SET(PINF, 5))
        {
            pressed = 1;
        }
        draw_string(5, 0, "Zachary Nicoll", FG_COLOUR);
        draw_string(5, 10, "n10214453", FG_COLOUR);
        draw_string(10, 30, "Tom And Jerry", FG_COLOUR);
        draw_string(5, 40, "-On the Teensy-", FG_COLOUR);
        show_screen();
    }
}

void setup_vars(void)
{
    if (current_level == 1)
    {
        jerry.score = 0;
        jerry.lives = 5;
        jerry.x = 0;
        jerry.y = STATUS_BAR_HEIGHT + 1;

        tom.x = LCD_X - 5;
        tom.y = LCD_Y - 9;
    }

    for (int i = 0; i < NUM_SWITCHES; i++)
    {
        state_counts[i] = 0b00000000;
        switch_states[i] = 0b00000000;
    }
}

void setup(void)
{
    // Setup LCD Display
    set_clock_speed(CPU_8MHz);
    lcd_init(LCD_DEFAULT_CONTRAST);
    lcd_clear();

    // Setup Data Direction Register //

    // Joystick - Input
    CLEAR_BIT(DDRD, 2); // Up Joystick
    CLEAR_BIT(DDRB, 2); // Left Joystick
    CLEAR_BIT(DDRB, 8); // Down Joystick
    CLEAR_BIT(DDRD, 1); // Right Joystick
    CLEAR_BIT(DDRB, 1); // Centre Joystick

    // Tactile Buttons - Input
    CLEAR_BIT(DDRF, 5); // SW2 (Left Button)
    CLEAR_BIT(DDRF, 6); // SW3 (Right Button)

    // Thumbwheels - Input
    CLEAR_BIT(DDRF, 1); // Left Pot
    CLEAR_BIT(DDRF, 2); // Right Pot

    // LEDs - Output
    SET_BIT(DDRB, 2); // Left LED
    SET_BIT(DDRB, 3); // Right LED

    // Setup Timers

    // Timer 0, normal mode, 0.08s overflow
    TCCR0A = 0;
    TCCR0B = 4;
    TIMSK0 = 1; // Enable  overflow interupts for this timer.

    // Enable interupts
    sei();

    // Set Initial Var Values
    setup_vars();

    start_screen();
}

// Interupts
ISR(TIMER0_OVF_vect)
{
    uint8_t pin_arr_1[NUM_SWITCHES] = {PINF, PINF, PINB, PINB, PIND, PIND, PINB};
    uint8_t pin_arr_2[NUM_SWITCHES] = {5, 6, 7, 2, 2, 1, 1};
    for (int i = 0; i < NUM_SWITCHES; i++)
    {
        state_counts[i] = state_counts[i] << 1;
        uint8_t mask = 0b00001111;
        state_counts[i] &= mask;
        state_counts[i] |= BIT_IS_SET(pin_arr_1[i], pin_arr_2[i]);

        if (state_counts[i] == mask)
        {
            switch_states[i] = 1;
        }
        else if (state_counts[i] == 0)
        {
            switch_states[i] = 0;
        }
    }
}

void draw_gui(void)
{
    char str_buffer[20];

    sprintf(str_buffer, "L:%d", current_level);
    draw_string(0, 0, str_buffer, FG_COLOUR);

    sprintf(str_buffer, "H:%d", jerry.lives);
    draw_string(18, 0, str_buffer, FG_COLOUR);

    sprintf(str_buffer, "S:%d", jerry.score);
    draw_string(36, 0, str_buffer, FG_COLOUR);

    // FIX TIME!!!!!!!!!!!!!!
    //sprintf(str_buffer, "T:%d", "00:00");
    draw_string(55, 0, "00:00", FG_COLOUR);

    draw_line(0, STATUS_BAR_HEIGHT, LCD_X, STATUS_BAR_HEIGHT, FG_COLOUR);
}

void draw_players(void)
{
    draw_string(jerry.x, jerry.y, "J", FG_COLOUR);
    draw_string(tom.x, tom.y, "T", FG_COLOUR);
}

void draw_walls(void)
{
}

void draw_objs(void)
{
}

void draw(void)
{
    draw_gui();
    draw_players();
    draw_walls();
    draw_objs();
}

void handle_player(void){
    if(switch_states[2] == 1){
        jerry.y++;
    }
}

void process(void)
{
    clear_screen();

    handle_player();
    draw();

    show_screen();
}

int main(void)
{
    setup();

    for (;;)
    {
        process();
    }
}