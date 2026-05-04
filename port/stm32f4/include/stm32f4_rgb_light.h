/**
 * @file stm32f4_rgb_light.h
 * @brief Header for stm32f4_rgb_light.c file.
 * @author Mario Medina
 * @author Alejandro Garcia
 * @date 2026-04-25
 */

#ifndef STM32F4_RGB_LIGHT_SYSTEM_H_
#define STM32F4_RGB_LIGHT_SYSTEM_H_

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdint.h>

/* HW dependent includes */
#include "stm32f4xx.h"

/* Defines and enums ----------------------------------------------------------*/
/* Defines */

/** * @brief GPIO and Pin definitions for RGB LED. 
 * These must match the Unity Test expectations.
 */
#define STM32F4_RGB_LIGHT_R_GPIO  GPIOB
#define STM32F4_RGB_LIGHT_R_PIN   6

#define STM32F4_RGB_LIGHT_G_GPIO  GPIOB
#define STM32F4_RGB_LIGHT_G_PIN   8

#define STM32F4_RGB_LIGHT_B_GPIO  GPIOB
#define STM32F4_RGB_LIGHT_B_PIN   9

/* Typedefs --------------------------------------------------------------------*/

/**
 * @brief Estructura que define el HW del LED RGB para STM32F4.
 * Contiene los puertos y pines de cada GPIO asociada a los colores.
 */
typedef struct {
    GPIO_TypeDef *p_port_red;   /**< Puerto GPIO del LED rojo */
    uint16_t pin_red;           /**< Pin GPIO del LED rojo */
    
    GPIO_TypeDef *p_port_green; /**< Puerto GPIO del LED verde */
    uint16_t pin_green;         /**< Pin GPIO del LED verde */
    
    GPIO_TypeDef *p_port_blue;  /**< Puerto GPIO del LED azul */
    uint16_t pin_blue;          /**< Pin GPIO del LED azul */
    
    TIM_TypeDef *p_timer;       /**< Puntero al temporizador (TIM4) */
} stm32f4_rgb_light_hw_t;

/* Global variables --------------------------------------------------------*/

/**
 * @brief Array que contiene las configuraciones de hardware de todos los LED RGB del sistema.
 * Declarado como extern para ser definido en el fichero .c
 */
extern stm32f4_rgb_light_hw_t stm32f4_rgb_light_hw[];

#endif /* STM32F4_RGB_LIGHT_SYSTEM_H_ */