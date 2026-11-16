/***********************************************************
 *            INCLUSION LIBRERIAS .h                        *
 ***********************************************************/

#include "RtosIncludes.h"

/***********************************************************
 *              VARIABLES GLOBALES DEL RTOS                 *
 ***********************************************************/

/* Tarea Idle: corre solo cuando todas las demas estan dormidas */
static u8 IdleTaskStack[RTOS_MIN_STACK_SIZE] __attribute__((aligned(8)));
static TCB_t IdleTask;

/***********************************************************
 *    DEFINICION DE FUNCIONES  PRIVADAS  DEL RTOS           *
 ***********************************************************/

static void Rtos_IdleTask(void);

/***********************************************************
 *  IMPLEMENTACION FUNCIONES PUBLICAS  DEL RTOS             *
 ***********************************************************/

/*!
 * @brief  Inicializa reloj y scheduler, y crea la tarea Idle.
 *         Se llama al inicio de main, antes de crear las tareas.
 */
u16 Rtos_Init(void) {
  /* Si el PLL no arranca, se sigue con el HSI a 8 MHz (el tick se ajusta
   * solo porque Portable usa la frecuencia real del reloj) */
  Portable_InitInternalClock16MHz();
  Scheduler_Init();
  if (Task_Create(&IdleTask, Rtos_IdleTask, RTOS_MIN_PRIORITY, IdleTaskStack,
                  RTOS_MIN_STACK_SIZE, "Idle") != TASK_OK) {
    return RTOS_ERROR_FAIL;
  }
  return RTOS_OK;
}

/*!
 * @brief  Arranca el tick y la primera tarea. No retorna.
 */
void Rtos_Start(void) {
  Portable_DisableInterrupts();
  Portable_InitTickTimer();
  TCB_Handle_t FirstTask = Scheduler_SelectNextTask();
  Portable_StartFirstTask(FirstTask->TaskStackPointer);
}

/***********************************************************
 *  IMPLEMENTACION FUNCIONES PRIVADAS  DEL RTOS             *
 ***********************************************************/

static void Rtos_IdleTask(void) {
  for (;;) {
  }
}
