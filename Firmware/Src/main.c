/**
  ******************************************************************************
  * File Name          : main.c
  * Date               : 06/03/2015 23:45:18
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2015 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f3xx_hal.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usb_device.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include "oscilloscope.h"

extern uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len);
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

uint8_t adc_data[ADC_BUFFER_LENGTH*4];
extern DMA_HandleTypeDef hdma_adc1;
extern DMA_HandleTypeDef hdma_adc3;
uint8_t data_request_flag = 0;
volatile uint8_t config_change_flag = 0;
uint16_t points_per_screen;
uint8_t trigger_found;
uint16_t trigger_point;
struct _config_struct config;
struct _config_struct temp_config;

const uint16_t sampling_parameters[] = {	
    /*   N     Res.     ADC freq.     Pts/scr   Tim.period	*/
		
		/*   0 -  500 ns - 7200000 Hz */    36,       10,
		/*   1 -    1 us - 7200000 Hz */    72,       10,
		/*   2 -    2 us - 7200000 Hz */    144,      10,
		/*   3 -    5 us - 7200000 Hz */    360,      10,
		/*   4 -   10 us - 7200000 Hz */    720,      10,
		/*   5 -   20 us - 7200000 Hz */    1440,     10,
		/*   6 -   50 us - 7200000 Hz */    3600,     10,
		/*   7 -  100 us - 4000000 Hz */    4000,     18,
		/*   8 -  200 us - 2000000 Hz */    4000,     36,
		/*   9 -  500 us -  800000 Hz */    4000,     90,
		/*  10 -    1 ms -  400000 Hz */    4000,     180,
		/*  11 -    2 ms -  200000 Hz */    4000,     360,
		/*  12 -    5 ms -   80000 Hz */    4000,     900,
		/*  13 -   10 ms -   40000 Hz */    4000,     1800,
		/*  14 -   20 ms -   20000 Hz */    4000,     3600,
		/*  15 -   50 ms -    8000 Hz */    4000,     9000,
		/*  16 -  100 ms -    2000 Hz */    2000,    36000, 
		/*  17 -  200 ms -    2000 Hz */    4000,    36000, 
	  /*  18 -  500 ms -    2000 Hz */   10000,    36000, 
		/*  19 -    1 s  -    2000 Hz */   20000,    36000, 

		/* Conversion time - 8.5 cycles for 8 bit */			
};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */


void Apply_Sampling_Parameters(void)
{

  htim1.Init.Period = sampling_parameters[config.resolution*2+1]-1;
	htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	
  HAL_TIM_Base_Init(&htim1);
	points_per_screen = sampling_parameters[config.resolution*2];

};

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */
	#define HEADER_SIZE 4
	uint8_t header[HEADER_SIZE] = {0};
	uint16_t offset;
	uint8_t triggered = 0;
	uint16_t i;

	config.channel_mask = OSCIL_CH1 | OSCIL_CH2 | OSCIL_CH3 | OSCIL_CH4;
	config.resolution = 7;
	config.trigger_channel = 1;
	config.trigger_type = TRIGGER_TYPE_RISING;
	config.trigger_level = 100;
	config.trigger_position = 6;
	
	temp_config = config;
	
  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_ADC3_Init();
  MX_ADC4_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_USB_DEVICE_Init();

  /* USER CODE BEGIN 2 */
	HAL_GPIO_WritePin(GPIOF,GPIO_PIN_9,GPIO_PIN_RESET);
	HAL_Delay(500);
	HAL_GPIO_WritePin(GPIOF,GPIO_PIN_9,GPIO_PIN_SET);
	HAL_Delay(500);
	HAL_ADCEx_MultiModeStop_DMA(&hadc1);
	HAL_ADCEx_MultiModeStop_DMA(&hadc3);
	HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_1);
	Apply_Sampling_Parameters();
  
	/* USER CODE END 2 */

  /* USER CODE BEGIN 3 */
  /* Infinite loop */
  while (1)
  {
		if (config_change_flag)
		{
			config.resolution = temp_config.resolution;
			config.trigger_position = temp_config.trigger_position;
			config.trigger_level = temp_config.trigger_level;
			config.trigger_type = temp_config.trigger_type;
			config.trigger_channel = temp_config.trigger_channel;
			Apply_Sampling_Parameters();
			config_change_flag = 0;
		};
		
		if (config.resolution <= 15) { // sampling mode 
			
			HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, GPIO_PIN_SET);

			HAL_ADCEx_MultiModeStart_DMA(&hadc1, (uint32_t*)adc_data, ADC_BUFFER_LENGTH);
			HAL_ADCEx_MultiModeStart_DMA(&hadc3, (uint32_t*)adc_data + ADC_BUFFER_LENGTH/2, ADC_BUFFER_LENGTH);			
			HAL_TIM_Base_Start(&htim1);
			HAL_DMA_PollForTransfer(&hdma_adc1,HAL_DMA_FULL_TRANSFER,1000);
			HAL_DMA_PollForTransfer(&hdma_adc3,HAL_DMA_FULL_TRANSFER,1000);	
			HAL_TIM_Base_Stop(&htim1);
			HAL_ADCEx_MultiModeStop_DMA(&hadc1);
			HAL_ADCEx_MultiModeStop_DMA(&hadc3);
			HAL_ADC_Stop(&hadc1);
			HAL_ADC_Stop(&hadc3);

			HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, GPIO_PIN_RESET);

			trigger_point = 0;
			triggered = 0;
			offset = (points_per_screen*config.trigger_position)/10*2;
			if ((config.trigger_channel == 3) || (config.trigger_channel == 4))
				offset += ADC_BUFFER_LENGTH*2;
			if ((config.trigger_channel == 2) || (config.trigger_channel == 4))
				offset += 1;

			if (config.trigger_type == TRIGGER_TYPE_RISING) {
				for (i = 0; i < (ADC_BUFFER_LENGTH-points_per_screen-2); i++)
					if ((adc_data[i*2+offset] < config.trigger_level) && (adc_data[(i+1)*2+offset] >= config.trigger_level)) {
						trigger_point = i+1;	
						triggered = 1;
					};
			} else {
				for (i = 0; i < (ADC_BUFFER_LENGTH-points_per_screen-2); i++)
					if ((adc_data[i*2+offset] >= config.trigger_level) && (adc_data[(i+1)*2+offset] < config.trigger_level)) {
						trigger_point = i+1;	
						triggered = 1;
					};
			};						
					
			if (data_request_flag) {
				header[0] = config.resolution;
				header[1] = points_per_screen>>8;
				header[2] = points_per_screen&0xFF;
				header[3] = triggered;
				CDC_Transmit_FS(header,HEADER_SIZE);
				CDC_Transmit_FS(adc_data+trigger_point*2, points_per_screen);
				CDC_Transmit_FS(adc_data+trigger_point*2+points_per_screen, points_per_screen);
				CDC_Transmit_FS(adc_data+trigger_point*2+ADC_BUFFER_LENGTH*2, points_per_screen);
				CDC_Transmit_FS(adc_data+trigger_point*2+ADC_BUFFER_LENGTH*2+points_per_screen, points_per_screen);				
				data_request_flag = 0;
			};				
				
		} else { // scan mode
			
			#define SHORT_BUFFER_LENGTH 100		
			#define SND_BUF SHORT_BUFFER_LENGTH*4
			static uint8_t current_buffer = 0;			
			HAL_ADCEx_MultiModeStart_DMA(&hadc1, (uint32_t*)adc_data + SND_BUF/4*current_buffer, SHORT_BUFFER_LENGTH);
			HAL_ADCEx_MultiModeStart_DMA(&hadc3, (uint32_t*)adc_data + SND_BUF/4*current_buffer + SHORT_BUFFER_LENGTH/2, SHORT_BUFFER_LENGTH);			
			HAL_TIM_Base_Start(&htim1);
			HAL_DMA_PollForTransfer(&hdma_adc1,HAL_DMA_FULL_TRANSFER,1000);
			HAL_DMA_PollForTransfer(&hdma_adc3,HAL_DMA_FULL_TRANSFER,1000);	
			HAL_TIM_Base_Stop(&htim1);
			HAL_ADCEx_MultiModeStop_DMA(&hadc1);
			HAL_ADCEx_MultiModeStop_DMA(&hadc3);
			HAL_ADC_Stop(&hadc1);
			HAL_ADC_Stop(&hadc3);

			if (data_request_flag) {
				HAL_GPIO_TogglePin(GPIOE,GPIO_PIN_15);
				header[0] = config.resolution;
				header[1] = SHORT_BUFFER_LENGTH>>8;
				header[2] = SHORT_BUFFER_LENGTH&0xFF;
				header[3] = triggered;
				CDC_Transmit_FS(header,HEADER_SIZE);
				// fix wrong first value
				*(adc_data+SND_BUF*current_buffer + 0) = *(adc_data+SND_BUF*current_buffer + 2);
				*(adc_data+SND_BUF*current_buffer + 1) = *(adc_data+SND_BUF*current_buffer + 3);
				*(adc_data+SND_BUF*current_buffer + SHORT_BUFFER_LENGTH*2 + 0) = *(adc_data+SND_BUF*current_buffer + SHORT_BUFFER_LENGTH*2 + 2);
				*(adc_data+SND_BUF*current_buffer + SHORT_BUFFER_LENGTH*2 + 1) = *(adc_data+SND_BUF*current_buffer + SHORT_BUFFER_LENGTH*2 + 3);
				CDC_Transmit_FS(adc_data + SND_BUF*current_buffer, SHORT_BUFFER_LENGTH*4);	
				data_request_flag = 0;
			};
			current_buffer ^= 1;	
		};
  }
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB|RCC_PERIPHCLK_TIM1;
  PeriphClkInit.USBClockSelection = RCC_USBPLLCLK_DIV1_5;
  PeriphClkInit.Tim1ClockSelection = RCC_TIM1CLK_HCLK;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

  __SYSCFG_CLK_ENABLE();

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
