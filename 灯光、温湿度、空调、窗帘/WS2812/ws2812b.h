#ifndef __WS2812B_H
#define __WS2812B_H

#include <stdint.h>
#include "bitmap.h"

#define NUM_LEDS    3

typedef struct {
    uint16_t kelvin;  // 色温值
    RGB_t rgb;        // 对应RGB值
} ColorTemperature;



//kitchen--------------------------------------------------
void WS2812B_DMA_HANDLER_kit(void);
void ws2812b_Init_kit(void);
void ws2812b_SendRGB_kit(RGB_t *rgb, unsigned count);
void WS2812B_SetTemperature_kit(uint16_t kelvin);	//色温控制
void WS2812B_SetBrightness_kit(uint8_t brightness);  //亮度控制

//studyroom--------------------------------------------------
void WS2812B_DMA_HANDLER_stu(void);
void ws2812b_Init_stu(void);
void ws2812b_SendRGB_stu(RGB_t *rgb, unsigned count);
void WS2812B_SetTemperature_stu(uint16_t kelvin);
void WS2812B_SetBrightness_stu(uint8_t brightness);

//bathroom------------------------------------------------
void WS2812B_DMA_HANDLER_bat(void);
void ws2812b_Init_bat(void);
void ws2812b_SendRGB_bat(RGB_t *rgb, unsigned count);
void WS2812B_SetTemperature_bat(uint16_t kelvin);
void WS2812B_SetBrightness_bat(uint8_t brightness);

//bedroom-------------------------------------------------
void WS2812B_DMA_HANDLER_bed(void);
void ws2812b_Init_bed(void);
void ws2812b_SendRGB_bed(RGB_t *rgb, unsigned count);
void WS2812B_SetTemperature_bed(uint16_t kelvin);
void WS2812B_SetBrightness_bed(uint8_t brightness);

//livingroom----------------------------------------------
void WS2812B_DMA_HANDLER_liv(void);
void ws2812b_Init_liv(void);
void ws2812b_SendRGB_liv(RGB_t *rgb, unsigned count);
void WS2812B_SetTemperature_liv(uint16_t kelvin);
void WS2812B_SetBrightness_liv(uint8_t brightness);

#endif //__WS2812B_H
