#ifndef __BODY_HW_H
#define	__BODY_HW_H
#include "stm32f10x.h"
#include "adcx.h"
#include "Delay.h"
#include "math.h"

void BODY_HW_Init(void);
uint16_t BODY_HW_GetData(uint16_t sensor_index);

#endif /* __ADC_H */

