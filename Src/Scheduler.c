/***********************************************************
 *            INCLUSION LIBRERIAS .h                        *
 ***********************************************************/

#include "RtosIncludes.h"

/***********************************************************
 *              VARIABLES GLOBALES DEL SCHEDULER            *
 ***********************************************************/

/* Una cola de tareas listas por cada prioridad (0 = la mas alta) */
static Queue_t ReadyQueue[RTOS_MAX_TASKS_PRIORITYS];

/* Tareas dormidas por Task_Delay */
static Queue_t DelayedQueue;

/* Tarea que esta usando la CPU en este momento */
static TCB_Handle_t CurrentTask = NULL;

/***********************************************************
 *  IMPLEMENTACION FUNCIONES PUBLICAS  DEL SCHEDULER        *
 ***********************************************************/

u16 Scheduler_Init(void) {
  for (u8 i = 0; i < RTOS_MAX_TASKS_PRIORITYS; i++) {
    Queue_Init(&ReadyQueue[i]);
  }
  Queue_Init(&DelayedQueue);
  CurrentTask = NULL;
  return SCHEDULER_OK;
}

TCB_Handle_t Scheduler_GetCurrentTask(void) { return CurrentTask; }

/*!
 * @brief  Mete una tarea al final de la cola de su prioridad.
 * @note   Llamar con interrupciones deshabilitadas (o antes de Rtos_Start).
 */
u16 Scheduler_AddTaskToReadyQueue(TCB_Handle_t Task) {
  if (Task == NULL) {
    return SCHEDULER_ERROR_NULL_PARAM;
  }
  Task->TaskStatus = TASK_READY;
  Task->TaskQueueElement.Data = (pv)Task;
  if (Queue_Enqueue(&ReadyQueue[Task->TaskPriority], &Task->TaskQueueElement) !=
      QUEUE_OK) {
    return SCHEDULER_ERROR_OPERATION_FAILED;
  }
  return SCHEDULER_OK;
}

/*!
 * @brief  Bloquea una tarea hasta que el contador de ticks llegue a WakeTick.
 * @note   Llamar con interrupciones deshabilitadas.
 */
u16 Scheduler_AddTaskToDelayedQueue(TCB_Handle_t Task, u64 WakeTick) {
  if (Task == NULL) {
    return SCHEDULER_ERROR_NULL_PARAM;
  }
  Task->TaskStatus = TASK_BLOCKED;
  Task->TaskWakeTick = WakeTick;
  Task->TaskQueueElement.Data = (pv)Task;
  if (Queue_Enqueue(&DelayedQueue, &Task->TaskQueueElement) != QUEUE_OK) {
    return SCHEDULER_ERROR_OPERATION_FAILED;
  }
  return SCHEDULER_OK;
}

/*!
 * @brief  Se llama en cada tick (desde SysTick). Revisa las tareas dormidas
 *         y pasa a "lista" las que ya cumplieron su tiempo.
 */
void Scheduler_Tick(u64 SystemTicks) {
  u16 Pending = Queue_Size(&DelayedQueue);
  while (Pending > 0) {
    QueueElement_t Element;
    if (Queue_Dequeue(&DelayedQueue, &Element) != QUEUE_OK) {
      break;
    }
    TCB_Handle_t Task = (TCB_Handle_t)Element.Data;
    if (SystemTicks >= Task->TaskWakeTick) {
      Scheduler_AddTaskToReadyQueue(Task); /* ya desperto */
    } else {
      Queue_Enqueue(&DelayedQueue, &Task->TaskQueueElement); /* sigue */
    }
    Pending--;
  }
}

/*!
 * @brief  Escoge la siguiente tarea a ejecutar: la primera de la cola con
 *         mayor prioridad. Tareas de igual prioridad se turnan (round-robin).
 * @note   Se llama desde PendSV con interrupciones deshabilitadas.
 */
TCB_Handle_t Scheduler_SelectNextTask(void) {
  /* Si la tarea actual seguia corriendo, vuelve al final de su cola */
  if ((CurrentTask != NULL) && (CurrentTask->TaskStatus == TASK_RUNNING)) {
    Scheduler_AddTaskToReadyQueue(CurrentTask);
  }

  for (u8 Priority = 0; Priority < RTOS_MAX_TASKS_PRIORITYS; Priority++) {
    if (Queue_Size(&ReadyQueue[Priority]) > 0) {
      QueueElement_t Element;
      if (Queue_Dequeue(&ReadyQueue[Priority], &Element) == QUEUE_OK) {
        CurrentTask = (TCB_Handle_t)Element.Data;
        break;
      }
    }
  }

  /* Siempre hay al menos una lista: la tarea Idle */
  CurrentTask->TaskStatus = TASK_RUNNING;
  return CurrentTask;
}
