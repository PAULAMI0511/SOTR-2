#ifndef _TASK_H_
#define _TASK_H_

/***********************************************************
 *            INCLUSION LIBRERIAS .h                        *
 ***********************************************************/

#include "AppTypes.h"

/***********************************************************
 *           DEFINICION DE ERRORES  DEL _TASK_H_          *
 ***********************************************************/

#ifndef TASK_BASE_ERROR
#define TASK_BASE_ERROR 0x0200
#endif

#define TASK_OK ERROR_OK
#define TASK_ERROR_FAIL TASK_BASE_ERROR | ERROR_FAIL
#define TASK_ERROR_UNKNOWN TASK_BASE_ERROR | ERROR_UNKNOWN
#define TASK_ERROR_NULL_PARAM TASK_BASE_ERROR | ERROR_NULL_PARAM
#define TASK_ERROR_OUT_OF_RANGE TASK_BASE_ERROR | ERROR_OUT_OF_MEMORY
#define TASK_ERROR_TIMEOUT TASK_BASE_ERROR | ERROR_TIMEOUT
#define TASK_ERROR_NOT_FOUND TASK_BASE_ERROR | ERROR_NOT_FOUND
#define TASK_ERROR_INVALID_PARAM TASK_BASE_ERROR | ERROR_INVALID_PARAM
#define TASK_ERROR_ALREADY_EXISTS TASK_BASE_ERROR | ERROR_ALREADY_EXISTS
#define TASK_ERROR_NOT_IMPLEMENTED TASK_BASE_ERROR | ERROR_NOT_IMPLEMENTED
#define TASK_ERROR_BUSY TASK_BASE_ERROR | ERROR_BUSY
#define TASK_ERROR_UNSUPPORTED TASK_BASE_ERROR | ERROR_UNSUPPORTED
#define TASK_ERROR_HARDWARE_FAILURE TASK_BASE_ERROR | ERROR_HARDWARE_FAILURE
#define TASK_ERROR_SOFTWARE_FAILURE TASK_BASE_ERROR | ERROR_SOFTWARE_FAILURE
#define TASK_ERROR_COMMUNICATION_FAILURE
TASK_BASE_ERROR |
    ERROR_COMMUNICATION_FAILURE
#define TASK_ERROR_SECURITY_FAILURE TASK_BASE_ERROR | ERROR_SECURITY_FAILURE
#define TASK_ERROR_EMPTY TASK_BASE_ERROR | ERROR_EMPTY
#define TASK_ERROR_INVALID_STATE TASK_BASE_ERROR | ERROR_INVALID_STATE
#define TASK_ERROR_NOT_READY TASK_BASE_ERROR | ERROR_NOT_READY
#define TASK_ERROR_INVALID_OPERATION TASK_BASE_ERROR | ERROR_INVALID_OPERATION
#define TASK_ERROR_RESOURCE_UNAVAILABLE
        TASK_BASE_ERROR |
    ERROR_RESOURCE_UNAVAILABLE
#define TASK_ERROR_OPERATION_FAILED TASK_BASE_ERROR | ERROR_OPERATION_FAILED
#define TASK_ERROR_DATA_CORRUPTION TASK_BASE_ERROR | ERROR_DATA_CORRUPTION
#define TASK_ERROR_FULL TASK_BASE_ERROR | ERROR_FULL

    /***********************************************************
     *           DEFINICIONES PUBLICAS  DEL _TASK_H_          *
     ***********************************************************/

    /***********************************************************
     *              TIPOS PUBLICOS DEL _TASK_H_               *
     ***********************************************************/

    typedef void (*TaskFunctionCallback)(void);

typedef enum {
  TASK_READY = 0,
  TASK_RUNNING,
  TASK_BLOCKED,
  TASK_SUSPENDED
} TaskState_t;

typedef struct {
  u16 taskId;
  const char *TaskName;
  TaskState_t TaskStatus;
  u8 TaskPriority;
  pu8 TaskStack;
  u16 TaskStackSize;
  u32 TaskStatePointer;
  TaskFunctionCallback TaskFunction;
} TCB_t;

/***********************************************************
 *           FUNCIONES PUBLICAS  DEL _TASK_H_             *
 ***********************************************************/

u16 Task_Create(TCB_t *TaskControlBlock, TaskFunctionCallback TaskFunction,
                u8 TaskPriority, pu8 TaskStack, u16 TaskStackSize,
                const char *TaskName);
u16 Task_GetIdByName(const char *TaskName);
u16 Task_Delete(TCB_t *TaskControlBlock);
u16 Task_Start(TCB_t *TaskControlBlock);
u16 Task_Stop(TCB_t *TaskControlBlock);
u16 Task_Suspend(TCB_t *TaskControlBlock);
u16 Task_Resume(TCB_t *TaskControlBlock);
u16 Task_Yield(TCB_t *TaskControlBlock);

#endif // _TASK_H_
