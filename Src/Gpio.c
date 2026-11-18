/***********************************************************
 *  Gpio.c - Driver de pines digitales del STM32F103.
 ***********************************************************/

#include "Gpio.h"

/*!
 * @brief  Habilita el reloj del puerto (GPIOA o GPIOB).
 */
void Gpio_EnablePort(u32 Port) {
  if (Port == GPIOA_BASE) {
    RCC_APB2ENR |= RCC_APB2ENR_IOPAEN;
  } else if (Port == GPIOB_BASE) {
    RCC_APB2ENR |= RCC_APB2ENR_IOPBEN;
  }
}

/*!
 * @brief  Configura un pin. Cada pin usa 4 bits: los pines 0..7 en CRL y
 *         los pines 8..15 en CRH.
 */
void Gpio_Config(u32 Port, u8 Pin, u32 Mode) {
  if (Pin < 8) {
    GPIO_CRL(Port) =
        (GPIO_CRL(Port) & ~(0xFU << (Pin * 4))) | (Mode << (Pin * 4));
  } else {
    u8 Shift = (Pin - 8) * 4;
    GPIO_CRH(Port) = (GPIO_CRH(Port) & ~(0xFU << Shift)) | (Mode << Shift);
  }
}

/*!
 * @brief  Escribe un pin usando BSRR/BRR (escritura atomica: no hace falta
 *         leer-modificar-escribir, asi dos tareas no se pisan).
 */
void Gpio_Write(u32 Port, u8 Pin, u8 State) {
  if (State) {
    GPIO_BSRR(Port) = (1U << Pin);
  } else {
    GPIO_BRR(Port) = (1U << Pin);
  }
}

u8 Gpio_Read(u32 Port, u8 Pin) {
  return (GPIO_IDR(Port) & (1U << Pin)) ? TRUE : FALSE;
}
