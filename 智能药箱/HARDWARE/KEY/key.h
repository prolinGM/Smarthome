#ifndef __KEY_H
#define	__KEY_H
#include "stm32f10x.h"
#include "delay.h"
#include "math.h"

#define		KEY_CLK								RCC_APB2Periph_GPIOA
// 硬件连接定义
#define KEY1_PIN     GPIO_Pin_8
#define KEY2_PIN     GPIO_Pin_9
#define KEY3_PIN     GPIO_Pin_10
#define KEY4_PIN     GPIO_Pin_11
#define KEY5_PIN     GPIO_Pin_12
#define KEY_PORT     GPIOA

#define key_clock  GPIO_ReadInputDataBit(KEY_PORT,KEY1_PIN)		//读取按键1
#define key_shift  GPIO_ReadInputDataBit(KEY_PORT,KEY2_PIN)		//读取按键2
#define key_up  GPIO_ReadInputDataBit(KEY_PORT,KEY3_PIN)		//读取按键3
#define key_down  GPIO_ReadInputDataBit(KEY_PORT,KEY4_PIN)		//读取按键4
#define key_beep  GPIO_ReadInputDataBit(KEY_PORT,KEY5_PIN)		//读取按键5

void Key_Init(void);
uint8_t Key_GetNum(void);
uint8_t Key_Num(void);

#endif /* __ADC_H */

