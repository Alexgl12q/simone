/**
 * @file stm32f4_keyboard.h
 * @brief Header for stm32f4_keyboard.c file.
 * @author Mario Medina
 * @author Alejandro Garcia
 * @date date
 */
#ifndef STM32F4_KEYBOARD_H_
#define STM32F4_KEYBOARD_H_

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdint.h>
#include <stdbool.h>

/* HW dependent includes */
#include "stm32f4xx.h"
#include "keyboards.h"
/* Defines and enums ----------------------------------------------------------*/
/* Defines */
//ROW PINS CONFIG (Output)
#define STM32F4_KEYBOARD_MAIN_ROW_0_GPIO GPIOA
#define STM32F4_KEYBOARD_MAIN_ROW_0_PIN  0
#define STM32F4_KEYBOARD_MAIN_ROW_1_GPIO GPIOA
#define STM32F4_KEYBOARD_MAIN_ROW_1_PIN  1
#define STM32F4_KEYBOARD_MAIN_ROW_2_GPIO GPIOA
#define STM32F4_KEYBOARD_MAIN_ROW_2_PIN  4
#define STM32F4_KEYBOARD_MAIN_ROW_3_GPIO GPIOB
#define STM32F4_KEYBOARD_MAIN_ROW_3_PIN  0
//COLUMN PINS CONFIG (Input)
#define STM32F4_KEYBOARD_MAIN_COL_0_GPIO GPIOA
#define STM32F4_KEYBOARD_MAIN_COL_0_PIN  8
#define STM32F4_KEYBOARD_MAIN_COL_1_GPIO GPIOB
#define STM32F4_KEYBOARD_MAIN_COL_1_PIN  10
#define STM32F4_KEYBOARD_MAIN_COL_2_GPIO GPIOB
#define STM32F4_KEYBOARD_MAIN_COL_2_PIN  4
#define STM32F4_KEYBOARD_MAIN_COL_3_GPIO GPIOB
#define STM32F4_KEYBOARD_MAIN_COL_3_PIN  5

/**
 * @brief Estructura que mapea el hardware real con la lógica de la FSM.
 */
typedef struct {
    const keyboard_t *p_keyboard;
    GPIO_TypeDef **p_row_ports;      /**< Array de punteros a los puertos de las filas */
    uint8_t *p_row_pins;             /**< Array de máscaras de pins de las filas */
    GPIO_TypeDef **p_col_ports;      /**< Array de punteros a los puertos de las columnas */
    uint8_t *p_col_pins;             /**< Array de máscaras de pins (ej. 0x0100) */
    bool flag_key_pressed;           /**< 'true' si una columna detectó un pulso */
    bool flag_row_timeout;           /**< 'true' si el Timer 5 pide cambio de fila */
    uint8_t col_idx_interrupt;       /**< Fila que está alimentada actualmente (0-3) */
    uint8_t current_excited_row;     /**< Columna capturada en la interrupción */
} stm32f4_keyboard_hw_t;

/**
 * @brief Array containing characteristics of keyboards in the system
 */
extern stm32f4_keyboard_hw_t keyboards_arr[];
/* Function prototypes and explanation -------------------------------------------------*/


#endif /* STM32F4_KEYBOARD_H_ */
