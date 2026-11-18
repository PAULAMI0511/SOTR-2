#ifndef _ADC_H_
#define _ADC_H_

/***********************************************************
 *  Driver del conversor analogo-digital ADC1 (12 bits).
 *  Lee el voltaje de los potenciometros: 0 V -> 0, 3.3 V -> 4095.
 ***********************************************************/

#include "AppTypes.h"

#define ADC_MAX_VALUE 4095

void Adc_Init(void);
u16 Adc_Read(u8 Channel);

#endif // _ADC_H_
