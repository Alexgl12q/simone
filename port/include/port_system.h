/**
 * @file port_system.h
 * @brief Header for port_system.c file.
 * @author SDG2. Román Cárdenas (r.cardenas@upm.es) and Josué Pagán (j.pagan@upm.es)
 * @date 2026-01-01
 */

#ifndef PORT_SYSTEM_H_
#define PORT_SYSTEM_H_

/* Includes del sistema */
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Initializes the system.
 */
uint32_t port_system_init(void);

/**
 * @brief Returns the number of milliseconds since the system started.
 *
 * @retval number of milliseconds since the system started.
 */
uint32_t port_system_get_millis(void);

/**
 * @brief Sets the number of milliseconds since the system started.
 *
 * @param ms New number of milliseconds since the system started.
 */
void port_system_set_millis(uint32_t ms);

/**
 * @brief Delays the program execution for the specified number of milliseconds.
 *
 * @param ms Number of milliseconds to delay.
 */
void port_system_delay_ms(uint32_t ms);

/**
 * @brief Delays the program execution until the specified number of milliseconds since the system started.
 *
 * @param t Pointer to the variable that stores the number of milliseconds to delay until.
 * @param ms Number of milliseconds to delay until.
 *
 * @note This function modifies the value of the variable pointed by t to the number of milliseconds to delay until.
 * @note This function is useful to implement periodic tasks.
 */
void port_system_delay_until_ms(uint32_t *t, uint32_t ms);

/*----------------------------------------------------------------------------
 * POWER AND TIMER RELATED FUNCTIONS (Low Power Modes)
 *---------------------------------------------------------------------------*/

/**
 * @brief Suspends the SysTick interrupt.
 *
 * This function disables the SysTick interrupt request to avoid waking up
 * the processor every 1ms during sleep modes.
 */
void port_system_systick_suspend(void);

/**
 * @brief Resumes the SysTick interrupt.
 *
 * This function enables the SysTick interrupt request to resume time counting
 * after waking up from a low power mode.
 */
void port_system_systick_resume(void);

/**
 * @brief Enters a light sleep mode (CPU clock off).
 *
 * In this mode, the CPU stops executing instructions but peripherals remain
 * active. It uses port_system_systick_suspend/resume to manage the time base.
 */
void port_system_sleep(void);

/**
 * @brief Enters a deep sleep mode (Stop mode).
 *
 * This mode stops the CPU and all high-speed clocks. It requires clock
 * reconfiguration upon wakeup.
 */
void port_system_power_stop(void);

#endif /* PORT_SYSTEM_H_ */