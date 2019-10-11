#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <cpu_speed.h>

#include <graphics.h>
#include <macros.h>
#include "lcd_model.h"

// Contant Vars
#define STATUS_BAR_HEIGHT 8
#define NUM_SWITCHES 7
#define PLR_WIDTH 5
#define PLR_HEIGHT 4
#define MINSPEED 0.2

// Jerry Bitmap
uint8_t jerry_bitmap[PLR_HEIGHT][PLR_WIDTH] = {{1, 1, 0, 1, 1}, {1, 1, 0, 1, 1}, {1, 1, 1, 1, 1}, {0, 1, 1, 1, 0}};

// Tom Bitmap
uint8_t tom_bitmap[PLR_HEIGHT][PLR_WIDTH] = {{0, 1, 0, 1, 0}, {1, 1, 1, 1, 1}, {1, 0, 1, 0, 1}, {0, 1, 1, 1, 0}};

// Global Vars
int current_level = 1;

struct player
{
    int lives, score, fireworks;
    double x, y, speed, direction;
} tom, jerry;

struct wall
{
    int x1, y1, x2, y2;
} wall_1, wall_2, wall_3, wall_4, wall_5, wall_6;

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
        tom.speed = (double)rand() / (double)RAND_MAX * MINSPEED + MINSPEED;
        tom.direction = ((double)rand() / (double)RAND_MAX) * M_PI * 2;

        wall_1.x1 = 18;
        wall_1.y1 = 15;
        wall_1.x2 = 13;
        wall_1.y2 = 25;

        wall_2.x1 = 25;
        wall_2.y1 = 35;
        wall_2.x2 = 25;
        wall_2.y2 = 45;

        wall_3.x1 = 45;
        wall_3.y1 = 10;
        wall_3.x2 = 60;
        wall_3.y2 = 10;

        wall_4.x1 = 58;
        wall_4.y1 = 25;
        wall_4.x2 = 72;
        wall_4.y2 = 30;
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

// Interrupts
ISR(TIMER0_OVF_vect)
{
    uint8_t pin_arr_1[NUM_SWITCHES] = {PINF, PINF, PINB, PINB, PIND, PIND, PINB};
    uint8_t pin_arr_2[NUM_SWITCHES] = {5, 6, 7, 1, 1, 0, 0};
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
    // Draw Bitmaps
    for (int i = 0; i < PLR_WIDTH; i++)
    {
        for (int j = 0; j < PLR_HEIGHT; j++)
        {
            if (jerry_bitmap[j][i] == 1)
            {
                draw_pixel(jerry.x + i, jerry.y + j, FG_COLOUR);
            }

            if (tom_bitmap[j][i] == 1)
            {
                draw_pixel(tom.x + i, tom.y + j, FG_COLOUR);
            }
        }
    }
}

void draw_walls(void)
{
    struct wall wall_arr[6] = {wall_1, wall_2, wall_3, wall_4, wall_5, wall_6};

    for (int i = 0; i < 6; i++)
    {
        draw_line(wall_arr[i].x1, wall_arr[i].y1, wall_arr[i].x2, wall_arr[i].y2, FG_COLOUR);
    }
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

bool wall_collision(struct player plyr, int dx, int dy)
{
    if (dx < 0 || dx > 0)
    {
        for (int i = 0; i < PLR_HEIGHT; i++)
        {
            int x = dx < 0 ? plyr.x + dx : plyr.x + PLR_WIDTH;
            int y = plyr.y + i;

            uint8_t bank = y >> 3;
            uint8_t pixel = y & 7;

            if (((screen_buffer[bank * LCD_X + (int)x] >> pixel) & 1) == 1)
            {
                return true;
            }
        }
    }
    if (dy < 0 || dy > 0)
    {
        for (int i = 0; i < PLR_WIDTH; i++)
        {
            int x = plyr.x + i;
            int y = dy > 0 ? plyr.y + PLR_HEIGHT : plyr.y + dy;

            uint8_t bank = y >> 3;
            uint8_t pixel = y & 7;

            if (((screen_buffer[bank * LCD_X + (int)x] >> pixel) & 1) == 1)
            {
                return true;
            }
        }
    }

    return false;
}

bool check_collision(struct player plyr, double dx, double dy, char obj)
{
    bool collided = false;
    if (dx > 0)
    {
        dx = 1;
    }
    else if (dx < 0)
    {
        dx = -1;
    }

    if (dy > 0)
    {
        dy = 1;
    }
    else if (dy < 0)
    {
        dy = -1;
    }

    if (obj == 'W') // MAKE PIXEL PERFECT!!!!!!!!!!!!!
    {
        collided = wall_collision(plyr, dx, dy);
    }

    return collided;
}

void handle_player(void)
{
    // Down
    if (switch_states[2] == 1 && jerry.y + PLR_HEIGHT + 1 < LCD_Y && !check_collision(jerry, 0, 1, 'W'))
    {
        jerry.y++;
    }
    // Left
    else if (switch_states[3] == 1 && jerry.x - 1 > 0 && !check_collision(jerry, -1, 0, 'W'))
    {
        jerry.x--;
    }
    // Up
    else if (switch_states[4] == 1 && jerry.y - 1 > STATUS_BAR_HEIGHT && !check_collision(jerry, 0, -1, 'W'))
    {
        jerry.y--;
    } // Right
    else if (switch_states[5] == 1 && jerry.x + 1 + PLR_WIDTH < LCD_X && !check_collision(jerry, 1, 0, 'W'))
    {
        jerry.x++;
    }
}

void update_enemy(void)
{
    double dx = cos(tom.direction) * tom.speed;
    double dy = sin(tom.direction) * tom.speed;
    uint8_t xdir = dx < 0 ? 0 : 1;
    uint8_t ydir = dy < 0 ? 0 : 1;

    if ((tom.x + dx + (PLR_WIDTH * xdir) > LCD_X) || (tom.x + dx < 0) || (tom.y + dy + (PLR_HEIGHT * ydir) > LCD_Y) || (tom.y + dy < STATUS_BAR_HEIGHT + 1) || check_collision(tom, dx, dy, 'W') || check_collision(tom, dx, 0, 'W') || check_collision(tom, 0, dy, 'W'))
    {
        tom.speed = (double)rand() / (double)RAND_MAX * MINSPEED + MINSPEED;
        tom.direction = ((double)rand() / (double)RAND_MAX) * M_PI * 2;
    }
    else
    {
        if ((tom.x + dx < LCD_X - 1) && (tom.x + dx > 0))
        {
            tom.x += dx;
        }

        if (tom.y + dy < LCD_Y - 1 && tom.y + dy > STATUS_BAR_HEIGHT)
        {
            tom.y += dy;
        }
    }
}

void process(void)
{
    clear_screen();
    draw();
    handle_player();
    update_enemy();
    show_screen();

    srand(TCNT0);
}

int main(void)
{
    setup();
    for (;;)
    {
        process();
    }
}