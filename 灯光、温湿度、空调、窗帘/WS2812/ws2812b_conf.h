#ifndef __WS2812B_CONF_H
#define __WS2812B_CONF_H

#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_tim.h>
#include <stm32f10x_dma.h>
#include <misc.h>

#define WS2812B_USE_GAMMA_CORRECTION
#define WS2812B_USE_PRECALCULATED_GAMMA_TABLE

#define WS2812B_BUFFER_SIZE     60
#define WS2812B_START_SIZE      2

#define WS2812B_IRQ_PRIO        0
#define WS2812B_IRQ_SUBPRIO     0

#define WS2812B_FREQUENCY       24000000
#define WS2812B_PERIOD          30

#define WS2812B_PULSE_HIGH      21
#define WS2812B_PULSE_LOW       9

//kitchen-----------------------------------------------------
#define WS2812B_APB1_RCC_kit        RCC_APB1Periph_TIM2
#define WS2812B_APB2_RCC_kit        RCC_APB2Periph_GPIOA

#define WS2812B_AHB_RCC_kit         RCC_AHBPeriph_DMA1

#define WS2812B_GPIO_kit            GPIOA
#define WS2812B_GPIO_PIN_kit        GPIO_Pin_0

#define WS2812B_TIM_kit             TIM2
#define WS2812B_TIM_OCINIT_kit      TIM_OC1Init
#define WS2812B_TIM_OCPRELOAD_kit   TIM_OC1PreloadConfig
#define WS2812B_TIM_DMA_CC_kit      TIM_DMA_CC1
#define WS2812B_TIM_DMA_CCR_kit     (WS2812B_TIM_kit->CCR1)

#define WS2812B_DMA_kit             DMA1
#define WS2812B_DMA_CHANNEL_kit     DMA1_Channel5
#define WS2812B_DMA_IRQ_kit         DMA1_Channel5_IRQn

#define WS2812B_DMA_HANDLER_kit     DMA1_Channel5_IRQHandler
#define WS2812B_DMA_IT_TC_kit       DMA1_IT_TC5
#define WS2812B_DMA_IT_HT_kit       DMA1_IT_HT5

//studyroom-----------------------------------------------------
#define WS2812B_APB1_RCC_stu        RCC_APB1Periph_TIM2
#define WS2812B_APB2_RCC_stu        RCC_APB2Periph_GPIOA

#define WS2812B_AHB_RCC_stu         RCC_AHBPeriph_DMA1

#define WS2812B_GPIO_stu            GPIOA
#define WS2812B_GPIO_PIN_stu        GPIO_Pin_1

#define WS2812B_TIM_stu             TIM2
#define WS2812B_TIM_OCINIT_stu      TIM_OC2Init
#define WS2812B_TIM_OCPRELOAD_stu   TIM_OC2PreloadConfig
#define WS2812B_TIM_DMA_CC_stu      TIM_DMA_CC2
#define WS2812B_TIM_DMA_CCR_stu     (WS2812B_TIM_stu->CCR2)

#define WS2812B_DMA_stu             DMA1
#define WS2812B_DMA_CHANNEL_stu     DMA1_Channel7
#define WS2812B_DMA_IRQ_stu         DMA1_Channel7_IRQn

#define WS2812B_DMA_HANDLER_stu     DMA1_Channel7_IRQHandler
#define WS2812B_DMA_IT_TC_stu       DMA1_IT_TC7
#define WS2812B_DMA_IT_HT_stu       DMA1_IT_HT7

//batroom-----------------------------------------------------
#define WS2812B_APB1_RCC_bat        RCC_APB1Periph_TIM3
#define WS2812B_APB2_RCC_bat        RCC_APB2Periph_GPIOB

#define WS2812B_AHB_RCC_bat         RCC_AHBPeriph_DMA1

#define WS2812B_GPIO_bat            GPIOB
#define WS2812B_GPIO_PIN_bat        GPIO_Pin_1

#define WS2812B_TIM_bat             TIM3
#define WS2812B_TIM_OCINIT_bat      TIM_OC4Init
#define WS2812B_TIM_OCPRELOAD_bat   TIM_OC4PreloadConfig
#define WS2812B_TIM_DMA_CC_bat      TIM_DMA_CC4
#define WS2812B_TIM_DMA_CCR_bat     (WS2812B_TIM_bat->CCR4)

#define WS2812B_DMA_bat             DMA1
#define WS2812B_DMA_CHANNEL_bat     DMA1_Channel3
#define WS2812B_DMA_IRQ_bat         DMA1_Channel3_IRQn

#define WS2812B_DMA_HANDLER_bat     DMA1_Channel3_IRQHandler
#define WS2812B_DMA_IT_TC_bat       DMA1_IT_TC3
#define WS2812B_DMA_IT_HT_bat       DMA1_IT_HT3

//bedroom-----------------------------------------------------
#define WS2812B_APB1_RCC_bed        RCC_APB1Periph_TIM3
#define WS2812B_APB2_RCC_bed        RCC_APB2Periph_GPIOB

#define WS2812B_AHB_RCC_bed         RCC_AHBPeriph_DMA1

#define WS2812B_GPIO_bed            GPIOB
#define WS2812B_GPIO_PIN_bed        GPIO_Pin_0

#define WS2812B_TIM_bed             TIM3
#define WS2812B_TIM_OCINIT_bed      TIM_OC3Init
#define WS2812B_TIM_OCPRELOAD_bed   TIM_OC3PreloadConfig
#define WS2812B_TIM_DMA_CC_bed      TIM_DMA_CC3
#define WS2812B_TIM_DMA_CCR_bed     (WS2812B_TIM_bed->CCR3)

#define WS2812B_DMA_bed             DMA1
#define WS2812B_DMA_CHANNEL_bed     DMA1_Channel2
#define WS2812B_DMA_IRQ_bed         DMA1_Channel2_IRQn

#define WS2812B_DMA_HANDLER_bed     DMA1_Channel2_IRQHandler
#define WS2812B_DMA_IT_TC_bed       DMA1_IT_TC2
#define WS2812B_DMA_IT_HT_bed       DMA1_IT_HT2

//livingroom-----------------------------------------------------
#define WS2812B_APB1_RCC_liv        RCC_APB1Periph_TIM2
#define WS2812B_APB2_RCC_liv        RCC_APB2Periph_GPIOA

#define WS2812B_AHB_RCC_liv         RCC_AHBPeriph_DMA1

#define WS2812B_GPIO_liv            GPIOA
#define WS2812B_GPIO_PIN_liv        GPIO_Pin_2

#define WS2812B_TIM_liv             TIM2
#define WS2812B_TIM_OCINIT_liv      TIM_OC3Init
#define WS2812B_TIM_OCPRELOAD_liv   TIM_OC3PreloadConfig
#define WS2812B_TIM_DMA_CC_liv      TIM_DMA_CC3
#define WS2812B_TIM_DMA_CCR_liv     (WS2812B_TIM_liv->CCR3)

#define WS2812B_DMA_liv             DMA1
#define WS2812B_DMA_CHANNEL_liv     DMA1_Channel1
#define WS2812B_DMA_IRQ_liv         DMA1_Channel1_IRQn

#define WS2812B_DMA_HANDLER_liv     DMA1_Channel1_IRQHandler
#define WS2812B_DMA_IT_TC_liv       DMA1_IT_TC1
#define WS2812B_DMA_IT_HT_liv       DMA1_IT_HT1


#endif //__WS2812B_CONF_H
