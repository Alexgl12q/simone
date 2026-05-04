/**
 * @file port_rgb_light.h
 * @brief Header for the portable functions to interact with the HW of the RGB light system.
 * @author Mario Medina
 * @author Alejandro García
 * @date 2026-04-25
 */
#ifndef PORT_RGB_LIGHT_SYSTEM_H_
#define PORT_RGB_LIGHT_SYSTEM_H_

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "rgb_colors.h" 

/* Defines --------------------------------------------------------------------*/

/**
 * @brief Identificador único para el LED RGB. 
 */
#define PORT_RGB_LIGHT_ID 0

/* Function prototypes --------------------------------------------------------*/

/**
 * @brief Inicializa el hardware necesario para el LED RGB.
 * @param rgb_light_id Identificador del LED (el test pasa un uint8_t).
 */
void port_rgb_light_init(uint8_t rgb_light_id);

/**
 * @brief Actualiza la intensidad de los canales del LED RGB.
 * @param rgb_light_id Identificador del LED.
 * @param color Estructura con valores R, G y B.
 */
void port_rgb_light_set_rgb(uint8_t rgb_light_id, rgb_color_t color);

#endif /* PORT_RGB_LIGHT_SYSTEM_H_ */