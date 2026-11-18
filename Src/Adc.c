/***********************************************************
 *  Adc.c - Driver del ADC1 del STM32F103.
 *  Canal 0 = PA0 (potenciometro jugador 1)
 *  Canal 1 = PA1 (potenciometro jugador 2)
 *  Reloj del ADC = PCLK2 / 2 = 8 MHz (maximo permitido: 14 MHz)
 ***********************************************************/

#include "Adc.h"
#include "Gpio.h"
#include "Stm32f103Regs.h"

#define ADC_TIMEOUT 10000U
#define ADC_SAMPLE_55_5_CYCLES 5U

void Adc_Init(void) {
  Gpio_EnablePort(GPIOA_BASE);
  Gpio_Config(GPIOA_BASE, 0, GPIO_MODE_ANALOG);
  Gpio_Config(GPIOA_BASE, 1, GPIO_MODE_ANALOG);

  RCC_APB2ENR |= RCC_APB2ENR_ADC1EN;

  /* Tiempo de muestreo de 55.5 ciclos para los canales 0 y 1 */
  ADC1_SMPR2 = (ADC_SAMPLE_55_5_CYCLES << 0) | (ADC_SAMPLE_55_5_CYCLES << 3);
  ADC1_SQR1 = 0; /* una sola conversion por secuencia */

  /* Encender el ADC y esperar que se estabilice */
  ADC1_CR2 = ADC_CR2_ADON;
  for (volatile u32 i = 0; i < 1000; i++) {
  }

  /* Calibracion (recomendada por el fabricante al encender) */
  u32 Timeout = ADC_TIMEOUT;
  ADC1_CR2 |= ADC_CR2_RSTCAL;
  while ((ADC1_CR2 & ADC_CR2_RSTCAL) && --Timeout) {
  }
  Timeout = ADC_TIMEOUT;
  ADC1_CR2 |= ADC_CR2_CAL;
  while ((ADC1_CR2 & ADC_CR2_CAL) && --Timeout) {
  }
}

/*!
 * @brief  Hace una conversion en el canal indicado y devuelve 0..4095.
 */
u16 Adc_Read(u8 Channel) {
  ADC1_SQR3 = Channel;
  ADC1_CR2 |= ADC_CR2_ADON; /* escribir ADON=1 otra vez inicia la conversion */
  u32 Timeout = ADC_TIMEOUT;
  while (!(ADC1_SR & ADC_SR_EOC)) {
    if (--Timeout == 0) {
      return 0;
    }
  }
  return (u16)(ADC1_DR & 0x0FFF); /* leer DR limpia EOC */
}