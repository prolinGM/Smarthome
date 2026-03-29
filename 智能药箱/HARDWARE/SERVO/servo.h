#ifndef __SERVO_H
#define __SERVO_H

#include "stm32f10x.h"

void Servo_Init(void);
void PWM_SetCompare(uint8_t Channel, uint16_t Compare);
void Servo_SetAngle(uint8_t Channel, float Angle);

#endif