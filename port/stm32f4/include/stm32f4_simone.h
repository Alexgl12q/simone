/**
 * @file stm32f4_simone.h
 * @brief Header for stm32f4_simone.c file.
 * @author Alejandro Garcia
 * @author Mario Medina
 * @date 2026-05-04
 */

#ifndef STM32F4_SIMONE_H_
#define STM32F4_SIMONE_H_

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdint.h>
#include <stdbool.h>

/* HW dependent includes */
#include "stm32f4xx.h"

/* Typedefs --------------------------------------------------------------------*/
/**
 * @brief Structure to define the HW dependencies of a Simone's timer status.
 */
typedef struct
{
    bool flag_timer_timeout; /*!< Flag to indicate that the timer passed */
} stm32f4_simone_hw_t;

/* Global variables ------------------------------------------------------------*/
/**
 * @brief Element that represents the HW characteristics of the Simone game.
 *
 * This is an **extern** variable that is defined in `stm32f4_simone.c`. 
 * Se utiliza para comunicar la ISR del temporizador con la lógica de la FSM.
 */
extern stm32f4_simone_hw_t simone_hw;

#endif /* STM32F4_SIMONE_H_ */