#ifndef _PONG_H_
#define _PONG_H_

/***********************************************************
 *  Aplicacion PONG sobre el RTOS (segundo entregable).
 *
 *  Tareas:
 *    TaskSound   (prioridad 0): espera el semaforo y hace sonar el buzzer
 *    TaskInput   (prioridad 1): lee potenciometros y boton cada 20 ms
 *    TaskGame    (prioridad 2): mueve la pelota cada 25 ms
 *    TaskDisplay (prioridad 3): dibuja y envia la imagen a la OLED
 *
 *  Sincronizacion / comunicacion entre tareas:
 *    - Mutex:    protege el estado del juego compartido (GameMutex)
 *    - Semaforo: avisa a la tarea de sonido que hay un evento (SoundSem)
 *
 *  Conexiones:
 *    OLED SSD1306: SCL -> PB6, SDA -> PB7, VCC -> 3.3V, GND -> GND
 *    Potenciometro J1 (pata central) -> PA0
 *    Potenciometro J2 (pata central) -> PA1
 *    Boton inicio/fin -> PB0 (el otro lado a GND)
 *    Buzzer activo (+) -> PA8, (-) -> GND
 ***********************************************************/

#include "AppTypes.h"

void Pong_Init(void);

#endif // _PONG_H_
