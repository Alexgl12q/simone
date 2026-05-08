/**
 * @file fsm_button.c
 * @brief Button FSM main file.
 * @author Mario Medina
 * @author Alejandro Garcia
 * @date 28 de Abril de 2026
 */

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdlib.h>
#include <stdbool.h>

/* HW dependent includes */
#include "port_button.h"
#include "port_system.h"
#include "fsm.h"

/* Project includes */
#include "fsm_button.h"

/* Macros --------------------------------------------------------------------*/
#define FSM_BUTTON(x) ((fsm_button_t *)(x))

/* State machine input or transition functions -------------------------------*/

/**
 * @brief Check if the button has been pressed.
 * @param p_this Pointer to an fsm_t struct than contains an fsm_button_t
 * @return true if the button is pressed, false otherwise
 */
static bool check_button_pressed(fsm_t * p_this)
{
    fsm_button_t *p_fsm = FSM_BUTTON(p_this);
    return port_button_get_pressed(p_fsm->button_id);
}

/**
 * @brief Check if the button has been released.
 * @param p_this Pointer to an fsm_t struct than contains an fsm_button_t
 * @return true if the button is released, false otherwise
 */
static bool check_button_released(fsm_t * p_this)
{
    fsm_button_t *p_fsm = FSM_BUTTON(p_this);
    return !port_button_get_pressed(p_fsm->button_id);
}

/**
 * @brief Check if the debounce-time has passed
 * @param p_this Pointer to an fsm_t struct than contains an fsm_button_t
 * @return true if its higher, false otherwise
 */
static bool check_timeout(fsm_t * p_this)
{
    fsm_button_t *p_fsm = FSM_BUTTON(p_this);
    uint32_t current_tick = port_system_get_millis();
    return (current_tick >= p_fsm->next_timeout);
}   

/* State machine output or action functions ----------------------------------*/

/**
 * @brief Store the system tick when the button was pressed.
 * @param p_this Pointer to an fsm_t struct than contains an fsm_button_t.
 */
static void do_store_tick_pressed(fsm_t * p_this)
{
    fsm_button_t *p_fsm = FSM_BUTTON(p_this);
    uint32_t current_tick = port_system_get_millis();
    p_fsm->tick_pressed = current_tick;
    p_fsm->next_timeout = current_tick + p_fsm->debounce_time_ms;
}   

/**
 * @brief Store the duration of the button press.
 * @param p_this Pointer to an fsm_t struct than contains an fsm_button_t
 */
static void do_set_duration(fsm_t * p_this)
{
    fsm_button_t *p_fsm = FSM_BUTTON(p_this);
    uint32_t current_tick = port_system_get_millis();
    
    p_fsm->duration = current_tick - p_fsm->tick_pressed;
    p_fsm->next_timeout = current_tick + p_fsm->debounce_time_ms;
}   

/* Transition table ----------------------------------------------------------*/
static fsm_trans_t fsm_trans_button[] = {
    {BUTTON_RELEASED,      check_button_pressed,  BUTTON_PRESSED_WAIT,  do_store_tick_pressed},
    {BUTTON_PRESSED_WAIT,  check_timeout,         BUTTON_PRESSED,       NULL},
    {BUTTON_PRESSED,       check_button_released, BUTTON_RELEASED_WAIT, do_set_duration},
    {BUTTON_RELEASED_WAIT, check_timeout,         BUTTON_RELEASED,      NULL},
    {-1, NULL, -1, NULL}
};

/* Other auxiliary functions --------------------------------------------------*/

/**
 * @brief Initialize a button FSM.
 */
void fsm_button_init(fsm_button_t *p_fsm_button, uint32_t debounce_time, uint8_t button_id)
{
    fsm_init(&p_fsm_button->f, fsm_trans_button);

    p_fsm_button->debounce_time_ms = debounce_time;
    p_fsm_button->button_id = button_id;

    p_fsm_button->tick_pressed = 0;
    p_fsm_button->duration = 0;
    p_fsm_button->next_timeout = 0;

    port_button_init(button_id);
}

/* Public functions -----------------------------------------------------------*/

fsm_button_t* fsm_button_new (uint32_t debounce_time_ms, uint8_t button_id)
{
    fsm_button_t *p_fsm_button = malloc(sizeof(fsm_button_t)); 
    if (p_fsm_button != NULL) {
        fsm_button_init(p_fsm_button, debounce_time_ms, button_id);
    }
    return p_fsm_button;
}

uint32_t fsm_button_get_duration(fsm_button_t *p_fsm)
{
    return p_fsm->duration;
}

void fsm_button_reset_duration(fsm_button_t *p_fsm)
{
    p_fsm->duration = 0;
}

uint32_t fsm_button_get_debounce_time_ms(fsm_button_t *p_fsm)
{
    return p_fsm->debounce_time_ms;
}

void fsm_button_fire(fsm_button_t *p_fsm)
{
    fsm_fire(&p_fsm->f);
}

void fsm_button_destroy(fsm_button_t *p_fsm)
{
    free(p_fsm);
}

/**
 * @brief Comprueba si la FSM del botón tiene actividad pendiente.
 * 
 * De acuerdo a la Versión 4, la FSM está inactiva solo en BUTTON_RELEASED.
 * 
 * @param p_fsm Puntero a la estructura de la FSM.
 * @return true si el estado actual es diferente a BUTTON_RELEASED.
 */
bool fsm_button_check_activity (fsm_button_t *p_fsm) 
{
    /* Obtenemos el estado de la FSM genérica interna */
    int current_state = fsm_get_state(&(p_fsm->f));
    
    /* Si no está en reposo (RELEASED), hay actividad pendiente (rebotes o pulsado) */
    return (current_state != BUTTON_RELEASED);
}