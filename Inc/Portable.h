#ifndef _PORTABLE_H_
#define _PORTABLE_H_

/***********************************************************
 *            INCLUSION LIBRERIAS .h                        *
 ***********************************************************/

#include "AppTypes.h"

/***********************************************************
 *           DEFINICION DE ERRORES  DEL _PORTABLE_H_        *
 ***********************************************************/

#ifndef PORTABLE_BASE_ERROR
#define PORTABLE_BASE_ERROR 0x0500
#endif

#define PORTABLE_OK ERROR_OK
#define PORTABLE_ERROR_TIMEOUT (PORTABLE_BASE_ERROR | ERROR_TIMEOUT)
#define PORTABLE_ERROR_HARDWARE_FAILURE                                        \
  (PORTABLE_BASE_ERROR | ERROR_HARDWARE_FAILURE)

/***********************************************************
 *           FUNCIONES PUBLICAS  DEL _PORTABLE_H_           *
 ***********************************************************/

u16 Portable_InitInternalClock16MHz(void);
void Portable_InitTickTimer(void);
u64 Portable_GetSystemTicks(void);

void Portable_DisableInterrupts(void);
void Portable_EnableInterrupts(void);

u32 Portable_CreateContext(pu8 StackPointer, u16 StackSize, u32 Function);
void Portable_CallScheduller(void);
void Portable_StartFirstTask(u32 StackPointer);

#endif // _PORTABLE_H_
