#include "stm32f10x.h"
#include "servo.h"

/**
  * 函    数：舵机初始化（三通道）
  * 参    数：无
  * 返 回 值：无
  */
void Servo_Init(void)
{
    // 开启时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);   // TIM3时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE); // GPIOA/B时钟
    
    // GPIO初始化（PA6/PA7/PB0）
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    
    // 配置PA6和PA7
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 配置PB0
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // 定时器基础配置
    TIM_InternalClockConfig(TIM3);  // 选择内部时钟
    
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period = 20000 - 1;   // ARR = 19999
    TIM_TimeBaseInitStructure.TIM_Prescaler = 72 - 1;   // PSC = 71，1MHz时钟
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);
    
    // 配置三个通道为PWM模式1
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCStructInit(&TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;  // CCR初始值
    
    // 初始化各通道
    TIM_OC1Init(TIM3, &TIM_OCInitStructure);  // PA6
    TIM_OC2Init(TIM3, &TIM_OCInitStructure);  // PA7
    TIM_OC3Init(TIM3, &TIM_OCInitStructure);  // PB0
    
    TIM_Cmd(TIM3, ENABLE);  // 启动定时器
}

/**
  * 函    数：设置指定通道的PWM占空比
  * 参    数：Channel - 通道号（1/2/3），Compare - CCR值（500~2500）
  * 返 回 值：无
  */
void PWM_SetCompare(uint8_t Channel, uint16_t Compare)
{
    switch(Channel) {
        case 1: TIM_SetCompare1(TIM3, Compare); break;
        case 2: TIM_SetCompare2(TIM3, Compare); break;
        case 3: TIM_SetCompare3(TIM3, Compare); break;
    }
}

/**
  * 函    数：设置舵机角度（带通道选择）
  * 参    数：Channel - 通道号（1/2/3），Angle - 角度（0~180）
  * 返 回 值：无
  */
void Servo_SetAngle(uint8_t Channel, float Angle)
{
    // 角度转CCR：0°→500，180°→2500，占空比2.5%~12.5%
    uint16_t Compare = Angle * (2000.0 / 180) + 500;
    PWM_SetCompare(Channel, Compare);
}
