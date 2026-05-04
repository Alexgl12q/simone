/**
 * @file fsm_button.h
 * @brief Header for fsm_button.c file.
 * @author Mario Medina
 * @author Alejandro Garcia
 * @date 28 de Abril de 2026
 */

#ifndef FSM_BUTTON_H_
#define FSM_BUTTON_H_

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdint.h>
#include <stdbool.h>

/* Other includes */
#include "fsm.h"

/* Defines and enums ----------------------------------------------------------*/
/* Enums */
/** @brief Different states for the button */
enum FSM_BUTTON {
    BUTTON_RELEASED,      /**< Reposo: botón sin pulsar */
    BUTTON_RELEASED_WAIT, /**< Espera tras soltar (anti-rebotes) */
    BUTTON_PRESSED,       /**< Botón pulsado detectado */
    BUTTON_PRESSED_WAIT   /**< Espera tras pulsar (anti-rebotes) */
};

/* Typedefs --------------------------------------------------------------------*/
typedef struct
{
    fsm_t f;                    /*!< Button FSM */
    uint32_t debounce_time_ms;  /*!< Debounce time in ms */
    uint32_t next_timeout;      /*!< Next timeout for the anti-debounce in ms */
    uint32_t tick_pressed;      /*!< Number of ticks when the button was pressed */
    uint32_t duration;          /*!< How much time the button has been pressed */
    uint32_t button_id;         /*!< Button ID. Must be unique */
} fsm_button_t;

/* Function prototypes and explanation -------------------------------------------------*/

/**
 * @brief Create a new button FSM
 * 
 * @param debounce_time_ms Debounce time in ms
 * @param button_id Button ID. Must be unique
 * 
 * @return fsm_button_t* Pointer to the button FSM
 */
fsm_button_t* fsm_button_new (uint32_t debounce_time_ms, uint8_t button_id);

/**
 * @brief Destroy a button FSM
 * 
 * @param p_fsm Pointer to an fsm_button_t struct
 */
void fsm_button_destroy (fsm_button_t *p_fsm);

/**
 * @brief Fire the button FSM
 * 
 * @param p_fsm Pointer to an fsm_button_t struct
 */
void fsm_button_fire (fsm_button_t *p_fsm);

/**
 * @brief Return the duration of the last button press.
 * 
 * @param p_fsm Pointer to an fsm_button_t struct
 * @return uint32_t Duration of the last button press in milliseconds
 */
uint32_t fsm_button_get_duration (fsm_button_t *p_fsm);

/**
 * @brief Reset the duration of the last button press
 * 
 * @param p_fsm Pointer to an fsm_button_t struct
 */
void fsm_button_reset_duration (fsm_button_t *p_fsm);

/**
 * @brief Get the debounce time of the button FSM
 * 
 * @param p_fsm Pointer to an fsm_button_t struct
 * @return uint32_t Debounce time in ms
 */
uint32_t fsm_button_get_debounce_time_ms (fsm_button_t *p_fsm);

/**
 * @brief Check if the button FSM is active, or not.
 * 
 * Se considera activa si el estado actual es diferente a BUTTON_RELEASED.
 * 
 * @param p_fsm Pointer to an fsm_button_t struct
 * @return true si hay actividad (pulsado o procesando rebotes), false si está inactivo.
 */
bool fsm_button_check_activity (fsm_button_t *p_fsm);

#endif /* FSM_BUTTON_H_ */