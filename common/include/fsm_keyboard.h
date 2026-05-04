/**
 * @file fsm_keyboard.h
 * @brief Header for fsm_keyboard.c file.
 * @author Mario Eugenio
 * @author Medina
 * @date fecha
 */

#ifndef FSM_KEYBOARD_H_
#define FSM_KEYBOARD_H_

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

/* Standard C includes */
#include "fsm.h"
#include <stdint.h>
#include <stdbool.h>
/* Defines and enums ----------------------------------------------------------*/
/**
 * @brief Enumerator for the keyboard finite state machine.
 */
typedef enum {
    KEYBOARD_RELEASED_WAIT_ROW = 0,
    KEYBOARD_PRESSED_WAIT,
    KEYBOARD_PRESSED,
    KEYBOARD_RELEASED_WAIT
} FSM_KEYBOARD; 

/* Typedefs --------------------------------------------------------------------*/
/**
 * @brief Structure of the Keyboard FSM.
 */
typedef struct {
    fsm_t f;                   /*!< Instance of the fsm_t structure (must be the first element) */
    uint32_t debounce_time_ms; /*!< Debounce time in milliseconds */
    uint8_t keyboard_id;       /*!< Unique keyboard identifier */
    uint32_t tick_pressed;     /*!< System tick when the key was pressed */
    uint32_t next_timeout;     /*!< Next system tick timeout for debounce */
    char key_value;            /*!< Last valid key pressed */
    char invalid_key;          /*!< Null key character according to layout */
} fsm_keyboard_t;

/* Function prototypes and explanation -------------------------------------------------*/
/**
 * @brief Create a new keyboard FSM.
 * This function creates a new keyboard FSM with the given debounce time for all keys and keyboard ID.
 * 
 * @param debounce_time_ms	Debounce time in milliseconds for all keys.
 * @param keyboard_id
 * @returns fsm_keyboard_t* Pointer to keyboard fsm
 */
fsm_keyboard_t* fsm_keyboard_new(uint32_t debounce_time_ms, uint8_t keyboard_id);

/**
 * @brief destroys a keyboard FsM and frees memory
 * 
 * @param *p_fsm pointer to an fsm_keyboard_t struct
 */
void fsm_keyboard_destroy(fsm_keyboard_t *p_fsm);

/**
 * @brief fires the keyboard fsm, checks the transitions and execute actions from FSM
 * 
 * @param *p_fsm pointer to an fsm_keyboard_t struct
 */
void fsm_keyboard_fire(fsm_keyboard_t *p_fsm);

/**
 * @brief start keyboard scanning
 * 
 * @param *p_fsm pointer to an fsm_keyboard_t struct
 */
void fsm_keyboard_start_scan(fsm_keyboard_t *p_fsm);

/**
 * @brief stops keyboard scanning
 * 
 * @param *p_fsm pointer to an fsm_keyboard_t struct
 */
void fsm_keyboard_stop_scan(fsm_keyboard_t *p_fsm);

/**
 * @brief return key pressed
 * 
 * @param *p_fsm pointer to an fsm_keyboard_t struct
 * @return char Key pressed of the last keyboard press
 */
char fsm_keyboard_get_key_value(fsm_keyboard_t *p_fsm);

/**
 * @brief checks if the key pressed is valid
 * 
 * @param *p_fsm pointer to an fsm_keyboard_t struct
 * @return true if key_preesed is different from invalid_key
 */
bool fsm_keyboard_get_is_valid_key(fsm_keyboard_t *p_fsm);

/**
 * @brief reset the key presse
 * 
 * @param *p_fsm pointer to an fsm_keyboard_t struct
 */
void fsm_keyboard_reset_key_value(fsm_keyboard_t *p_fsm);

/**
 * @brief checks if FSM is active. FSM always inactive because we are working with events
 * 
 * @param *p_fsm pointer to an fsm_keyboard_t struct
 * @return false always
 */
bool fsm_keyboard_check_activity(fsm_keyboard_t *p_fsm);
#endif /* FSM_KEYBOARD_H_ */
