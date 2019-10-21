#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <cpu_speed.h>

#include <graphics.h>
#include <macros.h>
#include "lcd_model.h"
#include <usb_serial.h>
#include <cab202_adc.h>

// Contant Vars
#define STATUS_BAR_HEIGHT 8
#define NUM_SWITCHES 7
#define OBJ_SIZE 5
#define MINSPEED 0.2
#define MAX_PLR_SPEED 2
#define MAX_WALL_SPEED 2
#define MAX_BRIGHTNESS 15

// Jerry Bitmap
uint8_t jerry_bitmap[OBJ_SIZE][OBJ_SIZE] = {{1, 1, 1, 1, 1}, {1, 0, 0, 0, 1}, {1, 1, 0, 1, 1}, {1, 0, 0, 1, 1}, {1, 1, 1, 1, 1}};

// Super Jerry Bitmap
uint8_t super_jerry_bitmap[OBJ_SIZE + 1][OBJ_SIZE + 1] = {{1, 1, 1, 1, 1, 1}, {1, 0, 0, 0, 0, 1}, {1, 1, 1, 0, 1, 1}, {1, 0, 1, 0, 1, 1}, {1, 0, 0, 0, 1, 1}, {1, 1, 1, 1, 1, 1}};

// Tom Bitmap
uint8_t tom_bitmap[OBJ_SIZE][OBJ_SIZE] = {{1, 1, 1, 1, 1}, {1, 0, 0, 0, 1}, {1, 1, 0, 1, 1}, {1, 1, 0, 1, 1}, {1, 1, 1, 1, 1}};

// Cheese Bitmap
uint8_t cheese_bitmap[OBJ_SIZE][OBJ_SIZE] = {{1, 1, 1, 1, 1}, {1, 0, 0, 0, 1}, {1, 0, 1, 1, 1}, {1, 0, 0, 0, 1}, {1, 1, 1, 1, 1}};

// Trap Bitmap
uint8_t trap_bitmap[OBJ_SIZE][OBJ_SIZE] = {{1, 1, 1, 1, 1}, {1, 0, 1, 0, 1}, {1, 0, 1, 0, 1}, {1, 0, 1, 0, 1}, {1, 1, 1, 1, 1}};

// Door Bitmap
uint8_t door_bitmap[OBJ_SIZE][OBJ_SIZE] = {{1, 1, 1, 1, 1}, {1, 0, 0, 0, 1}, {1, 0, 1, 0, 1}, {1, 0, 0, 0, 1}, {1, 1, 1, 1, 1}};

// Milk Bitmap
uint8_t milk_bitmap[OBJ_SIZE][OBJ_SIZE] = {{1, 1, 1, 1, 1}, {1, 1, 0, 1, 1}, {1, 0, 0, 0, 1}, {1, 1, 0, 1, 1}, {1, 1, 1, 1, 1}};

// Global Vars
int current_level = 1, cheese, cheese_collected, cheese_time, traps, trap_time, placing_trap, milk_time, placing_milk, milk_placed, super_activated, super_time;
double game_time, pause_start, pause_end, pause_time;
int cheese_positions[5][2], trap_positions[5][2], door_position[2], milk_position[2];
bool pause = false;
bool game_over = false;
bool pause_check = false;
double player_speed = 1;
double wall_speed = 1;
struct player
{
    int lives, score, fireworks;
    double init_x, init_y, x, y, speed, direction;
} tom, jerry;

struct firework
{
    double x, y;
} fireworks[20];

struct wall
{
    double x1, y1, x2, y2;
} wall_1, wall_2, wall_3, wall_4, wall_5, wall_6;

volatile uint8_t state_counts[7];
volatile uint8_t switch_states[7];
// [SW1, SW2, SWA, SWB, SWC, SWD, SWCENTER]
volatile uint32_t cycle_count = 0;
volatile uint8_t brightness = MAX_BRIGHTNESS;
uint8_t brightness_dir = 1;
volatile uint8_t pwm_counter = 0;

// Fucntion Declarations
bool check_collision(struct player plyr, double dx, double dy);
double elapsed_time();
void paused();
void setup();
void setup_vars();
void place_cheese_door(char c);
void shoot_firework();

// FOR DEBUGGING
void send_str(const char *s)
{
    char c;
    while (1)
    {
        c = pgm_read_byte(s++);
        if (!c)
            break;
        usb_serial_putchar(c);
    }
}

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

void handle_gameover()
{
    int pressed = 0;
    while (pressed == 0)
    {
        clear_screen();
        if (BIT_IS_SET(PINF, 5))
        {
            pressed = 1;
            game_over = false;
            cycle_count = 0;
            current_level = 1;
            setup_vars();
        }
        draw_string(LCD_X / 2 - 28, LCD_Y / 3, "-GAME OVER-", FG_COLOUR);
        draw_string(LCD_X / 2 - 33, LCD_Y / 3 + 10, "SW3 to Restart", FG_COLOUR);
        show_screen();
    }
}

void level1_walls()
{
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

void level2_walls()
{
    wall_1.x1 = 24;
    wall_1.y1 = 15;
    wall_1.x2 = 13;
    wall_1.y2 = 13;

    wall_2.x1 = 25;
    wall_2.y1 = 40;
    wall_2.x2 = 37;
    wall_2.y2 = 45;

    wall_3.x1 = 33;
    wall_3.y1 = 10;
    wall_3.x2 = 48;
    wall_3.y2 = 10;

    wall_4.x1 = 58;
    wall_4.y1 = 25;
    wall_4.x2 = 58;
    wall_4.y2 = 30;
}

void reset_walls()
{
    struct wall *wall_arr[6] = {&wall_1, &wall_2, &wall_3, &wall_4, &wall_5, &wall_6};
    for (int i = 0; i < 6; i++)
    {
        wall_arr[i]->x1 = -1;
        wall_arr[i]->y1 = -1;
        wall_arr[i]->x2 = -1;
        wall_arr[i]->y2 = -1;
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
        jerry.fireworks = 0;

        tom.x = LCD_X - 5;
        tom.y = LCD_Y - 9;
        tom.speed = ((double)rand() / (double)RAND_MAX * MINSPEED + MINSPEED) * player_speed;
        tom.direction = ((double)rand() / (double)RAND_MAX) * M_PI * 2;

        level1_walls();
    }
    else
    {
        reset_walls();
        jerry.x = 0;
        jerry.y = STATUS_BAR_HEIGHT + 1;
        jerry.fireworks = 20;
        tom.x = LCD_X - 5;
        tom.y = LCD_Y - 9;
        level2_walls();
    }

    jerry.init_x = jerry.x;
    jerry.init_y = jerry.y;
    super_activated = 0;
    cheese_collected = 0;

    tom.init_x = tom.x;
    tom.init_y = tom.y;

    for (int i = 0; i < NUM_SWITCHES; i++)
    {
        state_counts[i] = 0b00000000;
        switch_states[i] = 0b00000000;
    }

    for (int i = 0; i < 5; i++)
    {
        cheese_positions[i][0] = -10;
        cheese_positions[i][1] = -10;
        trap_positions[i][0] = -10;
        trap_positions[i][1] = -10;
    }
    door_position[0] = -10;
    door_position[1] = -10;
    milk_position[0] = -10;
    milk_position[1] = -10;

    pause_time = 0;
    cheese_time = elapsed_time();
    cheese = 0;
    trap_time = elapsed_time();
    traps = 0;
    placing_trap = 0;
    milk_time = elapsed_time();
    milk_placed = 0;
    placing_milk = 0;

    for (int i = 0; i < 20; i++)
    {
        fireworks[i].x = -1;
        fireworks[i].y = -1;
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
    adc_init();

    // LEDs - Output
    SET_BIT(DDRB, 2); // Left LED
    SET_BIT(DDRB, 3); // Right LED

    // Setup Timers

    // Timer 0 (debouncer), normal mode, 0.08s overflow
    TCCR0A = 0;
    TCCR0B = 1;
    TIMSK0 = 1; // Enable  overflow interupts for this timer.

    // Timer 1 (usb_serial inputs), normal mode, 0.08s overflow
    CLEAR_BIT(TCCR1A, WGM10);
    CLEAR_BIT(TCCR1A, WGM11);
    CLEAR_BIT(TCCR1B, WGM12);
    CLEAR_BIT(TCCR1B, WGM13);
    SET_BIT(TCCR1B, CS10);
    TIMSK1 = 1; // Enable  overflow interupts for this timer.

    // Enable interupts
    sei();

    // Enable USB Serial
    usb_init();
    while (!usb_configured())
    {
        // Block until USB is ready.
    }

    // Set Initial Var Values
    setup_vars();

    start_screen();

    // Timer 3 (game time), normal mode, 1s overflow
    TCCR3A = 0;
    TCCR3B = 1;
    TIMSK3 = 1; // Enable  overflow interupts for this timer.
}

void send_formatted(char *buffer, int buffer_size, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, buffer_size, format, args);
    usb_serial_write((uint8_t *)buffer, strlen(buffer));
}

void output_state()
{
    char str_buffer[80];

    int i_minutes = floor(game_time / 60.0);
    double fl_minutes = game_time / 60.0;
    double fraction = fl_minutes - floor(fl_minutes);
    int seconds = 60.0 * fraction;

    send_formatted(str_buffer, sizeof(str_buffer), "\r\n\rGame Time: %02d:%02d\n", i_minutes, seconds);
    send_formatted(str_buffer, sizeof(str_buffer), "\rCurrent Level: %d\n", current_level);
    send_formatted(str_buffer, sizeof(str_buffer), "\rLives: %d\n", jerry.lives);
    send_formatted(str_buffer, sizeof(str_buffer), "\rScore: %d\n", jerry.score);
    send_formatted(str_buffer, sizeof(str_buffer), "\rFireworks on Screen: %d\n", jerry.score >= 3 ? 20-jerry.fireworks : 0);
    send_formatted(str_buffer, sizeof(str_buffer), "\rMoustraps on Screen: %d\n", traps);
    send_formatted(str_buffer, sizeof(str_buffer), "\rCheese on Screen: %d\n", cheese);
    send_formatted(str_buffer, sizeof(str_buffer), "\rCheese Collected in Room: %d\n", cheese_collected);
    send_formatted(str_buffer, sizeof(str_buffer), "\rSuper Mode Active: %d\n", super_activated);
    send_formatted(str_buffer, sizeof(str_buffer), "\rPaused: %d\r\n", pause);
}

// Interrupts
ISR(TIMER0_OVF_vect)
{
    if (super_activated)
    {
        if (pwm_counter % brightness == 0)
        {
            SET_BIT(PORTB, 2);
            SET_BIT(PORTB, 3);
        }
        else
        {
            CLEAR_BIT(PORTB, 2);
            CLEAR_BIT(PORTB, 3);
        }
        pwm_counter++;
    }
    else
    {
        CLEAR_BIT(PORTB, 2);
        CLEAR_BIT(PORTB, 3);
    }
}

ISR(TIMER1_OVF_vect)
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

    int16_t c = usb_serial_getchar();

    // Down
    if (c == 's' && jerry.y + OBJ_SIZE + 1 < LCD_Y && !check_collision(jerry, 0, 1))
    {
        jerry.y++;
    }
    // Left
    else if (c == 'a' && jerry.x - 1 > 0 && !check_collision(jerry, -1, 0))
    {
        jerry.x--;
    }
    // Up
    else if (c == 'w' && jerry.y - 1 > STATUS_BAR_HEIGHT && !check_collision(jerry, 0, -1))
    {
        jerry.y--;
    } // Right
    else if (c == 'd' && jerry.x + 1 + OBJ_SIZE < LCD_X && !check_collision(jerry, 1, 0))
    {
        jerry.x++;
    }
    else if (c == 'i')
    {
        output_state();
    }
    else if (c == 'p')
    {
        paused();
    }
    else if (c == 'l')
    {
        if (current_level == 1)
        {
            current_level = 2;
            setup_vars();
        }
        else
        {
            game_over = true;
        }
    }
    else if (c == 'f')
    {
        shoot_firework();
    }
}

ISR(TIMER3_OVF_vect)
{
    cycle_count++;
}

double elapsed_time()
{
    return (cycle_count * 65536.0 + TCNT3) * (1.0 / 8000000.0);
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

    int i_minutes = floor(game_time / 60.0);
    double fl_minutes = game_time / 60.0;
    double fraction = fl_minutes - floor(fl_minutes);
    int seconds = 60.0 * fraction;
    sprintf(str_buffer, "%02d:%02d", i_minutes, seconds);
    draw_string(55, 0, str_buffer, FG_COLOUR);

    draw_line(0, STATUS_BAR_HEIGHT, LCD_X, STATUS_BAR_HEIGHT, FG_COLOUR);
}

void draw_jerry(void)
{
    // Draw Bitmap
    for (int i = 0; i < OBJ_SIZE; i++)
    {
        for (int j = 0; j < OBJ_SIZE; j++)
        {
            if (jerry_bitmap[j][i] == 1)
            {
                draw_pixel(jerry.x + i, jerry.y + j, FG_COLOUR);
            }
        }
    }
}

void draw_tom(void)
{
    // Draw Bitmap
    for (int i = 0; i < OBJ_SIZE; i++)
    {
        for (int j = 0; j < OBJ_SIZE; j++)
        {
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
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < OBJ_SIZE; j++)
        {
            for (int k = 0; k < OBJ_SIZE; k++)
            {
                if (cheese_positions[i][0] != -10)
                {
                    if (cheese_bitmap[j][k] == 1)
                    {
                        draw_pixel(cheese_positions[i][0] + k, cheese_positions[i][1] + j, FG_COLOUR);
                    }
                }

                if (trap_positions[i][0] != -10)
                {
                    if (trap_bitmap[j][k] == 1)
                    {
                        draw_pixel(trap_positions[i][0] + k, trap_positions[i][1] + j, FG_COLOUR);
                    }
                }

                if (i == 0 && door_position[0] != -10)
                {
                    if (door_bitmap[j][k] == 1)
                    {
                        draw_pixel(door_position[0] + k, door_position[1] + j, FG_COLOUR);
                    }
                }

                if (i == 0 && milk_position[0] != -10)
                {
                    if (milk_bitmap[j][k] == 1)
                    {
                        draw_pixel(milk_position[0] + k, milk_position[1] + j, FG_COLOUR);
                    }
                }
            }
        }
    }
}

void draw_fireworks()
{
    for (int i = 0; i < 20; i++)
    {
        if (fireworks[i].x != -1)
        {
            draw_pixel(fireworks[i].x, fireworks[i].y, FG_COLOUR);
        }
    }
}

void draw_super_jerry()
{
    // Draw Bitmap
    for (int i = 0; i < OBJ_SIZE + 1; i++)
    {
        for (int j = 0; j < OBJ_SIZE + 1; j++)
        {
            if (super_jerry_bitmap[j][i] == 1)
            {
                draw_pixel(jerry.x + i, jerry.y + j, FG_COLOUR);
            }
        }
    }
}

void draw(void)
{
    draw_gui();
    if (!super_activated)
    {
        draw_jerry();
    }
    draw_tom();
}

bool is_pixel(int x, int y)
{
    uint8_t bank = y >> 3;
    uint8_t pixel = y & 7;

    if (((screen_buffer[bank * LCD_X + (int)x] >> pixel) & 1) == 1)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool wall_collision(struct player plyr, int dx, int dy)
{
    if (dx < 0 || dx > 0)
    {
        for (int i = 0; i < OBJ_SIZE; i++)
        {
            int x = dx < 0 ? plyr.x + dx : plyr.x + OBJ_SIZE;
            int y = plyr.y + i;

            if (is_pixel(x, y))
            {
                return true;
            }
        }
    }
    if (dy < 0 || dy > 0)
    {
        for (int i = 0; i < OBJ_SIZE; i++)
        {
            int x = plyr.x + i;
            int y = dy > 0 ? plyr.y + OBJ_SIZE : plyr.y + dy;

            if (is_pixel(x, y))
            {
                return true;
            }
        }
    }

    return false;
}

bool box_collision(double dx, double dy, int x1, int y1, int x2, int y2, int offset)
{
    // If one rectangle is on left side of other
    if (x1 + dx > x2 + OBJ_SIZE - offset || x2 > x1 + dx + OBJ_SIZE - offset)
    {
        return false;
    }

    // If one rectangle is above other
    if (y1 + dy > y2 + OBJ_SIZE - offset || y2 > y1 + dy + OBJ_SIZE - offset)
    {
        return false;
    }

    return true;
}

bool check_collision(struct player plyr, double dx, double dy)
{
    bool collided = false;
    if (!super_activated)
    {
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

        collided = wall_collision(plyr, dx, dy);
    }

    return collided;
}

void randomize_tom()
{
    tom.speed = ((double)rand() / (double)RAND_MAX * MINSPEED + MINSPEED) * player_speed;
    tom.direction = ((double)rand() / (double)RAND_MAX) * M_PI * 2;
}

void reset_jerry()
{
    jerry.x = jerry.init_x;
    jerry.y = jerry.init_y;
}

void reset_tom()
{
    tom.x = tom.init_x;
    tom.y = tom.init_y;
}

void check_tom_collision(double dx, double dy)
{
    int x = round(jerry.x);
    int y = round(jerry.y);
    if (!box_collision(dx, dy, x, y, tom.x, tom.y, 1))
    {
        if (x + dx + OBJ_SIZE + super_activated < LCD_X && x + dx >= 0 && y + dy + OBJ_SIZE + super_activated < LCD_Y + 1 && y + dy > STATUS_BAR_HEIGHT && !check_collision(jerry, dx, dy))
        {
            jerry.x += dx;
            jerry.y += dy;
        }
    }
    else
    {
        if (!super_activated)
        {
            reset_jerry();
            jerry.lives--;
        }
        else
        {
            jerry.score++;
        }
        reset_tom();
        randomize_tom();
    }
}

void check_cheese_trap_collision()
{
    for (int i = 0; i < 5; i++)
    {
        if (box_collision(0, 0, jerry.x, jerry.y, cheese_positions[i][0], cheese_positions[i][1], 1))
        {
            jerry.score++;
            cheese_collected++;
            cheese--;
            cheese_positions[i][0] = -10;
            cheese_positions[i][1] = -10;
        }

        if (!super_activated && box_collision(0, 0, jerry.x, jerry.y, trap_positions[i][0], trap_positions[i][1], 1))
        {
            jerry.lives--;
            traps--;
            trap_positions[i][0] = -10;
            trap_positions[i][1] = -10;
        }
    }

    if (milk_placed == 1 && box_collision(0, 0, jerry.x, jerry.y, milk_position[0], milk_position[1], 1))
    {
        super_activated = 1;
        super_time = round(elapsed_time());
        milk_position[0] = -10;
        milk_position[1] = -10;
        milk_placed = 0;
    }
}

void firework_homing(struct firework *frwrk)
{
    double t1 = tom.x - frwrk->x;
    double t2 = tom.y - frwrk->y;
    double d = sqrt(t1 * t1 + t2 * t2);

    double dx = t1 * (1 / d);
    double dy = t2 * (1 / d);

    if (frwrk->x + dx < LCD_X && frwrk->x + dx > 1 && frwrk->y + dy < LCD_Y && frwrk->y + dy > 5)
    {
        if (!is_pixel(frwrk->x + dx, frwrk->y + dy) && !is_pixel(frwrk->x + dx, frwrk->y) && !is_pixel(frwrk->x, frwrk->y + dy))
        {
            frwrk->x += dx;
            frwrk->y += dy;
        }
        else
        {
            frwrk->x = -1;
            frwrk->y = -1;
            jerry.fireworks++;
        }
    }
    else
    {
        frwrk->x = -1;
        frwrk->y = -1;
        jerry.fireworks++;
    }
}

void update_fireworks()
{
    for (int i = 0; i < 20; i++)
    {
        if (fireworks[i].x != -1)
        {
            if ((round(fireworks[i].x) >= round(tom.x) && round(fireworks[i].x) < round(tom.x + OBJ_SIZE)) && (round(fireworks[i].y) >= round(tom.y) && round(fireworks[i].y) < round(tom.y + OBJ_SIZE)))
            {
                reset_tom();
                fireworks[i].x = -1;
                fireworks[i].y = -1;
                jerry.fireworks++;
            }
            else if (!pause)
            {

                struct firework *ptr = &fireworks[i];
                firework_homing(ptr);
            }
        }
    }
}

void shoot_firework()
{
    for (int i = 0; i < 20; i++)
    {
        if (fireworks[i].x == -1)
        {
            fireworks[i].x = jerry.x;
            fireworks[i].y = jerry.y;
            jerry.fireworks--;
            break;
        }
    }
}

void handle_player(void)
{
    double dx = 0;
    double dy = 0;

    if (jerry.lives == 0)
    {
        game_over = true;
    }

    if (jerry.score == 3 && jerry.fireworks == 0 && current_level == 1)
    {
        jerry.fireworks = 20;
    }

    if ((door_position[0] != -10 && box_collision(0, 0, jerry.x, jerry.y, door_position[0], door_position[1], 0)) || (switch_states[1] == 1 && current_level == 1))
    {
        if (current_level == 1)
        {
            current_level = 2;
            setup_vars();
        }
        else
        {
            game_over = true;
        }
    }

    if (round(elapsed_time()) - super_time >= 10 && !pause)
    {
        super_activated = 0;
    }

    // Down
    if (switch_states[2] == 1)
    {
        dy = 1 * player_speed;
    }
    // Left
    else if (switch_states[3] == 1)
    {
        dx = -1 * player_speed;
    }
    // Up
    else if (switch_states[4] == 1)
    {
        dy = -1 * player_speed;
    } // Right
    else if (switch_states[5] == 1)
    {
        dx = 1 * player_speed;
    }
    else if (jerry.fireworks > 0 && switch_states[6] == 1)
    {
        shoot_firework();
    }

    check_tom_collision(dx, dy);
    check_cheese_trap_collision();

    if (switch_states[0] == 1 && pause_check == false && elapsed_time() > 2)
    {
        paused();
        pause_check = true;
    }
    else if (switch_states[0] == 0 && pause_check == true)
    {
        pause_check = false;
    }
}

void update_enemy(void)
{
    double dx = cos(tom.direction) * (tom.speed * player_speed);
    double dy = sin(tom.direction) * (tom.speed * player_speed);
    uint8_t xdir = dx < 0 ? 0 : 1;
    uint8_t ydir = dy < 0 ? 0 : 1;

    if ((tom.x + dx + (OBJ_SIZE * xdir) > LCD_X) || (tom.x + dx < 0) || (tom.y + dy + (OBJ_SIZE * ydir) > LCD_Y) || (tom.y + dy < STATUS_BAR_HEIGHT + 1) || check_collision(tom, dx, dy) || check_collision(tom, dx, 0) || check_collision(tom, 0, dy))
    {
        randomize_tom();
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

int *find_clear()
{
    int x, y;
    int blocked = 1;

    do
    {
        x = round(((double)rand() / (double)RAND_MAX) * (LCD_X - OBJ_SIZE));
        y = round(((double)rand() / (double)RAND_MAX) * (LCD_Y - STATUS_BAR_HEIGHT - OBJ_SIZE)) + STATUS_BAR_HEIGHT + OBJ_SIZE;
        blocked = 0;
        if (x + OBJ_SIZE < LCD_X && x > 0 && y > STATUS_BAR_HEIGHT && y + OBJ_SIZE < LCD_Y)
        {
            for (int j = 0; j < OBJ_SIZE; j++)
            {
                for (int k = 0; k < OBJ_SIZE; k++)
                {
                    if (is_pixel(x + j, y + k))
                    {
                        blocked = 1;
                        break;
                    }
                }
            }
        }
        else
        {
            blocked = 1;
        }
    } while (blocked == 1);

    static int xy[2];
    xy[0] = x;
    xy[1] = y;

    return xy;
}

void place_cheese_door(char c)
{
    int *xy = find_clear();
    int x = *(xy);
    int y = *(xy + 1);

    if (c == 'C')
    {
        for (int i = 0; i < 5; i++)
        {
            if (cheese_positions[i][0] == -10)
            {
                cheese_positions[i][0] = x;
                cheese_positions[i][1] = y;
                cheese++;
                break;
            }
        }
    }
    else if (c == 'D')
    {
        door_position[0] = x;
        door_position[1] = y;
    }

    cheese_time = round(elapsed_time());
}

void place_trap()
{
    int blocked = 0;
    for (int i = 0; i < 5; i++)
    {
        if (box_collision(0, 0, tom.x, tom.y, cheese_positions[i][0], cheese_positions[i][1], 0) || box_collision(0, 0, tom.x, tom.y, trap_positions[i][0], trap_positions[i][1], 0) || box_collision(0, 0, tom.x, tom.y, door_position[0], door_position[1], 0) || box_collision(0, 0, tom.x, tom.y, milk_position[0], milk_position[1], 0))
        {
            blocked = 1;
            break;
        }
    }

    if (blocked == 0)
    {
        for (int i = 0; i < 5; i++)
        {
            if (trap_positions[i][0] == -10)
            {
                trap_positions[i][0] = round(tom.x);
                trap_positions[i][1] = round(tom.y);
                traps++;
                placing_trap = 0;
                break;
            }
        }
    }
    trap_time = round(elapsed_time());
}

void place_milk()
{
    int blocked = 0;
    for (int i = 0; i < 5; i++)
    {
        if (box_collision(0, 0, tom.x, tom.y, cheese_positions[i][0], cheese_positions[i][1], 0) || box_collision(0, 0, tom.x, tom.y, trap_positions[i][0], trap_positions[i][1], 0))
        {
            blocked = 1;
            break;
        }
    }

    if (blocked == 0)
    {
        milk_position[0] = tom.x;
        milk_position[1] = tom.y;
        milk_placed = 1;
        placing_milk = 0;
    }

    milk_time = round(elapsed_time());
}

void place_cheese_traps()
{
    int current_time = round(elapsed_time());

    if (cheese_collected == 5 && door_position[0] == -10)
    {
        place_cheese_door('D');
    }

    if (cheese < 5 && current_time - cheese_time == 2 && !pause)
    {
        place_cheese_door('C');
    }
    else if (cheese == 5 || pause)
    {
        cheese_time = round(elapsed_time());
    }

    if (placing_trap || (traps < 5 && current_time - trap_time == 3 && !pause))
    {
        placing_trap = 1;
        place_trap();
    }
    else if (traps == 5 || pause)
    {
        trap_time = round(elapsed_time());
    }

    if (current_level == 2)
    {
        if (placing_milk || (milk_placed == 0 && current_time - milk_time == 5 && !pause))
        {
            placing_milk = 1;
            place_milk();
        }
        else if (milk_placed == 1 || pause)
        {
            milk_time = round(elapsed_time());
        }
    }
}

void check_wall_wrap(struct wall *w)
{
    double dx = w->x2 - w->x1;
    double dy = w->y2 - w->y1;
    if (w->y1 < STATUS_BAR_HEIGHT && w->y2 < STATUS_BAR_HEIGHT)
    {
        if (dy != 0)
        {
            w->y1 = dy > 0 ? LCD_Y : LCD_Y - dy;
            w->y2 = dy > 0 ? LCD_Y + dy : LCD_Y;
        }
        else
        {
            w->y1 = LCD_Y;
            w->y2 = LCD_Y;
        }
    }
    if (w->y1 > LCD_Y && w->y2 > LCD_Y)
    {
        if (dy != 0)
        {
            w->y1 = dy > 0 ? STATUS_BAR_HEIGHT - dy : STATUS_BAR_HEIGHT;
            w->y2 = dy > 0 ? STATUS_BAR_HEIGHT : STATUS_BAR_HEIGHT + dy;
        }
        else
        {
            w->y1 = STATUS_BAR_HEIGHT;
            w->y2 = STATUS_BAR_HEIGHT;
        }
    }
    if (w->x1 > LCD_X && w->x2 > LCD_X)
    {
        w->x1 = dx > 0 ? 0 - dx : 0;
        w->x2 = dx > 0 ? 0 : 0 + dx;
    }
    if (w->x1 < 0 && w->x2 < 0)
    {
        w->x1 = dx > 0 ? LCD_X : LCD_X - dx;
        w->x2 = dx > 0 ? LCD_X + dx : LCD_X;
    }
}

void move_walls()
{
    double speed = 0.05 * wall_speed;
    struct wall *wall_arr[6] = {&wall_1, &wall_2, &wall_3, &wall_4, &wall_5, &wall_6};
    for (int i = 0; i < 6; i++)
    {
        if (wall_arr[i]->x1 != 0 && wall_arr[i]->x2 != 0 && wall_arr[i]->y1 != 0 && wall_arr[i]->y2 != 0)
        {
            check_wall_wrap(wall_arr[i]);

            double dx, dy;
            if (wall_arr[i]->y2 - wall_arr[i]->y1 != 0 && wall_arr[i]->x2 - wall_arr[i]->x1 != 0)
            {
                double theta = atan(((wall_arr[i]->y2 - wall_arr[i]->y1) / (wall_arr[i]->x2 - wall_arr[i]->x1)));
                double new_theta = M_PI - theta;
                dx = cos(new_theta) * speed;
                dy = sin(new_theta) * speed;
            }
            else if (wall_arr[i]->y2 - wall_arr[i]->y1 == 0)
            {
                dx = 0;
                dy = speed;
            }
            else
            {
                dx = speed;
                dy = 0;
            }

            wall_arr[i]->x1 += dx;
            wall_arr[i]->x2 += dx;
            wall_arr[i]->y1 += dy;
            wall_arr[i]->y2 += dy;
        }
    }
}

void check_wall_overlap()
{
    for (int i = 0; i < OBJ_SIZE; i++)
    {
        for (int j = 0; j < OBJ_SIZE; j++)
        {
            if (!super_activated && is_pixel(jerry.x + i, jerry.y + j))
            {
                reset_jerry();
            }

            if (is_pixel(tom.x + i, tom.y + j))
            {
                reset_tom();
            }
        }
    }
}

void set_speeds()
{
    double left_adc = adc_read(0);
    double right_adc = adc_read(1);

    player_speed = (left_adc / 1024.0) * 2;
    wall_speed = ((512.0 - right_adc) / 512.0) * 2;
}

void adjust_brightness()
{
    if (brightness >= MAX_BRIGHTNESS)
    {
        brightness_dir = 0;
    }
    else if (brightness <= 1)
    {
        brightness_dir = 1;
    }

    if (brightness_dir == 1)
    {
        brightness++;
    }
    else
    {
        brightness--;
    }
}

void paused()
{
    pause = !pause;
    if (pause)
    {
        pause_start = elapsed_time();
        pause_end = 0;
    }
    else
    {
        pause_end = elapsed_time();
        pause_time += pause_end - pause_start;
    }
}

void process(void)
{
    clear_screen();

    if (!game_over)
    {
        set_speeds();

        if (super_activated)
        {
            draw_super_jerry();
            adjust_brightness();
        }

        if (!pause)
        {
            move_walls();
        }
        draw_walls();
        if (!pause)
        {
            check_wall_overlap();
        }

        if (!pause)
        {
            game_time = elapsed_time() - pause_time;
            update_enemy();
            update_fireworks();
        }

        draw();
        handle_player();
        draw_fireworks();
        draw_objs();
        place_cheese_traps();
    }
    else
    {
        handle_gameover();
    }

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