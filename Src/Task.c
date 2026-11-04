/***********************************************************
 *            INCLUSION LIBRERIAS .h                        *
 ***********************************************************/
#include "RtosIncludes.h"

/***********************************************************
 *           DEFINICIONES PRIVADAS  DEL _TASK_H_          *
 ***********************************************************/

/***********************************************************
 *              TIPOS PRIVADOS DEL _TASK_H_               *
 ***********************************************************/

/***********************************************************
 *    DEFINICION DE FUNCIONES  PRIVADAS  DEL _TASK_H_      *
 ***********************************************************/

/***********************************************************
 *  IMPLEMENTACION FUNCIONES PUBLICAS  DEL _TASK_H_        *
 ***********************************************************/

u16 Task_Create(TCB_t *TaskControlBlock, TaskFunctionCallback TaskFunction,
                u8 TaskPriority, pu8 TaskStack, u16 TaskStackSize,
                const char *TaskName);
{
  if (TaskControlBlock == NULL) {
    return TASK_ERROR_NULL_PARAM;
  }
  if (TaskFunction == NULL || TaskStack == NULL || TaskName == NULL) {
    return TASK_ERROR_INVALID_PARAM;
  }
  if (TaskStackSize == 0) {
    return TASK_ERROR_OUT_OF_RANGE;
  }

  return TASK_OK;
}

u16 Task_GetIdByName(const char *TaskName);
{
  if (TaskName == NULL) {
    return TASK_ERROR_NULL_PARAM;
  }
  return TASK_OK;
}

u16 Task_Delete(TCB_t *TaskControlBlock);
{
  if (TaskControlBlock == NULL) {
    return TASK_ERROR_NULL_PARAM;
  }
  return TASK_OK;
}

u16 Task_Start(TCB_t *TaskControlBlock);
{
  if (TaskControlBlock == NULL) {
    return TASK_ERROR_NULL_PARAM;
  }
  return TASK_OK;
}

u16 Task_Stop(TCB_t *TaskControlBlock);
{
  if (TaskControlBlock == NULL) {
    return TASK_ERROR_NULL_PARAM;
  }
  return TASK_OK;
}

u16 Task_Suspend(TCB_t *TaskControlBlock);
{
  if (TaskControlBlock == NULL) {
    return TASK_ERROR_NULL_PARAM;
  }
  return TASK_OK;
}

u16 Task_Resume(TCB_t *TaskControlBlock);
{
  if (TaskControlBlock == NULL) {
    return TASK_ERROR_NULL_PARAM;
  }
  return TASK_OK;
}

u16 Task_Yield(TCB_t *TaskControlBlock);
{
  if (TaskControlBlock == NULL) {
    return TASK_ERROR_NULL_PARAM;
  }
  return TASK_OK;
}

/***********************************************************
 *  IMPLEMENTACION FUNCIONES PRIVADAS  DEL _TASK_H_        *
 ***********************************************************/
PRUEBA
