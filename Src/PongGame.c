/***********************************************************
 *  PongGame.c - Reglas y dibujo del juego PONG.
 ***********************************************************/

#include "PongGame.h"
#include "Ssd1306.h"

#define BALL_SPEED_X_START 24 /* 1.5 px por paso  */
#define BALL_SPEED_X_MAX 56   /* 3.5 px por paso  */
#define BALL_SPEED_X_STEP 3   /* aumenta en cada golpe */
#define BALL_SPEED_Y_MAX 40

/***********************************************************
 *                  FUNCIONES PRIVADAS                      *
 ***********************************************************/

/* Generador de numeros pseudoaleatorios simple (congruencial lineal) */
static u32 PongGame_Random(PongGame_t *Game) {
  Game->Seed = Game->Seed * 1103515245U + 12345U;
  return (Game->Seed >> 16) & 0x7FFF;
}

static s16 Abs16(s16 v) { return (v < 0) ? (s16)-v : v; }

/* Pone la pelota en el centro, lista para salir hacia Direction (+1 / -1) */
static void PongGame_ResetBall(PongGame_t *Game, s8 Direction) {
  Game->BallX = ((128 - PONG_BALL_SIZE) / 2) * PONG_FP;
  Game->BallY =
      ((PONG_FIELD_TOP + PONG_FIELD_BOTTOM - PONG_BALL_SIZE) / 2) * PONG_FP;
  Game->VelX = (s16)(Direction * BALL_SPEED_X_START);
  Game->VelY = (s16)((s32)(PongGame_Random(Game) % 33) - 16); /* -1..+1 px */
  Game->ServeWait = PONG_SERVE_STEPS;
}

/* Convierte la lectura del potenciometro (0..4095) en la posicion de la
 * paleta dentro de la cancha */
static s16 PongGame_AdcToPaddle(u16 Adc) {
  s32 Range = PONG_FIELD_BOTTOM - PONG_FIELD_TOP + 1 - PONG_PADDLE_HEIGHT;
  return (s16)(PONG_FIELD_TOP + ((s32)Adc * Range) / 4096);
}

/* Revisa si la pelota choca con una paleta. Devuelve TRUE si reboto. */
static u8 PongGame_HitPaddle(PongGame_t *Game, s16 PaddleY) {
  s16 BallY = (s16)(Game->BallY / PONG_FP);
  if (BallY + PONG_BALL_SIZE <= PaddleY ||
      BallY >= PaddleY + PONG_PADDLE_HEIGHT) {
    return FALSE;
  }
  /* El angulo depende de donde golpea: en el centro sale recta,
   * en los bordes sale mas inclinada */
  s16 Offset =
      (s16)((BallY + PONG_BALL_SIZE / 2) - (PaddleY + PONG_PADDLE_HEIGHT / 2));
  s16 VelY = (s16)(Offset * 6);
  if (VelY > BALL_SPEED_Y_MAX) {
    VelY = BALL_SPEED_Y_MAX;
  } else if (VelY < -BALL_SPEED_Y_MAX) {
    VelY = -BALL_SPEED_Y_MAX;
  }
  Game->VelY = VelY;

  /* Invertir la direccion y acelerar un poco */
  s16 Speed = Abs16(Game->VelX) + BALL_SPEED_X_STEP;
  if (Speed > BALL_SPEED_X_MAX) {
    Speed = BALL_SPEED_X_MAX;
  }
  Game->VelX = (Game->VelX < 0) ? Speed : (s16)-Speed;
  return TRUE;
}

/* Suma un punto. Devuelve el sonido correspondiente. */
static PongSound_t PongGame_Score(PongGame_t *Game, u8 Player) {
  if (Player == 1) {
    Game->Score1++;
  } else {
    Game->Score2++;
  }
  if (Game->Score1 >= PONG_WIN_SCORE || Game->Score2 >= PONG_WIN_SCORE) {
    Game->Winner = Player;
    Game->State = PONG_STATE_GAME_OVER;
    return PONG_SOUND_WIN;
  }
  /* Saca hacia el jugador que perdio el punto */
  PongGame_ResetBall(Game, (Player == 1) ? 1 : -1);
  return PONG_SOUND_POINT;
}

static void PongGame_DrawCentered(s16 y, const char *Text, u8 Scale) {
  Ssd1306_DrawString((s16)((128 - Ssd1306_StringWidth(Text, Scale)) / 2), y,
                     Text, Scale);
}

/***********************************************************
 *                  FUNCIONES PUBLICAS                      *
 ***********************************************************/

void PongGame_Init(PongGame_t *Game) {
  Game->State = PONG_STATE_MENU;
  Game->Score1 = 0;
  Game->Score2 = 0;
  Game->Winner = 0;
  Game->Seed = 0x1234;
  Game->Paddle1Y =
      (PONG_FIELD_TOP + PONG_FIELD_BOTTOM - PONG_PADDLE_HEIGHT) / 2;
  Game->Paddle2Y = Game->Paddle1Y;
  PongGame_ResetBall(Game, 1);
}

void PongGame_SetPaddles(PongGame_t *Game, u16 Adc1, u16 Adc2) {
  Game->Paddle1Y = PongGame_AdcToPaddle(Adc1);
  Game->Paddle2Y = PongGame_AdcToPaddle(Adc2);
  /* el movimiento de los jugadores tambien "revuelve" la semilla */
  Game->Seed += (u32)Adc1 + ((u32)Adc2 << 12);
}

/*!
 * @brief  Boton de inicio/fin:
 *         - en el menu o al terminar: empieza una partida nueva.
 *         - durante la partida: la termina y vuelve al menu.
 */
PongSound_t PongGame_StartPressed(PongGame_t *Game) {
  if (Game->State == PONG_STATE_PLAYING) {
    Game->State = PONG_STATE_MENU;
    return PONG_SOUND_WIN;
  }
  Game->Score1 = 0;
  Game->Score2 = 0;
  Game->Winner = 0;
  Game->State = PONG_STATE_PLAYING;
  PongGame_ResetBall(Game, (PongGame_Random(Game) & 1) ? 1 : -1);
  return PONG_SOUND_START;
}

/*!
 * @brief  Avanza el juego un paso: mueve la pelota y revisa choques.
 * @retval Sonido que se debe reproducir (o PONG_SOUND_NONE).
 */
PongSound_t PongGame_Step(PongGame_t *Game) {
  if (Game->State != PONG_STATE_PLAYING) {
    return PONG_SOUND_NONE;
  }
  if (Game->ServeWait > 0) {
    Game->ServeWait--;
    return PONG_SOUND_NONE;
  }

  PongSound_t Sound = PONG_SOUND_NONE;
  Game->BallX += Game->VelX;
  Game->BallY += Game->VelY;

  /* Rebote en la pared de arriba y la de abajo */
  s32 Top = PONG_FIELD_TOP * PONG_FP;
  s32 Bottom = (PONG_FIELD_BOTTOM + 1 - PONG_BALL_SIZE) * PONG_FP;
  if (Game->BallY < Top) {
    Game->BallY = Top + (Top - Game->BallY);
    Game->VelY = (s16)-Game->VelY;
    Sound = PONG_SOUND_WALL;
  } else if (Game->BallY > Bottom) {
    Game->BallY = Bottom - (Game->BallY - Bottom);
    Game->VelY = (s16)-Game->VelY;
    Sound = PONG_SOUND_WALL;
  }

  s16 BallX = (s16)(Game->BallX / PONG_FP);

  /* Paleta del jugador 1 (izquierda) */
  if (Game->VelX < 0 && BallX <= PONG_PADDLE1_X + PONG_PADDLE_WIDTH &&
      BallX + PONG_BALL_SIZE > PONG_PADDLE1_X) {
    if (PongGame_HitPaddle(Game, Game->Paddle1Y)) {
      Game->BallX = (PONG_PADDLE1_X + PONG_PADDLE_WIDTH) * PONG_FP;
      Sound = PONG_SOUND_PADDLE;
    }
  }
  /* Paleta del jugador 2 (derecha) */
  if (Game->VelX > 0 && BallX + PONG_BALL_SIZE >= PONG_PADDLE2_X &&
      BallX < PONG_PADDLE2_X + PONG_PADDLE_WIDTH) {
    if (PongGame_HitPaddle(Game, Game->Paddle2Y)) {
      Game->BallX = (PONG_PADDLE2_X - PONG_BALL_SIZE) * PONG_FP;
      Sound = PONG_SOUND_PADDLE;
    }
  }

  /* La pelota salio de la cancha: punto para el otro jugador */
  if (Game->BallX < 0) {
    Sound = PongGame_Score(Game, 2);
  } else if (Game->BallX > (128 - PONG_BALL_SIZE) * PONG_FP) {
    Sound = PongGame_Score(Game, 1);
  }
  return Sound;
}

/*!
 * @brief  Dibuja el estado del juego en el framebuffer de la pantalla.
 */
void PongGame_Draw(const PongGame_t *Game) {
  Ssd1306_Clear();

  if (Game->State == PONG_STATE_MENU) {
    PongGame_DrawCentered(8, "PONG", 3);
    PongGame_DrawCentered(38, "PRESIONE", 1);
    PongGame_DrawCentered(48, "INICIO", 1);
    return;
  }

  if (Game->State == PONG_STATE_GAME_OVER) {
    PongGame_DrawCentered(4, "FIN DEL JUEGO", 1);
    PongGame_DrawCentered(18, (Game->Winner == 1) ? "GANA J1" : "GANA J2", 2);
    char Score[] = {(char)('0' + Game->Score1), ' ', '-', ' ',
                    (char)('0' + Game->Score2), '\0'};
    PongGame_DrawCentered(38, Score, 1);
    PongGame_DrawCentered(52, "PRESIONE INICIO", 1);
    return;
  }

  /* Marcador */
  char S1[] = {(char)('0' + Game->Score1), '\0'};
  char S2[] = {(char)('0' + Game->Score2), '\0'};
  Ssd1306_DrawString(46, 1, S1, 1);
  Ssd1306_DrawString(78, 1, S2, 1);
  Ssd1306_DrawString(4, 1, "J1", 1);
  Ssd1306_DrawString(113, 1, "J2", 1);

  /* Linea superior de la cancha y red punteada en el centro */
  Ssd1306_FillRect(0, PONG_FIELD_TOP - 1, 128, 1, TRUE);
  for (s16 y = PONG_FIELD_TOP + 1; y <= PONG_FIELD_BOTTOM; y += 4) {
    Ssd1306_FillRect(63, y, 1, 2, TRUE);
  }

  /* Paletas y pelota */
  Ssd1306_FillRect(PONG_PADDLE1_X, Game->Paddle1Y, PONG_PADDLE_WIDTH,
                   PONG_PADDLE_HEIGHT, TRUE);
  Ssd1306_FillRect(PONG_PADDLE2_X, Game->Paddle2Y, PONG_PADDLE_WIDTH,
                   PONG_PADDLE_HEIGHT, TRUE);
  Ssd1306_FillRect((s16)(Game->BallX / PONG_FP), (s16)(Game->BallY / PONG_FP),
                   PONG_BALL_SIZE, PONG_BALL_SIZE, TRUE);
}