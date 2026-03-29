#include "key.h"
#include "stm32f10x.h"
#include "globals.h"

//记录按键状态
static uint8_t key1_last = 1;
static uint8_t key2_last = 1;
static uint8_t key3_last = 1;
static uint8_t key4_last = 1;

void Key_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(KEY_CLK, ENABLE);		//开启GPIOC的时钟
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = KEY1_PIN|KEY2_PIN|KEY3_PIN|KEY4_PIN|KEY5_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(KEY_PORT, &GPIO_InitStructure);						//将PC15引脚初始化为上拉输入
}

/**
  * 函    数：按键获取键码
  * 参    数：无
  * 返 回 值：按下按键的键码值，范围：0~2，返回0代表没有按键按下
  * 注意事项：此函数是阻塞式操作，当按键按住不放时，函数会卡住，直到按键松手
  */
uint8_t Key_GetNum(void)
{
	if(!key_clock)//clock键按下
		{	
			while(key_clock==0);//等待按键松开
//			delay_ms(1);//延时消抖	
			return 1;
		}
	if(!key_shift)//shift键按下
		{	
			while(key_shift==0);//等待按键松开
//			delay_ms(1);//延时消抖	
			return 2;
		}
	if(!key_up)//up键按下
		{	
			while(key_up==0);//等待按键松开
//			delay_ms(1);//延时消抖	
			return 3;
		}
	if(!key_down)//down键按下
		{	
			while(key_down==0);//等待按键松开
//			delay_ms(1);//延时消抖	
			return 4;
		}
		return 0;
}

uint8_t Key_Num(void)
{
	uint8_t Num = 0;		//定义变量，默认键码值为0
	
	if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_12) == 0)			//读PC15输入寄存器的状态，如果为0，则代表按键2按下
	{										
		while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_12) == 0);	//等待按键松手
//		delay_ms(1);											//延时消抖
		Num = 5;												//置键码为2
	}
	
	return Num;			//返回键码值，如果没有按键按下，所有if都不成立，则键码为默认值0
}
