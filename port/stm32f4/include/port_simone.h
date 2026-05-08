/**
 * @file port_simone.h
 * @brief Header for the portable functions of the Simone board game.
 * @author Alejandro Garcia
 * @author Mario Medina
 * @date 2026-05-04
 */

#ifndef PORT_SIMONE_H_
#define PORT_SIMONE_H_

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdint.h>
#include <stdbool.h>

/* Function prototypes -------------------------------------------------------*/

/**
 * @brief Initializes the hardware-dependent timer for the Simone game.
 * Internally, it must call the setup for TIM3 with priority 3 and subpriority 0.
 */
void port_simone_init(void);

/**
 * @brief Configures and starts the timer with a specific timeout.
 * @param duration_ms Time in milliseconds for the timeout (e.g., 300, 1000, 2000, 3000, 5000).
 */
void port_simone_set_timer_timeout(uint32_t duration_ms);

/**
 * @brief Stops the timer and disables its interrupts.
 */
void port_simone_stop_timer(void);

/**
 * @brief Returns the current status of the timer flag.
 * @return true if the timer has expired, false otherwise.
 */
bool port_simone_get_timeout_status(void);

/**
 * @brief Sets the timer flag status manually.
 * This is typically used by the ISR to set it to true or by the FSM to reset it.
 * @param status The new status for the timer flag.
 */
void port_simone_set_timeout_status(bool status);

#endif /* PORT_SIMONE_H_ */