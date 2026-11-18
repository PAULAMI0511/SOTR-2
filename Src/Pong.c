/***********************************************************
 *  Pong.c - Tareas del juego PONG y su sincronizacion.
 ***********************************************************/

#include "Pong.h"
#include "Adc.h"
#include "Buzzer.h"
#include "Gpio.h"
#include "PongGame.h"
#include "RtosIncludes.h"
#include "Ssd1306.h"

/***********************************************************
 *                    DEFINICIONES                          *
 ***********************************************************/

#define BUTTON_PORT GPIOB_BASE
#define BUTTON_PIN 0
#define POT1_CHANNEL 0 /* PA0 */
#define POT2_CHANNEL 1 /* PA1 */

#define INPUT_PERIOD_MS 20
#define GAME_PERIOD_MS 25
#define DISPLAY_PERIOD_MS 30

#define SOUND_STACK_SIZE 384
#define INPUT_STACK_SIZE 384
#define GAME_STACK_SIZE 384
#define DISPLAY_STACK_SIZE 512

/***********************************************************
 *              VARIABLES COMPARTIDAS                       *
 ***********************************************************/

/* Estado del juego: lo leen y escriben 3 tareas -> protegido por mutex */
static PongGame_t Game;
static Mutex_t GameMutex;

/* Aviso de sonido: el juego "da" el semaforo y la tarea de sonido lo
 * "toma". PendingSound dice que sonido toca. */
static Semaphore_t SoundSem;
static volatile PongSound_t PendingSound = PONG_SOUND_NONE;

/***********************************************************
 *                 PILAS Y TCB DE LAS TAREAS                *
 ***********************************************************/

static u8 SoundStack[SOUND_STACK_SIZE] __attribute__((aligned(8)));
static u8 InputStack[INPUT_STACK_SIZE] __attribute__((aligned(8)));
static u8 GameStack[GAME_STACK_SIZE] __attribute__((aligned(8)));
static u8 DisplayStack[DISPLAY_STACK_SIZE] __attribute__((aligned(8)));

static TCB_t SoundTcb;
static TCB_t InputTcb;
static TCB_t GameTcb;
static TCB_t DisplayTcb;

/***********************************************************
 *                 FUNCIONES PRIVADAS                       *
 ***********************************************************/

static void TaskSound(void);
static void TaskInput(void);
static void TaskGame(void);
static void TaskDisplay(void);

/* Pide un sonido a la tarea de sonido (no bloquea a quien lo pide) */
static void Pong_RequestSound(PongSound_t Sound) {
  if (Sound != PONG_SOUND_NONE) {
    PendingSound = Sound;
    Semaphore_Give(&SoundSem);
  }
}

/* Promedio de varias lecturas para que la paleta no "tiemble" */
static u16 Pong_ReadPot(u8 Channel) {
  u32 Sum = 0;
  for (u8 i = 0; i < 4; i++) {
    Sum += Adc_Read(Channel);
  }
  return (u16)(Sum / 4);
}

static void Pong_Beep(u32 OnMs, u32 OffMs, u8 Times) {
  for (u8 i = 0; i < Times; i++) {
    Buzzer_On();
    Task_Delay(OnMs);
    Buzzer_Off();
    if (OffMs > 0) {
      Task_Delay(OffMs);
    }
  }
}

/***********************************************************
 *                 FUNCIONES PUBLICAS                       *
 ***********************************************************/

/*!
 * @brief  Inicializa perifericos, mutex, semaforo y crea las 4 tareas.
 *         Se llama desde main despues de Rtos_Init().
 */
void Pong_Init(void) {
  /* Perifericos (la OLED se inicializa en su propia tarea) */
  Adc_Init();
  Buzzer_Init();
  Gpio_EnablePort(BUTTON_PORT);
  Gpio_Config(BUTTON_PORT, BUTTON_PIN, GPIO_MODE_INPUT_PULL);
  Gpio_Write(BUTTON_PORT, BUTTON_PIN, TRUE); /* activa el pull-up */

  /* Sincronizacion */
  Mutex_Init(&GameMutex);
  Semaphore_Init(&SoundSem, 0, 1); /* semaforo binario, empieza "vacio" */

  PongGame_Init(&Game);

  /* Tareas (numero menor = mayor prioridad) */
  Task_Create(&SoundTcb, TaskSound, 0, SoundStack, SOUND_STACK_SIZE, "Sound");
  Task_Create(&InputTcb, TaskInput, 1, InputStack, INPUT_STACK_SIZE, "Input");
  Task_Create(&GameTcb, TaskGame, 2, GameStack, GAME_STACK_SIZE, "Game");
  Task_Create(&DisplayTcb, TaskDisplay, 3, DisplayStack, DISPLAY_STACK_SIZE,
              "Display");
}

/***********************************************************
 *                        TAREAS                            *
 ***********************************************************/

/*!
 * @brief  Duerme en el semaforo hasta que haya un sonido que reproducir.
 *         Mientras espera NO consume CPU (esta bloqueada).
 */
static void TaskSound(void) {
  for (;;) {
    Semaphore_Take(&SoundSem);
    PongSound_t Sound = PendingSound;
    PendingSound = PONG_SOUND_NONE;

    switch (Sound) {
    case PONG_SOUND_WALL:
      Pong_Beep(15, 0, 1);
      break;
    case PONG_SOUND_PADDLE:
      Pong_Beep(35, 0, 1);
      break;
    case PONG_SOUND_POINT:
      Pong_Beep(80, 60, 2);
      break;
    case PONG_SOUND_START:
      Pong_Beep(60, 50, 3);
      break;
    case PONG_SOUND_WIN:
      Pong_Beep(400, 0, 1);
      break;
    default:
      break;
    }
  }
}

/*!
 * @brief  Lee los potenciometros y el boton de inicio/fin.
 *         El boton se "filtra" (antirrebote): solo cuenta si se lee
 *         presionado dos veces seguidas, y solo en el flanco de bajada.
 */
static void TaskInput(void) {
  u8 LastStable = FALSE;
  u8 LastRead = FALSE;
  for (;;) {
    u16 Pot1 = Pong_ReadPot(POT1_CHANNEL);
    u16 Pot2 = Pong_ReadPot(POT2_CHANNEL);

    /* Con pull-up: presionado = 0 */
    u8 Pressed = (Gpio_Read(BUTTON_PORT, BUTTON_PIN) == FALSE);
    u8 NewPress = FALSE;
    if (Pressed == LastRead && Pressed != LastStable) {
      LastStable = Pressed;
      NewPress = Pressed;
    }
    LastRead = Pressed;

    PongSound_t Sound = PONG_SOUND_NONE;
    Mutex_Take(&GameMutex);
    PongGame_SetPaddles(&Game, Pot1, Pot2);
    if (NewPress) {
      Sound = PongGame_StartPressed(&Game);
    }
    Mutex_Release(&GameMutex);

    Pong_RequestSound(Sound);
    Task_Delay(INPUT_PERIOD_MS);
  }
}

/*!
 * @brief  Avanza la fisica del juego a ritmo constante.
 */
static void TaskGame(void) {
  for (;;) {
    Mutex_Take(&GameMutex);
    PongSound_t Sound = PongGame_Step(&Game);
    Mutex_Release(&GameMutex);

    Pong_RequestSound(Sound);
    Task_Delay(GAME_PERIOD_MS);
  }
}

/*!
 * @brief  Copia el estado (con el mutex tomado el menor tiempo posible),
 *         dibuja y envia la imagen por I2C.
 */
static void TaskDisplay(void) {
  Task_Delay(100); /* la OLED necesita un momento despues de encender */
  while (Ssd1306_Init() != 0) {
    Task_Delay(500); /* si no responde, reintentar */
  }

  for (;;) {
    PongGame_t Snapshot;
    Mutex_Take(&GameMutex);
    Snapshot = Game;
    Mutex_Release(&GameMutex);

    PongGame_Draw(&Snapshot);
    if (Ssd1306_Update() != 0) {
      Ssd1306_Init(); /* error de comunicacion: reiniciar la pantalla */
    }
    Task_Delay(DISPLAY_PERIOD_MS);
  }
}
