/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*                        The Embedded Experts                        *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------

File    : main.c
Purpose : Generic application start

*/

#include "../lib/STM32L432KC.h"
#include <cmsis_gcc.h>
#include <core_cm4.h>
#include <stdio.h>
#include <stm32l432xx.h>
#include <stm32l4xx.h>

/*********************************************************************
 *
 *       main()
 *
 *  Function description
 *   Application entry point.
 */
int main(void) {
  gpioEnable(GPIO_PORT_B);
  gpioEnable(GPIO_PORT_A);

  // Set encoder1 as input (PA5)
  // Set encoder2 as input (PA6)
  pinMode(PA5, GPIO_INPUT);
  pinMode(PA6, GPIO_INPUT);

  GPIOA->PUPDR |= _VAL2FLD(GPIO_PUPDR_PUPD5, 0b01);
  GPIOA->PUPDR |= _VAL2FLD(GPIO_PUPDR_PUPD6, 0b01);

  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

  SYSCFG->EXTICR[1] |= _VAL2FLD(SYSCFG_EXTICR2_EXTI5, 0b000);
  SYSCFG->EXTICR[1] |= _VAL2FLD(SYSCFG_EXTICR2_EXTI6, 0b000);

  __enable_irq();

  // Initialize timer
  RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
  initTIM(TIM2);

  EXTI->IMR1 |= (1 << gpioPinOffset(PA5));
  EXTI->IMR1 |= (1 << gpioPinOffset(PA6));

  EXTI->RTSR1 |= (1 << gpioPinOffset(PA5));
  EXTI->RTSR1 |= (1 << gpioPinOffset(PA6));

  EXTI->FTSR1 &= ~(1 << gpioPinOffset(PA5));
  EXTI->FTSR1 &= ~(1 << gpioPinOffset(PA6));

  NVIC->ISER[0] |= (1 << EXTI2_IRQn);

  while (1) {
    delay_millis(TIM2, 200);
  }
}

void EXTI5_IRQHandler(void) {
  if (EXTI->PR1 & (1 << 5)) {
    EXTI->PR1 |= (1 << 5);
    togglePin(PB3);
  }
}

void EXTI6_IRQHandler(void) {
  if (EXTI->PR1 & (1 << 6)) {
    EXTI->PR1 |= (1 << 6);
    togglePin(PB4);
  }
}

/*************************** End of file ****************************/
