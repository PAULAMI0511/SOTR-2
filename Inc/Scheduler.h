#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

/***********************************************************
 *            INCLUSION LIBRERIAS .h                        *
 ***********************************************************/

#include "AppTypes.h"
#include "Task.h"

/***********************************************************
 *           DEFINICION DE ERRORES  DEL _SCHEDULER_H_       *
 ***********************************************************/

#ifndef SCHEDULER_BASE_ERROR
#define SCHEDULER_BASE_ERROR 0x0400
#endif

#define SCHEDULER_OK ERROR_OK
#define SCHEDULER_ERROR_FAIL (SCHEDULER_BASE_ERROR | ERROR_FAIL)
#define SCHEDULER_ERROR_NULL_PARAM (SCHEDULER_BASE_ERROR | ERROR_NULL_PARAM)
#define SCHEDULER_ERROR_OPERATION_FAILED                                       \
  (SCHEDULER_BASE_ERROR | ERROR_OPERATION_FAILED)

/***********************************************************
 *           FUNCIONES PUBLICAS  DEL _SCHEDULER_H_          *
 ***********************************************************/

u16 Scheduler_Init(void);
TCB_Handle_t Scheduler_GetCurrentTask(void);
u16 Scheduler_AddTaskToReadyQueue(TCB_Handle_t Task);
u16 Scheduler_AddTaskToDelayedQueue(TCB_Handle_t Task, u64 WakeTick);
void Scheduler_Tick(u64 SystemTicks);
TCB_Handle_t Scheduler_SelectNextTask(void);

#endif // _SCHEDULER_H_
