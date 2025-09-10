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
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_hid.h"
#include "keypad.h"
#include "string.h"
#include "oled.h"
#include <stdio.h>
#include "ws2812.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
//uint8_t Key_row[1]={0xff};
extern USBD_HandleTypeDef hUsbDeviceFS;

typedef struct {
	uint8_t MODIFIER;
	uint8_t RESERVED;
	uint8_t KEYCODE1;
	uint8_t KEYCODE2;
	uint8_t KEYCODE3;
	uint8_t KEYCODE4;
	uint8_t KEYCODE5;
	uint8_t KEYCODE6;
}keyboardReportDes;

// 确保有这样的定义
uint8_t status = 0;

/*version 1
uint32_t color;
*/

//keyboardReportDes HIDkeyBoard={0,0,0,0,0,0,0,0};


//
//uint8_t MatrixKey[20]={
//		0x59,0x5A,0x5B,0x00,
//		0x59,0x5A,0x5B,0x5C,
//		0x59,0x5A,0x5B,0x5C,
//		0x59,0x5A,0x5B,0x00,
//		0x59,0x5A,0x5B,0x00
//};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

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
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_TIM3_Init();
  MX_USB_DEVICE_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

  	  /*Init module*/
  	  OLED_Init();
  /*ws2812 module*/
  uint16_t encoder_value; // 您的编码器值
  uint8_t r, g, b;
  HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_ALL);

    OLED_NewFrame();
    OLED_DrawImage((128 - (OLED_KeyVerseImg.w)) / 2, 0, &OLED_KeyVerseImg, OLED_COLOR_NORMAL);
    OLED_ShowFrame();

	  encoder_value = __HAL_TIM_GET_COUNTER(&htim1);
	  getColorFromEncoder(encoder_value, &r, &g, &b);
	  WS2812_SetAll(r, g, b);
	  WS2812_Update();
	  HAL_Delay(10);
  /*version 1
  OLED_Init();

  HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_ALL);

  int count=0;
  char message[20]="";

  OLED_NewFrame();
  OLED_DrawImage((128 - (OLED_KeyVerseImg.w)) / 2, 0, &OLED_KeyVerseImg, OLED_COLOR_NORMAL);
  OLED_ShowFrame();
  */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  if (status == 0)
	  {
		  /*status0 module*/
		  	  /* scan keypad module*/
		  	  ScanKeypad();
	  }
	  else if (status == 1)
	  {
		  /*status1 module*/
		  /*status1 ws2812 module*/
		  encoder_value = __HAL_TIM_GET_COUNTER(&htim1);
		  getColorFromEncoder(encoder_value, &r, &g, &b);
		  WS2812_SetAll(r, g, b);
		  WS2812_Update();
		  HAL_Delay(10);
		  /*status2 OLED module*/
		  OLED_NewFrame();
//	  	  OLED_ShowNum(0, 0, 1, &font16x16, OLED_COLOR_NORMAL); // 中文、英文、符号混合显示
		  OLED_DrawImage((128 - (logoImg.w)) / 2, 0, &logoImg, OLED_COLOR_NORMAL);
		  OLED_ShowFrame();
	  }
	  else if (status == 2)
	  {
		  OLED_NewFrame();
//		  OLED_ShowNum(0, 0, 2, &font16x16, OLED_COLOR_NORMAL); // 中文、英文、符号混合显示
		  OLED_DrawImage((128 - (OLED_KeyVerseImg.w)) / 2, 0, &OLED_KeyVerseImg, OLED_COLOR_NORMAL);
		  OLED_ShowFrame();

		  status = 0;
	  }




	  /*status0 module*/
//	  	  /* scan keypad module*/
//	  	  ScanKeypad();

	  /*status1 module*/
//	  /*status1 ws2812 module*/
//	  encoder_value = __HAL_TIM_GET_COUNTER(&htim1);
//	  getColorFromEncoder(encoder_value, &r, &g, &b);
//	  WS2812_SetAll(r, g, b);
//	  WS2812_Update();
//	  HAL_Delay(10);
//	  /*status2 OLED module*/
//	  OLED_NewFrame();
//	  OLED_DrawImage((128 - (OLED_KeyVerseImg.w)) / 2, 0, &OLED_KeyVerseImg, OLED_COLOR_NORMAL);
//	  OLED_ShowFrame();


	  /*test status*/
//	  	  OLED_NewFrame(); // 新建一个空白缓冲区
//	  //	  OLED_PrintString(0, 0, "Count:", &font16x16, OLED_COLOR_NORMAL); // 中文、英文、符号混合显示
//	  //	  OLED_PrintASCIIChar(1, 0, count, &font16x16, OLED_COLOR_NORMAL);
//	  	  OLED_ShowNum(0, 0, status, &font16x16, OLED_COLOR_NORMAL); // 中文、英文、符号混合显示
//	  	  OLED_ShowFrame(); // 将缓冲区内容显示到屏幕上


//	  encoder_value = __HAL_TIM_GET_COUNTER(&htim1);

	  /*test blue*/
//	  WS2812_SetAll(0x01, 0, 0xFF);
//	  WS2812_Update();
//	  HAL_Delay(100);



//	  	  OLED_NewFrame(); // 新建一个空白缓冲区
//	  //	  OLED_PrintString(0, 0, "Count:", &font16x16, OLED_COLOR_NORMAL); // 中文、英文、符号混合显示
//	  //	  OLED_PrintASCIIChar(1, 0, count, &font16x16, OLED_COLOR_NORMAL);
//	  	  OLED_ShowNum(0, 0, encoder_value, &font16x16, OLED_COLOR_NORMAL); // 中文、英文、符号混合显示
//	  	  OLED_ShowFrame(); // 将缓冲区内容显示到屏幕上

//	  // 让RGB轮流主导
//	  if (encoder_value < 21845) {        // 0-21844: 红主导
//	      r = 255;
//	      g = (encoder_value * 255) / 21845;
//	      b = 0;
//	  } else if (encoder_value < 43690) { // 21845-43689: 绿主导
//	      r = 255 - ((encoder_value - 21845) * 255) / 21845;
//	      g = 255;
//	      b = 0;
//	  } else {                            // 43690-65535: 蓝主导
//	      r = 0;
//	      g = 255 - ((encoder_value - 43690) * 255) / 21845;
//	      b = ((encoder_value - 43690) * 255) / 21845;
//	  }

//		    WS2812_SetAll(r, g, b);
//		    WS2812_Update();
//		    HAL_Delay(10);

//	  // no 将0-65535均匀分配到3个颜色通道
//	  r = (encoder_value * 255) / 65535;        // 红色：完整范围
//	  g = ((encoder_value + 21845) * 255) / 65535 & 0xFF; // 绿色：偏移1/3
//	  b = ((encoder_value + 43690) * 255) / 65535 & 0xFF; // 蓝色：偏移2/3

//	  // no 将16位值拆分为3个8位值，颜色变化不均匀，r,g,b变化快慢不同
//	  r = (encoder_value >> 8) & 0xFF;    // 高8位作为红色
//	  g = (encoder_value >> 4) & 0xFF;    // 中间8位作为绿色
//	  b = encoder_value & 0xFF;           // 低8位作为蓝色

//  	  OLED_NewFrame(); // 新建一个空白缓冲区
//  //	  OLED_PrintString(0, 0, "Count:", &font16x16, OLED_COLOR_NORMAL); // 中文、英文、符号混合显示
//  //	  OLED_PrintASCIIChar(1, 0, count, &font16x16, OLED_COLOR_NORMAL);
//  	  OLED_ShowNum(0, 0, r, &font16x16, OLED_COLOR_NORMAL); // 中文、英文、符号混合显示
//  	 OLED_ShowNum(1, 0, g, &font16x16, OLED_COLOR_NORMAL); // 中文、英文、符号混合显示
//  	 OLED_ShowNum(2, 0, b, &font16x16, OLED_COLOR_NORMAL); // 中文、英文、符号混合显示
//  	  OLED_ShowFrame(); // 将缓冲区内容显示到屏幕上



//	  OLED_NewFrame(); // 新建一个空白缓冲区
////	  OLED_PrintString(0, 0, "Count:", &font16x16, OLED_COLOR_NORMAL); // 中文、英文、符号混合显示
////	  OLED_PrintASCIIChar(1, 0, count, &font16x16, OLED_COLOR_NORMAL);
//	  OLED_ShowNum(0, 0, count, &font16x16, OLED_COLOR_NORMAL); // 中文、英文、符号混合显示
//	  OLED_ShowFrame(); // 将缓冲区内容显示到屏幕上



	  /*test ws2812 led_all*/
//	  for (uint8_t r = 0; r <= 0xFF; r++){
//		  WS2812_SetAll(r, 0, 0xFF);
//		  WS2812_Update();
//		  HAL_Delay(10);
//	  }

/*version 1
	  ScanKeypad();

	  count = __HAL_TIM_GET_COUNTER(&htim1);
	  color=count/65536*16777216;
//	  ws2812_color=count;

	  ws2812_set_all(color);
	  ws2812_update();
*/


//	  OLED_NewFrame();
//
//	  sprintf(message,"%ld",ws2812_color);
//	  OLED_PrintString(0,0,message,&font16x16,OLED_COLOR_NORMAL);
//
//	  OLED_ShowFrame();


//	  	  //test
//	  	  HAL_Delay(50);
//	  	  HIDkeyBoard.MODIFIER=0x00; //Print char in Capital
//	  	  HIDkeyBoard.KEYCODE1=0x5C; //Print 'A'
//	  	  USBD_HID_SendReport(&hUsbDeviceFS,&HIDkeyBoard,sizeof(HIDkeyBoard));
//	  	  HAL_Delay(50);
//	  	  HIDkeyBoard.MODIFIER=0x00; //Print char in Capital
//	  	  HIDkeyBoard.KEYCODE1=0x00; //Release Key
//	  	  USBD_HID_SendReport(&hUsbDeviceFS,&HIDkeyBoard,sizeof(HIDkeyBoard));
//	  	  HAL_Delay(1000);
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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
#ifdef USE_FULL_ASSERT
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
