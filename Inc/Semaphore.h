#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

/***********************************************************
 *            INCLUSION LIBRERIAS .h                        *
 ***********************************************************/

#include "AppTypes.h"
#include "Queue.h"

/***********************************************************
 *         DEFINICION DE ERRORES  DEL _SEMAPHORE_H_         *
 ***********************************************************/

#ifndef SEMAPHORE_BASE_ERROR
#define SEMAPHORE_BASE_ERROR 0x0800
#endif

#define SEMAPHORE_OK ERROR_OK
#define SEMAPHORE_ERROR_NULL_PARAM (SEMAPHORE_BASE_ERROR | ERROR_NULL_PARAM)
#define SEMAPHORE_ERROR_INVALID_PARAM                                          \
  (SEMAPHORE_BASE_ERROR | ERROR_INVALID_PARAM)
#define SEMAPHORE_ERROR_FULL (SEMAPHORE_BASE_ERROR | ERROR_FULL)
#define SEMAPHORE_ERROR_OPERATION_FAILED                                       \
  (SEMAPHORE_BASE_ERROR | ERROR_OPERATION_FAILED)

/***********************************************************
 *              TIPOS PUBLICOS DEL _SEMAPHORE_H_            *
 ***********************************************************/

/* Semaforo contador: Count = cuantas "fichas" hay disponibles.
 * Take consume una ficha (si no hay, la tarea se bloquea en WaitQueue).
 * Give devuelve una ficha (o despierta a una tarea que estaba esperando). */
typedef struct {
  u16 Count;
  u16 MaxCount;
  Queue_t WaitQueue;
} Semaphore_t, *SemaphoreHandler_t;

/***********************************************************
 *           FUNCIONES PUBLICAS  DEL _SEMAPHORE_H_          *
 ***********************************************************/

u16 Semaphore_Init(SemaphoreHandler_t sem, u16 InitialCount, u16 MaxCount);
u16 Semaphore_Take(SemaphoreHandler_t sem);
u16 Semaphore_Give(SemaphoreHandler_t sem);

#endif // _SEMAPHORE_H_