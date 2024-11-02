#include "../lib/STM32L432KC.h"
#include <stdio.h>
#include <stm32l4xx.h>

volatile uint32_t pulse_count = 0;
volatile uint32_t elapsed_time_ms = 0;

void EXTI9_5_IRQHandler(void) {
  if (EXTI->PR1 & EXTI_PR1_PIF5) {
    EXTI->PR1 = EXTI_PR1_PIF5; // Clear interrupt flag
    pulse_count++;
  }
  if (EXTI->PR1 & EXTI_PR1_PIF6) {
    EXTI->PR1 = EXTI_PR1_PIF6; // Clear interrupt flag
    pulse_count++;
  }
}

void SysTick_Handler(void) { elapsed_time_ms++; }

int main(void) {
  gpioEnable(GPIO_PORT_A);

  // Set PA5 and PA6 as input with pull-up resistors
  pinMode(PA5, GPIO_INPUT);
  pinMode(PA6, GPIO_INPUT);
  GPIOA->PUPDR |= _VAL2FLD(GPIO_PUPDR_PUPD5, 0b01);
  GPIOA->PUPDR |= _VAL2FLD(GPIO_PUPDR_PUPD6, 0b01);

  // Enable SYSCFG clock
  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

  // Configure EXTI for PA5 and PA6
  SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI5_PA | SYSCFG_EXTICR2_EXTI6_PA;
  EXTI->IMR1 |= EXTI_IMR1_IM5 | EXTI_IMR1_IM6; // Unmask the interrupts

  // Enable both rising and falling edge triggers
  EXTI->RTSR1 |= EXTI_RTSR1_RT5 | EXTI_RTSR1_RT6; // Rising edge trigger
  EXTI->FTSR1 |= EXTI_FTSR1_FT5 | EXTI_FTSR1_FT6; // Falling edge trigger

  // Enable EXTI interrupt in NVIC
  NVIC_EnableIRQ(EXTI9_5_IRQn);

  // Configure SysTick for 1 ms interrupts
  SysTick_Config(SystemCoreClock / 1000);

  while (1) {
    // Calculate RPM every 1000 ms
    if (elapsed_time_ms >= 1000) {
      uint32_t pulses_per_revolution = 240; // Set according to your encoder
      uint32_t rpm =
          (pulse_count * 60000) / (pulses_per_revolution * elapsed_time_ms);
      printf("RPM: %lu\n", rpm);
      pulse_count = 0;
      elapsed_time_ms = 0;
    }
  }
}