// globals.h
#ifndef __GLOBALS_H__
#define __GLOBALS_H__

// 系统模式标志
extern volatile int living_auto;      // 0:手动 1:自动
extern volatile int bed_auto;
extern volatile int study_auto;
extern volatile int kit_auto;
extern volatile int bat_auto;

// 传感器数据
extern volatile uint16_t global_light;     // 光照强度

extern volatile uint8_t  living_human;     // 人体检测
extern volatile uint8_t  bed_human;     // 人体检测
extern volatile uint8_t  study_human;     // 人体检测
extern volatile uint8_t  kit_human;     // 人体检测
extern volatile uint8_t  bat_human;     // 人体检测

extern volatile uint8_t upload_flag;

#endif /* __GLOBALS_H__ */
