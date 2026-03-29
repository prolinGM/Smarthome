#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

#include "main.h"

struct keys
{
	unsigned char judge_sta;
	unsigned int key_sta;
	unsigned int single_flag;
//	unsigned int long_flag;
//	unsigned int key_time;
};
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim);
#endif
