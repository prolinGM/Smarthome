#include <stdint.h>
#include "led.h"


void led_Init(void)
{
	ws2812b_Init_bed();
	ws2812b_Init_liv();
	ws2812b_Init_bat();
	ws2812b_Init_kit();
	ws2812b_Init_stu();
}
//int led_IsReady(void)
//{
//  return ws2812b_IsReady_bed();
//}
void led_Add(void)
{

}
// show all
void led_Show_RGB(RGB_t *rgbLeds, uint32_t count)
{
  ws2812b_SendRGB_bed(rgbLeds, count);
}

void led_Loop_Show_RGB(RGB_t *start, uint32_t count, uint32_t offset, uint32_t length, uint32_t delayTime, uint32_t clickTimes)
{
	RGB_t rgbStart[count];
	uint32_t i = offset;
	led_Clear_RGB(rgbStart, count);
	while (clickTimes--)
	{
		for (uint32_t j = 0; j < length; j++)
		{
			(rgbStart + (i + j) % count)->r = (start + (i + j) % count)->r;
			(rgbStart + (i + j) % count)->g = (start + (i + j) % count)->g;
			(rgbStart + (i + j) % count)->b = (start + (i + j) % count)->b;	
		}
		led_Show_RGB(rgbStart, count);
		Delay_ms(delayTime);
		for (uint32_t j = 0; j < length; j++)
		{
			(rgbStart + (i + j) % count)->r = 0;
			(rgbStart + (i + j) % count)->g = 0;
			(rgbStart + (i + j) % count)->b = 0;	
		}
		i += length;
	}
}

void led_PLoop_Show_RGB(RGB_t *start, uint32_t count, uint32_t offset, uint32_t length, Delaytime_t *delaytimeF, uint32_t clickTimes, uint32_t time)
{
	RGB_t rgbStart[count];
	uint32_t i = offset;
	Delaytime_t *delayTime = delaytimeF;
	led_Clear_RGB(rgbStart, count);
	while (clickTimes--)
	{
		for (uint32_t j = 0; j < length; j++)
		{
			(rgbStart + (i + j) % count)->r = (start + (i + j) % count)->r;
			(rgbStart + (i + j) % count)->g = (start + (i + j) % count)->g;
			(rgbStart + (i + j) % count)->b = (start + (i + j) % count)->b;	
		}
		led_Show_RGB(rgbStart, count);
		Delay_ms(delayTime(count, length, clickTimes, time));
		for (uint32_t j = 0; j < length; j++)
		{
			(rgbStart + (i + j) % count)->r = 0;
			(rgbStart + (i + j) % count)->g = 0;
			(rgbStart + (i + j) % count)->b = 0;	
		}
		i += length;
	}
}

void led_Gradual_Show_RGB(RGB_t *start, uint32_t count, uint32_t offset, uint32_t length, uint32_t delayTime)
{
	RGB_t rgbStart[count];
	uint32_t i = offset;
	uint32_t clickTimes = count * 1;
	led_Clear_RGB(rgbStart, count);
	while (clickTimes--)
	{
		for (uint32_t j = 0; j < length; j++)
		{
			(rgbStart + (i + j) % count)->r = (start + (i + j) % count)->r;
			(rgbStart + (i + j) % count)->g = (start + (i + j) % count)->g;
			(rgbStart + (i + j) % count)->b = (start + (i + j) % count)->b;	
		}
		led_Show_RGB(rgbStart, count);
		Delay_ms(delayTime);
		i += length;
	}
}

void led_Gradual_Cover_Show_RGB(RGB_t *source, RGB_t *end, uint32_t count, uint32_t offset, uint32_t length, uint32_t delayTime)
{
	uint32_t i = offset;
	uint32_t clickTimes = count / length;
	while (clickTimes--)
	{
		for (int j = 0; j < length; j++)
		{
			(source + (i + j) % count)->r = (end + (i + j) % count)->r;
			(source + (i + j) % count)->g = (end + (i + j) % count)->g;
			(source + (i + j) % count)->b = (end + (i + j) % count)->b;	
		}
		led_Show_RGB(source, count);
		Delay_ms(delayTime);
		i += length;
	}
}


void led_Tran_Show_RGB(RGB_t *source, RGB_t *end, uint32_t count, uint32_t tranTime)
{
	RGB_t ledRgb[count];
	int32_t rGap[count], gGap[count], bGap[count];
	int32_t init = 125;
	int32_t timeGap = tranTime / init;
	int32_t clickNum = init;
	int32_t r, g, b;
	for (int32_t i = 0; i < count; i++)
	{
		rGap[i] = (int32_t)((end + i)->r) - (int32_t)((source + i)->r);
		gGap[i] = (int32_t)((end + i)->g) - (int32_t)((source + i)->g);
		bGap[i] = (int32_t)((end + i)->b) - (int32_t)((source + i)->b);
	}
	for ( ; clickNum >= 0; clickNum--)
	{
		for (int i = 0; i < count; i++)
		{
			r = (int32_t)((source + i)->r) + rGap[i] / init * ( init - clickNum);
			g = (int32_t)((source + i)->g) + gGap[i] / init * ( init - clickNum);
			b = (int32_t)((source + i)->b) + bGap[i] / init * ( init - clickNum);
			(ledRgb + i)->r = (uint8_t)r;
			(ledRgb + i)->g = (uint8_t)g;
			(ledRgb + i)->b = (uint8_t)b;
		}
		led_Show_RGB(ledRgb, count);
		Delay_ms(timeGap);
	}
}

void led_Circle_Show_RGB(RGB_t *start, uint32_t count, uint32_t delayTime, uint32_t clickTimes)
{
	RGB_t tmpRgb;
	while (clickTimes--)
	{
		tmpRgb.r = (start + count - 1)->r;
		tmpRgb.g = (start + count - 1)->g;
		tmpRgb.b = (start + count - 1)->b;
		for (int i = count - 1; i > 0; i--)
		{
			(start + i)->r = (start + i - 1)->r;
			(start + i)->g = (start + i - 1)->g;
			(start + i)->b = (start + i - 1)->b;
		}
		start->r = tmpRgb.r;
		start->g = tmpRgb.g;
		start->b = tmpRgb.b;
		led_Show_RGB(start, count);
		Delay_ms(delayTime);
	}
}

void led_Shut_RGB(uint32_t count)
{
	RGB_t rgbLeds[count];
	for (int i = 0; i < count; i++)
	{
		rgbLeds[i].r = 0;
		rgbLeds[i].g = 0;
		rgbLeds[i].b = 0;
	}
	led_Show_RGB(rgbLeds, count);
}

// clear all
void led_Clear_RGB(RGB_t *rgbLeds, uint32_t count)
{
	for (uint32_t i = 0; i < count; i++)
	{
		(rgbLeds + i)->r = 0;
		(rgbLeds + i)->g = 0;
		(rgbLeds + i)->b = 0;
	}
}



RGB_t RGB(uint8_t r, uint8_t g, uint8_t b)
{
  RGB_t tmp = { r, g, b };
  return tmp;
}


void led_Fill_Solid_RGB(RGB_t *start, uint32_t count, RGB_t color)
{
	for (int i = 0; i < count; i++)
	{
		(start + i)->r = color.r;
		(start + i)->g = color.g;
		(start + i)->b = color.b;
	}
}


void led_Fill_Gradient_RGB(RGB_t *start, uint32_t count, RGB_t startColor, RGB_t endColor)
{
	int32_t r, g, b;
	for (int32_t i = 1; i < count - 1; i++)
	{
		r = ((int32_t)(startColor.r) + ((int32_t)endColor.r - (int32_t)startColor.r) * i / ((int32_t)count - 1));
		g = ((int32_t)(startColor.g) + ((int32_t)endColor.g - (int32_t)startColor.g) * i / ((int32_t)count - 1));
		b = ((int32_t)(startColor.b) + ((int32_t)endColor.b - (int32_t)startColor.b) * i / ((int32_t)count - 1));
		(start + i)->r = (uint8_t)(r);
		(start + i)->g = (uint8_t)(g);
		(start + i)->b = (uint8_t)(b);
	}
	start->r = startColor.r;
	start->g = startColor.g;
	start->b = startColor.b;
	(start + count - 1)->r = endColor.r;
	(start + count - 1)->g = endColor.g;
	(start + count - 1)->b = endColor.b;
}

