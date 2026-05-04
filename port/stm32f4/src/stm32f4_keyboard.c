/**
 * @file stm32f4_keyboard.c
 * @brief Portable functions to interact with the keyboard FSM library. All portable functions must be implemented in this file.
 * @author alumno1
 * @author alumno2
 * @date date
 */

/* Standard C includes */
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>


/* HW dependent includes */
#include "port_keyboard.h"
#include "port_system.h"
#include "stm32f4xx.h"

/* Microcontroller dependent includes */
#include "stm32f4_system.h"
#include "stm32f4_keyboard.h"
#include "keyboards.h"
/* Typedefs --------------------------------------------------------------------*/

/* Global variables */
/* Static arrays for main keyboard (pointed by the double pointers in the struct) */

/**
 * @brief Array que contiene el estado y mapeo del hardware.
 */
stm32f4_keyboard_hw_t keyboards_arr[1] = {
    {
        .p_keyboard = &standard_keyboard,
        .p_row_ports = NULL,
        .p_row_pins = NULL,
        .p_col_ports = NULL,
        .p_col_pins = NULL,
        .flag_key_pressed = false,
        .flag_row_timeout = false,
        .col_idx_interrupt = 0,
        .current_excited_row = 0xFF
    }
};

/**
 * @brief Array of GPIO ports for the rows of the main keyboard.
 *
 */
static GPIO_TypeDef *keyboard_main_row_ports[] = {
    STM32F4_KEYBOARD_MAIN_ROW_0_GPIO,
    STM32F4_KEYBOARD_MAIN_ROW_1_GPIO,
    STM32F4_KEYBOARD_MAIN_ROW_2_GPIO,
    STM32F4_KEYBOARD_MAIN_ROW_3_GPIO};

/**
 * @brief Array of GPIO pins for the rows of the main keyboard.
 *
 */
static uint8_t keyboard_main_row_pins[] = {
    STM32F4_KEYBOARD_MAIN_ROW_0_PIN,
    STM32F4_KEYBOARD_MAIN_ROW_1_PIN,
    STM32F4_KEYBOARD_MAIN_ROW_2_PIN,
    STM32F4_KEYBOARD_MAIN_ROW_3_PIN};

/**
 * @brief Array of GPIO ports for the columns of the main keyboard.
 *
 */
static GPIO_TypeDef *keyboard_main_col_ports[] = {
    STM32F4_KEYBOARD_MAIN_COL_0_GPIO,
    STM32F4_KEYBOARD_MAIN_COL_1_GPIO,
    STM32F4_KEYBOARD_MAIN_COL_2_GPIO,
    STM32F4_KEYBOARD_MAIN_COL_3_GPIO};

/**
 * @brief Array of GPIO pins for the columns of the main keyboard.
 *
 */
static uint8_t keyboard_main_col_pins[] = {
    STM32F4_KEYBOARD_MAIN_COL_0_PIN,
    STM32F4_KEYBOARD_MAIN_COL_1_PIN,
    STM32F4_KEYBOARD_MAIN_COL_2_PIN,
    STM32F4_KEYBOARD_MAIN_COL_3_PIN
};

/* Private functions ----------------------------------------------------------*/

/**
 * @brief Devuelve un puntero a la estructura HW del teclado solicitado.
 * 
 * @param keyboard_id
 */
static stm32f4_keyboard_hw_t* _stm32f4_keyboard_get(uint8_t keyboard_id) {
    return &keyboards_arr[keyboard_id];
}

/**
 * @brief Configure the timer that controls the duration of the column scanning.
 */
static void _timer_scan_column_config(void) {
    /*  1. Enable the clock of the timer (TIM5 is on APB1) */
    RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;

    /*  2. Disable the counter of the timer */
    TIM5->CR1 &= ~TIM_CR1_CEN;

    /*  3. Enable the autoreload preload */
    TIM5->CR1 |= TIM_CR1_ARPE;

    /*  4. Set the counter of the timer to 0 */
    TIM5->CNT = 0;

    /*  5 & 6. Compute PSC and ARR (Option 1. Efficient algorithm) */
    double sysclk_as_double = (double)SystemCoreClock; 
    double ms_as_double = (double)PORT_KEYBOARDS_TIMEOUT_MS; 
    
    /* Calcular los ticks totales necesarios para los milisegundos dados */
    double total_ticks = (sysclk_as_double / 1000.0) * ms_as_double;
    
    /* 6b. Calcular PSC inicial asumiendo ARR máximo (65535.0) */
    double psc_val = round(total_ticks / 65535.0);
    if (psc_val < 1.0) psc_val = 1.0; /* Protección para no dividir por 0 */
    
    /* 6c. Recalcular ARR con el PSC obtenido */
    double arr_val = round(total_ticks / psc_val);
     
    /* 6d. Check if ARR is greater than 0xFFFF */
    if (arr_val > 65535.0) {
        psc_val += 1.0;
        arr_val = round(total_ticks / psc_val);
    }

    /*  6. Load the values computed (Restando 1 porque los registros cuentan el 0) */
    TIM5->PSC = (uint32_t)psc_val - 1;
    TIM5->ARR = (uint32_t)arr_val - 1;

    /*  7. Generate an update event to load preload registers */
    TIM5->EGR |= TIM_EGR_UG;

    /*  8. Clear the update interrupt flag */
    TIM5->SR &= ~TIM_SR_UIF;

    /*  9. Enable the interrupts of the timer */
    TIM5->DIER |= TIM_DIER_UIE;

    /*  10. Set the priority of the timer interrupt (Level 2, Subpriority 0) */
    NVIC_SetPriority(TIM5_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 2, 0));
}

/* Public functions -----------------------------------------------------------*/
void port_keyboard_init(uint8_t keyboard_id)
{
    /* Get the keyboard sensor */
    stm32f4_keyboard_hw_t *p_keyboard = _stm32f4_keyboard_get(keyboard_id);

    /* TO-DO alumnos: */
    /* Habilitar relojes de los puertos y de SYSCFG */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN;
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    /* Enlazar los punteros de la estructura a los arrays estáticos */
    p_keyboard->p_row_ports = keyboard_main_row_ports;
    p_keyboard->p_row_pins = keyboard_main_row_pins;
    p_keyboard->p_col_ports = keyboard_main_col_ports;
    p_keyboard->p_col_pins = keyboard_main_col_pins;

    /* Rows configuration */
    for(uint8_t i = 0; i < p_keyboard->p_keyboard->num_rows; i++) {
        /* Configurar como salida (OUTPUT) sin Pull-up/down (NO_PULL) */
        stm32f4_system_gpio_config(p_keyboard->p_row_ports[i], p_keyboard->p_row_pins[i], STM32F4_GPIO_MODE_OUT, STM32F4_GPIO_PUPDR_NOPULL);
    }
    /* Columns configuration */
    for(uint8_t i = 0; i < p_keyboard->p_keyboard->num_cols; i++) {
        /* Configurar como entrada (INPUT) con Pull-Down (PULL_DOWN) */
        stm32f4_system_gpio_config(p_keyboard->p_col_ports[i], p_keyboard->p_col_pins[i], STM32F4_GPIO_MODE_IN, STM32F4_GPIO_PUPDR_PULLDOWN);
        
        /* Configurar interrupción en ambos flancos (RISING_FALLING) */
        stm32f4_system_gpio_config_exti(p_keyboard->p_col_ports[i], p_keyboard->p_col_pins[i], (STM32F4_TRIGGER_BOTH_EDGE | STM32F4_TRIGGER_ENABLE_INTERR_REQ));
        
        /* Habilitar la línea EXTI con prioridad 1, subprioridad 1 */
        stm32f4_system_gpio_exti_enable(p_keyboard->p_col_pins[i], 1, 1);
    }

    /* Clean/set all configurations */
    NVIC_SetPriority(EXTI4_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 1, 1));
    NVIC_EnableIRQ(EXTI4_IRQn);
    NVIC_SetPriority(EXTI9_5_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 1, 1));
    NVIC_EnableIRQ(EXTI9_5_IRQn);
    NVIC_SetPriority(EXTI15_10_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 1, 1));
    NVIC_EnableIRQ(EXTI15_10_IRQn);

    /* Configure timer */
    _timer_scan_column_config();
    p_keyboard->current_excited_row = 0xFF;
}

void port_keyboard_excite_row(uint8_t keyboard_id, uint8_t row_idx) {
    stm32f4_keyboard_hw_t *p_hw = _stm32f4_keyboard_get(keyboard_id);
    
    /*  2. Iterate through all rows and set them to LOW */
    for(uint8_t i = 0; i < p_hw->p_keyboard->num_rows; i++) {
        p_hw->p_row_ports[i]->BSRR = (1U << (p_hw->p_row_pins[i] + 16)); 
    }
    
    /*  3. Set the given row to HIGH */
    p_hw->p_row_ports[row_idx]->BSRR = (1U << p_hw->p_row_pins[row_idx]);
}

void port_keyboard_excite_next_row(uint8_t keyboard_id) {
    stm32f4_keyboard_hw_t *p_hw = _stm32f4_keyboard_get(keyboard_id);
    
    /* ✅ 1. Update current_excited_row using modulo */
    p_hw->current_excited_row = (p_hw->current_excited_row + 1) % p_hw->p_keyboard->num_rows;
    
    /* ✅ 2. Call function to excite the new row */
    port_keyboard_excite_row(keyboard_id, p_hw->current_excited_row);
}

void port_keyboard_start_scan(uint8_t keyboard_id) {
    stm32f4_keyboard_hw_t *p_hw = _stm32f4_keyboard_get(keyboard_id);
    
    /* ✅ 1. Reset the flag_row_timeout */
    p_hw->flag_row_timeout = false;
    
    /* ✅ 2. Reset the counter of the timer */
    TIM5->CNT = 0; 
    
    /* ✅ 3. Set the first row to be excited to HIGH */
    p_hw->current_excited_row = 0;
    port_keyboard_excite_row(keyboard_id, 0);
    
    /* ✅ 4. Enable the timer interrupt in the NVIC */
    NVIC_EnableIRQ(TIM5_IRQn);
    
    /* ✅ 5. Enable the counter of the timer */
    TIM5->CR1 |= TIM_CR1_CEN; 
}

void port_keyboard_stop_scan(uint8_t keyboard_id) {
    stm32f4_keyboard_hw_t *p_hw = _stm32f4_keyboard_get(keyboard_id);
    
    /* ✅ 1. Disable the counter of the timer */
    TIM5->CR1 &= ~TIM_CR1_CEN; 
    
    /* ✅ 2. Disable the timer interrupt in the NVIC */
    NVIC_DisableIRQ(TIM5_IRQn);
    
    /* ✅ 3. Set all rows to LOW */
    for(uint8_t i = 0; i < p_hw->p_keyboard->num_rows; i++) {
        p_hw->p_row_ports[i]->BSRR = (1U << (p_hw->p_row_pins[i] + 16)); 
    }
}

bool port_keyboard_get_key_pressed_status(uint8_t keyboard_id) { 
    return _stm32f4_keyboard_get(keyboard_id)->flag_key_pressed; 
}

void port_keyboard_set_key_pressed_status(uint8_t keyboard_id, bool status) { 
    _stm32f4_keyboard_get(keyboard_id)->flag_key_pressed = status; 
}

bool port_keyboard_get_row_timeout_status(uint8_t keyboard_id) { 
    return _stm32f4_keyboard_get(keyboard_id)->flag_row_timeout; 
}

void port_keyboard_set_row_timeout_status(uint8_t keyboard_id, bool status) { 
    _stm32f4_keyboard_get(keyboard_id)->flag_row_timeout = status; 
}

char port_keyboard_get_invalid_key_value(uint8_t keyboard_id) { 
    return _stm32f4_keyboard_get(keyboard_id)->p_keyboard->null_key; 
}

char port_keyboard_get_key_value(uint8_t keyboard_id) {
    stm32f4_keyboard_hw_t *p_hw = _stm32f4_keyboard_get(keyboard_id);
    
    /*  Determine the value (char) using the flat array formula */
    uint8_t key_index = (p_hw->current_excited_row * p_hw->p_keyboard->num_cols) + p_hw->col_idx_interrupt;
    
    return p_hw->p_keyboard->keys[key_index];
}
