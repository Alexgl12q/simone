/**
 * @file fsm_rgb_light.h
 * @brief Header for fsm_rgb_light.c file.
 * @author Mario Medina
 * @author Alejandro García
 * @date 25 de Abril de 2026
 */

#ifndef FSM_RGB_LIGHT_SYSTEM_H_
#define FSM_RGB_LIGHT_SYSTEM_H_

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdint.h>
#include <stdbool.h>

/* Project includes */
#include "fsm.h"          /* Librería de la FSM genérica */
#include "rgb_colors.h"   /* Definición de rgb_color_t */

/* Defines and enums ----------------------------------------------------------*/

/**
 * @brief Enumerado con los nombres de los estados de la FSM.
 */
typedef enum {
    IDLE_RGB = 0,   /**< Estado de espera/apagado (IDLE_RGB) */
    SET_COLOR       /**< Estado de actualización de color e intensidad (SET_COLOR) */
} FSM_RGB_LIGHT_SYSTEM;

/* Defines --------------------------------------------------------------------*/

/**
 * @brief Máximo nivel de intensidad porcentual (100%).
 */
#define MAX_LEVEL_INTENSITY 100

/* Typedefs --------------------------------------------------------------------*/

/**
 * @brief Estructura de la FSM del RGB light.
 * @note El campo fsm_t DEBE ser el primero para que funcione el motor de la FSM.
 */
typedef struct {
    fsm_t f;                /**< Objeto FSM base */
    uint8_t rgb_light_id;   /**< Identificador del hardware LED RGB */
    
    rgb_color_t color;      /**< Color base a representar */
    uint8_t intensity;      /**< Nivel de intensidad en porcentaje (0-100) */
    
    bool status;            /**< Campo status: indica si el RGB light está activo */
    bool new_color;         /**< Flag new_color: indica cambio de color solicitado */
    bool idle;              /**< Flag idle: indica modo ocioso (ahorro de energía) */
} fsm_rgb_light_t;

/* Function prototypes and explanation -------------------------------------------------*/

/**
 * @brief Crea y reserva memoria para una nueva FSM del LED RGB.
 * @param rgb_light_id Identificador del LED (gestionado en el PORT).
 * @return fsm_rgb_light_t* Puntero a la FSM creada.
 */
fsm_rgb_light_t *fsm_rgb_light_new(uint8_t rgb_light_id);

/**
 * @brief Inicializa la estructura de la FSM y el hardware asociado.
 * @param p_fsm Puntero a la estructura de la FSM específica.
 * @param rgb_light_id ID del LED RGB.
 */
void fsm_rgb_light_init(fsm_rgb_light_t *p_fsm, uint8_t rgb_light_id);

/**
 * @brief Lanza una iteración de la máquina de estados.
 * @param p_fsm Puntero a la estructura de la FSM específica.
 */
void fsm_rgb_light_fire(fsm_rgb_light_t *p_fsm);

/**
 * @brief Libera la memoria de la FSM.
 * @param p_fsm Puntero a la estructura de la FSM específica.
 */
void fsm_rgb_light_destroy(fsm_rgb_light_t *p_fsm);

/**
 * @brief Modifica el campo status de la FSM.
 * @param p_fsm Puntero a la estructura de la FSM específica.
 * @param status Nuevo estado de actividad (true activo, false inactivo).
 */
void fsm_rgb_light_set_status(fsm_rgb_light_t *p_fsm, bool status);

/**
 * @brief Devuelve el valor del campo status.
 * @param p_fsm Puntero a la estructura de la FSM específica.
 * @return Estado actual del flag status.
 */
bool fsm_rgb_light_get_status(fsm_rgb_light_t *p_fsm);

/**
 * @brief Actualiza color e intensidad y activa el flag new_color.
 * @param p_fsm Puntero a la estructura de la FSM específica.
 * @param color Nuevo color rgb_color_t.
 * @param intensity Nueva intensidad en porcentaje (0-100).
 */
void fsm_rgb_light_set_color_intensity(fsm_rgb_light_t *p_fsm, rgb_color_t color, uint8_t intensity);

/**
 * @brief Comprueba si el LED está activo (no está en idle).
 * @param p_fsm Puntero a la estructura de la FSM específica.
 * @return true si NO está en idle (hay actividad), false si está ocioso.
 */
bool fsm_rgb_light_check_activity(fsm_rgb_light_t *p_fsm);

#endif /* FSM_RGB_LIGHT_SYSTEM_H_ */