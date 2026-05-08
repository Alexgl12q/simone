/**
 * @file fsm_rgb_light.c
 * @brief RGB light system FSM main file.
 * @author Mario Medina
 * @author Alejandro García
 * @date 2026-05-04
 */

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/* HW dependent includes */
#include "port_rgb_light.h"

/* Project includes */
#include "fsm.h"
#include "fsm_rgb_light.h"

/* Typedefs --------------------------------------------------------------------*/

/* Private functions -----------------------------------------------------------*/

/**
 * @brief Calcula los niveles RGB corregidos aplicando la intensidad.
 * Multiplica cada componente por el factor de intensidad porcentual.
 * @param p_fsm Puntero a la estructura de la FSM.
 */
static void _correct_rgb_light_levels(fsm_rgb_light_t *p_fsm) 
{
    rgb_color_t corrected;
    
    /* La intensidad se trata como un valor entre 0 y 100 */
    corrected.r = (uint8_t)((p_fsm->color.r * p_fsm->intensity) / 100);
    corrected.g = (uint8_t)((p_fsm->color.g * p_fsm->intensity) / 100);
    corrected.b = (uint8_t)((p_fsm->color.b * p_fsm->intensity) / 100);

    /* Se actualiza el hardware a través del PORT */
    port_rgb_light_set_rgb(p_fsm->rgb_light_id, corrected);
}

/* State machine input or transition functions -------------------------------*/

/**
 * @brief Comprueba si el sistema está activo para encender el LED.
 * @param self Puntero a la FSM genérica.
 * @return true si status es verdadero.
 */
static int check_active(fsm_t *self) 
{
    fsm_rgb_light_t *p_fsm = (fsm_rgb_light_t *)(self);
    return p_fsm->status;
}

/**
 * @brief Comprueba si hay un nuevo color pendiente de representar.
 * @param self Puntero a la FSM genérica.
 * @return true si hay un cambio de color y el sistema sigue activo.
 */
static int check_set_new_color(fsm_t *self) 
{
    fsm_rgb_light_t *p_fsm = (fsm_rgb_light_t *)(self);
    return p_fsm->status && p_fsm->new_color;
}

/**
 * @brief Comprueba si el sistema se ha desactivado.
 * @param self Puntero a la FSM genérica.
 * @return true si status es falso.
 */
static int check_off(fsm_t *self) 
{
    fsm_rgb_light_t *p_fsm = (fsm_rgb_light_t *)(self);
    return !p_fsm->status;
}

/* State machine output or action functions ----------------------------------*/

/**
 * @brief Acción de encendido: aplica el color actual y limpia flags.
 */
static void do_set_on(fsm_t *self) 
{
    fsm_rgb_light_t *p_fsm = (fsm_rgb_light_t *)(self);
    _correct_rgb_light_levels(p_fsm);
    p_fsm->new_color = false;
    p_fsm->idle = true;
}

/**
 * @brief Acción de actualización: aplica el nuevo color y limpia flags.
 */
static void do_set_color(fsm_t *self) 
{
    fsm_rgb_light_t *p_fsm = (fsm_rgb_light_t *)(self);
    _correct_rgb_light_levels(p_fsm);
    p_fsm->new_color = false;
    p_fsm->idle = true;
}

/**
 * @brief Acción de apagado: pone el LED a negro y marca idle.
 */
static void do_set_off(fsm_t *self) 
{
    fsm_rgb_light_t *p_fsm = (fsm_rgb_light_t *)(self);
    rgb_color_t black = {0, 0, 0};
    port_rgb_light_set_rgb(p_fsm->rgb_light_id, black);
    p_fsm->new_color = false;
    p_fsm->idle = true;
}

/* Transition table ----------------------------------------------------------*/

static fsm_trans_t fsm_trans_rgb_light[] = {
    { IDLE_RGB,  check_active,        SET_COLOR, do_set_on    },
    { SET_COLOR, check_set_new_color, SET_COLOR, do_set_color },
    { SET_COLOR, check_off,           IDLE_RGB,  do_set_off   },
    { -1, NULL, -1, NULL }
};

/* Public functions -----------------------------------------------------------*/

fsm_rgb_light_t *fsm_rgb_light_new(uint8_t rgb_light_id) 
{
    fsm_rgb_light_t *p_fsm = (fsm_rgb_light_t *)malloc(sizeof(fsm_rgb_light_t));
    if (p_fsm != NULL) {
        fsm_rgb_light_init(p_fsm, rgb_light_id);
    }
    return p_fsm;
}

void fsm_rgb_light_init(fsm_rgb_light_t *p_fsm, uint8_t rgb_light_id) 
{
    fsm_init(&(p_fsm->f), fsm_trans_rgb_light);
    
    p_fsm->rgb_light_id = rgb_light_id;
    p_fsm->intensity = MAX_LEVEL_INTENSITY;
    p_fsm->status = false;
    p_fsm->new_color = false;
    p_fsm->idle = true;
    p_fsm->color = (rgb_color_t){0, 0, 0};

    port_rgb_light_init(rgb_light_id);
}

void fsm_rgb_light_fire(fsm_rgb_light_t *p_fsm) 
{
    fsm_fire(&(p_fsm->f));
}

void fsm_rgb_light_destroy(fsm_rgb_light_t *p_fsm) 
{
    free(p_fsm);
}

void fsm_rgb_light_set_status(fsm_rgb_light_t *p_fsm, bool status) 
{
    p_fsm->status = status;
    p_fsm->idle = false;
}

/**
 * @brief Establece color e intensidad y activa el sistema.
 * Requisito para la integración con FSM Simone.
 */
void fsm_rgb_light_set_color(fsm_rgb_light_t *p_fsm, rgb_color_t color, uint8_t intensity) 
{
    p_fsm->color = color;
    p_fsm->intensity = intensity;
    p_fsm->status = (color.r > 0 || color.g > 0 || color.b > 0);
    p_s->new_color = true;
    p_fsm->idle = false;
}

void fsm_rgb_light_set_color_intensity(fsm_rgb_light_t *p_fsm, rgb_color_t color, uint8_t intensity) 
{
    p_fsm->color = color;
    p_fsm->intensity = intensity;
    p_fsm->new_color = true;
    p_fsm->idle = false;
}

bool fsm_rgb_light_check_activity(fsm_rgb_light_t *p_fsm) 
{
    /* Se considera activo si el status es true y tiene pendiente procesar el cambio (idle false) 
       O si el LED está actualmente encendido. */
    return (p_fsm->status || !p_fsm->idle);
}