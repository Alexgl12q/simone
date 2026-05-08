/**
 * @file fsm_simone.c
 * @brief Simone FSM main file.
 * @author Alejandro Garcia
 * @author Mario Medina
 * @date fecha
 */

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

/* HW dependent includes */
#include "stm32f4_button.h"
#include "stm32f4_keyboard.h"

/* Project includes */
#include "fsm_simone.h"
#include "port_simone.h"
#include "port_system.h"

const rgb_color_t *p_colors_library[] = {&color_red, &color_green, &color_blue, &color_yellow, &color_turquoise, &color_white};

/* --- Funciones Auxiliares --- */

static char _get_key_from_color(rgb_color_t color)
{
    if (color.r == color_red.r && color.g == color_red.g && color.b == color_red.b) return KEY_RED;
    if (color.r == color_green.r && color.g == color_green.g && color.b == color_green.b) return KEY_GREEN;
    if (color.r == color_blue.r && color.g == color_blue.g && color.b == color_blue.b) return KEY_BLUE;
    if (color.r == color_yellow.r && color.g == color_yellow.g && color.b == color_yellow.b) return KEY_YELLOW;
    if (color.r == color_turquoise.r && color.g == color_turquoise.g && color.b == color_turquoise.b) return KEY_TURQUOISE;
    if (color.r == color_white.r && color.g == color_white.g && color.b == color_white.b) return KEY_WHITE;
    return KEY_INVALID_COLOR;
}

static rgb_color_t _get_color_from_key(char key)
{
    switch (key) {
        case KEY_RED: return color_red;
        case KEY_GREEN: return color_green;
        case KEY_BLUE: return color_blue;
        case KEY_YELLOW: return color_yellow;
        case KEY_TURQUOISE: return color_turquoise;
        case KEY_WHITE: return color_white;
        default: return color_off;
    }
}

static void _add_color(fsm_simone_t *p_fsm_simone)
{
    if (p_fsm_simone->sequence_count < SEQUENCE_LENGTH) {
        int random_index = rand() % NUMBER_OF_COLORS_GAME;
        p_fsm_simone->sequence[p_fsm_simone->sequence_count] = *p_colors_library[random_index];
        p_fsm_simone->sequence_count++;
    }
}

/* --- State machine input or transition functions (Guards) --- */

static int check_button_pressed(fsm_t *self) {
    fsm_simone_t *p_s = (fsm_simone_t *)self;
    return fsm_button_check_activity(p_s->p_fsm_button);
}

static int check_timer_expired(fsm_t *self) {
    return port_simone_get_timeout_status();
}

static int check_playback_finished(fsm_t *self) {
    fsm_simone_t *p_s = (fsm_simone_t *)self;
    return (port_simone_get_timeout_status() && (p_s->current_step >= p_s->sequence_count));
}

static int check_key_pressed(fsm_t *self) {
    fsm_simone_t *p_s = (fsm_simone_t *)self;
    return fsm_keyboard_check_activity(p_s->p_fsm_keyboard);
}

/* --- State machine output or action functions (Actions) --- */

static void do_idle(fsm_t *self) {
    fsm_simone_t *p_s = (fsm_simone_t *)self;
    p_s->sequence_count = 0;
    fsm_rgb_light_set_color(p_s->p_fsm_rgb_light, color_off, 0);
    port_system_sleep();
}

static void do_add_color_and_start(fsm_t *self) {
    fsm_simone_t *p_s = (fsm_simone_t *)self;
    _add_color(p_s);
    p_s->current_step = 0;
    port_simone_set_timeout_status(true); // Forzamos entrada a PLAYBACK
}

static void do_playback_on(fsm_t *self) {
    fsm_simone_t *p_s = (fsm_simone_t *)self;
    port_simone_set_timeout_status(false);
    
    uint32_t durations[] = {SIMONE_TIME_ON_LEVEL_EASY_MS, SIMONE_TIME_ON_LEVEL_MEDIUM_MS, SIMONE_TIME_ON_LEVEL_HARD_MS};
    uint8_t intensities[] = {LEVEL_EASY_MIN_INTENSITY, LEVEL_MEDIUM_MIN_INTENSITY, LEVEL_HARD_MIN_INTENSITY};
    
    fsm_rgb_light_set_color(p_s->p_fsm_rgb_light, p_s->sequence[p_s->current_step], intensities[p_s->level]);
    port_simone_set_timer_timeout(durations[p_s->level]);
    p_s->current_step++;
}

static void do_playback_off(fsm_t *self) {
    fsm_simone_t *p_s = (fsm_simone_t *)self;
    port_simone_set_timeout_status(false);
    fsm_rgb_light_set_color(p_s->p_fsm_rgb_light, color_off, 0);
    port_simone_set_timer_timeout(SIMONE_TIME_OFF_BETWEEN_COLORS_MS);
}

static void do_prepare_wait_key(fsm_t *self) {
    fsm_simone_t *p_s = (fsm_simone_t *)self;
    p_s->current_step = 0;
    port_simone_set_timer_timeout(SIMONE_TIME_WAIT_INPUT_MS);
}

static void do_verify_input(fsm_t *self) {
    fsm_simone_t *p_s = (fsm_simone_t *)self;
    char pressed_key = stm32f4_keyboard_get_last_key();
    char expected_key = _get_key_from_color(p_s->sequence[p_s->current_step]);

    if (pressed_key == expected_key) {
        fsm_rgb_light_set_color(p_s->p_fsm_rgb_light, _get_color_from_key(pressed_key), 100);
        p_s->current_step++;
        
        if (p_s->current_step >= p_s->sequence_count) {
            if (p_s->sequence_count >= SEQUENCE_LENGTH) {
                if (p_s->level < LEVEL_HARD) p_s->level++;
                p_s->sequence_count = 0; 
            }
            // Transición automática por tabla a ADD_COLOR
        } else {
            port_simone_set_timer_timeout(SIMONE_TIME_WAIT_INPUT_MS);
        }
    } else {
        do_idle(self); // Game Over
    }
}

/* --- Transition Table --- */

static fsm_trans_t simone_transition_table[] = {
    { IDLE,         check_button_pressed, ADD_COLOR,    do_add_color_and_start },
    { ADD_COLOR,    check_timer_expired,  PLAYBACK,     do_playback_on },
    { PLAYBACK,     check_playback_finished, WAIT_KEY,  do_prepare_wait_key },
    { PLAYBACK,     check_timer_expired,  ADD_COLOR,    do_playback_off },
    { WAIT_KEY,     check_timer_expired,  IDLE,         do_idle },
    { WAIT_KEY,     check_key_pressed,    VERIFY_INPUT, do_verify_input },
    { VERIFY_INPUT, check_timer_expired,  ADD_COLOR,    do_add_color_and_start },
    { -1, NULL, -1, NULL }
};

/* --- Public Functions --- */

static void fsm_simone_init(fsm_simone_t *p_fsm_simone, fsm_button_t *p_fsm_button, uint32_t on_off_press_time_ms, fsm_keyboard_t *p_fsm_keyboard, fsm_rgb_light_t *p_fsm_rgb_light, uint8_t level)
{
    fsm_init((fsm_t *)p_fsm_simone, simone_transition_table);
    p_fsm_simone->p_fsm_button = p_fsm_button;
    p_fsm_simone->p_fsm_keyboard = p_fsm_keyboard;
    p_fsm_simone->p_fsm_rgb_light = p_fsm_rgb_light;
    p_fsm_simone->level = level;
    p_fsm_simone->sequence_count = 0;
    p_fsm_simone->current_step = 0;
    p_fsm_simone->timer_expired = false;
    srand(time(NULL));
}

fsm_simone_t *fsm_simone_new(fsm_button_t *p_fsm_button, uint32_t on_off_press_time_ms, fsm_keyboard_t *p_fsm_keyboard, fsm_rgb_light_t *p_fsm_rgb_light, uint8_t level)
{
    fsm_simone_t *p_fsm_simone = (fsm_simone_t *)malloc(sizeof(fsm_simone_t));
    if (p_fsm_simone != NULL) {
        fsm_simone_init(p_fsm_simone, p_fsm_button, on_off_press_time_ms, p_fsm_keyboard, p_fsm_rgb_light, level);
    }
    return p_fsm_simone;
}

void fsm_simone_fire(fsm_simone_t *p_fsm) {
    fsm_fire((fsm_t *)p_fsm);
}

void fsm_simone_destroy(fsm_simone_t *p_fsm) {
    free(p_fsm);
}