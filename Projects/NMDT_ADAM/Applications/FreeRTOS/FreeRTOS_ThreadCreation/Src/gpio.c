/**
  ******************************************************************************
  * File Name          : gpio.c
  * Description        : This file provides code for the configuration
  *                      of all used GPIO pins.
  ******************************************************************************
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
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
#include "gpio.h"
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

			GPIO_InitTypeDef GPIO_InitStruct;

			/* GPIO Ports Clock Enable */
			__HAL_RCC_GPIOB_CLK_ENABLE();
			__HAL_RCC_GPIOH_CLK_ENABLE();
			__HAL_RCC_GPIOC_CLK_ENABLE();
			__HAL_RCC_GPIOA_CLK_ENABLE();
			__HAL_RCC_GPIOD_CLK_ENABLE();
			
			/*Configure GPIO pin : PB1、PB2、PB3、PB4、PB5 、PB6、PB7、PB8、PB9、PB10、PB11、PB14（背光）显示屏引脚初始化为开漏输出*/
			GPIO_InitStruct.Pin 	= GPIO_PIN_1 | GPIO_PIN_2 |GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 |  GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_14;
			GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
			GPIO_InitStruct.Pull  = GPIO_NOPULL;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
			HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
			 /*Configure GPIO pin Output Level 初始化显示屏引脚为高电平*/
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1 | GPIO_PIN_2 |GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 |  GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_11 | GPIO_PIN_14, GPIO_PIN_SET);

			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_RESET);     /*初始化显示屏的E为低电平*/
			
			/*Configure GPIO pin : PC0、PC1、PC2、PC3、PC4、PC5 、PC6风向引脚*/
			GPIO_InitStruct.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 |GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6;
			GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
			GPIO_InitStruct.Pull  = GPIO_PULLUP;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
			HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
			
			/*Configure GPIO pin : PC7  风向转换器控制引脚 */
			GPIO_InitStruct.Pin   = GPIO_PIN_7;
			GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
			GPIO_InitStruct.Pull  = GPIO_NOPULL;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
			HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
			
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_7,GPIO_PIN_RESET);     /*初始化PC7为低电平 风向转换开*/

			/*Configure GPIO pin : PD2  蜂鸣器引脚 */
			GPIO_InitStruct.Pin   = GPIO_PIN_2;
			GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
			GPIO_InitStruct.Pull  = GPIO_NOPULL;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
			HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
			
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);     /*初始化PD2为低电平 蜂鸣器不响*/
			
			/*Configure GPIO pin : PA0  PCF8563控制引脚 */
			GPIO_InitStruct.Pin   = GPIO_PIN_0;
			GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
			GPIO_InitStruct.Pull  = GPIO_NOPULL;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
			HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
			
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);     /*初始化PA0为高电平 PCF8563工作*/
			
			/*Configure GPIO pins : PB12 PB13 初始化PCF8563  IIC的模拟引脚PB10/PB11*/
			GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13;
			GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
			GPIO_InitStruct.Pull = GPIO_NOPULL;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
			HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
			
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12 | GPIO_PIN_13, GPIO_PIN_SET);
			
			/*Configure GPIO pin : PA1  485控制引脚 */
			GPIO_InitStruct.Pin   = GPIO_PIN_1;
			GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
			GPIO_InitStruct.Pull  = GPIO_NOPULL;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
			HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
			
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);     /*初始化PA1为低电平 485工作在接收模式*/
			
			
			/*Configure GPIO pin : PB15  LED3引脚, PB0  LED2引脚，雨量报警指示灯,PB14显示屏背光灯控制引脚*/
			GPIO_InitStruct.Pin   = GPIO_PIN_15 | GPIO_PIN_0 | GPIO_PIN_14;
			GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
			GPIO_InitStruct.Pull  = GPIO_NOPULL;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
			HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
			
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);     /*初始化PB0为高电平 LED2不亮*/
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_15,GPIO_PIN_SET);     /*初始化PB15为高电平 LED3不亮*/
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_SET);     /*初始化PB14为高电平 背光灯亮*/
			
			 /*Configure GPIO pin :PC8、PC9、PC10、PC11、PC12、PC13按键引脚 */
			GPIO_InitStruct.Pin   = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 ;
			GPIO_InitStruct.Mode  = GPIO_MODE_IT_FALLING;
			GPIO_InitStruct.Pull  = GPIO_PULLUP;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
			HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
			
			 /*Configure GPIO pin : PA7  雨量引脚 */
			GPIO_InitStruct.Pin   = GPIO_PIN_7;
			GPIO_InitStruct.Mode  = GPIO_MODE_IT_FALLING;
			GPIO_InitStruct.Pull  = GPIO_PULLUP;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
			HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
			
			HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0x0A, 0);
			HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
			
			HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0x0A, 0);
			HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
