/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"
#include "interrupt.h"
#include "oled.h"	
#include "rc522.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TARGET_STRING1 "success" 
#define TARGET_STRING2 "+MQTTSUBRECV:0,\"smarthome/lock\",1,1" 
#define TARGET_STRING3 "+MQTTSUBRECV:0,\"smarthome/lock\",13,resetpassword" 
uint8_t ESP8266_INIT_OK = 0;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t Key_row[1]={0xff};   //保存按键行扫描情况的状态数组
int key_Data;
#define START_KEY 5  // ?????? 5 ??????
#define PASSWORD_LEN 4
char current_password[PASSWORD_LEN];
#define USART_REC_LEN 200 // ???????
#define RXBUFFERSIZE 1 // ????
unsigned char USART1_RX_BUF[USART_REC_LEN]; // ????
unsigned int USART1_RX_STA = 0; // ??????
unsigned char aRxBuffer1[RXBUFFERSIZE]; // ????
unsigned char len1; // ????????
void MY_USART1_Printf(const char* str);

unsigned char USART2_RX_BUF[USART_REC_LEN]; // ????
unsigned int USART2_RX_STA = 0; // ??????
unsigned char aRxBuffer2[RXBUFFERSIZE]; // ????
unsigned char len2; // ????????
void MY_USART2_Printf(const char* str);

unsigned char USART3_RX_BUF[USART_REC_LEN]; // ????
unsigned int USART3_RX_STA = 0; // ??????
unsigned char aRxBuffer3[RXBUFFERSIZE]; // ????
unsigned char len3; // ????????

unsigned char aRxBuffer[RXBUFFERSIZE]; // ???????
unsigned char len;                     // ????????

const char* sendString = "Please send the target string.\n";	
const char* targetString = "your_target_string";
extern void MFRC522_Init(void);
extern struct keys key[];
uint8_t zhiling1[]={0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x03,0x01,0x00,0x05};
uint8_t zhiling2[]={0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x04,0x02,0x01,0x00,0x08};
uint8_t zhiling3[]={0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x08,0x04,0x01,0x00,0x01,0x00,0x01,0x00,0x10};
uint8_t zhiling4[]={0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x08,0x32,0x02,0xFF,0xFF,0x00,0x05,0x02,0x40};//直接
uint8_t reply1[]={0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x07,0x00,0x08,0x00};//使用指令4正确的返回格式
extern char rxdata[];
extern uint8_t rxdat;
extern unsigned char rx_pointer;
uint8_t flag=0;
uint8_t 	k;
uint8_t 	i;
uint8_t 	j;
uint8_t 	b;
uint8_t 	comand;
uint8_t		text2[9] = "Card ID: ";
uint8_t		end[1] = "\r";
uint8_t		txBuffer[18] = "00000000";
uint8_t		NFCka[18] = "C3D1BB0F";
uint8_t		NFCka1[18] = "030B8016";
uint8_t 	retstr[10];
uint8_t 	rxBuffer[8];
uint8_t		lastID[4];
uint8_t		memID[8] = "9C55A1B5";
uint8_t		str[MFRC522_MAX_LEN];																						// MFRC522_MAX_LEN = 16
uint8_t MFRC522_Check(uint8_t* id);
uint8_t MFRC522_Compare(uint8_t* CardID, uint8_t* CompareID);
void MFRC522_WriteRegister(uint8_t addr, uint8_t val);
uint8_t MFRC522_ReadRegister(uint8_t addr);
void MFRC522_SetBitMask(uint8_t reg, uint8_t mask);
void MFRC522_ClearBitMask(uint8_t reg, uint8_t mask);
uint8_t MFRC522_Request(uint8_t reqMode, uint8_t* TagType);
uint8_t MFRC522_ToCard(uint8_t command, uint8_t* sendData, uint8_t sendLen, uint8_t* backData, uint16_t* backLen);
uint8_t MFRC522_Anticoll(uint8_t* serNum);
void MFRC522_CalulateCRC(uint8_t* pIndata, uint8_t len, uint8_t* pOutData);
uint8_t MFRC522_SelectTag(uint8_t* serNum);
uint8_t MFRC522_Auth(uint8_t authMode, uint8_t BlockAddr, uint8_t* Sectorkey, uint8_t* serNum);
uint8_t MFRC522_Read(uint8_t blockAddr, uint8_t* recvData);
uint8_t MFRC522_Write(uint8_t blockAddr, uint8_t* writeData);
void MFRC522_Init(void);
void MFRC522_Reset(void);
void MFRC522_AntennaOn(void);
void MFRC522_AntennaOff(void);
void MFRC522_Halt(void);

void MY_USART3_Printf(const char* str);
void char_to_hex(uint8_t data);
void MY_USART_Printf(UART_HandleTypeDef *huart, const char* str);
int compareReceivedString(int uart_num, const char* sendStr, const char* targetStr);
char KEY_ROW_SCAN(void)
{
    //读出行扫描状态
    Key_row[0] = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_12)<<3;//列1读取
    Key_row[0] = Key_row[0] | (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_13)<<2);//列2读取
    Key_row[0] = Key_row[0] | (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_14)<<1);//列3读取
    Key_row[0] = Key_row[0] | (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_15));//列4读取
    
    if(Key_row[0] != 0x0f)         //行扫描有变化，判断该列有按键按下
    {
      HAL_Delay(10);                    //消抖
      if(Key_row[0] != 0x0f)
        {   
                switch(Key_row[0])
                {
                    case 0x07:         //0111 判断为该列第1行的按键按下
                        return 1;
                    case 0x0b:         //1011 判断为该列第2行的按键按下
                        return 2;
                    case 0x0d:         //1101 判断为该列第3行的按键按下
                        return 3;
                    case 0x0e:         //1110 判断为该列第4行的按键按下
                        return 4;
                    default :
                        return 0;
                }
        }
        else return 0;
    }
    else return 0;
}
char KEY_SCAN(void)
{    
    char Key_Num=0;       //1-16对应的按键数
    char key_row_num=0;        //行扫描结果记录
    
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);	//行1置低
    if( (key_row_num=KEY_ROW_SCAN()) != 0 )
    { 
        while(KEY_ROW_SCAN() != 0);  //消抖
        Key_Num = 0 + key_row_num;
    
    }
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);	//行1置高
    
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_RESET);	//行2置低       
    if( (key_row_num=KEY_ROW_SCAN()) != 0 )
    { 
        while(KEY_ROW_SCAN() != 0);
        Key_Num = 4 + key_row_num;
    }
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_SET);	//行2置高
    
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_RESET);	//行3置低     
    if( (key_row_num=KEY_ROW_SCAN()) != 0 )
    { 
        while(KEY_ROW_SCAN() != 0);
    Key_Num = 8 + key_row_num;
    }
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_SET);	//行3置高
    
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);	//行4置低    
    if( (key_row_num=KEY_ROW_SCAN()) != 0 )
    {
        while(KEY_ROW_SCAN() != 0);
        Key_Num = 12 + key_row_num;

    }
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);	//行4置高  
    
    return Key_Num;
}
void initialize_password() {
    // ??????????,?? 1, 2, 3, 4
    for (int i = 0; i < PASSWORD_LEN; i++) {
        current_password[i] = i + 1;
    }
}

//// ???????
void change_password() {
    char new_password[PASSWORD_LEN];
    int i = 0;
    char display_str[PASSWORD_LEN + 1];  // ???? * ????

    OLED_FullyClear();
    OLED_ShowStr(0, 0, (unsigned char *)"new password", 2);

    while (i < PASSWORD_LEN) {
        char key = KEY_SCAN();
        if (key != 0) {  // ???????
            new_password[i] = key;
            // ??????????? *
            display_str[i] = '*';
            display_str[i + 1] = '\0';  // ??????
            // ??????? * ???
            OLED_ShowStr(0, 10, (unsigned char *)display_str, 2);
            i++;
        }
    }

    // ?????????????
    for (i = 0; i < PASSWORD_LEN; i++) {
        current_password[i] = new_password[i];
    }
    OLED_FullyClear();
    OLED_ShowStr(0, 0, (unsigned char *)"Password changed", 2);
		HAL_Delay(500);
		OLED_FullyClear();
}

//// ?????????


//// ?????????
void password_input_and_verify() {
    char input_password[PASSWORD_LEN];
    int i = 0;
    char display_str[PASSWORD_LEN + 1];  // ???? * ????
		OLED_FullyClear();
    OLED_ShowStr(0, 0, (unsigned char *)"Enter password", 2);

    // ??????
    while (i < PASSWORD_LEN) {
        char key = KEY_SCAN();
        if (key != 0) {  // ???????
            input_password[i] = key;
            // ??????????? *
            display_str[i] = '*';
            display_str[i + 1] = '\0';  // ??????
            // ??????? * ???
            OLED_ShowStr(0, 10, (unsigned char *)display_str, 2);
            i++;
        }
    }

    // ????
    for (i = 0; i < PASSWORD_LEN; i++) {
        if (input_password[i] != current_password[i]) {
            OLED_FullyClear();
            OLED_ShowStr(0, 0, (unsigned char *)"Password wrong", 2);
        }
        else if (i==3&&input_password[i] == current_password[i])
					{
				  OLED_FullyClear();
					OLED_ShowStr(0, 0, (unsigned char *)"Password correct", 2);
					__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_1,1500);//90
			    HAL_Delay(1000);
				  __HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_1,500);//0
					}
    }
				HAL_Delay(500);
				OLED_FullyClear();
}
void char_to_hex(uint8_t data) {
	uint8_t digits[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	
	if (data < 16) {
		retstr[0] = '0';
		retstr[1] = digits[data];
	} else {
		retstr[0] = digits[(data & 0xF0)>>4];
		retstr[1] = digits[(data & 0x0F)];
	}
}

void ESP8266_Init(void)//初始化wifi模块
{
	  HAL_Delay(3000);

		USART2_RX_STA = 0;
	  while ((USART2_RX_STA & 0x8000)==0){   	
			MY_USART2_Printf("AT+CWMODE=1\r\n");
			HAL_Delay(500);
		}
		HAL_Delay(500);
		USART2_RX_STA = 0;
		while ((USART2_RX_STA & 0x8000)==0){   	
    MY_USART2_Printf("AT+CWJAP=\"lin\",\"210629939\"\r\n");
			HAL_Delay(500);
		}	
		HAL_Delay(2000);
		USART2_RX_STA = 0;
		while ((USART2_RX_STA & 0x8000)==0){   	
		MY_USART2_Printf("AT+MQTTUSERCFG=0,1,\"MQTT_ID\",\"username\",\"password\",0,0,\"\"\r\n");
			HAL_Delay(500);
		}	
		HAL_Delay(500);
    USART2_RX_STA = 0;
		while ((USART2_RX_STA & 0x8000)==0){   	
    MY_USART2_Printf("AT+MQTTCONN=0,\"47.95.171.151\",1883,1\r\n");
			HAL_Delay(500);
		}	
		HAL_Delay(1000);
		USART2_RX_STA = 0;
		while ((USART2_RX_STA & 0x8000)==0){   	
	  MY_USART2_Printf("AT+MQTTSUB=0,\"smarthome/lock\",0\r\n");
			HAL_Delay(500);
		}	
		HAL_Delay(500);
		USART2_RX_STA = 0;
		while ((USART2_RX_STA & 0x8000)==0){   	
    MY_USART2_Printf("AT+MQTTPUB=0,\"smarthome/lock\",\"init\",0,0\r\n");
			HAL_Delay(500);
		}
		USART2_RX_STA = 0;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
//    unsigned short timeCount = 0;	//??????
//    unsigned char *dataPtr = NULL;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_TIM3_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
	MFRC522_Init();//RC522初始化
  HAL_TIM_Base_Start_IT(&htim3);
  HAL_UART_Receive_IT(&huart1, (unsigned char*)aRxBuffer1, RXBUFFERSIZE);
	HAL_UART_Receive_IT(&huart2, (unsigned char*)aRxBuffer2, RXBUFFERSIZE);
  HAL_UART_Receive_IT(&huart3, (unsigned char*)aRxBuffer3, RXBUFFERSIZE);
	HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_1);
//  ESP8266_Init();					//???ESP8266
  OLED_Init();
	OLED_FullyClear();
	OLED_ShowStr(35,10, (unsigned char *)"init",2);
	initialize_password();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
//		HAL_Delay(500);
//		OLED_FullyClear();
		OLED_ShowStr(35,10, (unsigned char *)"LOCK",2);
		key_Data = KEY_SCAN();
		if(key_Data==13)
		{
//		   OLED_ShowStr1(35,10, &key_Data,1,2);
			password_input_and_verify();
		}
	 if (USART1_RX_STA & 0x8000) {
		len1 = USART1_RX_STA & 0x3fff;
    USART1_RX_STA = 0;

    if (len1 >= strlen(TARGET_STRING1)) {
			USART1_RX_BUF[len1] = '\0'; // ????????
      if (strcmp((char*)USART1_RX_BUF, TARGET_STRING1) == 0) {
				OLED_ShowStr(35,10, (unsigned char *)"pass",2);
				__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_1,1500);//90
				HAL_Delay(1000);
				__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_1,500);//0
		    OLED_FullyClear();
      }
    }

	 }
	 if (USART2_RX_STA & 0x8000) {
		len2 = USART2_RX_STA & 0x3fff;
    USART2_RX_STA = 0;
    if (len2 >= strlen(TARGET_STRING2)) {
			USART2_RX_BUF[len2] = '\0'; // ????????
      if (strcmp((char*)USART2_RX_BUF, TARGET_STRING2) == 0) {
        OLED_ShowStr(35,10, (unsigned char *)"pass",2);
				__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_1,1500);//90
				HAL_Delay(1000);
				__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_1,500);//0
		    OLED_FullyClear();

      }
			else if(strcmp((char*)USART2_RX_BUF, TARGET_STRING3) == 0) {
        change_password();
      }
    }
	 }
	 		if(key[0].single_flag==1)
	  {
		HAL_UART_Transmit(&huart3,(uint8_t *)zhiling4,sizeof(zhiling4),50);
						key[0].single_flag=0;
	  }
 if (USART3_RX_STA & 0x8000) {
        len3 = USART3_RX_STA & 0x3fff;
        USART3_RX_STA = 0;
        // ????????????
        if (len3 >= sizeof(reply1)) {
            USART3_RX_BUF[len3] = '\0'; 
            if (memcmp(USART3_RX_BUF, reply1, sizeof(reply1)) == 0) {
                OLED_ShowStr(35, 10, (unsigned char *)"pass", 2);
							  __HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_1,1500);//90
								HAL_Delay(1000);
							  __HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_1,500);//0
		            OLED_FullyClear();
            } else {
                OLED_ShowStr(35, 10, (unsigned char *)"fail", 2);
							  HAL_Delay(1000);
		            OLED_FullyClear();
            }
        }
    }
// 					  HAL_UART_Transmit(&huart3, USART3_RX_BUF, len3, HAL_MAX_DELAY);

	 if (memcmp(txBuffer, NFCka1, sizeof(txBuffer)) == 0) {
		 OLED_FullyClear();
		 OLED_ShowStr(35,10, (unsigned char *)"pass",2);
		 __HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_1,1500);//90
		 HAL_Delay(1000);
		 __HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_1,500);//0
		 OLED_FullyClear();
		 txBuffer[0] = 0xFF;
    } 
	 else if(memcmp(txBuffer, NFCka, sizeof(txBuffer)) == 0){
		OLED_FullyClear();
		OLED_ShowStr(35,10, (unsigned char *)"fail",2);
		HAL_Delay(1000);
		OLED_FullyClear();
		txBuffer[0] = 0xFF;
    }
	if (!MFRC522_Request(PICC_REQIDL, str)) { // ??
    if (!MFRC522_Anticoll(str)) { // ???????
        int b = 0;

        for (int i = 0; i < 4; i++) {
            lastID[i] = str[i];
        }

        for (int i = 0; i < 4; i++) {
            char_to_hex(str[i]);
            txBuffer[b] = retstr[0]; // ????? txBuffer ?
            b++;
            txBuffer[b] = retstr[1];
            b++;
        }
    }
}


  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void MY_USART_Printf(UART_HandleTypeDef *huart, const char* str)
{
    if (str == NULL) {
        return;
    }
    unsigned int usart_len = strlen(str);
    unsigned int index = 0;
    while (index < usart_len) {
        HAL_UART_Transmit(huart, (unsigned char*)&str[index], 1, 0xff);
        index++;
    }
}

void MY_USART1_Printf(const char* str)
{
    if (str == NULL) {
        return;
    }
    unsigned int usart1_len = strlen(str);
    unsigned int index = 0;
    while (index < usart1_len) {
        HAL_UART_Transmit(&huart1, (unsigned char*)&str[index], 1, 0xff);
        index++;
    }
}
void MY_USART2_Printf(const char* str)
{
    if (str == NULL) {
        return;
    }
    unsigned int usart2_len = strlen(str);
    unsigned int index = 0;
    while (index < usart2_len) {
        HAL_UART_Transmit(&huart2, (unsigned char*)&str[index], 1, 0xff);
        index++;
    }
}
void MY_USART3_Printf(const char* str)
{
    if (str == NULL) {
        return;
    }
    unsigned int usart3_len = strlen(str);
    unsigned int index = 0;
    while (index < usart3_len) {
        HAL_UART_Transmit(&huart3, (unsigned char*)&str[index], 1, 0xff);
        index++;
    }
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{
    if (huart->Instance == USART1) {
        if ((USART1_RX_STA & 0x8000) == 0) // ?????
        {
            if (USART1_RX_STA & 0x4000) // ????0x0d
            {
                if (aRxBuffer1[0] != 0x0a) // ????,????
                    USART1_RX_STA = 0;
                else // ?????
                    USART1_RX_STA |= 0x8000;
            } else // ????0X0D
            {
                if (aRxBuffer1[0] == 0x0d)
                    USART1_RX_STA |= 0x4000;
                else {
                    USART1_RX_BUF[USART1_RX_STA & 0X3FFF] = aRxBuffer1[0];
                    USART1_RX_STA++;
                    if (USART1_RX_STA > (USART_REC_LEN - 1))
                        USART1_RX_STA = 0; // ??????,??????
                }
            }
        }
    }
		if (huart->Instance == USART2) {
        if ((USART2_RX_STA & 0x8000) == 0) // ?????
        {
            if (USART2_RX_STA & 0x4000) // ????0x0d
            {
                if (aRxBuffer2[0] != 0x0a) // ????,????
                    USART2_RX_STA = 0;
                else // ?????
                    USART2_RX_STA |= 0x8000;
            } else // ????0X0D
            {
                if (aRxBuffer2[0] == 0x0d)
                    USART2_RX_STA |= 0x4000;
                else {
                    USART2_RX_BUF[USART2_RX_STA & 0X3FFF] = aRxBuffer2[0];
                    USART2_RX_STA++;
                    if (USART2_RX_STA > (USART_REC_LEN - 1))
                        USART2_RX_STA = 0; // ??????,??????
                }
            }
        }
    }
		if (huart->Instance == USART3) {
        if ((USART3_RX_STA & 0x8000) == 0) // ?????
        {
            if (USART3_RX_STA & 0x4000) // ????0x0d
            {
                if (aRxBuffer3[0] != 0x0a) // ????,????
                    USART3_RX_STA = 0;
                else // ?????
                    USART3_RX_STA |= 0x8000;
            } else // ????0X0D
            {
                if (aRxBuffer3[0] == 0x0d)
                    USART3_RX_STA |= 0x4000;
                else {
                    USART3_RX_BUF[USART3_RX_STA & 0X3FFF] = aRxBuffer3[0];
                    USART3_RX_STA++;
										            // ??????????17???
										if ((USART3_RX_STA & 0x3FFF) >= 17) 
										{
												// ?????17???,? USART3_RX_STA ?????? 1,??????
												USART3_RX_STA |= 0x8000;
										}
										else
										{
                // ?????????????????????
												if (USART3_RX_STA > (USART_REC_LEN - 1)) 
												{
                    // ?????????,????????,??????,??????
														USART3_RX_STA = 0; 
												}
										}
                }
            }
        }
    }
}

int compareReceivedString(int uart_num, const char* sendStr, const char* targetStr)//参数为第几个串口 发送字符串，接接收字符串
{
    UART_HandleTypeDef *huart;
    unsigned int *rx_sta;
    unsigned char *rx_buf;

    // ???????????????????????????
    switch (uart_num) {
        case 1:
            huart = &huart1;
            rx_sta = &USART1_RX_STA;
            rx_buf = USART1_RX_BUF;
            break;
        case 2:
            huart = &huart2;
            rx_sta = &USART2_RX_STA;
            rx_buf = USART2_RX_BUF;
            break;
        case 3:
            huart = &huart3;
            rx_sta = &USART3_RX_STA;
            rx_buf = USART3_RX_BUF;
            break;
        default:
            return 1;
    }

    // ??????????
    MY_USART_Printf(huart, sendStr);

    // ????????
    while ((*rx_sta & 0x8000) == 0) {
        // ??????????,??????
    }

    // ??????????
    len = *rx_sta & 0x3fff;
    *rx_sta = 0;

    // ????????????
    if (len >= strlen(targetStr)) {
        rx_buf[len] = '\0'; // ????????
        if (strcmp((char*)rx_buf, targetStr) == 0) {
            return 0; // ????????????
        }
    }
    return 1; // ?????????????
}
void USART1_IRQHandler(void)
{
    unsigned int timeout = 0;
    unsigned int maxDelay = 0x1FFFF;

    HAL_UART_IRQHandler(&huart1); // ??HAL?????????

    timeout = 0;
    while (HAL_UART_GetState(&huart1) != HAL_UART_STATE_READY) // ????
    {
        timeout++; // ????
        if (timeout > maxDelay)
            break;
    }

    timeout = 0;
    // ????????,?????????RxXferCount?1
    while (HAL_UART_Receive_IT(&huart1, (unsigned char*)aRxBuffer1, RXBUFFERSIZE) != HAL_OK) {
        timeout++; // ????
        if (timeout > maxDelay)
            break;
    }
}
void USART2_IRQHandler(void)
{
    unsigned int timeout = 0;
    unsigned int maxDelay = 0x1FFFF;

    HAL_UART_IRQHandler(&huart2); // ??HAL?????????

    timeout = 0;
    while (HAL_UART_GetState(&huart2) != HAL_UART_STATE_READY) // ????
    {
        timeout++; // ????
        if (timeout > maxDelay)
            break;
    }

    timeout = 0;
    // ????????,?????????RxXferCount?1
    while (HAL_UART_Receive_IT(&huart2, (unsigned char*)aRxBuffer2, RXBUFFERSIZE) != HAL_OK) {
        timeout++; // ????
        if (timeout > maxDelay)
            break;
    }
}
void USART3_IRQHandler(void)
{
    unsigned int timeout = 0;
    unsigned int maxDelay = 0x1FFFF;

    HAL_UART_IRQHandler(&huart3); // ??HAL?????????

    timeout = 0;
    while (HAL_UART_GetState(&huart3) != HAL_UART_STATE_READY) // ????
    {
        timeout++; // ????
        if (timeout > maxDelay)
            break;
    }

    timeout = 0;
    // ????????,?????????RxXferCount?1
    while (HAL_UART_Receive_IT(&huart3, (unsigned char*)aRxBuffer3, RXBUFFERSIZE) != HAL_OK) {
        timeout++; // ????
        if (timeout > maxDelay)
            break;
    }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
