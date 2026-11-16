#ifndef _RTOS_H_
#define _RTOS_H_

/***********************************************************
 *            INCLUSION LIBRERIAS .h                        *
 ***********************************************************/

#include "AppTypes.h"

/***********************************************************
 *           DEFINICION DE ERRORES  DEL _RTOS_H_            *
 ***********************************************************/

#ifndef RTOS_BASE_ERROR
#define RTOS_BASE_ERROR 0x0600
#endif

#define RTOS_OK ERROR_OK
#define RTOS_ERROR_FAIL (RTOS_BASE_ERROR | ERROR_FAIL)

/***********************************************************
 *           FUNCIONES PUBLICAS  DEL _RTOS_H_               *
 ***********************************************************/

u16 Rtos_Init(void);
void Rtos_Start(void);

#endif // _RTOS_H_