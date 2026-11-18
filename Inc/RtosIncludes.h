#ifndef _RTOS_INCLUDES_H_
#define _RTOS_INCLUDES_H_

/***********************************************************
 *      DEFINICIONES PUBLICAS (CONFIGURACION DEL RTOS)       *
 ***********************************************************/

#define RTOS_MIN_STACK_SIZE 256     /* bytes minimos de pila por tarea  */
#define RTOS_MAX_TASKS_PRIORITYS 16 /* prioridades 0 (alta) .. 15 (baja) */
#define RTOS_MIN_PRIORITY (RTOS_MAX_TASKS_PRIORITYS - 1)
#define RTOS_TICK_RATE_HZ 1000 /* 1 tick = 1 ms                     */

/***********************************************************
 *            INCLUSION LIBRERIAS .h                        *
 ***********************************************************/

#include "AppTypes.h"
#include "Mutex.h"
#include "Portable.h"
#include "Queue.h"
#include "Rtos.h"
#include "Scheduler.h"
#include "Semaphore.h"
#include "Task.h"

#endif // _RTOS_INCLUDES_H_
