#ifndef _SSD1306_H_
#define _SSD1306_H_

/***********************************************************
 *  Driver de la pantalla OLED SSD1306 128x64 por I2C.
 *  Se dibuja en un "framebuffer" en RAM y luego se envia
 *  completo a la pantalla con Ssd1306_Update().
 ***********************************************************/

#include "AppTypes.h"

#define SSD1306_WIDTH 128
#define SSD1306_HEIGHT 64
#define SSD1306_I2C_ADDRESS 0x3C /* algunas pantallas usan 0x3D */

u16 Ssd1306_Init(void);
u16 Ssd1306_Update(void);

void Ssd1306_Clear(void);
void Ssd1306_DrawPixel(s16 x, s16 y, u8 On);
void Ssd1306_FillRect(s16 x, s16 y, s16 w, s16 h, u8 On);
void Ssd1306_DrawChar(s16 x, s16 y, char c, u8 Scale);
void Ssd1306_DrawString(s16 x, s16 y, const char *Text, u8 Scale);
s16 Ssd1306_StringWidth(const char *Text, u8 Scale);

/* Acceso al framebuffer (para pruebas) */
const u8 *Ssd1306_GetBuffer(void);

#endif // _SSD1306_H_
