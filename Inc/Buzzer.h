#ifndef _BUZZER_H_
#define _BUZZER_H_

/***********************************************************
 *  Driver del buzzer activo (PA8).
 ***********************************************************/

#include "AppTypes.h"

/* Si tu modulo de buzzer suena con 0 V (activo en bajo), cambia a FALSE */
#define BUZZER_ACTIVE_LEVEL TRUE

void Buzzer_Init(void);
void Buzzer_On(void);
void Buzzer_Off(void);

#endif // _BUZZER_H_
