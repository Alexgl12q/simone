/**
 * @file fsm_simone.c
 * @brief Simone FSM main file.
 * @author Mario Medina 
 * @author Alejandro Garcia
 * @date 2026-05-04
 */
/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "fsm_simone.h"
#include "port_simone.h"
#include "port_system.h"
#include "stm32f4_system.h"
#include "stm32f4_button.h"   // Para acceder a los flags del driver
#include "stm32f4_keyboard.h" // Para acceder a las teclas del driver

/* Predefined colors library */
const rgb_color_t *p_colors_library[] = {&color_red, &color_green, &color_blue, &color_yellow, &color_turquoise, &color_white};

/* --- Funciones Auxiliares de Conversión --- */

static char _get_key_from_color(rgb_color_t color) {
    if (color.r == color_red.r && color.g == color_red.g && color.b == color_red.b) return KEY_RED;
    if (color.r == color_green.r && color.g == color_green.g && color.b == color_green.b) return KEY_GREEN;
    if (color.r == color_blue.r && color.g == color_blue.g && color.b == color_blue.b) return KEY_BLUE;
    if (color.r == color_yellow.r && color.g == color_yellow.g && color.b == color_yellow.b) return KEY_YELLOW;
    if (color.r == color_turquoise.r && color.g == color_turquoise.g && color.b == color_turquoise.b) return KEY_TURQUOISE;
    if (color.r == color_white.r && color.g == color_white.g && color.b == color_white.b) return KEY_WHITE;
    return KEY_INVALID_COLOR;
}

static rgb_color_t _get_color_from_key(char key) {
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

/* --- Funciones de Guarda (Inputs) --- */

static int check_button_pressed(fsm_t *self) {
    fsm_simone_t *p_s = (fsm_simone_t *)self;
    // Usamos la función de actividad para evitar el error de "no field flag_pressed"
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
    // El teclado está activo si no está en estado de espera inicial
    return fsm_keyboard_check_activity(p_s->p_fsm_keyboard);
}

/* --- Funciones de Acción (Outputs) --- */

static void do_idle(fsm_t *self) {
    fsm_simone_t *p_s = (fsm_simone_t *)self;
    p_s->sequence_count = 0;
    fsm_rgb_light_set_color(p_s->p_fsm_rgb_light, color_off, 0);
    
    // Gestión de bajo consumo: dormir si no hay actividad
    if (!fsm_button_check_activity(p_s->p_fsm_button) && 
        !fsm_keyboard_check_activity(p_s->p_fsm_keyboard)) {
        port_system_sleep();
    }
}

static void do_add_color(fsm_t *self) {
    fsm_simone_t *p_s = (fsm_simone_t *)self;
    if (p_s->sequence_count < SEQUENCE_LENGTH) {
        p_s->sequence[p_s->sequence_count] = *p_colors_library[rand() % NUMBER_OF_COLORS_GAME];
        p_s->sequence_count++;
    }
    p_s->current_step = 0;
    port_simone_set_timeout_status(true); // Disparar inicio inmediato de playback
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

static void do_verify_input(fsm_t *self) {
    fsm_simone_t *p_s = (fsm_simone_t *)self;
    
    // Obtener la tecla real pulsada desde el driver
    char pressed_key = stm32f4_keyboard_get_last_key(); 
    char expected_key = _get_key_from_color(p_s->sequence[p_s->current_step]);

    if (pressed_key == expected_key) {
        // Feedback visual correcto
        fsm_rgb_light_set_color(p_s->p_fsm_rgb_light, _get_color_from_key(pressed_key), 100);
        p_s->current_step++;
        
        if (p_s->current_step >= p_s->sequence_count) {
            if (p_s->sequence_count >= SEQUENCE_LENGTH) {
                // Victoria de nivel: subir dificultad y resetear
                if (p_s->level < LEVEL_HARD) p_s->level++;
                p_s->sequence_count = 0; 
            }
            // Preparar para añadir color en la siguiente transición
        } else {
            port_simone_set_timer_timeout(SIMONE_TIME_WAIT_INPUT_MS);
        }
    } else {
        do_idle(self); // Error: Game Over
    }
}

/* --- Tabla de Transiciones --- */

static fsm_trans_t simone_tt[] = {
    { IDLE,         check_button_pressed, ADD_COLOR,    do_add_color },
    { ADD_COLOR,    check_timer_expired,  PLAYBACK,     do_playback_on },
    { PLAYBACK,     check_playback_finished, WAIT_KEY,  do_playback_off },
    { PLAYBACK,     check_timer_expired,  ADD_COLOR,    do_playback_off },
    { WAIT_KEY,     check_timer_expired,  IDLE,         do_idle },
    { WAIT_KEY,     check_key_pressed,    VERIFY_INPUT, do_verify_input },
    { VERIFY_INPUT, check_timer_expired,  ADD_COLOR,    do_add_color },
    { -1, NULL, -1, NULL }
};

/* --- Funciones Públicas --- */

fsm_simone_t *fsm_simone_new(fsm_button_t *p_fsm_button, uint32_t on_off_press_time_ms, 
                            fsm_keyboard_t *p_fsm_keyboard, fsm_rgb_light_t *p_fsm_rgb_light, uint8_t level) {
    fsm_simone_t *p_s = (fsm_simone_t *)malloc(sizeof(fsm_simone_t));
    if (p_s) {
        fsm_init((fsm_t *)p_s, simone_tt);
        p_s->p_fsm_button = p_fsm_button;
        p_s->p_fsm_keyboard = p_fsm_keyboard;
        p_s->p_fsm_rgb_light = p_fsm_rgb_light;
        p_s->level = level;
        p_s->sequence_count = 0;
        p_s->timer_expired = false;
        srand(time(NULL));
    }
    return p_s;
}

void fsm_simone_fire(fsm_simone_t *p_fsm) {
    fsm_fire((fsm_t *)p_fsm);
}

void fsm_simone_destroy(fsm_simone_t *p_fsm) {
    free(p_fsm);
}