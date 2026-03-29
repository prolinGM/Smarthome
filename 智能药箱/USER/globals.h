// globals.h
#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#define MAX_BRIGHTNESS 255
#define INTERRUPT_REG 0X00

extern volatile uint32_t aun_ir_buffer[500]; 	 //IR LED   红外光数据，用于计算血氧
extern volatile int32_t n_ir_buffer_length;    //数据长度
extern volatile uint32_t aun_red_buffer[500];  //Red LED	红光数据，用于计算心率曲线以及计算心率
extern volatile int32_t n_sp02; //SPO2值
extern volatile int8_t ch_spo2_valid;   //用于显示SP02计算是否有效的指示符
extern volatile int32_t n_heart_rate;   //心率值
extern volatile int8_t  ch_hr_valid;    //用于显示心率计算是否有效的指示符

extern volatile uint8_t Temp;

extern volatile uint32_t un_min, un_max, un_prev_data;  
extern volatile int i;
extern volatile int32_t n_brightness;
extern volatile float f_temp;

extern volatile u8 temp[6];
extern volatile u8 str[100];
extern volatile u8 dis_hr,dis_spo2;

#endif /* __GLOBALS_H__ */
