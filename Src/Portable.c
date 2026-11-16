/***********************************************************
 *  Portable.c - Todo lo que depende del STM32F103 (Cortex-M3):
 *    - Reloj del sistema (HSI + PLL a 16 MHz)
 *    - SysTick: interrupcion cada 1 ms (tick del RTOS)
 *    - PendSV: cambio de contexto entre tareas
 *    - Contexto inicial de cada tarea en su pila
 ***********************************************************/

/***********************************************************
 *            INCLUSION LIBRERIAS .h                        *
 ***********************************************************/

#include "RtosIncludes.h"

/***********************************************************
 *           DEFINICIONES PRIVADAS  DEL PORTABLE            *
 ***********************************************************/

/* RCC (reloj) */
#define RCC_CR (*(volatile u32 *)0x40021000)
#define RCC_CFGR (*(volatile u32 *)0x40021004)
#define RCC_CR_HSION (1U << 0)
#define RCC_CR_HSIRDY (1U << 1)
#define RCC_CR_PLLON (1U << 24)
#define RCC_CR_PLLRDY (1U << 25)
#define RCC_CFGR_SW_MASK (3U << 0)
#define RCC_CFGR_SW_PLL (2U << 0)
#define RCC_CFGR_SWS_MASK (3U << 2)
#define RCC_CFGR_SWS_PLL (2U << 2)
#define RCC_CFGR_PLLSRC (1U << 16)
#define RCC_CFGR_PLLMUL_MASK (0xFU << 18)
#define RCC_CFGR_PLLMUL_X4 (0x2U << 18)

/* SysTick */
#define SYST_CSR (*(volatile u32 *)0xE000E010)
#define SYST_RVR (*(volatile u32 *)0xE000E014)
#define SYST_CVR (*(volatile u32 *)0xE000E018)
#define SYST_CSR_ENABLE (1U << 0)
#define SYST_CSR_TICKINT (1U << 1)
#define SYST_CSR_CLKSOURCE (1U << 2)

/* SCB */
#define SCB_ICSR (*(volatile u32 *)0xE000ED04)
#define SCB_ICSR_PENDSVSET (1U << 28)
#define SCB_SHPR3 (*(volatile u32 *)0xE000ED20)

#define CLOCK_TIMEOUT 100000U
#define INITIAL_XPSR 0x01000000U /* bit T = 1 (modo Thumb) */

/***********************************************************
 *              VARIABLES GLOBALES DEL PORTABLE             *
 ***********************************************************/

static u32 SystemCoreClock = 8000000U; /* HSI por defecto */
static volatile u64 SystemTicks = 0;
static volatile u32 CriticalNesting = 0;

/***********************************************************
 *    DEFINICION DE FUNCIONES  PRIVADAS  DEL PORTABLE       *
 ***********************************************************/

static void Portable_TaskExit(void);
u32 Portable_SwitchContext(u32 StackPointer);
void Portable_RestoreFirstContext(u32 StackPointer);
void SysTick_Handler(void);
void PendSV_Handler(void);

/***********************************************************
 *  IMPLEMENTACION FUNCIONES PUBLICAS  DEL PORTABLE         *
 ***********************************************************/

/*!
 * @brief  HSI (8 MHz) / 2 = 4 MHz -> PLL x4 = 16 MHz como reloj del sistema.
 */
u16 Portable_InitInternalClock16MHz(void) {
  u32 Timeout = CLOCK_TIMEOUT;

  RCC_CR |= RCC_CR_HSION;
  while (!(RCC_CR & RCC_CR_HSIRDY)) {
    if (--Timeout == 0) {
      return PORTABLE_ERROR_TIMEOUT;
    }
  }

  RCC_CFGR &= ~(RCC_CFGR_PLLMUL_MASK | RCC_CFGR_PLLSRC); /* PLL <- HSI/2 */
  RCC_CFGR |= RCC_CFGR_PLLMUL_X4;

  RCC_CR |= RCC_CR_PLLON;
  Timeout = CLOCK_TIMEOUT;
  while (!(RCC_CR & RCC_CR_PLLRDY)) {
    if (--Timeout == 0) {
      return PORTABLE_ERROR_TIMEOUT;
    }
  }

  RCC_CFGR = (RCC_CFGR & ~RCC_CFGR_SW_MASK) | RCC_CFGR_SW_PLL;
  Timeout = CLOCK_TIMEOUT;
  while ((RCC_CFGR & RCC_CFGR_SWS_MASK) != RCC_CFGR_SWS_PLL) {
    if (--Timeout == 0) {
      return PORTABLE_ERROR_TIMEOUT;
    }
  }

  SystemCoreClock = 16000000U;
  return PORTABLE_OK;
}

/*!
 * @brief  Configura SysTick para interrumpir RTOS_TICK_RATE_HZ veces por
 *         segundo. PendSV y SysTick quedan con la prioridad mas baja.
 */
void Portable_InitTickTimer(void) {
  SCB_SHPR3 |= (0xFFU << 16) | (0xFFU << 24); /* PendSV y SysTick */
  SYST_RVR = (SystemCoreClock / RTOS_TICK_RATE_HZ) - 1U;
  SYST_CVR = 0;
  SYST_CSR = SYST_CSR_CLKSOURCE | SYST_CSR_TICKINT | SYST_CSR_ENABLE;
}

u64 Portable_GetSystemTicks(void) {
  u64 Ticks;
  Portable_DisableInterrupts();
  Ticks = SystemTicks;
  Portable_EnableInterrupts();
  return Ticks;
}

/*!
 * @brief  Seccion critica. Se pueden anidar: solo la ultima llamada a
 *         Portable_EnableInterrupts vuelve a habilitar las interrupciones.
 */
void Portable_DisableInterrupts(void) {
  __asm volatile("cpsid i" ::: "memory");
  CriticalNesting++;
}

void Portable_EnableInterrupts(void) {
  if (CriticalNesting > 0) {
    CriticalNesting--;
  }
  if (CriticalNesting == 0) {
    __asm volatile("cpsie i" ::: "memory");
  }
}

/*!
 * @brief  Arma en la pila de la tarea el mismo "marco" que guardaria el
 *         procesador al entrar a una interrupcion, para que al "volver" de
 *         PendSV la tarea empiece en su funcion.
 *
 *   (arriba)  xPSR, PC, LR, R12, R3, R2, R1, R0   <- los guarda el hardware
 *             R11, R10, R9, R8, R7, R6, R5, R4    <- los guarda PendSV
 *   (abajo)   <- valor que se devuelve (TaskStackPointer)
 */
u32 Portable_CreateContext(pu8 StackPointer, u16 StackSize, u32 Function) {
  if (StackPointer == NULL || StackSize < RTOS_MIN_STACK_SIZE) {
    return 0;
  }

  /* Tope de la pila alineado a 8 bytes (la pila crece hacia abajo) */
  pu32 Sp = (pu32)(((u32)StackPointer + StackSize) & ~7U);

  *(--Sp) = INITIAL_XPSR;           /* xPSR */
  *(--Sp) = Function & ~1U;         /* PC  -> funcion de la tarea */
  *(--Sp) = (u32)Portable_TaskExit; /* LR  -> si la tarea retorna */
  *(--Sp) = 0;                      /* R12 */
  *(--Sp) = 0;                      /* R3  */
  *(--Sp) = 0;                      /* R2  */
  *(--Sp) = 0;                      /* R1  */
  *(--Sp) = 0;                      /* R0  */
  for (u8 i = 0; i < 8; i++) {
    *(--Sp) = 0; /* R11 .. R4 */
  }
  return (u32)Sp;
}

/*!
 * @brief  Pide un cambio de contexto: activa PendSV, que se ejecuta en
 *         cuanto no haya otra interrupcion en curso.
 */
void Portable_CallScheduller(void) {
  SCB_ICSR = SCB_ICSR_PENDSVSET;
  __asm volatile("dsb\n isb" ::: "memory");
}

/*!
 * @brief  Arranca la primera tarea. No retorna.
 */
void Portable_StartFirstTask(u32 StackPointer) {
  __asm volatile("cpsid i" ::: "memory");
  CriticalNesting = 0;
  Portable_RestoreFirstContext(StackPointer);
}

/***********************************************************
 *              MANEJADORES DE INTERRUPCION                  *
 ***********************************************************/

/*!
 * @brief  Tick del RTOS (cada 1 ms): cuenta el tiempo, despierta tareas
 *         dormidas y pide al scheduler que revise quien debe correr.
 */
void SysTick_Handler(void) {
  SystemTicks++;
  Scheduler_Tick(SystemTicks);
  Portable_CallScheduller();
}

/*!
 * @brief  Cambio de contexto.
 *   1. El hardware ya guardo R0-R3, R12, LR, PC, xPSR en la pila de la tarea.
 *   2. Aqui se guardan R4-R11 y se anota el SP en el TCB de la tarea actual.
 *   3. El scheduler escoge la siguiente tarea y se recupera su SP.
 *   4. Se sacan R4-R11 y al salir el hardware saca el resto.
 */
__attribute__((naked)) void PendSV_Handler(void) {
  __asm volatile("cpsid i                    \n"
                 "mrs   r0, psp              \n"
                 "stmdb r0!, {r4-r11}        \n"
                 "bl    Portable_SwitchContext \n"
                 "ldmia r0!, {r4-r11}        \n"
                 "msr   psp, r0              \n"
                 "mvn   lr, #2               \n" /* LR = 0xFFFFFFFD */
                 "cpsie i                    \n"
                 "bx    lr                   \n");
}

/***********************************************************
 *  IMPLEMENTACION FUNCIONES PRIVADAS  DEL PORTABLE         *
 ***********************************************************/

/*!
 * @brief  Llamada desde PendSV: guarda el SP de la tarea que sale y
 *         devuelve el SP de la tarea que entra.
 */
__attribute__((used)) u32 Portable_SwitchContext(u32 StackPointer) {
  TCB_Handle_t Task = Scheduler_GetCurrentTask();
  if (Task != NULL) {
    Task->TaskStackPointer = StackPointer;
  }
  Task = Scheduler_SelectNextTask();
  return Task->TaskStackPointer;
}

/*!
 * @brief  Carga el contexto armado por Portable_CreateContext y salta a la
 *         primera tarea usando la pila de proceso (PSP).
 */
__attribute__((naked)) void Portable_RestoreFirstContext(u32 StackPointer) {
  __asm volatile("ldmia r0!, {r4-r11}        \n" /* R4-R11 iniciales     */
                 "msr   psp, r0              \n"
                 "movs  r0, #2               \n" /* CONTROL.SPSEL = PSP  */
                 "msr   control, r0          \n"
                 "isb                        \n"
                 "pop   {r0-r3, r12, lr}     \n" /* R0-R3, R12, LR       */
                 "pop   {r4, r5}             \n" /* PC, xPSR             */
                 "orr   r4, r4, #1           \n" /* bit Thumb            */
                 "cpsie i                    \n"
                 "bx    r4                   \n");
}

/*!
 * @brief  Una tarea nunca debe terminar (siempre es un for(;;)).
 *         Si lo hace, se queda aqui.
 */
static void Portable_TaskExit(void) {
  Portable_DisableInterrupts();
  for (;;) {
  }
}
