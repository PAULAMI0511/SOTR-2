#ifndef _I2C_H_
#define _I2C_H_

/***********************************************************
 *  Driver I2C1 (maestro, solo escritura) para el STM32F103.
 *  Pines: PB6 = SCL, PB7 = SDA. Velocidad: ~400 kHz.
 ***********************************************************/

#include "AppTypes.h"

#ifndef I2C_BASE_ERROR
#define I2C_BASE_ERROR 0x0900
#endif

#define I2C_OK ERROR_OK
#define I2C_ERROR_TIMEOUT (I2C_BASE_ERROR | ERROR_TIMEOUT)
#define I2C_ERROR_NACK (I2C_BASE_ERROR | ERROR_COMMUNICATION_FAILURE)

void I2c_Init(void);
u16 I2c_Write(u8 Address, u8 ControlByte, const u8 *Data, u16 Length);

#endif // _I2C_H_
