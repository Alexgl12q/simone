/**
 * @file port_keyboard.h
 * @brief Header for the portable functions to interact with the HW of the keyboards. The functions must be implemented in the platform-specific code.
 * @author Alejandro Garcia
 * @author Mario Medina
 * @date fecha
 */
#ifndef PORT_KEYBOARD_H_
#define PORT_KEYBOARD_H_

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdint.h>
#include <stdbool.h>

/* Defines and enums ----------------------------------------------------------*/
/** @brief Main keyboard for the game */
#define PORT_KEYBOARD_MAIN_ID 0

/** @brief Timeout for row excitation (25 ms following manual) */
#define PORT_KEYBOARDS_TIMEOUT_MS 25

/** @brief Debounce time for all keys */
#define PORT_KEYBOARD_MAIN_DEBOUNCE_TIME_MS 150

/** @brief Ennumeration of column indexes */
enum PORT_KEYBOARD_COL_IDS{
    PORT_KEYBOARD_COL_0,
    PORT_KEYBOARD_COL_1,
    PORT_KEYBOARD_COL_2,
    PORT_KEYBOARD_COL_3
};

/* Function prototypes and explanation -------------------------------------------------*/
/**
 * @brief Configure the HW specifications of a given keyboard.
 * 
 * @param keyboard_id Keyboard ID. This index is used to select the element of the keyboards_arr[] array

 */
void port_keyboard_init (uint8_t keyboard_id);

/**
 * @brief Set the given row to high and lower the others
 * 
 * @param keyboard_id ID of the keyboard to be scanned
 * @param row_idx Index of the row to be excited
 */
void port_keyboard_excite_row(uint8_t keyboard_id, uint8_t row_idx);

/**
 * @brief Start the scanning of a keyboard by enabling the timer that controls the duration of column scanning
 * 
 * @param keyboard_id
 */
void port_keyboard_start_scan (uint8_t keyboard_id);

/**
 * @brief Stop the scanning of a keyboard
 * 
 * @param keyboard_id
 */
void port_keyboard_stop_scan (uint8_t keyboard_id);

/**
 * @brief Update the row to be excited
 * 
 * @param keyboard_id
 */
void port_keyboard_excite_next_row (uint8_t keyboard_id);

/**
 * @brief Return the status of the keyboard (pressed or not)
 * 
 * @param keyboard_id
 * @return true if the keyboard has been pressed, false otherwise
 */
bool port_keyboard_get_key_pressed_status (uint8_t keyboard_id);

/**
 * @brief Set the status of the keyboard (pressed or not)
 * This function is used to update the status of the keyboard when a key press is detected or cleared
 * 
 * @param keyboard_id
 * @param status 
 */
void port_keyboard_set_key_pressed_status (uint8_t keyboard_id, bool status);

/**
 * @brief Return the status of the column timeout flag
 * 
 * @param keyboard_id
 * @return true if the column timeout has occured, false otherwise
 */
bool port_keyboard_get_row_timeout_status (uint8_t keyboard_id);

/**
 * @brief Set the status of the row timeout flag
 * This function is used to update the status of the row timeout flag when a timeout occurs or is cleared
 * 
 * @param keyboard_id
 * @param status New status of the row timeout flag
 */
void port_keyboard_set_row_timeout_status (uint8_t keyboard_id, bool status);

/**
 * @brief Return the char representing the key pressed of a given keyboard based on its row that is being excited,
 * assuming that the matrix of chars is flattened (i.e., it is not a 2D array, but all rows are in a single array), 
 * thus it is necessary to calculate only one index
 * 
 * @param keyboard_id
 * @return char Key value of the key pressed
 */
char port_keyboard_get_key_value (uint8_t keyboard_id);

/**
 * @brief Return the null key value of a given keyboard,
 * with the null value defined in the keyboard layout
 * 
 * @return char
 */
char port_keyboard_get_invalid_key_value (uint8_t keyboard_id);

#endif /* PORT_KEYBOARD_H_ */
