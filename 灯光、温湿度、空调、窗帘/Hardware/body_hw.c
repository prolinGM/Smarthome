#include "body_hw.h"

typedef struct {
    GPIO_TypeDef* port;    // GPIO端口
    uint16_t pin;          // 引脚号
    uint32_t clock;        // 时钟使能标志
} SensorGPIOConfig;

// 五个传感器的GPIO配置表
const SensorGPIOConfig sensor_configs[] = {
    {GPIOA, GPIO_Pin_15, RCC_APB2Periph_GPIOA},  // 传感器0 - PA15
    {GPIOB, GPIO_Pin_12, RCC_APB2Periph_GPIOB},  // 传感器1 - PB12
    {GPIOB, GPIO_Pin_13, RCC_APB2Periph_GPIOB},  // 传感器2 - PB13
    {GPIOB, GPIO_Pin_14, RCC_APB2Periph_GPIOB},  // 传感器3 - PB14
    {GPIOB, GPIO_Pin_15, RCC_APB2Periph_GPIOB}   // 传感器4 - PB15
};

void BODY_HW_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    uint8_t i;

    // 遍历所有传感器配置
    for (i = 0; i < sizeof(sensor_configs)/sizeof(SensorGPIOConfig); i++) {
        // 1. 使能GPIO时钟
        RCC_APB2PeriphClockCmd(sensor_configs[i].clock, ENABLE);

        // 2. 配置引脚为下拉输入模式
        GPIO_InitStructure.GPIO_Pin = sensor_configs[i].pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;  // 下拉输入
        GPIO_Init(sensor_configs[i].port, &GPIO_InitStructure);
    }
}

// 获取指定传感器状态
uint16_t BODY_HW_GetData(uint16_t sensor_index) {
    GPIO_TypeDef* port = sensor_configs[sensor_index].port;
    uint16_t pin = sensor_configs[sensor_index].pin;
    
    // 读取引脚电平（0：有人，1：无人）
    return GPIO_ReadInputDataBit(port, pin) ? 1 : 0;
}


