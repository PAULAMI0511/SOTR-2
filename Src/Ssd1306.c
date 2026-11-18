/***********************************************************
 *  Ssd1306.c - Driver de la pantalla OLED SSD1306 (128x64).
 *
 *  La memoria de la pantalla esta organizada en 8 "paginas"
 *  de 8 filas cada una: cada byte del framebuffer representa
 *  una columna de 8 pixeles (bit 0 = pixel de arriba).
 ***********************************************************/

#include "Ssd1306.h"
#include "I2c.h"

#define SSD1306_CONTROL_CMD 0x00
#define SSD1306_CONTROL_DATA 0x40
#define FONT_WIDTH 5
#define FONT_HEIGHT 7

static u8 FrameBuffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

/* Fuente de 5x7 pixeles (cada byte es una columna, bit 0 arriba).
 * Solo incluye los caracteres que usa el juego. */
static const struct {
  char c;
  u8 col[FONT_WIDTH];
} Font5x7[] = {
    {' ', {0x00, 0x00, 0x00, 0x00, 0x00}},
    {'!', {0x00, 0x00, 0x5F, 0x00, 0x00}},
    {'-', {0x08, 0x08, 0x08, 0x08, 0x08}},
    {':', {0x00, 0x36, 0x36, 0x00, 0x00}},
    {'0', {0x3E, 0x51, 0x49, 0x45, 0x3E}},
    {'1', {0x00, 0x42, 0x7F, 0x40, 0x00}},
    {'2', {0x42, 0x61, 0x51, 0x49, 0x46}},
    {'3', {0x21, 0x41, 0x45, 0x4B, 0x31}},
    {'4', {0x18, 0x14, 0x12, 0x7F, 0x10}},
    {'5', {0x27, 0x45, 0x45, 0x45, 0x39}},
    {'6', {0x3C, 0x4A, 0x49, 0x49, 0x30}},
    {'7', {0x01, 0x71, 0x09, 0x05, 0x03}},
    {'8', {0x36, 0x49, 0x49, 0x49, 0x36}},
    {'9', {0x06, 0x49, 0x49, 0x29, 0x1E}},
    {'A', {0x7E, 0x11, 0x11, 0x11, 0x7E}},
    {'B', {0x7F, 0x49, 0x49, 0x49, 0x36}},
    {'C', {0x3E, 0x41, 0x41, 0x41, 0x22}},
    {'D', {0x7F, 0x41, 0x41, 0x22, 0x1C}},
    {'E', {0x7F, 0x49, 0x49, 0x49, 0x41}},
    {'F', {0x7F, 0x09, 0x09, 0x09, 0x01}},
    {'G', {0x3E, 0x41, 0x49, 0x49, 0x7A}},
    {'H', {0x7F, 0x08, 0x08, 0x08, 0x7F}},
    {'I', {0x00, 0x41, 0x7F, 0x41, 0x00}},
    {'J', {0x20, 0x40, 0x41, 0x3F, 0x01}},
    {'K', {0x7F, 0x08, 0x14, 0x22, 0x41}},
    {'L', {0x7F, 0x40, 0x40, 0x40, 0x40}},
    {'M', {0x7F, 0x02, 0x0C, 0x02, 0x7F}},
    {'N', {0x7F, 0x04, 0x08, 0x10, 0x7F}},
    {'O', {0x3E, 0x41, 0x41, 0x41, 0x3E}},
    {'P', {0x7F, 0x09, 0x09, 0x09, 0x06}},
    {'Q', {0x3E, 0x41, 0x51, 0x21, 0x5E}},
    {'R', {0x7F, 0x09, 0x19, 0x29, 0x46}},
    {'S', {0x46, 0x49, 0x49, 0x49, 0x31}},
    {'T', {0x01, 0x01, 0x7F, 0x01, 0x01}},
    {'U', {0x3F, 0x40, 0x40, 0x40, 0x3F}},
    {'V', {0x1F, 0x20, 0x40, 0x20, 0x1F}},
    {'W', {0x3F, 0x40, 0x38, 0x40, 0x3F}},
    {'X', {0x63, 0x14, 0x08, 0x14, 0x63}},
    {'Y', {0x07, 0x08, 0x70, 0x08, 0x07}},
    {'Z', {0x61, 0x51, 0x49, 0x45, 0x43}},
};

#define FONT_COUNT (sizeof(Font5x7) / sizeof(Font5x7[0]))

/* Secuencia de inicializacion recomendada por el fabricante */
static const u8 InitCommands[] = {
    0xAE,       /* pantalla apagada                       */
    0xD5, 0x80, /* frecuencia del oscilador               */
    0xA8, 0x3F, /* 64 filas                               */
    0xD3, 0x00, /* sin desplazamiento vertical            */
    0x40,       /* linea de inicio = 0                    */
    0x8D, 0x14, /* activar la bomba de carga (charge pump) */
    0x20, 0x00, /* direccionamiento horizontal            */
    0xA1,       /* columnas invertidas (orientacion)      */
    0xC8,       /* filas invertidas (orientacion)         */
    0xDA, 0x12, /* configuracion de pines COM             */
    0x81, 0xCF, /* contraste                              */
    0xD9, 0xF1, /* periodo de pre-carga                   */
    0xDB, 0x40, /* nivel VCOMH                            */
    0xA4,       /* mostrar el contenido de la RAM         */
    0xA6,       /* modo normal (no invertido)             */
    0xAF,       /* pantalla encendida                     */
};

u16 Ssd1306_Init(void) {
  I2c_Init();
  Ssd1306_Clear();
  return I2c_Write(SSD1306_I2C_ADDRESS, SSD1306_CONTROL_CMD, InitCommands,
                   sizeof(InitCommands));
}

/*!
 * @brief  Envia todo el framebuffer (1024 bytes) a la pantalla.
 */
u16 Ssd1306_Update(void) {
  static const u8 Window[] = {
      0x21, 0, SSD1306_WIDTH - 1,        /* columnas 0..127 */
      0x22, 0, (SSD1306_HEIGHT / 8) - 1, /* paginas 0..7    */
  };
  u16 Res = I2c_Write(SSD1306_I2C_ADDRESS, SSD1306_CONTROL_CMD, Window,
                      sizeof(Window));
  if (Res != 0) {
    return Res;
  }
  return I2c_Write(SSD1306_I2C_ADDRESS, SSD1306_CONTROL_DATA, FrameBuffer,
                   sizeof(FrameBuffer));
}

void Ssd1306_Clear(void) {
  for (u16 i = 0; i < sizeof(FrameBuffer); i++) {
    FrameBuffer[i] = 0;
  }
}

void Ssd1306_DrawPixel(s16 x, s16 y, u8 On) {
  if (x < 0 || x >= SSD1306_WIDTH || y < 0 || y >= SSD1306_HEIGHT) {
    return;
  }
  u16 Index = (u16)(x + (y / 8) * SSD1306_WIDTH);
  if (On) {
    FrameBuffer[Index] |= (u8)(1U << (y % 8));
  } else {
    FrameBuffer[Index] &= (u8) ~(1U << (y % 8));
  }
}

void Ssd1306_FillRect(s16 x, s16 y, s16 w, s16 h, u8 On) {
  for (s16 i = x; i < x + w; i++) {
    for (s16 j = y; j < y + h; j++) {
      Ssd1306_DrawPixel(i, j, On);
    }
  }
}

void Ssd1306_DrawChar(s16 x, s16 y, char c, u8 Scale) {
  const u8 *Columns = Font5x7[0].col; /* por defecto: espacio */
  for (u16 k = 0; k < FONT_COUNT; k++) {
    if (Font5x7[k].c == c) {
      Columns = Font5x7[k].col;
      break;
    }
  }
  for (u8 col = 0; col < FONT_WIDTH; col++) {
    for (u8 row = 0; row < FONT_HEIGHT; row++) {
      if (Columns[col] & (1U << row)) {
        Ssd1306_FillRect(x + col * Scale, y + row * Scale, Scale, Scale, TRUE);
      }
    }
  }
}

void Ssd1306_DrawString(s16 x, s16 y, const char *Text, u8 Scale) {
  while (*Text) {
    Ssd1306_DrawChar(x, y, *Text++, Scale);
    x += (FONT_WIDTH + 1) * Scale;
  }
}

s16 Ssd1306_StringWidth(const char *Text, u8 Scale) {
  s16 n = 0;
  while (Text[n]) {
    n++;
  }
  return (n == 0) ? 0 : (s16)(n * (FONT_WIDTH + 1) * Scale - Scale);
}

const u8 *Ssd1306_GetBuffer(void) { return FrameBuffer; }
