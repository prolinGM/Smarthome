#ifndef __KEY_H
#define	__KEY_H
#include "stm32f10x.h"
#include "delay.h"
#include "math.h"


#define		KEY_GPIO_CLK								RCC_APB2Periph_GPIOC
#define 	KEY_GPIO_PORT								GPIOC
#define 	KEY_GPIO_PIN								GPIO_Pin_15	

#define KEY  GPIO_ReadInputDataBit(KEY_GPIO_PORT,KEY_GPIO_PIN)		//读取按键1

#define KEY_PRES 1	//KEY按下


void Key_Init(void);
uint8_t Key_GetNum(void);

#endif /* __ADC_H */


