/***********************************************************
 *  Buzzer.c - Buzzer activo: suena mientras el pin esta activo.
 ***********************************************************/

#include "Buzzer.h"
#include "Gpio.h"

#define BUZZER_PORT GPIOA_BASE
#define BUZZER_PIN 8

void Buzzer_Init(void) {
  Gpio_EnablePort(BUZZER_PORT);
  Gpio_Config(BUZZER_PORT, BUZZER_PIN, GPIO_MODE_OUTPUT_PP);
  Buzzer_Off();
}

void Buzzer_On(void) {
  Gpio_Write(BUZZER_PORT, BUZZER_PIN, BUZZER_ACTIVE_LEVEL);
}

void Buzzer_Off(void) {
  Gpio_Write(BUZZER_PORT, BUZZER_PIN, !BUZZER_ACTIVE_LEVEL);
}
