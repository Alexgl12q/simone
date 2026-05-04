/**
 * @file stm32f4_button.c
 * @brief Portable functions to interact with the button FSM library. All portable functions must be implemented in this file.
 * @author Alejandro García
 * @author Mario Medina
 * @date fecha
 */

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */

/* HW dependent includes */
#include "port_button.h" // Used to get general information about the buttons (ID, etc.)
#include "port_system.h" // Used to get the system tick

/* Microcontroller dependent includes */
// TO-DO alumnos: include the necessary files to interact with the GPIOs
#include <stddef.h>
#include "stm32f4xx.h"
#include "stm32f4_system.h"
#include "stm32f4_button.h"

/* Global variables ------------------------------------------------------------*/
/**
 * @brief Array of elements that represents the HW characteristics of the buttons
 * This is an **extern** variable that is declared in `stm32f4_button.h`. It represents an array of hardware buttons.
 */
stm32f4_button_hw_t buttons_arr[] = {
    [PORT_USER_BUTTON_ID]={.p_port = STM32F4_USER_BUTTON_GPIO,
                           .pin = STM32F4_USER_BUTTON_PIN,
                           .pupd_mode = STM32F4_GPIO_PUPDR_NOPULL
                           },
};


/* Private functions ----------------------------------------------------------*/
/**
 * @brief Get the button status struct with the given ID.
 *
 * @param button_id Button ID.
 *
 * @return Pointer to the button state struct.
 * @return NULL If the button ID is not valid.
 */
stm32f4_button_hw_t *_stm32f4_button_get(uint8_t button_id)
{
    // Return the pointer to the button with the given ID. If the ID is not valid, return NULL.
    if (button_id < sizeof(buttons_arr) / sizeof(buttons_arr[0]))
    {
        return &buttons_arr[button_id];
    }
    else
    {
        return NULL;
    }
}

/* Public functions -----------------------------------------------------------*/

/**
 * @brief Inicialize the button with the given ID.
 * 
 * @param button_id Identifier of the button to initialize.
 * @return void No return value.
 */
void port_button_init(uint8_t button_id)
{
    // Retrieve the button struct using the private function and the button ID
    stm32f4_button_hw_t *p_button = _stm32f4_button_get(button_id);

    /* TO-DO alumnos */
    if (p_button != NULL)
    {
        //Configure GPIO pin as input with the corresponding pull up/down configuration 
        stm32f4_system_gpio_config(p_button->p_port, p_button->pin, STM32F4_GPIO_MODE_IN, p_button->pupd_mode);
        //Confifure EXTI for the corresponding GPIO pin to trigger an interrupt on both edges
        stm32f4_system_gpio_config_exti(p_button->p_port, p_button->pin, STM32F4_TRIGGER_BOTH_EDGE | STM32F4_TRIGGER_ENABLE_INTERR_REQ);
        //Enable interrut, prio 1 and subprio 0
        stm32f4_system_gpio_exti_enable(p_button->pin, 1, 0);
    }
}

/**
 * @brief Read the button state.
 * 
 * @param button_id Identifier of the button to read.
 * @return true If the button is pressed.
 * @return false If the button is not pressed.
 */
bool port_button_get_pressed(uint8_t button_id)
{
    bool pressed = false;
    stm32f4_button_hw_t *p_button = _stm32f4_button_get(button_id);

    // If button not NULL, read the flag_pressed and clean it if it is true
    if(p_button != NULL)
    {
       pressed = p_button->flag_pressed;
    }
    return pressed;
}




