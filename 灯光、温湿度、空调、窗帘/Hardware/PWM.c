#include "stm32f10x.h"                  // Device header
#include "PWM.h"
#include "usart.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_gpio.h"

volatile uint8_t upload_flag = 0;  // 全局上传标志

// 1. 修改定时器初始化函数
void Timer_Init(void) {
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);//开启RCC内部时钟

	TIM_InternalClockConfig(TIM1);//定时器上电后默认使用内部时钟（有时不写）
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up;//选择向上计数

	TIM_TimeBaseInitStructure.TIM_Period=500-1;//周期，ARR自动重装器的值
	TIM_TimeBaseInitStructure.TIM_Prescaler=7200-1;//PSC预分频器的值
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter=0;//重复计数器的值（高级定时器才有，不用给0）
	TIM_TimeBaseInit(TIM1,&TIM_TimeBaseInitStructure);
	
	TIM_ClearFlag(TIM1,TIM_FLAG_Update);//手动清除更新中断标志位，避免刚刚初始化完就进入中断

	TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE);//选择更新中断到NVIC通路

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//选择2,2分组
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel=TIM1_UP_IRQn;//指定中断通道，选择TIM2通道
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;//指定中断通道是使能还是失能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;//指定抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=1;//指定响应优先级
	NVIC_Init(&NVIC_InitStructure);

	//启动定时器
	TIM_Cmd(TIM1,ENABLE);
}

void TIM1_UP_IRQHandler(void){
	static uint8_t time_count = 0;
	if(TIM_GetITStatus(TIM1,TIM_IT_Update)==SET){//获取中断标志位
		//写进行定时的操作
		time_count ++;
		if(time_count==200){			 
			upload_flag=1;
			time_count=0;
		}
		TIM_ClearITPendingBit(TIM1,TIM_IT_Update);//清除标志位
	}
}
