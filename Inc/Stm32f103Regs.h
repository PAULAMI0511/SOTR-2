#ifndef _STM32F103_REGS_H_
#define _STM32F103_REGS_H_

/***********************************************************
 *  Direcciones de los registros del STM32F103 usados por
 *  los drivers del PONG (sin usar la libreria HAL).
 *  Referencia: Manual de referencia RM0008.
 ***********************************************************/

#include "AppTypes.h"

#define REG32(addr) (*(volatile u32 *)(addr))

/* ---------------- RCC (habilitar relojes) ---------------- */
#define RCC_BASE 0x40021000U
#define RCC_APB2ENR REG32(RCC_BASE + 0x18)
#define RCC_APB1ENR REG32(RCC_BASE + 0x1C)
#define RCC_APB2ENR_IOPAEN (1U << 2)
#define RCC_APB2ENR_IOPBEN (1U << 3)
#define RCC_APB2ENR_ADC1EN (1U << 9)
#define RCC_APB1ENR_I2C1EN (1U << 21)

/* ---------------- GPIO ---------------- */
#define GPIOA_BASE 0x40010800U
#define GPIOB_BASE 0x40010C00U
#define GPIO_CRL(port) REG32((port) + 0x00)
#define GPIO_CRH(port) REG32((port) + 0x04)
#define GPIO_IDR(port) REG32((port) + 0x08)
#define GPIO_ODR(port) REG32((port) + 0x0C)
#define GPIO_BSRR(port) REG32((port) + 0x10)
#define GPIO_BRR(port) REG32((port) + 0x14)

/* ---------------- I2C1 ---------------- */
#define I2C1_BASE 0x40005400U
#define I2C1_CR1 REG32(I2C1_BASE + 0x00)
#define I2C1_CR2 REG32(I2C1_BASE + 0x04)
#define I2C1_DR REG32(I2C1_BASE + 0x10)
#define I2C1_SR1 REG32(I2C1_BASE + 0x14)
#define I2C1_SR2 REG32(I2C1_BASE + 0x18)
#define I2C1_CCR REG32(I2C1_BASE + 0x1C)
#define I2C1_TRISE REG32(I2C1_BASE + 0x20)

#define I2C_CR1_PE (1U << 0)
#define I2C_CR1_START (1U << 8)
#define I2C_CR1_STOP (1U << 9)
#define I2C_CR1_SWRST (1U << 15)
#define I2C_SR1_SB (1U << 0)
#define I2C_SR1_ADDR (1U << 1)
#define I2C_SR1_BTF (1U << 2)
#define I2C_SR1_TXE (1U << 7)
#define I2C_SR1_AF (1U << 10)
#define I2C_SR2_BUSY (1U << 1)
#define I2C_CCR_FS (1U << 15)

/* ---------------- ADC1 ---------------- */
#define ADC1_BASE 0x40012400U
#define ADC1_SR REG32(ADC1_BASE + 0x00)
#define ADC1_CR2 REG32(ADC1_BASE + 0x08)
#define ADC1_SMPR2 REG32(ADC1_BASE + 0x10)
#define ADC1_SQR1 REG32(ADC1_BASE + 0x2C)
#define ADC1_SQR3 REG32(ADC1_BASE + 0x34)
#define ADC1_DR REG32(ADC1_BASE + 0x4C)

#define ADC_SR_EOC (1U << 1)
#define ADC_CR2_ADON (1U << 0)
#define ADC_CR2_CAL (1U << 2)
#define ADC_CR2_RSTCAL (1U << 3)

#endif // _STM32F103_REGS_H_
