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
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"
#include "interrupt.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TARGET_STRING1 "success" 
#define TARGET_STRING2 "+MQTTSUBRECV:0,\"control\",13,{ \"lock\": 1 }" 
#define TARGET_STRING3 "+MQTTSUBRECV:0,\"smarthome/fan\",1,1" 
#define TARGET_STRING4 "+MQTTSUBRECV:0,\"smarthome/humidifier\",1,1" 
#define TARGET_STRING5 "+MQTTSUBRECV:0,\"smarthome/fan\",1,0" 
#define TARGET_STRING6 "+MQTTSUBRECV:0,\"smarthome/humidifier\",1,0" 

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


void MY_USART3_Printf(const char* str);
void MY_USART_Printf(UART_HandleTypeDef *huart, const char* str);



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
		MY_USART2_Printf("AT+MQTTUSERCFG=0,1,\"user1\",\"FANH\",\"password\",0,0,\"\"\r\n");
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
	  MY_USART2_Printf("AT+MQTTSUB=0,\"smarthome/fan\",0\r\n");
			HAL_Delay(500);
		}	
		HAL_Delay(500);
		USART2_RX_STA = 0;
				while ((USART2_RX_STA & 0x8000)==0){   	
	  MY_USART2_Printf("AT+MQTTSUB=0,\"smarthome/humidifier\",0\r\n");
			HAL_Delay(500);
		}	
		HAL_Delay(500);
		USART2_RX_STA = 0;
		while ((USART2_RX_STA & 0x8000)==0){   	
    MY_USART2_Printf("AT+MQTTPUB=0,\"smarthome/fan\",\"init\",0,0\r\n");
			HAL_Delay(500);
		}
		USART2_RX_STA = 0;
		while ((USART2_RX_STA & 0x8000)==0){   	
    MY_USART2_Printf("AT+MQTTPUB=0,\"smarthome/humidifier\",\"init\",0,0\r\n");
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
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim3);
  HAL_UART_Receive_IT(&huart1, (unsigned char*)aRxBuffer1, RXBUFFERSIZE);
	HAL_UART_Receive_IT(&huart2, (unsigned char*)aRxBuffer2, RXBUFFERSIZE);
  HAL_UART_Receive_IT(&huart3, (unsigned char*)aRxBuffer3, RXBUFFERSIZE);
  ESP8266_Init();					//???ESP8266
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

//	 if (USART1_RX_STA & 0x8000) {
//		len1 = USART1_RX_STA & 0x3fff;
//    USART1_RX_STA = 0;

//    if (len1 >= strlen(TARGET_STRING1)) {
//			USART1_RX_BUF[len1] = '\0'; // ????????
//      if (strcmp((char*)USART1_RX_BUF, TARGET_STRING1) == 0) {

//      }
//    }

//	 }
	 if (USART2_RX_STA & 0x8000) {
		len2 = USART2_RX_STA & 0x3fff;
    USART2_RX_STA = 0;
		if(len2 >= strlen(TARGET_STRING3)) {
			USART2_RX_BUF[len2] = '\0'; // ????????
      if (strcmp((char*)USART2_RX_BUF, TARGET_STRING3) == 0) {
	   		 HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_RESET);
      }
			else if (strcmp((char*)USART2_RX_BUF, TARGET_STRING5) == 0) {
	   		 HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_SET);
      }
    }
		if(len2 >= strlen(TARGET_STRING4)) {
			USART2_RX_BUF[len2] = '\0'; // ????????
      if (strcmp((char*)USART2_RX_BUF, TARGET_STRING4) == 0) {
         HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_RESET);
      }
			else if(strcmp((char*)USART2_RX_BUF, TARGET_STRING6) == 0) {
	   		 HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_SET);
      }
    }
	 }
//	 		if(key[0].single_flag==1)
//	  {
////		USART3_RX_STA = 0;
////		HAL_UART_Transmit(&huart3,(uint8_t *)zhiling4,sizeof(zhiling4),50);
//						key[0].single_flag=0;
//	  }
// if (USART3_RX_STA & 0x8000) {
//        len3 = USART3_RX_STA & 0x3fff;
//        USART3_RX_STA = 0;
//        // ????????????
//        if (len3 >= sizeof(reply1)) {
//            USART3_RX_BUF[len3] = '\0'; 
//            if (memcmp(USART3_RX_BUF, reply1, sizeof(reply1)) == 0) {
//            } else {
//							  HAL_Delay(1000);
//            }
//        }
//    }
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
