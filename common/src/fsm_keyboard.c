/**
 * @file fsm_keyboard.c
 * @brief Keyboard sensor FSM main file.
 * @author alumno1
 * @author alumno2
 * @date fecha
 */

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

/* HW dependent includes */
#include "port_system.h"
#include "port_keyboard.h"

/* Project includes */
#include "fsm.h"
#include "fsm_keyboard.h"
#include "keyboards.h"

/* Typedefs --------------------------------------------------------------------*/

/* Private functions -----------------------------------------------------------*/
/**
 * @brief Check if the timeout to activate a new row and scan columns has passed.
 * 
 * @param p_this Pointer to an fsm_t struct than contains an fsm_keyboard_t
 * @return the value of the field flag_row_timeout (true or false)
 */
static bool check_row_timeout(fsm_t *p_this) {
    fsm_keyboard_t *p_fsm = (fsm_keyboard_t *)p_this;
    return port_keyboard_get_row_timeout_status(p_fsm->keyboard_id);
}

/**
 * @brief Check if the keyboard has been pressed.
 * 
 * @param p_this Pointer to an fsm_t struct than contains an fsm_keyboard_t
 * @return the status (true or false)
 */
static bool check_keyboard_pressed(fsm_t *p_this) {
    fsm_keyboard_t *p_fsm = (fsm_keyboard_t *)p_this;
    return port_keyboard_get_key_pressed_status(p_fsm->keyboard_id);
}

/**
 * @brief Check if the keyboard has been released
 * 
 * @param p_this Pointer to an fsm_t struct than contains an fsm_keyboard_t
 * @return the inverse of the status 
 */
static bool check_keyboard_released(fsm_t *p_this) {
    fsm_keyboard_t *p_fsm = (fsm_keyboard_t *)p_this;
    return !port_keyboard_get_key_pressed_status(p_fsm->keyboard_id);
}

/**
 * @brief check if debounceTime has passed
 * 
 * @param p_this Pointer to an fsm_t struct than contains an fsm_keyboard_t
 * @return true if higher, false otherwise
 */
static bool check_timeout(fsm_t *p_this) {
    fsm_keyboard_t *p_fsm = (fsm_keyboard_t *)p_this;
    uint32_t current_tick = port_system_get_millis();
    return (current_tick >= p_fsm->next_timeout);
}

/**
 * @brief Clean the row timeout flag and update the row to be excited
 * 
 * @param p_this Pointer to an fsm_t struct than contains an fsm_keyboard_t
 */
static void do_excite_next_row(fsm_t *p_this) {
    fsm_keyboard_t *p_fsm = (fsm_keyboard_t *)p_this;
    port_keyboard_set_row_timeout_status(p_fsm->keyboard_id, false);
    port_keyboard_excite_next_row(p_fsm->keyboard_id);
}

/**
 * @brief Store the system tick when the keyboard was pressed
 * 
 * @param p_this Pointer to an fsm_t struct than contains an fsm_keyboard_t
 */
static void do_store_tick_pressed(fsm_t *p_this) {
    fsm_keyboard_t *p_fsm = (fsm_keyboard_t *)p_this;
    uint32_t current_tick = port_system_get_millis();
    p_fsm->tick_pressed = current_tick;
    p_fsm->next_timeout = current_tick + p_fsm->debounce_time_ms;
}

/**
 * @brief Store the key value of the keyboard press.
 * 
 * @param p_this Pointer to an fsm_t struct than contains an fsm_keyboard_t
 */
static void do_set_key_value(fsm_t *p_this) {
    fsm_keyboard_t *p_fsm = (fsm_keyboard_t *)p_this;
    uint32_t current_tick = port_system_get_millis();
    p_fsm->next_timeout = current_tick + p_fsm->debounce_time_ms;
    
    /* Guardar la tecla que se ha pulsado */
    p_fsm->key_value = port_keyboard_get_key_value(p_fsm->keyboard_id);
}

static fsm_trans_t fsm_trans_keyboard[] = {
    { KEYBOARD_RELEASED_WAIT_ROW, check_row_timeout,       KEYBOARD_RELEASED_WAIT_ROW, do_excite_next_row    },
    { KEYBOARD_RELEASED_WAIT_ROW, check_keyboard_pressed,  KEYBOARD_PRESSED_WAIT,      do_store_tick_pressed },
    { KEYBOARD_PRESSED_WAIT,      check_timeout,           KEYBOARD_PRESSED,           NULL                  },
    { KEYBOARD_PRESSED,           check_keyboard_released, KEYBOARD_RELEASED_WAIT,      do_set_key_value      },
    { KEYBOARD_RELEASED_WAIT,     check_timeout,           KEYBOARD_RELEASED_WAIT_ROW, NULL                  },
    { -1, NULL, -1, NULL }
};

/* Auxiliary functions -------------------------------------------------------*/
static void fsm_keyboard_init(fsm_keyboard_t *p_fsm_keyboard, uint32_t debounce_time_ms, uint8_t keyboard_id)
{
    // Initialize the FSM
    fsm_init(&p_fsm_keyboard->f, fsm_trans_keyboard);

    // Initialize the fields of the FSM structure
    p_fsm_keyboard->debounce_time_ms = debounce_time_ms;
    p_fsm_keyboard->keyboard_id = keyboard_id;
    p_fsm_keyboard->tick_pressed = 0;
    p_fsm_keyboard->next_timeout = 0;
    
    /* Inicializar el hardware a través de PORT */
    port_keyboard_init(keyboard_id);

    /* Obtener y setear tecla nula por defecto */
    p_fsm_keyboard->invalid_key = port_keyboard_get_invalid_key_value(keyboard_id);
    p_fsm_keyboard->key_value = p_fsm_keyboard->invalid_key;
}

/* Public functions -----------------------------------------------------------*/
fsm_keyboard_t *fsm_keyboard_new(uint32_t debounce_time_ms, uint8_t keyboard_id)
{
    fsm_keyboard_t *p_fsm_keyboard = malloc(sizeof(fsm_keyboard_t)); 
    if (p_fsm_keyboard != NULL) {
        fsm_keyboard_init(p_fsm_keyboard, debounce_time_ms, keyboard_id);
    }
    return p_fsm_keyboard;
}

void fsm_keyboard_destroy(fsm_keyboard_t *p_fsm) {
    free(p_fsm);
}

void fsm_keyboard_fire(fsm_keyboard_t *p_fsm) {
    fsm_fire(&(p_fsm->f));
}

void fsm_keyboard_start_scan(fsm_keyboard_t *p_fsm) {
    port_keyboard_start_scan(p_fsm->keyboard_id);
}

void fsm_keyboard_stop_scan(fsm_keyboard_t *p_fsm) {
    port_keyboard_stop_scan(p_fsm->keyboard_id);
}

char fsm_keyboard_get_key_value(fsm_keyboard_t *p_fsm) {
    return p_fsm->key_value;
}

bool fsm_keyboard_get_is_valid_key(fsm_keyboard_t *p_fsm) {
    return (p_fsm->key_value != p_fsm->invalid_key);
}

void fsm_keyboard_reset_key_value(fsm_keyboard_t *p_fsm) {
    p_fsm->key_value = p_fsm->invalid_key;
}

/**
 * @brief Comprueba si la FSM del teclado está activa.
 * 
 * Según los requisitos de la Versión 4, se considera activa si NO está en el 
 * estado KEYBOARD_RELEASED_WAIT_ROW.
 */
bool fsm_keyboard_check_activity(fsm_keyboard_t *p_fsm) {
    int current_state = fsm_get_state(&(p_fsm->f));
    return (current_state != KEYBOARD_RELEASED_WAIT_ROW);
}