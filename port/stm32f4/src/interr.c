/**
 * @file interr.c
 * @brief Interrupt service routines for the STM32F4 platform.
 * @author SDG2. Román Cárdenas (r.cardenas@upm.es) and Josué Pagán (j.pagan@upm.es)
 * @date 2026-01-01
 */
// Include HW dependencies:
#include "port_system.h"
#include "stm32f4_system.h"
#include "stm32f4_button.h"
#include "port_button.h"
#include "stm32f4xx.h"

// Include headers of different port elements:
#include "port_keyboard.h"
#include "stm32f4_keyboard.h"
#include "port_simone.h"
#include "stm32f4_simone.h"
//------------------------------------------------------
// PRIVATE FUNCTIONS :)
//------------------------------------------------------

/**
 * @brief Comprueba y maneja la interrupcion de una columna
 * 
 * @param column_index
 */
static void _check_column_interrupt(uint8_t column_index)
{
    /* 1. Get the GPIO port and pin */
    GPIO_TypeDef *p_port = keyboards_arr[PORT_KEYBOARD_MAIN_ID].p_col_ports[column_index];
    uint8_t pin = keyboards_arr[PORT_KEYBOARD_MAIN_ID].p_col_pins[column_index];

    /* 2. Read the GPIO pin state */
    bool is_high = stm32f4_system_gpio_read(p_port, pin);

    /* 3. Update flag_key_pressed and col_idx_interrupt */
    if (is_high) {
        keyboards_arr[PORT_KEYBOARD_MAIN_ID].flag_key_pressed = true;
    } else {
        keyboards_arr[PORT_KEYBOARD_MAIN_ID].flag_key_pressed = false;
        keyboards_arr[PORT_KEYBOARD_MAIN_ID].col_idx_interrupt = column_index;
    }

    /* 4. Clear the interrupt pending flag */
    EXTI->PR = BIT_POS_TO_MASK(pin);
}

//------------------------------------------------------
// INTERRUPT SERVICE ROUTINES
//------------------------------------------------------
/**
 * @brief Interrupt service routine for the System tick timer (SysTick).
 */
void SysTick_Handler(void)
{
    uint32_t ms = port_system_get_millis();
    port_system_set_millis(ms + 1);
}

/**
 * @brief Interrupt service routine for the EXTI line 15 to 10.
 */
void EXTI15_10_IRQHandler(void)
{
    /* Reanuda el SysTick inmediatamente al detectar la pulsación */
    port_system_systick_resume();

    /* ISR user button */
    if (EXTI->PR & BIT_POS_TO_MASK(buttons_arr[PORT_USER_BUTTON_ID].pin))
    {
        GPIO_TypeDef *p_port = buttons_arr[PORT_USER_BUTTON_ID].p_port;
        uint8_t pin = buttons_arr[PORT_USER_BUTTON_ID].pin;
        bool activated = stm32f4_system_gpio_read(p_port, pin);
        
        if(activated)
        {
            buttons_arr[PORT_USER_BUTTON_ID].flag_pressed = false;
        }
        else
        {
            buttons_arr[PORT_USER_BUTTON_ID].flag_pressed = true;
        }
        //Clear pending bit of the EXTI line associated with the USER BUTTON pin
        EXTI->PR = BIT_POS_TO_MASK(buttons_arr[PORT_USER_BUTTON_ID].pin);
    }

    uint8_t col1_pin = keyboards_arr[PORT_KEYBOARD_MAIN_ID].p_col_pins[PORT_KEYBOARD_COL_1];
    if (EXTI->PR & BIT_POS_TO_MASK(col1_pin))
    {
        _check_column_interrupt(PORT_KEYBOARD_COL_1);
    }
}

/**
 * @brief This function handles EXTI lines [9:5] interrupts.
 */
void EXTI9_5_IRQHandler(void)
{
    /* Reanuda el SysTick inmediatamente al detectar la pulsación */
    port_system_systick_resume();

    /* Check Column 0 */
    uint8_t col0_pin = keyboards_arr[PORT_KEYBOARD_MAIN_ID].p_col_pins[PORT_KEYBOARD_COL_0];
    if (EXTI->PR & BIT_POS_TO_MASK(col0_pin))
    {
        _check_column_interrupt(PORT_KEYBOARD_COL_0);
    }

    /* Check Column 3 */
    uint8_t col3_pin = keyboards_arr[PORT_KEYBOARD_MAIN_ID].p_col_pins[PORT_KEYBOARD_COL_3];
    if (EXTI->PR & BIT_POS_TO_MASK(col3_pin))
    {
        _check_column_interrupt(PORT_KEYBOARD_COL_3);
    }
}

/**
 * @brief This function handles EXTI line 4 interrupt.
 */
void EXTI4_IRQHandler(void)
{
    /* Reanuda el SysTick inmediatamente al detectar la pulsación */
    port_system_systick_resume();

    /* Keyboard Column 2 */
    _check_column_interrupt(PORT_KEYBOARD_COL_2);
}

/**
 * @brief Interrupt service routine for the TIM5 timer.
 */
void TIM5_IRQHandler(void)
{
    /* Row scanning timeout */
    if (TIM5->SR & TIM_SR_UIF) {
        /* 1. Clear the interrupt flag UIF */
        TIM5->SR &= ~TIM_SR_UIF;
        
        /* 2. Set the row timeout flag */
        port_keyboard_set_row_timeout_status(PORT_KEYBOARD_MAIN_ID, true);
    }
}
/**
 * @brief Interrupt service routine for the TIM3 timer (Simone Timer).
 * Gestiona el control de tiempos de reproducción y de espera del usuario.
 */
void TIM3_IRQHandler(void)
{
    /* Verificar si la interrupción es por actualización (Update Interrupt Flag) */
    if (TIM3->SR & TIM_SR_UIF) {
        
        /* 1. Limpiar el flag de interrupción de hardware */
        TIM3->SR &= ~TIM_SR_UIF;

        /* 2. Detener el temporizador inmediatamente (comportamiento One-shot) */
        /* Esto evita que el temporizador siga contando innecesariamente */
        TIM3->CR1 &= ~TIM_CR1_CEN;

        /* 3. Notificar a la capa de abstracción que el tiempo ha expirado */
        /* simone_hw.flag_timer_timeout se pondrá a true */
        port_simone_set_timeout_status(true);
        
        /* 4. Reanudar el SysTick si el sistema estaba en modo de bajo consumo */
        port_system_systick_resume();
    }
}