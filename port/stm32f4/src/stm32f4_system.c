/**
 * @file stm32f4_system.c
 * @brief This file implements port layer for the system functions in the STM32F4 platform.
 * @author SDG2. Román Cárdenas (r.cardenas@upm.es) and Josué Pagán (j.pagan@upm.es)
 * @date 2026-01-01
 */

/* HW dependent includes */
#include "stm32f4xx.h"
#include "port_system.h"
#include "stm32f4_system.h"

#ifdef USE_SEMIHOSTING
extern void initialise_monitor_handles(void);
#endif

//------------------------------------------------------
// FILE-SPECIFIC DEFINITIONS
//------------------------------------------------------

/* Power Control Definitions for Low Power Modes */
#ifndef PWR_MAINREGULATOR_ON
#define PWR_MAINREGULATOR_ON           ((uint32_t)0x00000000)
#endif

#ifndef PWR_LOWPOWERREGULATOR_ON
#define PWR_LOWPOWERREGULATOR_ON       PWR_CR_LPDS
#endif

#ifndef PWR_SLEEPENTRY_WFI
#define PWR_SLEEPENTRY_WFI             ((uint8_t)0x01)
#endif

#ifndef PWR_STOPENTRY_WFI
#define PWR_STOPENTRY_WFI              ((uint8_t)0x01)
#endif

#define HSI_VALUE ((uint32_t)16000000) /*!< Value of the Internal oscillator in Hz */

/* Timer configuration */
#define RCC_HSI_CALIBRATION_DEFAULT 0x10U             /*!< Default HSI calibration trimming value */
#define TICK_FREQ_1KHZ 1U                             /*!< Frequency in kHz of the System tick */
#define NVIC_PRIORITY_GROUP_0 ((uint32_t)0x00000007) 
#define NVIC_PRIORITY_GROUP_5 ((uint32_t)0x00000004) 

/* Power Regulator Scale */
#define POWER_REGULATOR_VOLTAGE_SCALE3 0x01 

//------------------------------------------------------
// PRIVATE (STATIC) VARIABLES
//------------------------------------------------------
static volatile uint32_t msTicks = 0; 

//------------------------------------------------------
// PUBLIC (GLOBAL) VARIABLES
//------------------------------------------------------
uint32_t SystemCoreClock = HSI_VALUE;
const uint8_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};
const uint8_t APBPrescTable[8] = {0, 0, 0, 0, 1, 2, 3, 4};

//------------------------------------------------------
// PRIVATE (STATIC) FUNCTIONS
//------------------------------------------------------

/**
 * @brief System Clock Configuration
 */
static void system_clock_config(void)
{
  /* Configure the main internal regulator output voltage */
  PWR->CR &= ~PWR_CR_VOS; 
  PWR->CR |= (PWR_CR_VOS & (POWER_REGULATOR_VOLTAGE_SCALE3 << PWR_CR_VOS_Pos));

  RCC->CR &= ~RCC_CR_HSITRIM; 
  RCC->CR |= (RCC_CR_HSITRIM & (RCC_HSI_CALIBRATION_DEFAULT << RCC_CR_HSITRIM_Pos));

  FLASH->ACR = FLASH_ACR_LATENCY_2WS; 

  RCC->CFGR &= ~RCC_CFGR_SW; 
  RCC->CFGR |= (RCC_CFGR_SW & (RCC_CFGR_SW_HSI << RCC_CFGR_SW_Pos));

  SystemCoreClock = HSI_VALUE >> AHBPrescTable[(RCC->CFGR & RCC_CFGR_HPRE) >> RCC_CFGR_HPRE_Pos];

  SysTick_Config(SystemCoreClock / (1000U / TICK_FREQ_1KHZ)); 
}

//------------------------------------------------------
// PUBLIC (GLOBAL) FUNCTIONS
//------------------------------------------------------

void SystemInit(void)
{
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
  SCB->CPACR |= ((3UL << 10 * 2) | (3UL << 11 * 2)); 
#endif

#if defined(USER_VECT_TAB_ADDRESS)
  SCB->VTOR = VECT_TAB_BASE_ADDRESS | VECT_TAB_OFFSET; 
#endif 
}

uint32_t port_system_init()
{
#ifdef USE_SEMIHOSTING
  initialise_monitor_handles();
#endif

  FLASH->ACR |= FLASH_ACR_ICEN;
  FLASH->ACR |= FLASH_ACR_DCEN;
  FLASH->ACR |= FLASH_ACR_PRFTEN;

  NVIC_SetPriorityGrouping(NVIC_PRIORITY_GROUP_5);
  NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0U, 0U)); 

  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; 
  RCC->APB1ENR |= RCC_APB1ENR_PWREN; 

  system_clock_config();

  return 0;
}

//------------------------------------------------------
// TIMER RELATED FUNCTIONS
//------------------------------------------------------
void port_system_delay_ms(uint32_t ms)
{
  uint32_t tickstart = port_system_get_millis();
  while ((port_system_get_millis() - tickstart) < ms)
  {
  }
}

void port_system_delay_until_ms(uint32_t *p_t, uint32_t ms)
{
  uint32_t until = *p_t + ms;
  uint32_t now = port_system_get_millis();
  if (until > now)
  {
    port_system_delay_ms(until - now);
  }
  *p_t = port_system_get_millis();
}

uint32_t port_system_get_millis()
{
  return msTicks; 
}

void port_system_set_millis(uint32_t ms)
{
  msTicks = ms;
}

void port_system_systick_suspend(void)
{
    /* Disable SysTick Interrupt */
    SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
}

void port_system_systick_resume(void)
{
    /* Enable SysTick Interrupt */
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
}

//------------------------------------------------------
// POWER RELATED FUNCTIONS
//------------------------------------------------------

void port_system_power_sleep(void)
{
    /* Suspende el conteo de milisegundos de la HAL si se usa */
    HAL_SuspendTick();
    /* Entra en modo Sleep (CPU off) */
    HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
    /* Reanuda el conteo */
    HAL_ResumeTick();
}

void port_system_power_stop(void)
{
    HAL_SuspendTick();
    /* Entra en modo Stop (Clocks off) */
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
    
    /* Muy importante: reconfigurar relojes tras despertar de STOP */
    system_clock_config();
    
    HAL_ResumeTick();
}

void port_system_sleep(void)
{
    /* Implementación según requisitos V4: suspender Tick propio */
    port_system_systick_suspend();
    
    /* Dormir */
    HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
    
    /* Reanudar tras despertar (activado por ISR externa) */
    port_system_systick_resume();
}

//------------------------------------------------------
// GPIO RELATED FUNCTIONS
//------------------------------------------------------
void stm32f4_system_gpio_config(GPIO_TypeDef *p_port, uint8_t pin, uint8_t mode, uint8_t pupd)
{
  if (p_port == GPIOA) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
  else if (p_port == GPIOB) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
  else if (p_port == GPIOC) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

  p_port->MODER &= ~(GPIO_MODER_MODER0 << (pin * 2U));
  p_port->MODER |= (mode << (pin * 2U));

  p_port->PUPDR &= ~(GPIO_PUPDR_PUPD0 << (pin * 2U));
  p_port->PUPDR |= (pupd << (pin * 2U));
}

void stm32f4_system_gpio_config_exti(GPIO_TypeDef *p_port, uint8_t pin, uint32_t mode)
{
  uint32_t port_selector = 0;
  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

  if (p_port == GPIOB) port_selector = 1;
  else if (p_port == GPIOC) port_selector = 2;

  uint32_t displacement = (pin % 4) * 4;
  SYSCFG->EXTICR[pin / 4] &= ~(0x0FU << displacement);
  SYSCFG->EXTICR[pin / 4] |= (port_selector << displacement);

  EXTI->RTSR &= ~BIT_POS_TO_MASK(pin);
  if (mode & STM32F4_TRIGGER_RISING_EDGE) EXTI->RTSR |= BIT_POS_TO_MASK(pin);

  EXTI->FTSR &= ~BIT_POS_TO_MASK(pin);
  if (mode & STM32F4_TRIGGER_FALLING_EDGE) EXTI->FTSR |= BIT_POS_TO_MASK(pin);

  EXTI->EMR &= ~BIT_POS_TO_MASK(pin);
  if (mode & STM32F4_TRIGGER_ENABLE_EVENT_REQ) EXTI->EMR |= BIT_POS_TO_MASK(pin);

  EXTI->IMR &= ~BIT_POS_TO_MASK(pin);
  if (mode & STM32F4_TRIGGER_ENABLE_INTERR_REQ) EXTI->IMR |= BIT_POS_TO_MASK(pin);
}

void stm32f4_system_gpio_exti_enable(uint8_t pin, uint8_t priority, uint8_t subpriority)
{
  NVIC_SetPriority(GET_PIN_IRQN(pin), NVIC_EncodePriority(NVIC_GetPriorityGrouping(), priority, subpriority));
  NVIC_EnableIRQ(GET_PIN_IRQN(pin));
}

void stm32f4_system_gpio_exti_disable(uint8_t pin)
{
  NVIC_DisableIRQ(GET_PIN_IRQN(pin));
}

void stm32f4_system_gpio_config_alternate(GPIO_TypeDef *p_port, uint8_t pin, uint8_t alternate)
{
  uint32_t displacement = (pin % 8) * 4;
  p_port->AFR[pin / 8] &= ~(0x0FU << displacement);
  p_port->AFR[pin / 8] |= (alternate << displacement);
}

bool stm32f4_system_gpio_read(GPIO_TypeDef *p_port, uint8_t pin)
{
  return (bool)(p_port->IDR & BIT_POS_TO_MASK(pin));
}

void stm32f4_system_gpio_write(GPIO_TypeDef *p_port, uint8_t pin, bool value)
{
  if (value) p_port->BSRR = BIT_POS_TO_MASK(pin);
  else p_port->BSRR = BIT_POS_TO_MASK(pin) << 16;
}

void stm32f4_system_gpio_toggle(GPIO_TypeDef *p_port, uint8_t pin)
{
  if (stm32f4_system_gpio_read(p_port, pin)) stm32f4_system_gpio_write(p_port, pin, false);
  else stm32f4_system_gpio_write(p_port, pin, true);
}