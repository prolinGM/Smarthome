#ifndef __LightSensor_H
#define	__LightSensor_H
#include "stm32f10x.h"
#include "adcx.h"
#include "delay.h"
#include "math.h"

#define LDR_READ_TIMES	10  //光照传感器ADC循环读取次数

// LDR GPIO宏定义
#define		LDR_GPIO_CLK							RCC_APB2Periph_GPIOA
#define 	LDR_GPIO_PORT							GPIOA
#define 	LDR_GPIO_PIN							GPIO_Pin_7

// ADC 通道宏定义
#define   ADC_CHANNEL               ADC_Channel_7

//#define    ADC_IRQ                       ADC3_IRQn
//#define    ADC_IRQHandler                ADC3_IRQHandler

void LightSensor_Init(void);
uint16_t LightSensor_Average_Data(void);
uint16_t LightSensor_LuxData(void);

#endif /* __ADC_H */

