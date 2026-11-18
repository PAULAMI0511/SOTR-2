#ifndef _GPIO_H_
#define _GPIO_H_

/***********************************************************
 *  Driver GPIO (pines digitales) para el STM32F103.
 ***********************************************************/

#include "AppTypes.h"
#include "Stm32f103Regs.h"

/* Modos de configuracion de un pin (4 bits CNF:MODE del registro CRL/CRH) */
#define GPIO_MODE_ANALOG 0x0U        /* entrada analoga (ADC)        */
#define GPIO_MODE_INPUT_PULL 0x8U    /* entrada con pull-up/down     */
#define GPIO_MODE_OUTPUT_PP 0x2U     /* salida push-pull 2 MHz       */
#define GPIO_MODE_AF_OPEN_DRAIN 0xFU /* funcion alterna open-drain   */

void Gpio_EnablePort(u32 Port);
void Gpio_Config(u32 Port, u8 Pin, u32 Mode);
void Gpio_Write(u32 Port, u8 Pin, u8 State);
u8 Gpio_Read(u32 Port, u8 Pin);

#endif // _GPIO_H_
