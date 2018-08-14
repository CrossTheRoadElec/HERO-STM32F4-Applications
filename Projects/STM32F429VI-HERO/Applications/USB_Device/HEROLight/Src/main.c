/*

The MIT License (MIT)

Copyright (c) 2016 Hubert Denkmair

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

/* Includes ------------------------------------------------------------------*/
#include "main.h"

#include <stdlib.h>
#include <string.h>

//#include "config.h"
#define CAN_QUEUE_SIZE 4

#include "stm32f4xx_hal.h"
#include "usbd_def.h"
#include "usbd_desc.h"
#include "usbd_core.h"
#include "usbd_gs_can.h"
#include "gpio.h"
#include "queue.h"
#include "gs_usb.h"
#include "can.h"
#include "led.h"
#include "dfu.h"
#include "timer.h"
//#include "flash.h"

void SystemClock_Config(void);
static bool send_to_host_or_enqueue(struct gs_host_frame *frame);
static void send_to_host(void);

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  while(1)
  {
  }
}

can_data_t hCAN;
USBD_HandleTypeDef hUSB;
led_data_t hLED;

queue_t *q_frame_pool;
queue_t *q_from_host;
queue_t *q_to_host;

uint32_t received_count=0;

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
	uint32_t last_can_error_status = 0;
	
	/* STM32F429xx HAL library initialization */
	HAL_Init();
	
	/* Configure the System clock to have a frequency of 168 MHz */
	SystemClock_Config();

	/* Configure LED1, LED2 and LED3 */
	BSP_LED_Init(LED1);
	BSP_LED_Init(LED2);
	BSP_LED_Init(LED3);

	BSP_LED_On(LED1);
	BSP_LED_On(LED2);
	BSP_LED_On(LED3);
	
	
	can_init(&hCAN, CAN1);
	can_disable(&hCAN);

	led_init(&hLED,
					LED1_GPIO_PORT, LED1_PIN, false, 
					LED2_GPIO_PORT, LED2_PIN, false);
	led_set_mode(&hLED, led_mode_off);

	timer_init();


	q_frame_pool = queue_create(CAN_QUEUE_SIZE);
	q_from_host  = queue_create(CAN_QUEUE_SIZE);
	q_to_host    = queue_create(CAN_QUEUE_SIZE);

	struct gs_host_frame *msgbuf = calloc(CAN_QUEUE_SIZE, sizeof(struct gs_host_frame));
	for (unsigned i=0; i<CAN_QUEUE_SIZE; i++) {
		queue_push_back(q_frame_pool, &msgbuf[i]);
	}

	USBD_Init(&hUSB, &FS_Desc, 0);
	USBD_RegisterClass(&hUSB, &USBD_GS_CAN);
	USBD_GS_CAN_Init(&hUSB, q_frame_pool, q_from_host, &hLED);
	USBD_GS_CAN_SetChannel(&hUSB, 0, &hCAN);
	USBD_Start(&hUSB);

#ifdef CAN_S_GPIO_Port
	HAL_GPIO_WritePin(CAN_S_GPIO_Port, CAN_S_Pin, GPIO_PIN_RESET);
#endif
    		
	while (1) {
		
		/* additional loop blink */
		static long time0_us = 0;
		long time1_us = timer_get();
		long time_dur = time1_us - time0_us;
		if (time_dur < 0 || time_dur > 500e3) {
			BSP_LED_Toggle(LED3);
			time0_us = time1_us;
		}
		
		struct gs_host_frame *frame = queue_pop_front(q_from_host);
		if (frame != 0) { // send can message from host
			if (can_send(&hCAN, frame)) {
			        // Echo sent frame back to host
			        frame->timestamp_us = timer_get();
				send_to_host_or_enqueue(frame);
				
				led_indicate_trx(&hLED, led_2);
			} else {
				queue_push_front(q_from_host, frame); // retry later
			}
		}

		if (USBD_GS_CAN_TxReady(&hUSB)) {
			send_to_host();
		}

		if (can_is_rx_pending(&hCAN)) {
			struct gs_host_frame *frame = queue_pop_front(q_frame_pool);
			if ((frame != 0) && can_receive(&hCAN, frame)) {
             			received_count++;

				frame->timestamp_us = timer_get();
				frame->echo_id = 0xFFFFFFFF; // not a echo frame
				frame->channel = 0;
				frame->flags = 0;
				frame->reserved = 0;
				
				send_to_host_or_enqueue(frame);

				led_indicate_trx(&hLED, led_1);

			} else {
				queue_push_back(q_frame_pool, frame);
			}

		}

		uint32_t can_err = can_get_error_status(&hCAN);
		if (can_err != last_can_error_status) {
			struct gs_host_frame *frame = queue_pop_front(q_frame_pool);
			if (frame != 0) {
				frame->timestamp_us = timer_get();
				if (can_parse_error_status(can_err, frame)) {
					send_to_host_or_enqueue(frame);
					last_can_error_status = can_err;
				} else {
					queue_push_back(q_frame_pool, frame);
				}

			}
		}

		led_update(&hLED);

		if (USBD_GS_CAN_DfuDetachRequested(&hUSB)) {
			dfu_run_bootloader();
		}

	}
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 168000000
  *            HCLK(Hz)                       = 168000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 8
  *            PLL_N                          = 336
  *            PLL_P                          = 2
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }
}

bool send_to_host_or_enqueue(struct gs_host_frame *frame)
{
	if (USBD_GS_CAN_GetProtocolVersion(&hUSB) == 2) {
		queue_push_back(q_to_host, frame);
		return true;

	} else {
		bool retval = false;
		if ( USBD_GS_CAN_SendFrame(&hUSB, frame) == USBD_OK ) {
			queue_push_back(q_frame_pool, frame);
			retval = true;
		} else {
			queue_push_back(q_to_host, frame);
		}
		return retval;
	}
}

void send_to_host(void)
{
        struct gs_host_frame *frame = queue_pop_front(q_to_host);

	if(!frame)
	  return;
	
	if (USBD_GS_CAN_SendFrame(&hUSB, frame) == USBD_OK) {
	        queue_push_back(q_frame_pool, frame);
	} else {
	        queue_push_front(q_to_host, frame);
	}
}


/**
  * @brief This function provides accurate delay (in milliseconds) based 
  *        on SysTick counter flag.
  * @note This function is declared as __weak to be overwritten in case of other
  *       implementations in user file.
  * @param Delay: specifies the delay time length, in milliseconds.
  * @retval None
  */

void HAL_Delay(__IO uint32_t Delay)
{
  while(Delay) 
  {
    if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) 
    {
      Delay--;
    }
  }
}


#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
