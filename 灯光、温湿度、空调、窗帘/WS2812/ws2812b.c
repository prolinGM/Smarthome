#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "bitmap.h"

#include "ws2812b.h"
#include "ws2812b_conf.h"

//------------------------------------------------------------
// Internal
//------------------------------------------------------------

// #define MIN(a, b)   ({ typeof(a) a1 = a; typeof(b) b1 = b; a1 < b1 ? a1 : b1; })

#if defined(__ICCARM__)
__packed struct PWM
#else
struct __attribute__((packed)) PWM
#endif
{
  uint16_t g[8], r[8], b[8];
};

typedef struct PWM PWM_t;
typedef void (SrcFilter_t)(void **, PWM_t **, unsigned *, unsigned);

#ifdef WS2812B_USE_GAMMA_CORRECTION
#ifdef WS2812B_USE_PRECALCULATED_GAMMA_TABLE
static const uint8_t LEDGammaTable[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
  2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10,
  10, 11, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21,
  22, 23, 23, 24, 24, 25, 26, 26, 27, 28, 28, 29, 30, 30, 31, 32, 32, 33, 34, 35, 35, 36, 37, 38,
  38, 39, 40, 41, 42, 42, 43, 44, 45, 46, 47, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 56, 57, 58,
  59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 84,
  85, 86, 87, 88, 89, 91, 92, 93, 94, 95, 97, 98, 99, 100, 102, 103, 104, 105, 107, 108, 109, 111,
  112, 113, 115, 116, 117, 119, 120, 121, 123, 124, 126, 127, 128, 130, 131, 133, 134, 136, 137,
  139, 140, 142, 143, 145, 146, 148, 149, 151, 152, 154, 155, 157, 158, 160, 162, 163, 165, 166,
  168, 170, 171, 173, 175, 176, 178, 180, 181, 183, 185, 186, 188, 190, 192, 193, 195, 197, 199,
  200, 202, 204, 206, 207, 209, 211, 213, 215, 217, 218, 220, 222, 224, 226, 228, 230, 232, 233,
  235, 237, 239, 241, 243, 245, 247, 249, 251, 253, 255 };
#endif
#endif

static inline uint8_t LEDGamma(uint8_t v)
{
#ifdef WS2812B_USE_GAMMA_CORRECTION
#ifdef WS2812B_USE_PRECALCULATED_GAMMA_TABLE
  return LEDGammaTable[v];
#else
  return (v * v + v) >> 8;
#endif
#else
  return v;
#endif
}

static volatile int DMABusy_bed;
static volatile int DMABusy_liv;
static volatile int DMABusy_bat;
static volatile int DMABusy_stu;
static volatile int DMABusy_kit;

static PWM_t DMABuffer_bed[WS2812B_BUFFER_SIZE];
static PWM_t DMABuffer_liv[WS2812B_BUFFER_SIZE];
static PWM_t DMABuffer_bat[WS2812B_BUFFER_SIZE];
static PWM_t DMABuffer_stu[WS2812B_BUFFER_SIZE];
static PWM_t DMABuffer_kit[WS2812B_BUFFER_SIZE];

static SrcFilter_t *DMAFilter;
static void *DMASrc;
static unsigned DMACount;

typedef struct {
  volatile int DMABusy;
  uint8_t current_brightness;
  RGB_t base_colors[NUM_LEDS];
} StripControl;

static StripControl ctrl_bed = {0};
static StripControl ctrl_liv = {0};
static StripControl ctrl_bat = {0};
static StripControl ctrl_stu = {0};
static StripControl ctrl_kit = {0};

//static uint8_t current_brightness = 100;  // 当前亮度值（0-100）
//static RGB_t base_colors[NUM_LEDS];       // 存储基础色温颜色

// 私有函数声明
static RGB_t kelvinToRGB(uint16_t kelvin);
static RGB_t calculateRGB(uint16_t kelvin);

static void WS2812B_UpdateLEDs_bed(void);
int ws2812b_IsReady_bed(void);
	
static void WS2812B_UpdateLEDs_liv(void);
int ws2812b_IsReady_liv(void);

static void WS2812B_UpdateLEDs_bat(void);
int ws2812b_IsReady_bat(void);

static void WS2812B_UpdateLEDs_stu(void);
int ws2812b_IsReady_stu(void);

static void WS2812B_UpdateLEDs_kit(void);
int ws2812b_IsReady_kit(void);

// 色温到RGB映射表
static const ColorTemperature temp_table[] __attribute__((section(".rodata"))) = {
    // 烛光色 (2000K)
    {2000, {.r = 232, .g = 81, .b = 2}},
    
    // 白炽灯 (2500K)
    {2500, {.r = 244, .g = 141, .b = 2}},
    
    // 暖白光 (3000K)
    {3000, {.r = 250, .g = 196, .b = 80}},
    
    // 柔和白 (3500K)
    {3500, {.r = 255, .g = 231, .b = 100}},
    
    // 中性白 (4000K)
    {4000, {.r = 255, .g = 245, .b = 140}},
    
    // 冷白光 (4500K)
    {4500, {.r = 255, .g = 247, .b = 180}},
    
    // 日光色 (5000K)
    {5000, {.r = 252, .g = 249, .b = 194}}, 
    
    // 高色温白 (5500K)
    {5500, {.r = 246, .g = 253, .b = 235}},
    
    // 极冷白 (6000K)
    {6000, {.r = 226, .g = 252, .b = 253}}
};

// 色温到RGB的转换函数（混合查表与计算）
RGB_t kelvinToRGB(uint16_t kelvin) {
    // 第一步：边界保护
    kelvin = (kelvin < 2000) ? 2000 : (kelvin > 6000) ? 6000 : kelvin;

    // 第二步：查表匹配
    const uint8_t table_size = sizeof(temp_table)/sizeof(ColorTemperature);
    for(uint8_t i=0; i<table_size; i++) {
        // 允许±50K的色温容差匹配
        if(abs(kelvin - temp_table[i].kelvin) <= 5) {
            return temp_table[i].rgb;
        }
    }

    // 第三步：算法计算
    return calculateRGB(kelvin);
}

static RGB_t calculateRGB(uint16_t kelvin) {
    
    // 1. 查找最近的两个色温点
    uint8_t i;
    for (i = 0; i < 9 - 1; i++) {
        if (kelvin <= temp_table[i+1].kelvin) break;
    }
    
    // 2. 线性插值
    float ratio = (float)(kelvin - temp_table[i].kelvin) / 
                  (temp_table[i+1].kelvin - temp_table[i].kelvin);
    
    RGB_t rgb;
    rgb.r = temp_table[i].rgb.r + ratio*(temp_table[i+1].rgb.r - temp_table[i].rgb.r);
    rgb.g = temp_table[i].rgb.g + ratio*(temp_table[i+1].rgb.g - temp_table[i].rgb.g);
    rgb.b = temp_table[i].rgb.b + ratio*(temp_table[i+1].rgb.b - temp_table[i].rgb.b);
    
    return rgb;
}


static unsigned min(unsigned a, unsigned b)
{
	if (a < b)
	{
		return a;
	} else {
		return b;
	}
}
static void SrcFilterNull(void **src, PWM_t **pwm, unsigned *count, unsigned size)
{
  memset(*pwm, 0, size * sizeof(PWM_t));
  *pwm += size;
}

static void RGB2PWM(RGB_t *rgb, PWM_t *pwm)
{
  uint8_t r = LEDGamma(rgb->r);
  uint8_t g = LEDGamma(rgb->g);
  uint8_t b = LEDGamma(rgb->b);

  uint8_t mask = 128;

  int i;
  for (i = 0; i < 8; i++)
  {
    pwm->r[i] = r & mask ? WS2812B_PULSE_HIGH : WS2812B_PULSE_LOW;
    pwm->g[i] = g & mask ? WS2812B_PULSE_HIGH : WS2812B_PULSE_LOW;
    pwm->b[i] = b & mask ? WS2812B_PULSE_HIGH : WS2812B_PULSE_LOW;

    mask >>= 1;
  }
}

static void SrcFilterRGB(void **src, PWM_t **pwm, unsigned *count, unsigned size)
{
  RGB_t *rgb = *src;
  PWM_t *p = *pwm;

  *count -= size;

  while (size--)
  {
    RGB2PWM(rgb++, p++);
  }

  *src = rgb;
  *pwm = p;
}



//kitchen------------------------------------------------------------------
static void DMASend_kit(SrcFilter_t *filter, void *src, unsigned count)
{
  if (!DMABusy_kit)
  {
    DMABusy_kit = 1;

    DMAFilter = filter;
    DMASrc = src;
    DMACount = count;

    PWM_t *pwm = DMABuffer_kit;
    PWM_t *end = &DMABuffer_kit[WS2812B_BUFFER_SIZE];

    // Start sequence
    SrcFilterNull(NULL, &pwm, NULL, WS2812B_START_SIZE);

    // RGB PWM data
    DMAFilter(&DMASrc, &pwm, &DMACount, min(DMACount, end - pwm));

    // Rest of buffer
    if (pwm < end)
      SrcFilterNull(NULL, &pwm, NULL, end - pwm);

    // Start transfer
    DMA_SetCurrDataCounter(WS2812B_DMA_CHANNEL_kit, sizeof(DMABuffer_kit) / sizeof(uint16_t));

    TIM_Cmd(WS2812B_TIM_kit, ENABLE);
    DMA_Cmd(WS2812B_DMA_CHANNEL_kit, ENABLE);
  }
}

static void DMASendNext_kit(PWM_t *pwm, PWM_t *end)
{
  if (!DMAFilter)
  {
    // Stop transfer
    TIM_Cmd(WS2812B_TIM_kit, DISABLE);
    DMA_Cmd(WS2812B_DMA_CHANNEL_kit, DISABLE);

    DMABusy_kit = 0;
  }
    else if (!DMACount)
  {
  // Rest of buffer
    SrcFilterNull(NULL, &pwm, NULL, end - pwm);

    DMAFilter = NULL;
  }
  else
  {
    // RGB PWM data
    DMAFilter(&DMASrc, &pwm, &DMACount, min(DMACount, end - pwm));

    // Rest of buffer
  if (pwm < end)
    SrcFilterNull(NULL, &pwm, NULL, end - pwm);
  }
}

void WS2812B_DMA_HANDLER_kit(void)
{
  if (DMA_GetITStatus(WS2812B_DMA_IT_HT_kit) != RESET)
  {
    DMA_ClearITPendingBit(WS2812B_DMA_IT_HT_kit);
    DMASendNext_kit(DMABuffer_kit, &DMABuffer_kit[WS2812B_BUFFER_SIZE / 2]);
  }

  if (DMA_GetITStatus(WS2812B_DMA_IT_TC_kit) != RESET)
  {
    DMA_ClearITPendingBit(WS2812B_DMA_IT_TC_kit);
    DMASendNext_kit(&DMABuffer_kit[WS2812B_BUFFER_SIZE / 2], &DMABuffer_kit[WS2812B_BUFFER_SIZE]);
  }
}

void ws2812b_Init_kit(void)
{
  RCC_APB1PeriphClockCmd(WS2812B_APB1_RCC_kit, ENABLE);  // 自定义
  // RCC_APB2Periph_GPIOB， GPIO端口使能
  RCC_APB2PeriphClockCmd(WS2812B_APB2_RCC_kit, ENABLE);  // 自定义
  // RCC_AHBPeriph_DMA1, AHB=Advanced High Performance Bus，高级高性能总线。
  RCC_AHBPeriphClockCmd(WS2812B_AHB_RCC_kit, ENABLE);

  GPIO_InitTypeDef GPIO_InitStruct;

  GPIO_InitStruct.GPIO_Pin = WS2812B_GPIO_PIN_kit;  // 自定义
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;

  GPIO_Init(WS2812B_GPIO_kit, &GPIO_InitStruct);

  TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;

  TIM_TimeBaseInitStruct.TIM_Prescaler = (SystemCoreClock / WS2812B_FREQUENCY) - 1;
  TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
  // WS2812B_PERIOD          30
  TIM_TimeBaseInitStruct.TIM_Period = WS2812B_PERIOD - 1;
  TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;

  TIM_TimeBaseInit(WS2812B_TIM_kit, &TIM_TimeBaseInitStruct);

  TIM_OCInitTypeDef TIM_OCInitStruct;

  TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStruct.TIM_Pulse = 0;
  TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;

  WS2812B_TIM_OCINIT_kit(WS2812B_TIM_kit, &TIM_OCInitStruct);  // 自定义
  // WS2812B_TIM_OCPRELOAD TIM_OC1PreloadConfig  CH1预装载使能
  WS2812B_TIM_OCPRELOAD_kit(WS2812B_TIM_kit, TIM_OCPreload_Enable);

  DMA_InitTypeDef DMA_InitStruct;

  DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t) & WS2812B_TIM_DMA_CCR_kit;  // 自定义
  DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t) DMABuffer_kit;
  DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_InitStruct.DMA_BufferSize = sizeof(DMABuffer_kit) / sizeof(uint16_t);
  DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;
	// DMA优先级
  DMA_InitStruct.DMA_Priority = DMA_Priority_High;  // 考虑自定义
  DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;

  DMA_Init(WS2812B_DMA_CHANNEL_kit, &DMA_InitStruct);

  TIM_DMACmd(WS2812B_TIM_kit, WS2812B_TIM_DMA_CC_kit, ENABLE); // 自定义,与CCR1对应

  NVIC_InitTypeDef NVIC_InitStruct;

  NVIC_InitStruct.NVIC_IRQChannel = WS2812B_DMA_IRQ_kit;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = WS2812B_IRQ_PRIO;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = WS2812B_IRQ_SUBPRIO;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;

  NVIC_Init(&NVIC_InitStruct);

  DMA_ITConfig(WS2812B_DMA_CHANNEL_kit, DMA_IT_HT | DMA_IT_TC, ENABLE);
}


int ws2812b_IsReady_kit(void)
{
  return !DMABusy_kit;
}

void ws2812b_SendRGB_kit(RGB_t *rgb, unsigned count)
{
  DMASend_kit(&SrcFilterRGB, rgb, count);
}

// 新功能：设置所有LED为指定色温
void WS2812B_SetTemperature_kit(uint16_t kelvin)
{
    RGB_t color = kelvinToRGB(kelvin);
    
    // 所有LED设置为相同颜色
    for(int i=0; i<NUM_LEDS; i++){
        ctrl_kit.base_colors[i] = color;
    }
    
    // 应用当前亮度并更新显示
    WS2812B_UpdateLEDs_kit();
}

// 新增亮度设置函数
void WS2812B_SetBrightness_kit(uint8_t brightness)
{
    // 限制亮度范围
    brightness = (brightness > 100) ? 100 : brightness;
    ctrl_kit.current_brightness = brightness;
    
    // 更新显示
    WS2812B_UpdateLEDs_kit();
}

// 新增内部更新函数
static void WS2812B_UpdateLEDs_kit(void)
{
    RGB_t ledsRGB[NUM_LEDS];
    
    // 应用亮度调整
    float factor = ctrl_kit.current_brightness / 100.0f;
    for(int i=0; i<NUM_LEDS; i++){
        ledsRGB[i].r = (uint8_t)(ctrl_kit.base_colors[i].r * factor);
        ledsRGB[i].g = (uint8_t)(ctrl_kit.base_colors[i].g * factor);
        ledsRGB[i].b = (uint8_t)(ctrl_kit.base_colors[i].b * factor);
    }
    
    // 发送数据
    if(ws2812b_IsReady_kit()){
    ws2812b_SendRGB_kit(ledsRGB, NUM_LEDS);}
}





//_studyroom------------------------------------------------------------------
static void DMASend_stu(SrcFilter_t *filter, void *src, unsigned count)
{
  if (!DMABusy_stu)
  {
    DMABusy_stu = 1;

    DMAFilter = filter;
    DMASrc = src;
    DMACount = count;

    PWM_t *pwm = DMABuffer_stu;
    PWM_t *end = &DMABuffer_stu[WS2812B_BUFFER_SIZE];

    // Start sequence
    SrcFilterNull(NULL, &pwm, NULL, WS2812B_START_SIZE);

    // RGB PWM data
    DMAFilter(&DMASrc, &pwm, &DMACount, min(DMACount, end - pwm));

    // Rest of buffer
    if (pwm < end)
      SrcFilterNull(NULL, &pwm, NULL, end - pwm);

    // Start transfer
    DMA_SetCurrDataCounter(WS2812B_DMA_CHANNEL_stu, sizeof(DMABuffer_stu) / sizeof(uint16_t));

    TIM_Cmd(WS2812B_TIM_stu, ENABLE);
    DMA_Cmd(WS2812B_DMA_CHANNEL_stu, ENABLE);
  }
}

static void DMASendNext_stu(PWM_t *pwm, PWM_t *end)
{
  if (!DMAFilter)
  {
    // Stop transfer
    TIM_Cmd(WS2812B_TIM_stu, DISABLE);
    DMA_Cmd(WS2812B_DMA_CHANNEL_stu, DISABLE);

    DMABusy_stu = 0;
  }
    else if (!DMACount)
  {
  // Rest of buffer
    SrcFilterNull(NULL, &pwm, NULL, end - pwm);

    DMAFilter = NULL;
  }
  else
  {
    // RGB PWM data
    DMAFilter(&DMASrc, &pwm, &DMACount, min(DMACount, end - pwm));

    // Rest of buffer
  if (pwm < end)
    SrcFilterNull(NULL, &pwm, NULL, end - pwm);
  }
}

void WS2812B_DMA_HANDLER_stu(void)
{
  if (DMA_GetITStatus(WS2812B_DMA_IT_HT_stu) != RESET)
  {
    DMA_ClearITPendingBit(WS2812B_DMA_IT_HT_stu);
    DMASendNext_stu(DMABuffer_stu, &DMABuffer_stu[WS2812B_BUFFER_SIZE / 2]);
  }

  if (DMA_GetITStatus(WS2812B_DMA_IT_TC_stu) != RESET)
  {
    DMA_ClearITPendingBit(WS2812B_DMA_IT_TC_stu);
    DMASendNext_stu(&DMABuffer_stu[WS2812B_BUFFER_SIZE / 2], &DMABuffer_stu[WS2812B_BUFFER_SIZE]);
  }
}

void ws2812b_Init_stu(void)
{
  RCC_APB1PeriphClockCmd(WS2812B_APB1_RCC_stu, ENABLE);  // 自定义
  // RCC_APB2Periph_GPIOB， GPIO端口使能
  RCC_APB2PeriphClockCmd(WS2812B_APB2_RCC_stu, ENABLE);  // 自定义
  // RCC_AHBPeriph_DMA1, AHB=Advanced High Performance Bus，高级高性能总线。
  RCC_AHBPeriphClockCmd(WS2812B_AHB_RCC_stu, ENABLE);

  // Initialize GPIO pin
  GPIO_InitTypeDef GPIO_InitStruct;

  // GPIO_StructInit(&GPIO_InitStruct);
  // GPIO_Pin_6
  GPIO_InitStruct.GPIO_Pin = WS2812B_GPIO_PIN_stu;  // 自定义
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;

  GPIO_Init(WS2812B_GPIO_stu, &GPIO_InitStruct);

  // Initialize timer clock
  TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;

  TIM_TimeBaseInitStruct.TIM_Prescaler = (SystemCoreClock / WS2812B_FREQUENCY) - 1;
  TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
  // WS2812B_PERIOD          30
  TIM_TimeBaseInitStruct.TIM_Period = WS2812B_PERIOD - 1;
  TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;

  TIM_TimeBaseInit(WS2812B_TIM_stu, &TIM_TimeBaseInitStruct);

  // Initialize timer PWM
  TIM_OCInitTypeDef TIM_OCInitStruct;

  //TIM_OCStructInit(&TIM_OCInitStruct);

  TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStruct.TIM_Pulse = 0;
  TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;

  // WS2812B_TIM TIM4
	// WS2812B_TIM_OCINIT TIM_OC1Init 初始化CH1
  WS2812B_TIM_OCINIT_stu(WS2812B_TIM_stu, &TIM_OCInitStruct);  // 自定义
  // WS2812B_TIM_OCPRELOAD TIM_OC1PreloadConfig  CH1预装载使能
  WS2812B_TIM_OCPRELOAD_stu(WS2812B_TIM_stu, TIM_OCPreload_Enable);

  // Initialize DMA channel
  DMA_InitTypeDef DMA_InitStruct;

  //DMA_StructInit(&DMA_InitStruct);
  // WS2812B_TIM_DMA_CCR     (WS2812B_TIM->CCR1) (TIM4->CCR1)  TIM4((TIM_TypeDef *)
  DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t) & WS2812B_TIM_DMA_CCR_stu;  // 自定义
  DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t) DMABuffer_stu;
  DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_InitStruct.DMA_BufferSize = sizeof(DMABuffer_stu) / sizeof(uint16_t);
  DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;
	// DMA优先级
  DMA_InitStruct.DMA_Priority = DMA_Priority_High;  // 考虑自定义
  DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;

  DMA_Init(WS2812B_DMA_CHANNEL_stu, &DMA_InitStruct);

  // Turn on timer DMA requests
	// WS2812B_TIM_DMA_CC      TIM_DMA_CC1
  TIM_DMACmd(WS2812B_TIM_stu, WS2812B_TIM_DMA_CC_stu, ENABLE); // 自定义,与CCR1对应

  // Initialize DMA interrupt
  NVIC_InitTypeDef NVIC_InitStruct;

  NVIC_InitStruct.NVIC_IRQChannel = WS2812B_DMA_IRQ_stu;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = WS2812B_IRQ_PRIO;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = WS2812B_IRQ_SUBPRIO;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;

  NVIC_Init(&NVIC_InitStruct);

  // Enable DMA interrupt
  DMA_ITConfig(WS2812B_DMA_CHANNEL_stu, DMA_IT_HT | DMA_IT_TC, ENABLE);
}


int ws2812b_IsReady_stu(void)
{
  return !DMABusy_stu;
}

void ws2812b_SendRGB_stu(RGB_t *rgb, unsigned count)
{
  DMASend_stu(&SrcFilterRGB, rgb, count);
}

// 新功能：设置所有LED为指定色温
void WS2812B_SetTemperature_stu(uint16_t kelvin)
{
    RGB_t color = kelvinToRGB(kelvin);
    
    // 所有LED设置为相同颜色
    for(int i=0; i<NUM_LEDS; i++){
        ctrl_stu.base_colors[i] = color;
    }
    
    // 应用当前亮度并更新显示
    WS2812B_UpdateLEDs_stu();
}

// 新增亮度设置函数
void WS2812B_SetBrightness_stu(uint8_t brightness)
{
    // 限制亮度范围
    brightness = (brightness > 100) ? 100 : brightness;
    ctrl_stu.current_brightness = brightness;
    
    // 更新显示
    WS2812B_UpdateLEDs_stu();
}

// 新增内部更新函数
static void WS2812B_UpdateLEDs_stu(void)
{
    RGB_t ledsRGB[NUM_LEDS];
    
    // 应用亮度调整
    float factor = ctrl_stu.current_brightness / 100.0f;
    for(int i=0; i<NUM_LEDS; i++){
        ledsRGB[i].r = (uint8_t)(ctrl_stu.base_colors[i].r * factor);
        ledsRGB[i].g = (uint8_t)(ctrl_stu.base_colors[i].g * factor);
        ledsRGB[i].b = (uint8_t)(ctrl_stu.base_colors[i].b * factor);
    }
    
    // 发送数据
    if(ws2812b_IsReady_stu()){
    ws2812b_SendRGB_stu(ledsRGB, NUM_LEDS);}
}





//_batroom------------------------------------------------------------------
static void DMASend_bat(SrcFilter_t *filter, void *src, unsigned count)
{
  if (!DMABusy_bat)
  {
    DMABusy_bat = 1;

    DMAFilter = filter;
    DMASrc = src;
    DMACount = count;

    PWM_t *pwm = DMABuffer_bat;
    PWM_t *end = &DMABuffer_bat[WS2812B_BUFFER_SIZE];

    // Start sequence
    SrcFilterNull(NULL, &pwm, NULL, WS2812B_START_SIZE);

    // RGB PWM data
    DMAFilter(&DMASrc, &pwm, &DMACount, min(DMACount, end - pwm));

    // Rest of buffer
    if (pwm < end)
      SrcFilterNull(NULL, &pwm, NULL, end - pwm);

    // Start transfer
    DMA_SetCurrDataCounter(WS2812B_DMA_CHANNEL_bat, sizeof(DMABuffer_bat) / sizeof(uint16_t));

    TIM_Cmd(WS2812B_TIM_bat, ENABLE);
    DMA_Cmd(WS2812B_DMA_CHANNEL_bat, ENABLE);
  }
}

static void DMASendNext_bat(PWM_t *pwm, PWM_t *end)
{
  if (!DMAFilter)
  {
    // Stop transfer
    TIM_Cmd(WS2812B_TIM_bat, DISABLE);
    DMA_Cmd(WS2812B_DMA_CHANNEL_bat, DISABLE);

    DMABusy_bat = 0;
  }
    else if (!DMACount)
  {
  // Rest of buffer
    SrcFilterNull(NULL, &pwm, NULL, end - pwm);

    DMAFilter = NULL;
  }
  else
  {
    // RGB PWM data
    DMAFilter(&DMASrc, &pwm, &DMACount, min(DMACount, end - pwm));

    // Rest of buffer
  if (pwm < end)
    SrcFilterNull(NULL, &pwm, NULL, end - pwm);
  }
}

void WS2812B_DMA_HANDLER_bat(void)
{
  if (DMA_GetITStatus(WS2812B_DMA_IT_HT_bat) != RESET)
  {
    DMA_ClearITPendingBit(WS2812B_DMA_IT_HT_bat);
    DMASendNext_bat(DMABuffer_bat, &DMABuffer_bat[WS2812B_BUFFER_SIZE / 2]);
  }

  if (DMA_GetITStatus(WS2812B_DMA_IT_TC_bat) != RESET)
  {
    DMA_ClearITPendingBit(WS2812B_DMA_IT_TC_bat);
    DMASendNext_bat(&DMABuffer_bat[WS2812B_BUFFER_SIZE / 2], &DMABuffer_bat[WS2812B_BUFFER_SIZE]);
  }
}

void ws2812b_Init_bat(void)
{
  RCC_APB1PeriphClockCmd(WS2812B_APB1_RCC_bat, ENABLE);  // 自定义
  // RCC_APB2Periph_GPIOB， GPIO端口使能
  RCC_APB2PeriphClockCmd(WS2812B_APB2_RCC_bat, ENABLE);  // 自定义
  RCC_AHBPeriphClockCmd(WS2812B_AHB_RCC_bat, ENABLE);

  // Initialize GPIO pin
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.GPIO_Pin = WS2812B_GPIO_PIN_bat;  // 自定义
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(WS2812B_GPIO_bat, &GPIO_InitStruct);

  // Initialize timer clock
  TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
  TIM_TimeBaseInitStruct.TIM_Prescaler = (SystemCoreClock / WS2812B_FREQUENCY) - 1;
  TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
  // WS2812B_PERIOD          30
  TIM_TimeBaseInitStruct.TIM_Period = WS2812B_PERIOD - 1;
  TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;

  TIM_TimeBaseInit(WS2812B_TIM_bat, &TIM_TimeBaseInitStruct);

  // Initialize timer PWM
  TIM_OCInitTypeDef TIM_OCInitStruct;

  //TIM_OCStructInit(&TIM_OCInitStruct);

  TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStruct.TIM_Pulse = 0;
  TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;

  // WS2812B_TIM TIM4
	// WS2812B_TIM_OCINIT TIM_OC1Init 初始化CH1
  WS2812B_TIM_OCINIT_bat(WS2812B_TIM_bat, &TIM_OCInitStruct);  // 自定义
  // WS2812B_TIM_OCPRELOAD TIM_OC1PreloadConfig  CH1预装载使能
  WS2812B_TIM_OCPRELOAD_bat(WS2812B_TIM_bat, TIM_OCPreload_Enable);

  // Initialize DMA channel
  DMA_InitTypeDef DMA_InitStruct;

  //DMA_StructInit(&DMA_InitStruct);
  // WS2812B_TIM_DMA_CCR     (WS2812B_TIM->CCR1) (TIM4->CCR1)  TIM4((TIM_TypeDef *)
  DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t) & WS2812B_TIM_DMA_CCR_bat;  // 自定义
  DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t) DMABuffer_bat;
  DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_InitStruct.DMA_BufferSize = sizeof(DMABuffer_bat) / sizeof(uint16_t);
  DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;
	// DMA优先级
  DMA_InitStruct.DMA_Priority = DMA_Priority_High;  // 考虑自定义
  DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;

  DMA_Init(WS2812B_DMA_CHANNEL_bat, &DMA_InitStruct);

  // Turn on timer DMA requests
	// WS2812B_TIM_DMA_CC      TIM_DMA_CC1
  TIM_DMACmd(WS2812B_TIM_bat, WS2812B_TIM_DMA_CC_bat, ENABLE); // 自定义,与CCR1对应

  // Initialize DMA interrupt
  NVIC_InitTypeDef NVIC_InitStruct;

  NVIC_InitStruct.NVIC_IRQChannel = WS2812B_DMA_IRQ_bat;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = WS2812B_IRQ_PRIO;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = WS2812B_IRQ_SUBPRIO;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;

  NVIC_Init(&NVIC_InitStruct);

  // Enable DMA interrupt
  DMA_ITConfig(WS2812B_DMA_CHANNEL_bat, DMA_IT_HT | DMA_IT_TC, ENABLE);
}


int ws2812b_IsReady_bat(void)
{
  return !DMABusy_bat;
}

void ws2812b_SendRGB_bat(RGB_t *rgb, unsigned count)
{
  DMASend_bat(&SrcFilterRGB, rgb, count);
}

// 新功能：设置所有LED为指定色温
void WS2812B_SetTemperature_bat(uint16_t kelvin)
{
    RGB_t color = kelvinToRGB(kelvin);
    
    // 所有LED设置为相同颜色
    for(int i=0; i<NUM_LEDS; i++){
        ctrl_bat.base_colors[i] = color;
    }
    
    // 应用当前亮度并更新显示
    WS2812B_UpdateLEDs_bat();
}

// 新增亮度设置函数
void WS2812B_SetBrightness_bat(uint8_t brightness)
{
    // 限制亮度范围
    brightness = (brightness > 100) ? 100 : brightness;
    ctrl_bat.current_brightness = brightness;
    
    // 更新显示
    WS2812B_UpdateLEDs_bat();
}

// 新增内部更新函数
static void WS2812B_UpdateLEDs_bat(void)
{
    RGB_t ledsRGB[NUM_LEDS];
    
    // 应用亮度调整
    float factor = ctrl_bat.current_brightness / 100.0f;
    for(int i=0; i<NUM_LEDS; i++){
        ledsRGB[i].r = (uint8_t)(ctrl_bat.base_colors[i].r * factor);
        ledsRGB[i].g = (uint8_t)(ctrl_bat.base_colors[i].g * factor);
        ledsRGB[i].b = (uint8_t)(ctrl_bat.base_colors[i].b * factor);
    }
    
    // 发送数据
    if(ws2812b_IsReady_bat()){
    ws2812b_SendRGB_bat(ledsRGB, NUM_LEDS);}
}




//bedroom------------------------------------------------------------------
static void DMASend_bed(SrcFilter_t *filter, void *src, unsigned count)
{
  if (!DMABusy_bed)
  {
    DMABusy_bed = 1;

    DMAFilter = filter;
    DMASrc = src;
    DMACount = count;

    PWM_t *pwm = DMABuffer_bed;
    PWM_t *end = &DMABuffer_bed[WS2812B_BUFFER_SIZE];

    // Start sequence
    SrcFilterNull(NULL, &pwm, NULL, WS2812B_START_SIZE);

    // RGB PWM data
    DMAFilter(&DMASrc, &pwm, &DMACount, min(DMACount, end - pwm));

    // Rest of buffer
    if (pwm < end)
      SrcFilterNull(NULL, &pwm, NULL, end - pwm);

    // Start transfer
    DMA_SetCurrDataCounter(WS2812B_DMA_CHANNEL_bed, sizeof(DMABuffer_bed) / sizeof(uint16_t));

    TIM_Cmd(WS2812B_TIM_bed, ENABLE);
    DMA_Cmd(WS2812B_DMA_CHANNEL_bed, ENABLE);
  }
}

static void DMASendNext_bed(PWM_t *pwm, PWM_t *end)
{
  if (!DMAFilter)
  {
    // Stop transfer
    TIM_Cmd(WS2812B_TIM_bed, DISABLE);
    DMA_Cmd(WS2812B_DMA_CHANNEL_bed, DISABLE);

    DMABusy_bed = 0;
  }
    else if (!DMACount)
  {
  // Rest of buffer
    SrcFilterNull(NULL, &pwm, NULL, end - pwm);

    DMAFilter = NULL;
  }
  else
  {
    // RGB PWM data
    DMAFilter(&DMASrc, &pwm, &DMACount, min(DMACount, end - pwm));

    // Rest of buffer
  if (pwm < end)
    SrcFilterNull(NULL, &pwm, NULL, end - pwm);
  }
}

void WS2812B_DMA_HANDLER_bed(void)
{
  if (DMA_GetITStatus(WS2812B_DMA_IT_HT_bed) != RESET)
  {
    DMA_ClearITPendingBit(WS2812B_DMA_IT_HT_bed);
    DMASendNext_bed(DMABuffer_bed, &DMABuffer_bed[WS2812B_BUFFER_SIZE / 2]);
  }

  if (DMA_GetITStatus(WS2812B_DMA_IT_TC_bed) != RESET)
  {
    DMA_ClearITPendingBit(WS2812B_DMA_IT_TC_bed);
    DMASendNext_bed(&DMABuffer_bed[WS2812B_BUFFER_SIZE / 2], &DMABuffer_bed[WS2812B_BUFFER_SIZE]);
  }
}

void ws2812b_Init_bed(void)
{

  RCC_APB1PeriphClockCmd(WS2812B_APB1_RCC_bed, ENABLE);  // 自定义
  // RCC_APB2Periph_GPIOB， GPIO端口使能
  RCC_APB2PeriphClockCmd(WS2812B_APB2_RCC_bed, ENABLE);  // 自定义
  // RCC_AHBPeriph_DMA1, AHB=Advanced High Performance Bus，高级高性能总线。
  RCC_AHBPeriphClockCmd(WS2812B_AHB_RCC_bed, ENABLE);

  // Initialize GPIO pin
  GPIO_InitTypeDef GPIO_InitStruct;

  // GPIO_StructInit(&GPIO_InitStruct);
  // GPIO_Pin_6
  GPIO_InitStruct.GPIO_Pin = WS2812B_GPIO_PIN_bed;  // 自定义
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;

  GPIO_Init(WS2812B_GPIO_bed, &GPIO_InitStruct);

  // Initialize timer clock
  TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;

  //TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
  // 72000000 / (72000000 / 24000000) = 24000000
  // 1 / 24000000 * 30 = 1.25us  = 0.85us + 0.4us
  // SystemCoreClock SYSCLK_FREQ_72MHz  72000000
  // WS2812B_FREQUENCY       24000000
  TIM_TimeBaseInitStruct.TIM_Prescaler = (SystemCoreClock / WS2812B_FREQUENCY) - 1;
  TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
  // WS2812B_PERIOD          30
  TIM_TimeBaseInitStruct.TIM_Period = WS2812B_PERIOD - 1;
  TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;

  TIM_TimeBaseInit(WS2812B_TIM_bed, &TIM_TimeBaseInitStruct);

  // Initialize timer PWM
  TIM_OCInitTypeDef TIM_OCInitStruct;

  //TIM_OCStructInit(&TIM_OCInitStruct);

  TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStruct.TIM_Pulse = 0;
  TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;

  // WS2812B_TIM TIM4
	// WS2812B_TIM_OCINIT TIM_OC1Init 初始化CH1
  WS2812B_TIM_OCINIT_bed(WS2812B_TIM_bed, &TIM_OCInitStruct);  // 自定义
  // WS2812B_TIM_OCPRELOAD TIM_OC1PreloadConfig  CH1预装载使能
  WS2812B_TIM_OCPRELOAD_bed(WS2812B_TIM_bed, TIM_OCPreload_Enable);

  // Initialize DMA channel
  DMA_InitTypeDef DMA_InitStruct;

  //DMA_StructInit(&DMA_InitStruct);
  // WS2812B_TIM_DMA_CCR     (WS2812B_TIM->CCR1) (TIM4->CCR1)  TIM4((TIM_TypeDef *)
  DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t) & WS2812B_TIM_DMA_CCR_bed;  // 自定义
  DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t) DMABuffer_bed;
  DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_InitStruct.DMA_BufferSize = sizeof(DMABuffer_bed) / sizeof(uint16_t);
  DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;
	// DMA优先级
  DMA_InitStruct.DMA_Priority = DMA_Priority_High;  // 考虑自定义
  DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;

  DMA_Init(WS2812B_DMA_CHANNEL_bed, &DMA_InitStruct);

  // Turn on timer DMA requests
	// WS2812B_TIM_DMA_CC      TIM_DMA_CC1
  TIM_DMACmd(WS2812B_TIM_bed, WS2812B_TIM_DMA_CC_bed, ENABLE); // 自定义,与CCR1对应

  // Initialize DMA interrupt
  NVIC_InitTypeDef NVIC_InitStruct;

  NVIC_InitStruct.NVIC_IRQChannel = WS2812B_DMA_IRQ_bed;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = WS2812B_IRQ_PRIO;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = WS2812B_IRQ_SUBPRIO;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;

  NVIC_Init(&NVIC_InitStruct);

  // Enable DMA interrupt
  DMA_ITConfig(WS2812B_DMA_CHANNEL_bed, DMA_IT_HT | DMA_IT_TC, ENABLE);
}


int ws2812b_IsReady_bed(void)
{
  return !DMABusy_bed;
}

void ws2812b_SendRGB_bed(RGB_t *rgb, unsigned count)
{
  DMASend_bed(&SrcFilterRGB, rgb, count);
}


RGB_t ledsRGB[NUM_LEDS];

// 新功能：设置所有LED为指定色温
void WS2812B_SetTemperature_bed(uint16_t kelvin)
{
    RGB_t color = kelvinToRGB(kelvin);
    
    // 所有LED设置为相同颜色
    for(int i=0; i<NUM_LEDS; i++){
        ctrl_bed.base_colors[i] = color;
    }
    
    // 应用当前亮度并更新显示
    WS2812B_UpdateLEDs_bed();
}

// 新增亮度设置函数
void WS2812B_SetBrightness_bed(uint8_t brightness)
{
    // 限制亮度范围
    brightness = (brightness > 100) ? 100 : brightness;
    ctrl_bed.current_brightness = brightness;
    
    // 更新显示
    WS2812B_UpdateLEDs_bed();
}

// 新增内部更新函数
static void WS2812B_UpdateLEDs_bed(void)
{
    RGB_t ledsRGB[NUM_LEDS];
    
    // 应用亮度调整
    float factor = ctrl_bed.current_brightness / 100.0f;
    for(int i=0; i<NUM_LEDS; i++){
        ledsRGB[i].r = (uint8_t)(ctrl_bed.base_colors[i].r * factor);
        ledsRGB[i].g = (uint8_t)(ctrl_bed.base_colors[i].g * factor);
        ledsRGB[i].b = (uint8_t)(ctrl_bed.base_colors[i].b * factor);
    }
    
    // 发送数据
    if(ws2812b_IsReady_bed()){
    ws2812b_SendRGB_bed(ledsRGB, NUM_LEDS);}
}



//livingroom--------------------------------------------------------------------
static void DMASend_liv(SrcFilter_t *filter, void *src, unsigned count)
{
  if (!DMABusy_liv)
  {
    DMABusy_liv = 1;

    DMAFilter = filter;
    DMASrc = src;
    DMACount = count;

    PWM_t *pwm = DMABuffer_liv;
    PWM_t *end = &DMABuffer_liv[WS2812B_BUFFER_SIZE];

    // Start sequence
    SrcFilterNull(NULL, &pwm, NULL, WS2812B_START_SIZE);

    // RGB PWM data
    DMAFilter(&DMASrc, &pwm, &DMACount, min(DMACount, end - pwm));

    // Rest of buffer
    if (pwm < end)
      SrcFilterNull(NULL, &pwm, NULL, end - pwm);

    // Start transfer
    DMA_SetCurrDataCounter(WS2812B_DMA_CHANNEL_liv, sizeof(DMABuffer_liv) / sizeof(uint16_t));

    TIM_Cmd(WS2812B_TIM_liv, ENABLE);
    DMA_Cmd(WS2812B_DMA_CHANNEL_liv, ENABLE);
  }
}

static void DMASendNext_liv(PWM_t *pwm, PWM_t *end)
{
  if (!DMAFilter)
  {
    // Stop transfer
    TIM_Cmd(WS2812B_TIM_liv, DISABLE);
    DMA_Cmd(WS2812B_DMA_CHANNEL_liv, DISABLE);

    DMABusy_liv = 0;
  }
    else if (!DMACount)
  {
  // Rest of buffer
    SrcFilterNull(NULL, &pwm, NULL, end - pwm);

    DMAFilter = NULL;
  }
  else
  {
    // RGB PWM data
    DMAFilter(&DMASrc, &pwm, &DMACount, min(DMACount, end - pwm));

    // Rest of buffer
  if (pwm < end)
    SrcFilterNull(NULL, &pwm, NULL, end - pwm);
  }
}

void WS2812B_DMA_HANDLER_liv(void)
{
  if (DMA_GetITStatus(WS2812B_DMA_IT_HT_liv) != RESET)
  {
    DMA_ClearITPendingBit(WS2812B_DMA_IT_HT_liv);
    DMASendNext_liv(DMABuffer_liv, &DMABuffer_liv[WS2812B_BUFFER_SIZE / 2]);
  }

  if (DMA_GetITStatus(WS2812B_DMA_IT_TC_liv) != RESET)
  {
    DMA_ClearITPendingBit(WS2812B_DMA_IT_TC_liv);
    DMASendNext_liv(&DMABuffer_liv[WS2812B_BUFFER_SIZE / 2], &DMABuffer_liv[WS2812B_BUFFER_SIZE]);
  }
}

//------------------------------------------------------------
// Interface
//------------------------------------------------------------

void ws2812b_Init_liv(void)
{
  // Turn on peripheral clock
  /*
    RCC_APB1Periph_TIM4
    Peripherals 外设
    APB (Advanced Peripheral Bus) 作为高级外设总线是AMBA协议之一，也是最基本的总线协议。
    2个高级定时器（1，8），4个普通定时器（2，3，4，5），2个基本定时器（6，7），2个看门狗定时器，1个系统嘀嗒定时器
    普通定时器，具有测量输入信号的脉冲长度( 输入捕获) 或者产生输出波形( 输出比较和PWM)
  */
  RCC_APB1PeriphClockCmd(WS2812B_APB1_RCC_liv, ENABLE);  // 自定义
  // RCC_APB2Periph_GPIOB， GPIO端口使能
  RCC_APB2PeriphClockCmd(WS2812B_APB2_RCC_liv, ENABLE);  // 自定义
  // RCC_AHBPeriph_DMA1, AHB=Advanced High Performance Bus，高级高性能总线。
  RCC_AHBPeriphClockCmd(WS2812B_AHB_RCC_liv, ENABLE);

  // Initialize GPIO pin
  GPIO_InitTypeDef GPIO_InitStruct;

  // GPIO_StructInit(&GPIO_InitStruct);
  // GPIO_Pin_6
  GPIO_InitStruct.GPIO_Pin = WS2812B_GPIO_PIN_liv;  // 自定义
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;

  GPIO_Init(WS2812B_GPIO_liv, &GPIO_InitStruct);

  // Initialize timer clock
  TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;

  //TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
  // 72000000 / (72000000 / 24000000) = 24000000
  // 1 / 24000000 * 30 = 1.25us  = 0.85us + 0.4us
  // SystemCoreClock SYSCLK_FREQ_72MHz  72000000
  // WS2812B_FREQUENCY       24000000
  TIM_TimeBaseInitStruct.TIM_Prescaler = (SystemCoreClock / WS2812B_FREQUENCY) - 1;
  TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
  // WS2812B_PERIOD          30
  TIM_TimeBaseInitStruct.TIM_Period = WS2812B_PERIOD - 1;
  TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;

  TIM_TimeBaseInit(WS2812B_TIM_liv, &TIM_TimeBaseInitStruct);

  // Initialize timer PWM
  TIM_OCInitTypeDef TIM_OCInitStruct;

  //TIM_OCStructInit(&TIM_OCInitStruct);

  TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStruct.TIM_Pulse = 0;
  TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;

  // WS2812B_TIM TIM4
	// WS2812B_TIM_OCINIT TIM_OC1Init 初始化CH1
  WS2812B_TIM_OCINIT_liv(WS2812B_TIM_liv, &TIM_OCInitStruct);  // 自定义
  // WS2812B_TIM_OCPRELOAD TIM_OC1PreloadConfig  CH1预装载使能
  WS2812B_TIM_OCPRELOAD_liv(WS2812B_TIM_liv, TIM_OCPreload_Enable);

  // Initialize DMA channel
  DMA_InitTypeDef DMA_InitStruct;

  //DMA_StructInit(&DMA_InitStruct);
  // WS2812B_TIM_DMA_CCR     (WS2812B_TIM->CCR1) (TIM4->CCR1)  TIM4((TIM_TypeDef *)
  DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t) & WS2812B_TIM_DMA_CCR_liv;  // 自定义
  DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t) DMABuffer_liv;
  DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_InitStruct.DMA_BufferSize = sizeof(DMABuffer_liv) / sizeof(uint16_t);
  DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;
	// DMA优先级
  DMA_InitStruct.DMA_Priority = DMA_Priority_High;  // 考虑自定义
  DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;

  DMA_Init(WS2812B_DMA_CHANNEL_liv, &DMA_InitStruct);

  // Turn on timer DMA requests
	// WS2812B_TIM_DMA_CC      TIM_DMA_CC1
  TIM_DMACmd(WS2812B_TIM_liv, WS2812B_TIM_DMA_CC_liv, ENABLE); // 自定义,与CCR1对应

  // Initialize DMA interrupt
  NVIC_InitTypeDef NVIC_InitStruct;

  NVIC_InitStruct.NVIC_IRQChannel = WS2812B_DMA_IRQ_liv;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = WS2812B_IRQ_PRIO;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = WS2812B_IRQ_SUBPRIO;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;

  NVIC_Init(&NVIC_InitStruct);

  // Enable DMA interrupt
  DMA_ITConfig(WS2812B_DMA_CHANNEL_liv, DMA_IT_HT | DMA_IT_TC, ENABLE);
}


int ws2812b_IsReady_liv(void)
{
  return !DMABusy_liv;
}

void ws2812b_SendRGB_liv(RGB_t *rgb, unsigned count)
{
  DMASend_liv(&SrcFilterRGB, rgb, count);
}

// 新功能：设置所有LED为指定色温
void WS2812B_SetTemperature_liv(uint16_t kelvin)
{
    RGB_t color = kelvinToRGB(kelvin);
    
    // 所有LED设置为相同颜色
    for(int i=0; i<NUM_LEDS; i++){
        ctrl_liv.base_colors[i] = color;
    }
    
    // 应用当前亮度并更新显示
    WS2812B_UpdateLEDs_liv();
}

// 新增亮度设置函数
void WS2812B_SetBrightness_liv(uint8_t brightness)
{
    // 限制亮度范围
    brightness = (brightness > 100) ? 100 : brightness;
    ctrl_liv.current_brightness = brightness;
    
    // 更新显示
    WS2812B_UpdateLEDs_liv();
}

// 新增内部更新函数
static void WS2812B_UpdateLEDs_liv(void)
{
    RGB_t ledsRGB[NUM_LEDS];
    
    // 应用亮度调整
    float factor = ctrl_liv.current_brightness / 100.0f;
    for(int i=0; i<NUM_LEDS; i++){
        ledsRGB[i].r = (uint8_t)(ctrl_liv.base_colors[i].r * factor);
        ledsRGB[i].g = (uint8_t)(ctrl_liv.base_colors[i].g * factor);
        ledsRGB[i].b = (uint8_t)(ctrl_liv.base_colors[i].b * factor);
    }
    
    // 发送数据
    if(ws2812b_IsReady_liv()){
    ws2812b_SendRGB_liv(ledsRGB, NUM_LEDS);}
	
}
