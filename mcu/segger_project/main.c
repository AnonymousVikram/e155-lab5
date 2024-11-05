#include "../lib/STM32L432KC.h"
#include <stdio.h>
#include <stm32l4xx.h>

volatile int32_t pulse_count = 0;
volatile uint32_t elapsed_time_ms = 0;

void EXTI9_5_IRQHandler(void) {
  // Clear interrupt flags
  EXTI->PR1 = EXTI->PR1;

  // Read current state of PA5 and PA6
  uint8_t A = (GPIOA->IDR & GPIO_IDR_ID5) ? 1 : 0;
  uint8_t B = (GPIOA->IDR & GPIO_IDR_ID6) ? 1 : 0;
  uint8_t curr_AB = (A << 1) | B;

  // Static variable to hold previous AB state
  static uint8_t last_AB = 0;

  // Lookup table for quadrature decoding
  const int8_t lookup_table[16] = {0,  -1, 1, 0, 1, 0, 0,  -1,
                                   -1, 0,  0, 1, 0, 1, -1, 0};

  uint8_t index = (last_AB << 2) | curr_AB;
  int8_t change = lookup_table[index & 0x0F];

  pulse_count += change;
  last_AB = curr_AB;
}

void SysTick_Handler(void) {
  elapsed_time_ms++;

  // Calculate revolutions per second every 1000 ms
  if (elapsed_time_ms >= 1000) {
    int curPulses = pulse_count;
    float rev_per_sec = (float)curPulses / 480.0;
    printf("Revolutions per second: %.2f\n", rev_per_sec);
    pulse_count = 0;
    elapsed_time_ms = 0;
  }
}

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
  }
}