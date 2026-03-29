#include "stm32f10x.h"                  // Device header
#include "stm32f10x_flash.h"
#include "Delay.h"
#include "adcx.h"
#include "usart.h"
#include "OLED.h"
#include "Key.h"
#include "Buzzer.h"
#include "LightSensor.h"
#include "BODY_HW.h"
#include "stepmotor.h"
#include "dht11.h"
#include "ws2812b.h"
#include "led.h"
#include "time.h"
#include "IRremot.h"
#include "PWM.h"

//网络协议层
#include "onenet.h"

//网络设备
#include "esp8266.h"

//全局变量
#include "globals.h"

//C库
#include <stdio.h>
#include <string.h>

#define EEPROM_ADDR 0x08080000  // STM32的EEPROM起始地址

uint8_t i;			//定义for循环的变量

u8 temp,humi;
uint8_t KeyNum;

volatile int living_auto = 0;      // 0:手动 1:自动
volatile int bed_auto = 0;
volatile int study_auto = 0;
volatile int kit_auto = 0;
volatile int bat_auto = 0;

volatile uint16_t global_light = 0;     // 光照强度

volatile uint8_t  living_human = 0;     // 人体检测 
volatile uint8_t  bed_human = 0;     // 人体检测
volatile uint8_t  study_human = 0;     // 人体检测
volatile uint8_t  kit_human = 0;     // 人体检测
volatile uint8_t  bat_human = 0;     // 人体检测

uint8_t light_state = 0;  // 0-未知 1-低光照 2-高光照
uint8_t systemModel = 0;	//存储系统当前模式，0手动，1自动

unsigned short timeCount = 0;	//发送间隔变量
unsigned char *dataPtr = NULL;

char DATA_BUF[256];
char test[256];

void Hardware_Init(void);
void Display_Init(void);
void Topic_Init(void);

int main()
{	
	
	Hardware_Init();				//初始化外围硬件
	
	ESP8266_Init();					//初始化ESP8266
	
	Topic_Init(); 	
	
	while(1)
	{
		
//获取传感器数值-------------------------------------------------------
		living_human = BODY_HW_GetData(0);      // 人体 A15    LED A2
		bed_human = BODY_HW_GetData(1);			// 人体 B12    LED B0 
		study_human = BODY_HW_GetData(2);		// 人体 B13    LED A1
		kit_human = BODY_HW_GetData(3);			// 人体 B14    LED A0
		bat_human = BODY_HW_GetData(4);			// 人体 B15    LED B1
		global_light = LightSensor_LuxData();  // 更新光照
		DHT11_Read_Data(&temp,&humi);
		
		OLED_ShowNum(80,0,global_light,3,16,1);	//光照强度
		OLED_ShowNum(44,20,temp,2,16,1); //温度值
		OLED_ShowNum(44,40,humi,2,16,1); //湿度值

//上传温湿度--------------------------------------------------		
		if(upload_flag) {		//定时10s上传
            upload_flag = 0;
            
            DHT11_Read_Data(&temp, &humi);

			sprintf(DATA_BUF, "{\"Humi\":%d}", humi);
			OneNET_Publish(MQTT_TOPIC8, DATA_BUF);
			
			memset(DATA_BUF, 0, sizeof(DATA_BUF));
			Delay_ms(500);
			
			sprintf(DATA_BUF, "{\"Temp\":%d}", temp);
			OneNET_Publish(MQTT_TOPIC8, DATA_BUF);
            
            ESP8266_Clear();
				
        }
		
//接收下发消息--------------------------------------------------	

		dataPtr = ESP8266_GetIPD(0);
		if(dataPtr)
		{
			OneNet_RevPro(dataPtr);
			ESP8266_Clear();
		}
	
		// 执行自动化逻辑（无论是否收到指令都要运行）
        if(living_auto) {
            HandleLivingLEDAuto();  // LivingLED自动化
        }
		if(bed_auto) {
            HandleBedLEDAuto();  // 新增的自动化核心逻辑
        }
        if(study_auto) {
            HandleStudyLEDAuto();  // 新增的自动化核心逻辑
        }
        if(kit_auto) {
            HandleKitLEDAuto();  // 新增的自动化核心逻辑
        }
        if(bat_auto) {
            HandleBathLEDAuto();  // 新增的自动化核心逻辑
        }
		
	}
}

void Hardware_Init(void)
{
	SystemInit();
	led_Init();  // 初始化LED控制
	Timer_Init();
	BODY_HW_Init();
	LightSensor_Init();		//光敏传感器初始化
	MOTOR_Init();			//步进电机初始化
	IR_init();
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//中断控制器分组设置
	
	Usart1_Init(115200);							//串口1，打印信息用
	
	Usart3_Init(115200);							//串口3，驱动ESP8266

	while(DHT11_Init())
	{
		UsartPrintf(USART_DEBUG, "DHT11 Error \r\n");
		Delay_ms(1000);
	}
	
	Display_Init();
	
	UsartPrintf(USART_DEBUG, " Hardware init OK\r\n");
	
}

void Display_Init(void)
{
	OLED_Init();		//OLED初始化
	OLED_Clear(); 
	
	//显示“光照强度:”
	OLED_ShowChinese(0,0,4,16,1); 	// 光
	OLED_ShowChinese(16,0,5,16,1); // 照
	OLED_ShowChinese(32,0,6,16,1); // 强
	OLED_ShowChinese(48,0,1,16,1); // 度
	OLED_ShowChar(64,0,':',16,1);
	
	OLED_ShowChinese(0,20,0,16,1); 	// 温
	OLED_ShowChinese(16,20,1,16,1); // 度
	OLED_ShowChar(32,20,':',16,1);
	OLED_ShowChinese(66,20,12,16,1); // ℃
	
	OLED_ShowChinese(0,40,2,16,1); 	// 湿
	OLED_ShowChinese(16,40,1,16,1); // 度
	OLED_ShowChar(32,40,':',16,1);
	OLED_ShowChar(66,40,'%',16,1); // %
	
}

void Topic_Init()
{
    const char *topics[] = { 
        MQTT_TOPIC1,    // smarthome/livingroom/led
        MQTT_TOPIC2,    // smarthome/bedroom/led
        MQTT_TOPIC3,    // smarthome/studyroom/led
        MQTT_TOPIC4,    // smarthome/kitchen/led
        MQTT_TOPIC5,    // smarthome/bathroom/led
        MQTT_TOPIC6,    // smarthome/bedroom/curtain
        MQTT_TOPIC7,    // smarthome/ac
        MQTT_TOPIC8     // smarthome/environment
    };
	
	UsartPrintf(USART_DEBUG, "Connect MQTTs Server...\r\n");
	while(ESP8266_SendCmd(ESP8266_ONENET_INFO, "CONNECT"))
		Delay_ms(500);
	UsartPrintf(USART_DEBUG, "Connect MQTT Server Success\r\n");
//	
	while(OneNet_DevLink())			//接入OneNET
		Delay_ms(500);

	OneNet_Subscribe(topics, 8);
	
}
