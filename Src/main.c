/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Primer entregable SOTR - 4 tareas en el RTOS propio
 *
 *  Tarea 1: LED1 parpadea a 0.5 Hz  (1000 ms apagado / 1000 ms prendido)
 *  Tarea 2: LED2 parpadea a 1 Hz    (500 ms / 500 ms)
 *  Tarea 3: LED3 parpadea a 5 Hz    (100 ms / 100 ms)
 *  Tarea 4: lee un boton y prende LED4 mientras este presionado
 *
 *  Conexiones (STM32F103 "Blue Pill") - cambialas si usas otros pines:
 *    LED1 -> PA0      LED2 -> PA1      LED3 -> PA2      LED4 -> PA3
 *    BOTON -> PB0 (el otro lado del boton a GND, se usa pull-up interno)
 *
 *  SEGUNDO ENTREGABLE (PONG): poner APP_PONG en 1. El juego esta en
 *  Pong.c / PongGame.c y sus conexiones estan descritas en Pong.h.
 ******************************************************************************
 */

/* 1 = segundo entregable (PONG), 0 = primer entregable (4 LEDs) */
#define APP_PONG 1

#include "Pong.h"
#include "RtosIncludes.h"

/***********************************************************
 *           REGISTROS DEL STM32F103 (GPIO y RCC)           *
 ***********************************************************/

#define RCC_APB2ENR (*(volatile u32 *)0x40021018)
#define RCC_IOPAEN (1U << 2)
#define RCC_IOPBEN (1U << 3)

#define GPIOA_BASE 0x40010800U
#define GPIOB_BASE 0x40010C00U

/* Offsets de los registros dentro de cada puerto */
#define GPIO_CRL(base) (*(volatile u32 *)((base) + 0x00)) /* pines 0..7  */
#define GPIO_CRH(base) (*(volatile u32 *)((base) + 0x04)) /* pines 8..15 */
#define GPIO_IDR(base) (*(volatile u32 *)((base) + 0x08)) /* lectura     */
#define GPIO_ODR(base) (*(volatile u32 *)((base) + 0x0C)) /* escritura   */

/* Configuracion de 4 bits por pin (CNF + MODE) */
#define GPIO_CFG_OUTPUT_PP_2MHZ 0x2U /* salida push-pull 2 MHz */
#define GPIO_CFG_INPUT_PULL 0x8U     /* entrada con pull-up/pull-down */

/* Pines usados */
#define LED1_PORT GPIOA_BASE
#define LED1_PIN 0
#define LED2_PORT GPIOA_BASE
#define LED2_PIN 1
#define LED3_PORT GPIOA_BASE
#define LED3_PIN 2
#define LED4_PORT GPIOA_BASE
#define LED4_PIN 3
#define BUTTON_PORT GPIOB_BASE
#define BUTTON_PIN 0

/***********************************************************
 *                 PROTOTIPOS DE FUNCIONES                  *
 ***********************************************************/

void TaskLed05Hz(void);
void TaskLed1Hz(void);
void TaskLed5Hz(void);
void TaskButton(void);

void GPIO_Init(void);
void GPIO_PinConfig(u32 Port, u8 Pin, u32 Config);
void GPIO_Write(u32 Port, u8 Pin, u8 State);
u8 GPIO_Read(u32 Port, u8 Pin);

/***********************************************************
 *              PILAS Y TCB DE CADA TAREA                   *
 ***********************************************************/

u8 StackLed05Hz[RTOS_MIN_STACK_SIZE] __attribute__((aligned(4)));
u8 StackLed1Hz[RTOS_MIN_STACK_SIZE] __attribute__((aligned(4)));
u8 StackLed5Hz[RTOS_MIN_STACK_SIZE] __attribute__((aligned(4)));
u8 StackButton[RTOS_MIN_STACK_SIZE] __attribute__((aligned(4)));

TCB_t TcbLed05Hz;
TCB_t TcbLed1Hz;
TCB_t TcbLed5Hz;
TCB_t TcbButton;

/***********************************************************
 *                          MAIN                            *
 ***********************************************************/

int main(void) {
  /* Inicializar el RTOS */
  Rtos_Init();

#if APP_PONG
  /* Segundo entregable: perifericos y tareas del PONG */
  Pong_Init();
#else
  /* Inicializar los pines */
  GPIO_Init();

  /* Crear las 4 tareas (prioridad: numero menor = mas prioridad) */
  Task_Create(&TcbButton, TaskButton, 0, StackButton, RTOS_MIN_STACK_SIZE,
              "Button");
  Task_Create(&TcbLed5Hz, TaskLed5Hz, 1, StackLed5Hz, RTOS_MIN_STACK_SIZE,
              "Led5Hz");
  Task_Create(&TcbLed1Hz, TaskLed1Hz, 2, StackLed1Hz, RTOS_MIN_STACK_SIZE,
              "Led1Hz");
  Task_Create(&TcbLed05Hz, TaskLed05Hz, 3, StackLed05Hz, RTOS_MIN_STACK_SIZE,
              "Led05Hz");
#endif

  /* Arrancar el scheduler (no deberia regresar) */
  Rtos_Start();

  /* Loop forever */
  for (;;)
    ;
}

/***********************************************************
 *                         TAREAS                           *
 ***********************************************************/

/* 0.5 Hz -> periodo de 2 s */
void TaskLed05Hz(void) {
  for (;;) {
    GPIO_Write(LED1_PORT, LED1_PIN, TRUE);
    Task_Delay(1000);
    GPIO_Write(LED1_PORT, LED1_PIN, FALSE);
    Task_Delay(1000);
  }
}

/* 1 Hz -> periodo de 1 s */
void TaskLed1Hz(void) {
  for (;;) {
    GPIO_Write(LED2_PORT, LED2_PIN, TRUE);
    Task_Delay(500);
    GPIO_Write(LED2_PORT, LED2_PIN, FALSE);
    Task_Delay(500);
  }
}

/* 5 Hz -> periodo de 200 ms */
void TaskLed5Hz(void) {
  for (;;) {
    GPIO_Write(LED3_PORT, LED3_PIN, TRUE);
    Task_Delay(100);
    GPIO_Write(LED3_PORT, LED3_PIN, FALSE);
    Task_Delay(100);
  }
}

/* Lee el boton cada 20 ms. Con pull-up: presionado = 0 */
void TaskButton(void) {
  for (;;) {
    if (GPIO_Read(BUTTON_PORT, BUTTON_PIN) == FALSE) {
      GPIO_Write(LED4_PORT, LED4_PIN, TRUE);
    } else {
      GPIO_Write(LED4_PORT, LED4_PIN, FALSE);
    }
    Task_Delay(20);
  }
}

/***********************************************************
 *                      DRIVER GPIO                         *
 ***********************************************************/

void GPIO_Init(void) {
  /* Habilitar reloj de GPIOA y GPIOB */
  RCC_APB2ENR |= RCC_IOPAEN | RCC_IOPBEN;

  /* LEDs como salida */
  GPIO_PinConfig(LED1_PORT, LED1_PIN, GPIO_CFG_OUTPUT_PP_2MHZ);
  GPIO_PinConfig(LED2_PORT, LED2_PIN, GPIO_CFG_OUTPUT_PP_2MHZ);
  GPIO_PinConfig(LED3_PORT, LED3_PIN, GPIO_CFG_OUTPUT_PP_2MHZ);
  GPIO_PinConfig(LED4_PORT, LED4_PIN, GPIO_CFG_OUTPUT_PP_2MHZ);

  /* Boton como entrada con pull-up (ODR = 1 activa el pull-up) */
  GPIO_PinConfig(BUTTON_PORT, BUTTON_PIN, GPIO_CFG_INPUT_PULL);
  GPIO_Write(BUTTON_PORT, BUTTON_PIN, TRUE);
}

void GPIO_PinConfig(u32 Port, u8 Pin, u32 Config) {
  if (Pin < 8) {
    GPIO_CRL(Port) &= ~(0xFU << (Pin * 4));
    GPIO_CRL(Port) |= (Config << (Pin * 4));
  } else {
    GPIO_CRH(Port) &= ~(0xFU << ((Pin - 8) * 4));
    GPIO_CRH(Port) |= (Config << ((Pin - 8) * 4));
  }
}

void GPIO_Write(u32 Port, u8 Pin, u8 State) {
  if (State) {
    GPIO_ODR(Port) |= (1U << Pin);
  } else {
    GPIO_ODR(Port) &= ~(1U << Pin);
  }
}

u8 GPIO_Read(u32 Port, u8 Pin) {
  if (GPIO_IDR(Port) & (1U << Pin)) {
    return TRUE;
  }
  return FALSE;
}