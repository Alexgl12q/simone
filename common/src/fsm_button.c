/**
 * @file fsm_button.c
 * @brief Button FSM main file.
 * @author Mario Medina
 * @author Alejandro Garcia
 * @date fecha
 */

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdlib.h>

/* HW dependent includes */
#include "port_button.h"
#include "port_system.h"
#include "fsm.h"
#define FSM_BUTTON(x) ((fsm_button_t *)(x))

/* Project includes */
#include "fsm_button.h"

/* State machine input or transition functions */
/**
 * @brief Check if the button has been pressed.
 * 
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
 * 
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
 * 
 * @param p_this Pointer to an fsm_t struct than contains an fsm_button_t
 * @return true if its higher, false otherwise
 */
static bool check_timeout(fsm_t * p_this)
{
    fsm_button_t *p_fsm = FSM_BUTTON(p_this);
    uint32_t current_tick = port_system_get_millis();
    return (current_tick > p_fsm->next_timeout);
}	

/* State machine output or action functions */

/**
 * @brief Store the system tick when the button was pressed.
 * 
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
 * 
 * @param p_this Pointer to an fsm_t struct than contains an fsm_button_t
 */
static void do_set_duration(fsm_t * p_this)
{
    fsm_button_t *p_fsm = FSM_BUTTON(p_this);
    uint32_t current_tick = port_system_get_millis();
    
    p_fsm->duration = current_tick - p_fsm->tick_pressed;
    p_fsm->next_timeout = current_tick + p_fsm->debounce_time_ms;
}	

static fsm_trans_t fsm_trans_button[] = {
    {BUTTON_RELEASED,      check_button_pressed,  BUTTON_PRESSED_WAIT,  do_store_tick_pressed},
    {BUTTON_PRESSED_WAIT,  check_timeout,         BUTTON_PRESSED,       NULL},
    {BUTTON_PRESSED,       check_button_released, BUTTON_RELEASED_WAIT, do_set_duration},
    {BUTTON_RELEASED_WAIT, check_timeout,         BUTTON_RELEASED,      NULL},
    {-1, NULL, -1, NULL}
};

/* Other auxiliary functions */

/**
 * @brief Initialize a button FSM.
 * This function initializes the default values of the FSM struct and calls to the port to initialize the associated HW given the ID.
 * This FSM implements an anti-debounce mechanism. Debounces (or very fast button presses) lasting less than the debounce_time_ms are filtered out.
 * The FSM stores the duration of the last button press. The user should ask for it using the function fsm_button_get_duration().
 * At start and reset, the duration value must be 0 ms. A value of 0 ms means that there has not been a new button press
 *
 * @param p_fsm_button Pointer to the button FSM.
 * @param debounce_time	Anti-debounce time in milliseconds
 * @param button_id	Unique button identifier number
 */
void fsm_button_init(fsm_button_t *p_fsm_button, uint32_t debounce_time, uint8_t button_id)
{
    fsm_init(&p_fsm_button->f, fsm_trans_button);

    /* TODO alumnos: */
    p_fsm_button->debounce_time_ms = debounce_time;
    p_fsm_button->button_id = button_id;

    p_fsm_button->tick_pressed = 0;
    p_fsm_button->duration = 0;
    p_fsm_button->next_timeout = 0;

    port_button_init(button_id);
}

/* Public functions -----------------------------------------------------------*/
/**
 * @brief Create a new button FSM
 * 
 * @param debounce_time_ms	Debounce time in milliseconds
 * @param button_id	Button ID. Must be unique
 * @return fsm_button_t* Pointer to the button FSM
 */
fsm_button_t* fsm_button_new (uint32_t debounce_time_ms, uint8_t button_id)
{
    fsm_button_t *p_fsm_button = malloc(sizeof(fsm_button_t)); /* Do malloc to reserve memory of all other FSM elements, although it is interpreted as fsm_t (the first element of the structure) */
    fsm_button_init(p_fsm_button, debounce_time_ms, button_id);   /* Initialize the FSM */
    return p_fsm_button;                                       /* Composite pattern: return the fsm_t pointer as a fsm_button_t pointer */
}

/**
 * @brief Return the duration of the last button press
 * 
 * @param Pointer to an fsm_button_t struct.
 * @return uint32_t Duration of the last button press in milliseconds
 */
uint32_t fsm_button_get_duration(fsm_button_t *p_fsm)
{
    return p_fsm->duration;
}

/**
 * @brief Reset the duration of the last button press
 * 
 * @param p_fsm	Pointer to an fsm_button_t struct.
 */
void fsm_button_reset_duration(fsm_button_t *p_fsm)
{
    p_fsm->duration = 0;
}

/**
 * @brief Get the debounce time of the button FSM.
 * 
 * @param p_fsm	Pointer to an fsm_button_t struct
 * @return uint32_t Debounce time in milliseconds
 */
uint32_t fsm_button_get_debounce_time_ms(fsm_button_t *p_fsm)
{
    return p_fsm->debounce_time_ms;
}

/* FSM-interface functions. These functions are used to interact with the FSM */
void fsm_button_fire(fsm_button_t *p_fsm)
{
    fsm_fire(&p_fsm->f); // Is it also possible to it in this way: fsm_fire((fsm_t *)p_fsm);
}


void fsm_button_destroy(fsm_button_t *p_fsm)
{
    free(&p_fsm->f);
}