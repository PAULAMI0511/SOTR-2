/***********************************************************
 *            INCLUSION LIBRERIAS .h                        *
 ***********************************************************/

#include "RtosIncludes.h"

/***********************************************************
 *           DEFINICIONES PRIVADAS  DEL _TASK_               *
 ***********************************************************/

/***********************************************************
 *              TIPOS PRIVADOS DEL _TASK_                    *
 ***********************************************************/

/***********************************************************
 *              VARIABLES GLOBALES DE _TASK_                 *
 ***********************************************************/

static u16 TaskIdCounter = 0;

/***********************************************************
 *  IMPLEMENTACION FUNCIONES PUBLICAS  DEL _TASK_           *
 ***********************************************************/

/*!
 * @brief  Crea una tarea: llena el TCB, prepara su pila y la deja lista.
 */
u16 Task_Create(TCB_t *TaskControlBlock, TaskFunctionCallback TaskFunction,
                u8 TaskPriority, pu8 TaskStack, u16 TaskStackSize,
                const char *TaskName) {
  if (TaskControlBlock == NULL) {
    return TASK_ERROR_NULL_PARAM;
  }
  if (TaskFunction == NULL || TaskStack == NULL || TaskName == NULL) {
    return TASK_ERROR_NULL_PARAM;
  }
  if (TaskStackSize < RTOS_MIN_STACK_SIZE) {
    return TASK_ERROR_INVALID_PARAM;
  }
  if (TaskPriority > RTOS_MIN_PRIORITY) {
    TaskPriority = RTOS_MIN_PRIORITY;
  }

  TaskControlBlock->TaskFunction = TaskFunction;
  TaskControlBlock->TaskPriority = TaskPriority;
  TaskControlBlock->TaskStack = TaskStack;
  TaskControlBlock->TaskStackSize = TaskStackSize;
  TaskControlBlock->TaskName = TaskName;
  TaskControlBlock->TaskStatus = TASK_READY;
  TaskControlBlock->taskId = TaskIdCounter++;
  TaskControlBlock->TaskWakeTick = 0;

  /* Arma en la pila el "contexto inicial" para que la tarea arranque en
   * TaskFunction la primera vez que el scheduler la elija */
  TaskControlBlock->TaskStackPointer =
      Portable_CreateContext(TaskStack, TaskStackSize, (u32)TaskFunction);
  if (TaskControlBlock->TaskStackPointer == 0) {
    return TASK_ERROR_FAIL;
  }

  /* La tarea queda en la cola de listas de su prioridad */
  if (Scheduler_AddTaskToReadyQueue(TaskControlBlock) != SCHEDULER_OK) {
    return TASK_ERROR_FAIL;
  }
  return TASK_OK;
}

/*!
 * @brief  Bloquea la tarea actual durante Milliseconds ms.
 *         Mientras espera, el scheduler ejecuta otras tareas.
 */
u16 Task_Delay(u32 Milliseconds) {
  TCB_Handle_t Task = Scheduler_GetCurrentTask();
  if (Task == NULL) {
    return TASK_ERROR_NOT_READY;
  }
  if (Milliseconds == 0) {
    Portable_CallScheduller();
    return TASK_OK;
  }

  u32 Ticks = (Milliseconds * RTOS_TICK_RATE_HZ) / 1000;
  if (Ticks == 0) {
    Ticks = 1;
  }

  Portable_DisableInterrupts();
  Scheduler_AddTaskToDelayedQueue(Task, Portable_GetSystemTicks() + Ticks);
  Portable_EnableInterrupts();

  /* Ceder la CPU: el scheduler escoge otra tarea */
  Portable_CallScheduller();
  return TASK_OK;
}

u16 Task_GetIdByName(const char *TaskName) {
  if (TaskName == NULL) {
    return TASK_ERROR_NULL_PARAM;
  }
  return TASK_OK;
}

u16 Task_Delete(TCB_t *TaskControlBlock) {
  if (TaskControlBlock == NULL) {
    return TASK_ERROR_NULL_PARAM;
  }
  return TASK_OK;
}

u16 Task_Start(TCB_t *TaskControlBlock) {
  if (TaskControlBlock == NULL) {
    return TASK_ERROR_NULL_PARAM;
  }
  return TASK_OK;
}

u16 Task_Stop(TCB_t *TaskControlBlock) {
  if (TaskControlBlock == NULL) {
    return TASK_ERROR_NULL_PARAM;
  }
  return TASK_OK;
}

u16 Task_Suspend(TCB_t *TaskControlBlock) {
  if (TaskControlBlock == NULL) {
    return TASK_ERROR_NULL_PARAM;
  }
  return TASK_OK;
}

u16 Task_Resume(TCB_t *TaskControlBlock) {
  if (TaskControlBlock == NULL) {
    return TASK_ERROR_NULL_PARAM;
  }
  return TASK_OK;
}

u16 Task_Yield(TCB_t *TaskControlBlock) {
  (void)TaskControlBlock;
  Portable_CallScheduller();
  return TASK_OK;
}

/***********************************************************
 *  IMPLEMENTACION FUNCIONES PRIVADAS  DEL _TASK_           *
 ***********************************************************/
