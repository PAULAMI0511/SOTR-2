#ifndef _PONG_GAME_H_
#define _PONG_GAME_H_

/***********************************************************
 *  Logica del juego PONG (independiente del RTOS y del
 *  hardware): estado del juego, movimiento de la pelota,
 *  choques, marcador y dibujo en el framebuffer.
 ***********************************************************/

#include "AppTypes.h"

/* --------- Configuracion del juego --------- */
#define PONG_WIN_SCORE 5  /* puntos para ganar               */
#define PONG_FIELD_TOP 10 /* debajo del marcador             */
#define PONG_FIELD_BOTTOM 63
#define PONG_PADDLE_HEIGHT 12
#define PONG_PADDLE_WIDTH 2
#define PONG_PADDLE1_X 2
#define PONG_PADDLE2_X 124
#define PONG_BALL_SIZE 3
#define PONG_SERVE_STEPS 40 /* pausa antes de sacar (pasos)    */

/* Las posiciones de la pelota se guardan en "punto fijo": valor * 16.
 * Asi se pueden manejar velocidades como 1.5 pixeles por paso sin usar
 * numeros decimales (float). */
#define PONG_FP 16

typedef enum {
  PONG_STATE_MENU = 0,
  PONG_STATE_PLAYING,
  PONG_STATE_GAME_OVER
} PongState_t;

typedef enum {
  PONG_SOUND_NONE = 0,
  PONG_SOUND_WALL,   /* rebote en la pared   */
  PONG_SOUND_PADDLE, /* rebote en una paleta */
  PONG_SOUND_POINT,  /* alguien anoto        */
  PONG_SOUND_START,  /* inicia el juego      */
  PONG_SOUND_WIN     /* fin del juego        */
} PongSound_t;

typedef struct {
  PongState_t State;
  s16 Paddle1Y; /* borde superior de cada paleta (pixeles) */
  s16 Paddle2Y;
  s32 BallX; /* posicion de la pelota (punto fijo) */
  s32 BallY;
  s16 VelX; /* velocidad (punto fijo por paso) */
  s16 VelY;
  u8 Score1;
  u8 Score2;
  u8 Winner;     /* 1 o 2 cuando termina el juego          */
  u16 ServeWait; /* pasos que faltan para sacar la pelota  */
  u32 Seed;      /* semilla para el numero pseudoaleatorio */
} PongGame_t;

void PongGame_Init(PongGame_t *Game);
void PongGame_SetPaddles(PongGame_t *Game, u16 Adc1, u16 Adc2);
PongSound_t PongGame_StartPressed(PongGame_t *Game);
PongSound_t PongGame_Step(PongGame_t *Game);
void PongGame_Draw(const PongGame_t *Game);

#endif // _PONG_GAME_H_