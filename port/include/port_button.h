/**
 * @file port_button.h
 * @brief Header for the portable functions to interact with the HW of the buttons. The functions must be implemented in the platform-specific code.
 * @author Mario Medina
 * @author Alejandro García
 */

#ifndef PORT_BUTTON_H_
#define PORT_BUTTON_H_

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdint.h>
#include <stdbool.h>

/* Defines and enums ----------------------------------------------------------*/
/* Defines */
// Define here all the button identifiers that are used in the system

#define PORT_USER_BUTTON_ID 0
#define PORT_USER_BUTTON_DEBOUNCE_TIME_MS 150

/* Function prototypes and explanation -------------------------------------------------*/

/**
 * @brief Inicialize the button with the given ID.
 * 
 * @param button_id Identifier of the button to initialize.
 * @return void.
 */
void port_button_init(uint8_t button_id);

/**
 * @brief Read the button state.
 * 
 * @param button_id Identifier of the button to read.
 * @return true If the button is pressed.
 * @return false If the button is not pressed.
 */
bool port_button_get_pressed(uint8_t button_id);
#endif