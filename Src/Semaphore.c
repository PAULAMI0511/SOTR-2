/***********************************************************
 *  Semaphore.c - Semaforo contador del RTOS.
 *  Se usa para que una tarea le "avise" a otra que ocurrio
 *  un evento (por ejemplo: el juego le avisa a la tarea de
 *  sonido que debe pitar).
 ***********************************************************/

/***********************************************************
 *            INCLUSION LIBRERIAS .h                        *
 ***********************************************************/

#include "RtosIncludes.h"

/***********************************************************
 *  IMPLEMENTACION FUNCIONES PUBLICAS  DEL SEMAPHORE        *
 ***********************************************************/

u16 Semaphore_Init(SemaphoreHandler_t sem, u16 InitialCount, u16 MaxCount) {
  if (sem == NULL) {
    return SEMAPHORE_ERROR_NULL_PARAM;
  }
  if (MaxCount == 0 || InitialCount > MaxCount) {
    return SEMAPHORE_ERROR_INVALID_PARAM;
  }
  sem->Count = InitialCount;
  sem->MaxCount = MaxCount;
  return Queue_Init(&sem->WaitQueue);
}

/*!
 * @brief  Toma una ficha. Si no hay, la tarea actual se bloquea hasta que
 *         otra tarea llame Semaphore_Give.
 */
u16 Semaphore_Take(SemaphoreHandler_t sem) {
  u16 Res = SEMAPHORE_ERROR_NULL_PARAM;
  if (sem != NULL) {
    u8 CallScheduller = FALSE;
    Portable_DisableInterrupts();
    if (sem->Count > 0) {
      /* Hay fichas: se toma una y la tarea sigue */
      sem->Count--;
      Res = SEMAPHORE_OK;
    } else {
      /* No hay fichas: la tarea se bloquea en la cola de espera */
      TCB_Handle_t Task = Scheduler_GetCurrentTask();
      if (Task != NULL) {
        Task->TaskStatus = TASK_BLOCKED;
        Task->TaskQueueElement.Data = (pv)Task;
        if (Queue_Enqueue(&sem->WaitQueue, &Task->TaskQueueElement) ==
            QUEUE_OK) {
          CallScheduller = TRUE;
          Res = SEMAPHORE_OK; /* al despertar, ya tiene su ficha */
        } else {
          Task->TaskStatus = TASK_RUNNING;
          Res = SEMAPHORE_ERROR_OPERATION_FAILED;
        }
      } else {
        Res = SEMAPHORE_ERROR_OPERATION_FAILED;
      }
    }
    Portable_EnableInterrupts();
    if (CallScheduller) {
      Portable_CallScheduller();
    }
  }
  return Res;
}

/*!
 * @brief  Devuelve una ficha. Si habia una tarea esperando, la ficha pasa
 *         directamente a ella y se despierta.
 */
u16 Semaphore_Give(SemaphoreHandler_t sem) {
  u16 Res = SEMAPHORE_ERROR_NULL_PARAM;
  if (sem != NULL) {
    u8 CallScheduller = FALSE;
    Portable_DisableInterrupts();
    if (Queue_Size(&sem->WaitQueue) > 0) {
      /* Hay tareas esperando: se despierta la primera */
      QueueElement_t Element;
      Res = SEMAPHORE_ERROR_OPERATION_FAILED;
      if (Queue_Dequeue(&sem->WaitQueue, &Element) == QUEUE_OK) {
        TCB_Handle_t WakeTask = (TCB_Handle_t)Element.Data;
        if (WakeTask != NULL &&
            Scheduler_AddTaskToReadyQueue(WakeTask) == SCHEDULER_OK) {
          Res = SEMAPHORE_OK;
          TCB_Handle_t ActualTask = Scheduler_GetCurrentTask();
          /* Numero menor = mayor prioridad */
          if (ActualTask != NULL &&
              WakeTask->TaskPriority < ActualTask->TaskPriority) {
            CallScheduller = TRUE;
          }
        }
      }
    } else if (sem->Count < sem->MaxCount) {
      sem->Count++;
      Res = SEMAPHORE_OK;
    } else {
      Res = SEMAPHORE_ERROR_FULL; /* ya estaba lleno: el aviso se pierde */
    }
    Portable_EnableInterrupts();
    if (CallScheduller) {
      Portable_CallScheduller();
    }
  }
  return Res;
}