/*
 * Mutex.c
 */

/***********************************************************
 *            INCLUSION LIBRERIAS .h                        *
 ***********************************************************/

#include "RtosIncludes.h"

/***********************************************************
 *           DEFINICIONES PRIVADAS  DEL MUTEX               *
 ***********************************************************/

/***********************************************************
 *              TIPOS PRIVADOS DEL MUTEX                    *
 ***********************************************************/

/***********************************************************
 *    DEFINICION DE FUNCIONES  PRIVADAS  DEL MUTEX          *
 ***********************************************************/

/***********************************************************
 *  IMPLEMENTACION DE FUNCIONES PUBLICAS DEL MUTEX          *
 ***********************************************************/

u16 Mutex_Init(MutexHandler_t mutex) {
  if (mutex != NULL) {
    mutex->Busy = FALSE;
    return Queue_Init(&mutex->WaitQueue);
  }
  return MUTEX_ERROR_NULL_PARAM;
}

u16 Mutex_Take(MutexHandler_t mutex) {
  u16 Res = MUTEX_ERROR_NULL_PARAM;
  if (mutex != NULL) {
    u8 CallScheduller = FALSE;
    Portable_DisableInterrupts();
    if (mutex->Busy) {
      /* El mutex esta ocupado: la tarea actual se bloquea y espera en la cola
       */
      TCB_Handle_t Task = Scheduler_GetCurrentTask();
      if (Task != NULL) {
        Task->TaskStatus = TASK_BLOCKED;
        Task->TaskQueueElement.Data = (pv)Task;
        Res = Queue_Enqueue(&mutex->WaitQueue, &Task->TaskQueueElement);
        if (Res != QUEUE_OK) {
          Task->TaskStatus = TASK_RUNNING;
        } else {
          CallScheduller = TRUE;
        }
      } else {
        Res = MUTEX_ERROR_OPERATION_FAILED;
      }
    } else {
      /* El mutex estaba libre: la tarea lo toma */
      mutex->Busy = TRUE;
      Res = MUTEX_OK;
    }
    Portable_EnableInterrupts();
    if (CallScheduller) {
      Portable_CallScheduller();
    }
  }
  return Res;
}

u16 Mutex_Release(MutexHandler_t mutex) {
  u16 Res = MUTEX_ERROR_NULL_PARAM;
  if (mutex != NULL) {
    u8 CallScheduller = FALSE;
    Portable_DisableInterrupts();
    if (mutex->Busy) {
      if (Queue_Size(&mutex->WaitQueue)) {
        /* Hay tareas esperando: se despierta la primera y el mutex pasa a ella
         */
        QueueElement_t Element;
        Res = Queue_Dequeue(&mutex->WaitQueue, &Element);
        if (Res == QUEUE_OK) {
          TCB_Handle_t WakeTask = (TCB_Handle_t)Element.Data;
          if (WakeTask != NULL) {
            Res = Scheduler_AddTaskToReadyQueue(WakeTask);
            if (Res == SCHEDULER_OK) {
              TCB_Handle_t ActualTask = Scheduler_GetCurrentTask();
              if (ActualTask != NULL) {
                /* Numero menor = mayor prioridad */
                if (WakeTask->TaskPriority < ActualTask->TaskPriority) {
                  CallScheduller = TRUE;
                }
              }
            }
          } else {
            Res = MUTEX_ERROR_OPERATION_FAILED;
          }
        } else {
          Res = MUTEX_ERROR_OPERATION_FAILED;
        }
      } else {
        /* Nadie esperando: el mutex queda libre */
        mutex->Busy = FALSE;
        Res = MUTEX_OK;
      }
    } else {
      Res = MUTEX_OK;
    }
    Portable_EnableInterrupts();
    if (CallScheduller) {
      Portable_CallScheduller();
    }
  }
  return Res;
}

/***********************************************************
 *  IMPLEMENTACION FUNCIONES PRIVADAS  DEL MUTEX            *
 ***********************************************************/