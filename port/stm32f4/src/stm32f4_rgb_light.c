/**
 * @file stm32f4_rgb_light.c
 * @brief Implementation of the RGB light port for STM32F4.
 * @author Mario Medina
 * @author Alejandro García
 * @date 2026-04-25
 */

/* Standard C includes */
#include <stdint.h>
#include <stddef.h>
#include <math.h>

/* HW dependent includes */
#include "port_rgb_light.h"

/* Microcontroller dependent includes */
#include "stm32f4_rgb_light.h"
#include "stm32f4xx.h"

/* Global variables */
/**
 * @brief Variable global con la configuración física de los LEDs RGB.
 */
stm32f4_rgb_light_hw_t stm32f4_rgb_light_hw[] = {
    [PORT_RGB_LIGHT_ID] = {
        .p_port_red   = STM32F4_RGB_LIGHT_R_GPIO,
        .pin_red      = STM32F4_RGB_LIGHT_R_PIN,
        .p_port_green = STM32F4_RGB_LIGHT_G_GPIO,
        .pin_green    = STM32F4_RGB_LIGHT_G_PIN,
        .p_port_blue  = STM32F4_RGB_LIGHT_B_GPIO,
        .pin_blue     = STM32F4_RGB_LIGHT_B_PIN,
        .p_timer      = TIM4
    }
};

/**
 * @brief Número de elementos en el array de hardware.
 */
#define NUM_RGB_LIGHTS (sizeof(stm32f4_rgb_light_hw) / sizeof(stm32f4_rgb_light_hw[0]))

/* Private functions -----------------------------------------------------------*/

/**
 * @brief Obtiene el puntero a la estructura de hardware dado un ID.
 */
static stm32f4_rgb_light_hw_t *_stm32f4_rgb_light_get(uint8_t rgb_light_id)
{
    if (rgb_light_id < NUM_RGB_LIGHTS) {
        return &stm32f4_rgb_light_hw[rgb_light_id];
    }
    return NULL;
}

/**
 * @brief Configura el temporizador para generar señales PWM.
 */
static void _timer_pwm_config(uint8_t rgb_light_id)
{
    stm32f4_rgb_light_hw_t *hw = _stm32f4_rgb_light_get(rgb_light_id);
    if (hw == NULL) return;

    /* 1. Habilitar reloj del temporizador (APB1 para TIM4) */
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

    /* 2. Configurar base de tiempos (50Hz) */
    hw->p_timer->PSC = 1599; 
    hw->p_timer->ARR = 199;
    hw->p_timer->CNT = 0;   /* Requisito del test: CNT reseteado */

    /* 3. Auto-reload preload enable (Requisito del test) */
    hw->p_timer->CR1 |= TIM_CR1_ARPE;

    /* 4. Configurar canales en Modo PWM 1 (OCxM = 110) y Preload Enable */
    /* Canal 1 (Rojo) */
    hw->p_timer->CCMR1 &= ~(TIM_CCMR1_OC1M | TIM_CCMR1_OC1PE);
    hw->p_timer->CCMR1 |= (0x6 << TIM_CCMR1_OC1M_Pos) | TIM_CCMR1_OC1PE;

    /* Canales 3 (Verde) y 4 (Azul) */
    hw->p_timer->CCMR2 &= ~(TIM_CCMR2_OC3M | TIM_CCMR2_OC3PE | TIM_CCMR2_OC4M | TIM_CCMR2_OC4PE);
    hw->p_timer->CCMR2 |= (0x6 << TIM_CCMR2_OC3M_Pos) | TIM_CCMR2_OC3PE;
    hw->p_timer->CCMR2 |= (0x6 << TIM_CCMR2_OC4M_Pos) | TIM_CCMR2_OC4PE;

    /* 5. El test exige que el Timer NO esté habilitado (CEN=0) ni las salidas (CCER=0) tras init */
    hw->p_timer->CR1 &= ~TIM_CR1_CEN;
    hw->p_timer->CCER &= ~(TIM_CCER_CC1E | TIM_CCER_CC3E | TIM_CCER_CC4E);

    /* 6. Generar evento de actualización para cargar PSC y ARR */
    hw->p_timer->EGR |= TIM_EGR_UG;
}

/* Public functions -----------------------------------------------------------*/

void port_rgb_light_init(uint8_t rgb_light_id)
{
    stm32f4_rgb_light_hw_t *hw = _stm32f4_rgb_light_get(rgb_light_id);
    if (hw == NULL) return;
    
    /* 1. Habilitar reloj del puerto GPIOB */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    /* 2. Configurar pines: Modo AF (10) y No-Pull (00) */
    /* Rojo (PB6) */
    hw->p_port_red->MODER &= ~(3 << (hw->pin_red * 2));
    hw->p_port_red->MODER |=  (2 << (hw->pin_red * 2));
    hw->p_port_red->PUPDR &= ~(3 << (hw->pin_red * 2));

    /* Verde (PB8) */
    hw->p_port_green->MODER &= ~(3 << (hw->pin_green * 2));
    hw->p_port_green->MODER |=  (2 << (hw->pin_green * 2));
    hw->p_port_green->PUPDR &= ~(3 << (hw->pin_green * 2));

    /* Azul (PB9) */
    hw->p_port_blue->MODER &= ~(3 << (hw->pin_blue * 2));
    hw->p_port_blue->MODER |=  (2 << (hw->pin_blue * 2));
    hw->p_port_blue->PUPDR &= ~(3 << (hw->pin_blue * 2));

    /* 3. Configurar AF2 (TIM4) en registros AFR */
    hw->p_port_red->AFR[0] &= ~(0xF << (hw->pin_red * 4));
    hw->p_port_red->AFR[0] |=  (0x2 << (hw->pin_red * 4));

    hw->p_port_green->AFR[1] &= ~(0xF << ((hw->pin_green - 8) * 4));
    hw->p_port_green->AFR[1] |=  (0x2 << ((hw->pin_green - 8) * 4));

    hw->p_port_blue->AFR[1] &= ~(0xF << ((hw->pin_blue - 8) * 4));
    hw->p_port_blue->AFR[1] |=  (0x2 << ((hw->pin_blue - 8) * 4));

    /* 4. Inicializar temporizador */
    _timer_pwm_config(rgb_light_id);
}

void port_rgb_light_set_rgb(uint8_t rgb_light_id, rgb_color_t color)
{
    stm32f4_rgb_light_hw_t *hw = _stm32f4_rgb_light_get(rgb_light_id);
    if (hw == NULL) return;

    /* 1. Calcular CCR proporcional al ARR (Fórmula exacta del test: (color * ARR) / 255) */
    hw->p_timer->CCR1 = (uint32_t)(color.r * hw->p_timer->ARR) / COLOR_RGB_MAX_VALUE;
    hw->p_timer->CCR3 = (uint32_t)(color.g * hw->p_timer->ARR) / COLOR_RGB_MAX_VALUE;
    hw->p_timer->CCR4 = (uint32_t)(color.b * hw->p_timer->ARR) / COLOR_RGB_MAX_VALUE;

    /* 2. El test requiere habilitar el Timer (CEN) y las salidas (CCER) AQUÍ */
    hw->p_timer->CCER |= (TIM_CCER_CC1E | TIM_CCER_CC3E | TIM_CCER_CC4E);
    hw->p_timer->CR1  |= TIM_CR1_CEN;
}