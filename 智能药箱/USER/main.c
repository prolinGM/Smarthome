#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "lcd.h"
#include "pic.h"
#include "led.h"
#include "ds1302.h"
#include "key.h"
#include "servo.h"
#include "ds18b20.h"
#include "max30102.h"

char st[50];
char count[3];
//float Angle;			//定义角度变量
uint8_t KeyNum;
uint8_t Num;

unsigned char p[16]=" ";
unsigned char hr_str[16]=" ";
unsigned char spo2_str[16]=" ";
short temperature = 0; 				//体温值

#define MAX_BRIGHTNESS 255
#define INTERRUPT_REG 0X00

volatile uint32_t aun_ir_buffer[500]; 	 //IR LED   红外光数据，用于计算血氧
volatile int32_t n_ir_buffer_length;    //数据长度
volatile uint32_t aun_red_buffer[500];  //Red LED	红光数据，用于计算心率曲线以及计算心率
volatile int32_t n_sp02; //SPO2值
volatile int8_t ch_spo2_valid;   //用于显示SP02计算是否有效的指示符
volatile int32_t n_heart_rate;   //心率值
volatile int8_t  ch_hr_valid;    //用于显示心率计算是否有效的指示符

volatile uint8_t Temp;

volatile uint32_t un_min, un_max, un_prev_data;  
volatile int i;
volatile int32_t n_brightness;
volatile float f_temp;

volatile u8 temp[6];
volatile u8 str[100];
volatile u8 dis_hr=0,dis_spo2=0;
	
// 添加全局状态标志位
uint8_t medicineTaken = 0;  // 0=未吃药，1=已吃药
uint32_t lastPlayTime = 0;  // 记录上次播放时间（总秒数）
uint8_t voiceTriggered = 0; // 语音触发标志

uint8_t medicine1_count = 2;   // 药1药量
uint8_t medicine2_count = 1;   // 药2药量
uint8_t medicine3_count = 2;   // 药3药量

uint8_t set_mode = 0;       // 0:正常模式 1:设置闹钟 2:设置药品
uint8_t set_pos = 0;        // 当前设置位置：0-小时 1-分钟 2-药1 3-药2 4-药3

void Hardware_Init(void);
void Display_Update(void);

int main(void)
{	

	Hardware_Init();
	Max_Date();
	
	while(1)
	{	

		Display_Update();
		
		Max_Update();
		
		KeyNum = Key_GetNum();  // 获取按键编号
		// 按键处理逻辑
		if (KeyNum != 0) {
			if (set_mode == 0) { // 正常模式
				if (KeyNum == 1) { // 进入设置模式
					set_mode = 1;
					set_pos = 0;
				}
			} else { // 设置模式
				switch (KeyNum) {
					case 1: // 保存设置
						// 将数据写入DS1302 RAM
						DS1302_wirte_rig(0x8e, 0x00); // 关闭写保护
						// 转换十进制到BCD
						uint8_t hour_bcd = ((TimeRAM.hour_kai / 10) << 4) | (TimeRAM.hour_kai % 10);
						uint8_t minute_bcd = ((TimeRAM.minute_kai / 10) << 4) | (TimeRAM.minute_kai % 10);
						
						DS1302_wirte_rig(0xC0, hour_bcd);    // 写入小时
						DS1302_wirte_rig(0xC2, minute_bcd);  // 写入分钟
						DS1302_wirte_rig(0x8e, 0x80); // 开启写保护
						set_mode = 0;
						break;
					case 2: // 切换设置位置
						set_pos = (set_pos + 1) % 5; // 0-4循环
						break;
					case 3: // 数值加
						if (set_pos == 0) {
							TimeRAM.hour_kai = (TimeRAM.hour_kai + 1) % 24;
						} 
						else if (set_pos == 1) {
							TimeRAM.minute_kai = (TimeRAM.minute_kai + 1) % 60;
						}
						else if (set_pos == 2) {
							if(medicine1_count < 99) medicine1_count++;
						}
						else if (set_pos == 3) {
							if(medicine2_count < 99) medicine2_count++;
						}
						else if (set_pos == 4) {
							if(medicine3_count < 99) medicine3_count++;
						}
						break;
					case 4: // 数值减
						if (set_pos == 0) {
							TimeRAM.hour_kai = (TimeRAM.hour_kai - 1 + 24) % 24;
						} 
						else if (set_pos == 1) {
							TimeRAM.minute_kai = (TimeRAM.minute_kai - 1 + 60) % 60;
						}
						else if (set_pos == 2) {
							if(medicine1_count > 0) medicine1_count--;
						}
						else if (set_pos == 3) {
							if(medicine2_count > 0) medicine2_count--;
						}
						else if (set_pos == 4) {
							if(medicine3_count > 0) medicine3_count--;
						}
						break;
				}
			}
		}
		
        // 判断是否到达定时时间
        if ((TimeData.hour == TimeRAM.hour_kai && TimeData.minute >= TimeRAM.minute_kai) || 
            (TimeData.hour > TimeRAM.hour_kai)) 
        {
			
			uint8_t medicine_status[3] = {0};  // 药品状态数组
			
			// 检测药品余量状态
			medicine_status[0] = (medicine1_count > 0) ? 1 : 0;  // 药1状态
			medicine_status[1] = (medicine2_count > 0) ? 1 : 0;  // 药2状态
			medicine_status[2] = (medicine3_count > 0) ? 1 : 0;  // 药3状态
			
			if (!medicineTaken) {
                LED_On();
                LCD_ShowChinese(84,140,"未吃药",BLUE,YELLOW,12,0);
				Servo_SetAngle(1,0);
				
				// 根据药品状态控制舵机
				for(uint8_t i = 0; i < 3; i++) {
					if(medicine_status[i]) {
						Servo_SetAngle(i+1, 0);  // 有药→0度（开仓）
					} else {
						Servo_SetAngle(i+1, 90); // 无药→90度（关仓）
					}
				}
                // 计算当前时间（单位：秒）
                uint32_t currentTime = TimeData.hour*3600 + TimeData.minute*60 + TimeData.second;
                
                // 语音播放逻辑
                if (!voiceTriggered || (currentTime - lastPlayTime >= 30)) {
                    printf("A7:00001"); // 发送语音指令
                    voiceTriggered = 1;
                    lastPlayTime = currentTime;
				
                }
            } else {
                LED_Off();
                LCD_ShowChinese(84,140,"已吃药",BLUE,YELLOW,12,0);
				Servo_SetAngle(1, 90);
				Servo_SetAngle(2, 90);
				Servo_SetAngle(3, 90);
            }

            // 按键检测
            Num = Key_Num();
            if (Num == 5) {
                medicineTaken = 1;
                LED_Off();
                LCD_ShowChinese(84,140,"已吃药",BLUE,YELLOW,12,0);
				Servo_SetAngle(1, 90);
				Servo_SetAngle(2, 90);
				Servo_SetAngle(3, 90);
                voiceTriggered = 0; // 重置语音触发标志
            }
        } 
        else 
        {
            // 未到定时时间重置所有状态
            medicineTaken = 0;
			voiceTriggered = 0;
            lastPlayTime = 0;
            LED_Off();
            LCD_ShowChinese(84,140,"已吃药",BLUE,YELLOW,12,0);
			Servo_SetAngle(1, 90);
			Servo_SetAngle(2, 90);
			Servo_SetAngle(3, 90);
        }
		
	}
}

void Hardware_Init(void)
{
	SystemInit();//配置系统时钟为72M	
	delay_init(72);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	LED_Init();
	LCD_Init();
	Servo_Init();
	MAX30102_Init();
	Usart3_Init(9600);
	DS1302_GPIO_Init();	
//	DS1302_Init();			//时间设置
	Key_Init();
	DS18B20_Init();			//体温传感器初始化
	SysTick_Config(SystemCoreClock / 1000); // 1ms中断
	
	printf("AF:5");//声音调到5档   较小(30最大)
	
	un_min=0x3FFFF;
	un_max=0; 
	
	LCD_Fill(0,0,LCD_W,LCD_H,WHITE);
}

void Display_Update(void)
{ 
    DS1302_read_realTime();    // 读取实时时间

	temperature=DS18B20_Get_Temp();	//读取温度
	
	LCD_ShowChinese(40,0,"智能药箱",BLACK,WHITE,12,0);
	
	sprintf(st,"%04d  ",TimeData.year);		//年
	LCD_ShowString(2,14,st,BLACK,WHITE,12,0);
	LCD_ShowChinese(26,14,"年",BLACK,WHITE,12,0);

	sprintf(st,"%02d  ",TimeData.month);	//月
	LCD_ShowString(38,14,st,BLACK,WHITE,12,0);
	LCD_ShowChinese(50,14,"月",BLACK,WHITE,12,0);
	
	sprintf(st,"%02d  ",TimeData.day);		//日
	LCD_ShowString(62,14,st,BLACK,WHITE,12,0);
	LCD_ShowChinese(74,14,"日",BLACK,WHITE,12,0);
	
	LCD_ShowChinese(2,30,"时间",BLACK,WHITE,12,0);
	sprintf(st,"%02d  ",TimeData.hour);		//时
	LCD_ShowString(36,27,st,BLACK,WHITE,16,0);

	sprintf(st,"%02d  ",TimeData.minute);	//分
	LCD_ShowString(62,27,st,BLACK,WHITE,16,0);
	
	LCD_ShowString(52,27,":",BLACK,WHITE,16,0);

	sprintf(st,"%02d  ",TimeData.second);	//秒
	LCD_ShowString(81,31,st,BLACK,WHITE,12,0);

	LCD_ShowChinese(96,14,"周",BLACK,WHITE,12,0);
	switch(TimeData.week)					//周
	{
		case 1:
		{
			LCD_ShowChinese(108,14,"一",BLACK,WHITE,12,0);
		}break;
		case 2:
		{
			LCD_ShowChinese(108,14,"二",BLACK,WHITE,12,0);
		}break;
		case 3:
		{
			LCD_ShowChinese(108,14,"三",BLACK,WHITE,12,0);
		}break;
		case 4:
		{
			LCD_ShowChinese(108,14,"四",BLACK,WHITE,12,0);
		}break;
		case 5:
		{
			LCD_ShowChinese(108,14,"五",BLACK,WHITE,12,0);
		}break;
		case 6:
		{
			LCD_ShowChinese(108,14,"六",BLACK,WHITE,12,0);
		}break;
		case 7:
		{
			LCD_ShowChinese(108,14,"日",BLACK,WHITE,12,0);
		}
	
	}
	
	LCD_ShowChinese(2,46,"闹钟",BLACK,WHITE,12,0);
	LCD_ShowString(36,46,"Clock",BLACK,WHITE,12,0);

// 新显示代码
	if (set_mode) {
		// 设置模式下的特殊显示
		char hour_part[3], minute_part[3];
		char count1[3], count2[3], count3[3];; 
		sprintf(hour_part, "%02d", TimeRAM.hour_kai);
		sprintf(minute_part, "%02d", TimeRAM.minute_kai);
		sprintf(count1, "%d", medicine1_count);
		sprintf(count2, "%d", medicine2_count);
		sprintf(count3, "%d", medicine3_count);
		
		// 小时部分
		if (set_pos == 0) {
			LCD_ShowString(72, 46, hour_part, BLACK, YELLOW, 12, 0);
		} else {
			LCD_ShowString(72, 46, hour_part, BLACK, YELLOW, 12, 0);
		}
		
		// 分隔符
		LCD_ShowString(84, 46, ":", BLACK, YELLOW, 12, 0);
		
		// 分钟部分
		if (set_pos == 1) {
			LCD_ShowString(90, 46, minute_part, BLACK, YELLOW, 12, 0);
		} else {
			LCD_ShowString(90, 46, minute_part, BLACK, YELLOW, 12, 0);
		}
		
		// 药1
		if(set_pos == 2) 
			LCD_ShowString(104,62, count1, BLACK, YELLOW, 12, 0);
		else
			LCD_ShowString(104,62, count1, BLACK, YELLOW, 12, 0);

		// 药2 
		if(set_pos == 3)
			LCD_ShowString(104,78, count2, BLACK, YELLOW, 12, 0);
		else
			LCD_ShowString(104,78, count2, BLACK, YELLOW, 12, 0);

		// 药3
		if(set_pos == 4)
			LCD_ShowString(104,94, count3, BLACK, YELLOW, 12, 0); 
		else
			LCD_ShowString(104,94, count3, BLACK, YELLOW, 12, 0);		
	
	} else {
		DS1302_RAM();              // 读取定时时间
		sprintf(st, "%02d:%02d", TimeRAM.hour_kai, TimeRAM.minute_kai);
		LCD_ShowString(36,46,"Clock",BLACK,WHITE,12,0);
		LCD_ShowString(72,46, st, BLACK, YELLOW, 12, 0);
		
		sprintf(count, "%d", medicine1_count);
		LCD_ShowString(104,62, count, BLACK, YELLOW, 12, 0);
		sprintf(count, "%d", medicine2_count);
		LCD_ShowString(104,78, count, BLACK, YELLOW, 12, 0);
		sprintf(count, "%d", medicine3_count);
		LCD_ShowString(104,94, count, BLACK, YELLOW, 12, 0);
	}
	
	LCD_ShowChinese(36,62,"药品",BLACK,WHITE,12,0);
	LCD_ShowChinese(60,62,"①",BLACK,WHITE,12,0);
	LCD_ShowChinese(76,62,"服用",BLACK,WHITE,12,0);
	LCD_ShowChinese(114,62,"粒",BLACK,WHITE,12,0);
	LCD_ShowChinese(36,78,"药品",BLACK,WHITE,12,0);
	LCD_ShowChinese(60,78,"②",BLACK,WHITE,12,0);
	LCD_ShowChinese(76,78,"服用",BLACK,WHITE,12,0);
	LCD_ShowChinese(114,78,"粒",BLACK,WHITE,12,0);
	LCD_ShowChinese(36,94,"药品",BLACK,WHITE,12,0);
	LCD_ShowChinese(60,94,"③",BLACK,WHITE,12,0);
	LCD_ShowChinese(76,94,"服用",BLACK,WHITE,12,0);
	LCD_ShowChinese(114,94,"粒",BLACK,WHITE,12,0);
	
	//MAX30102
	if(dis_hr==0)
	{
	LCD_ShowString(32,110,"000",BLACK,WHITE,12,0);
	LCD_ShowString(32,126,"00",BLACK,WHITE,12,0);
	}
	else
	{
	sprintf(hr_str, "%03d", dis_hr - 20);
	LCD_ShowString(32,110,hr_str,BLACK,WHITE,12,0);
	sprintf(spo2_str,"%02d", dis_spo2);
	LCD_ShowString(32,126,spo2_str,BLACK,WHITE,12,0);
	}
	
	LCD_ShowChinese(2,110,"心率",BLACK,WHITE,12,0);
	LCD_ShowString(54,110,"bmp",BLACK,WHITE,12,0);
	
	LCD_ShowChinese(2,126,"血氧",BLACK,WHITE,12,0);
	LCD_ShowString(44,126," ",BLACK,WHITE,12,0);
	LCD_ShowString(48,126,"%",BLACK,WHITE,12,0);
	
	LCD_ShowChinese(2,142,"体温",BLACK,WHITE,12,0);
	sprintf((char*)p,"%4.1f    ",(float)temperature/10);
	LCD_ShowString(32,142,p ,BLACK,WHITE,12,0);	
	LCD_ShowChinese(58,143,"℃",BLACK,WHITE,12,0);
	
}
	