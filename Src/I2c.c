/***********************************************************
 *  I2c.c - Driver del periferico I2C1 en modo maestro.
 *  Se usa para enviar comandos y la imagen a la pantalla OLED.
 *
 *  Reloj del periferico (APB1) = 16 MHz.
 *  Modo rapido (Fast mode): f_SCL = 16 MHz / (3 * CCR) = ~380 kHz
 ***********************************************************/

#include "I2c.h"
#include "Gpio.h"
#include "Stm32f103Regs.h"

#define I2C_PCLK_MHZ 16U
#define I2C_CCR_VALUE 14U  /* 16 MHz / (3 * 14) = 381 kHz       */
#define I2C_TRISE_VALUE 6U /* (300 ns * 16 MHz) + 1              */
#define I2C_TIMEOUT 20000U

#define I2C_SCL_PIN 6
#define I2C_SDA_PIN 7

/* Espera a que se cumpla una condicion, sin quedarse colgado para siempre */
static u16 I2c_WaitFlag(volatile u32 *Reg, u32 Flag) {
  u32 Timeout = I2C_TIMEOUT;
  while (!(*Reg & Flag)) {
    if (I2C1_SR1 & I2C_SR1_AF) {
      return I2C_ERROR_NACK; /* el esclavo no respondio */
    }
    if (--Timeout == 0) {
      return I2C_ERROR_TIMEOUT;
    }
  }
  return I2C_OK;
}

static u16 I2c_Fail(u16 Error) {
  I2C1_SR1 &= ~I2C_SR1_AF;
  I2C1_CR1 |= I2C_CR1_STOP;
  return Error;
}

/*!
 * @brief  Configura PB6/PB7 como funcion alterna open-drain y el I2C1 a
 *         ~400 kHz.
 */
void I2c_Init(void) {
  Gpio_EnablePort(GPIOB_BASE);
  Gpio_Config(GPIOB_BASE, I2C_SCL_PIN, GPIO_MODE_AF_OPEN_DRAIN);
  Gpio_Config(GPIOB_BASE, I2C_SDA_PIN, GPIO_MODE_AF_OPEN_DRAIN);

  RCC_APB1ENR |= RCC_APB1ENR_I2C1EN;

  /* Reset por software (soluciona el bit BUSY "pegado" del STM32F1) */
  I2C1_CR1 = I2C_CR1_SWRST;
  I2C1_CR1 = 0;

  I2C1_CR2 = I2C_PCLK_MHZ;               /* frecuencia del bus APB1 */
  I2C1_CCR = I2C_CCR_FS | I2C_CCR_VALUE; /* modo rapido            */
  I2C1_TRISE = I2C_TRISE_VALUE;
  I2C1_CR1 = I2C_CR1_PE; /* habilitar el periferico */
}

/*!
 * @brief  Envia: START, direccion, byte de control y Length bytes, STOP.
 * @param  Address: direccion de 7 bits del esclavo (0x3C para la OLED).
 */
u16 I2c_Write(u8 Address, u8 ControlByte, const u8 *Data, u16 Length) {
  u32 Timeout = I2C_TIMEOUT;
  while (I2C1_SR2 & I2C_SR2_BUSY) {
    if (--Timeout == 0) {
      I2c_Init(); /* bus bloqueado: reiniciar el periferico */
      return I2C_ERROR_TIMEOUT;
    }
  }

  /* 1. Condicion de START */
  I2C1_CR1 |= I2C_CR1_START;
  if (I2c_WaitFlag(&I2C1_SR1, I2C_SR1_SB) != I2C_OK) {
    return I2c_Fail(I2C_ERROR_TIMEOUT);
  }

  /* 2. Direccion del esclavo + bit de escritura (0) */
  I2C1_DR = (u32)(Address << 1);
  u16 Res = I2c_WaitFlag(&I2C1_SR1, I2C_SR1_ADDR);
  if (Res != I2C_OK) {
    return I2c_Fail(Res);
  }
  (void)I2C1_SR1; /* leer SR1 y SR2 limpia la bandera ADDR */
  (void)I2C1_SR2;

  /* 3. Byte de control (0x00 = comando, 0x40 = datos para la OLED) */
  if (I2c_WaitFlag(&I2C1_SR1, I2C_SR1_TXE) != I2C_OK) {
    return I2c_Fail(I2C_ERROR_TIMEOUT);
  }
  I2C1_DR = ControlByte;

  /* 4. Datos */
  for (u16 i = 0; i < Length; i++) {
    Res = I2c_WaitFlag(&I2C1_SR1, I2C_SR1_TXE);
    if (Res != I2C_OK) {
      return I2c_Fail(Res);
    }
    I2C1_DR = Data[i];
  }

  /* 5. Esperar el ultimo byte y generar STOP */
  Res = I2c_WaitFlag(&I2C1_SR1, I2C_SR1_BTF);
  if (Res != I2C_OK) {
    return I2c_Fail(Res);
  }
  I2C1_CR1 |= I2C_CR1_STOP;
  return I2C_OK;
}
