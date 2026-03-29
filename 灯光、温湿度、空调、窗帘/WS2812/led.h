#ifndef __LED_H
#define __LED_H

#include <stdint.h>
#include "delay.h"
#include "bitmap.h"
#include "ws2812b.h"
#include "time.h"

typedef uint32_t (Delaytime_t)(uint32_t num, uint32_t length, uint32_t clickTimes, uint32_t time);

void led_Init(void);
int led_IsReady(void);

void led_Show_RGB(RGB_t *rgbLeds, uint32_t count);

void led_Loop_Show_RGB(RGB_t *start, uint32_t count, uint32_t offset, uint32_t length, uint32_t delayTime, uint32_t clickTimes);

void led_Gradual_Show_RGB(RGB_t *start, uint32_t count, uint32_t offset, uint32_t length, uint32_t delayTime);

void led_Gradual_Cover_Show_RGB(RGB_t *source, RGB_t *end, uint32_t count, uint32_t offset, uint32_t length, uint32_t delayTime);

void led_PLoop_Show_RGB(RGB_t *start, uint32_t count, uint32_t offset, uint32_t length, Delaytime_t *delaytimeF, uint32_t clickTimes, uint32_t time);

void led_Circle_Show_RGB(RGB_t *start, uint32_t count, uint32_t delayTime, uint32_t clickTimes);

void led_Tran_Show_RGB(RGB_t *source, RGB_t *end, uint32_t count, uint32_t tranTime);

void led_Shut_RGB(uint32_t count);

void led_Clear_RGB(RGB_t *rgbLeds, uint32_t count);

void led_Bright_RGB(RGB_t *ledRgb, uint8_t bright);

void led_Fill_Solid_RGB(RGB_t *start, uint32_t count, RGB_t color);

void led_Fill_Gradient_RGB(RGB_t *start, uint32_t count, RGB_t startColor, RGB_t endColor);

// void led_Fill_Platte();
RGB_t RGB(uint8_t r, uint8_t g, uint8_t b);
HSV_t HSV(uint8_t h, uint8_t s, uint8_t v);
#define HEX(hex) hex
#endif //__LED_H
