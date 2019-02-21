/***************************************************************************//**
 *   @file   AD7705.h
 *   @brief  Header file of AD7705 Driver.
 *   @author Bancisor Mihai
********************************************************************************
 * Copyright 2012(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
********************************************************************************
 *   SVN Revision: 394
*******************************************************************************/
#ifndef __AD7705_H
#define __AD7705_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx_hal.h"
#include "main.h"
/******************************************************************************/
/* AD7705  SPI                                                                 */
/******************************************************************************/
#ifndef __SPI_1
#define __SPI_1    1     /* SPI1 */
#endif  /* __SPI_1 */
#ifndef __SPI_2
#define __SPI_2    2     /* SPI2 */
#endif  /* __SPI_2 */
#define AD7705_SPI_SELECTION     __SPI_1

/*###################### SPIx ###################################*/  
   
#if (AD7705_SPI_SELECTION==__SPI_1)     /* SPI1 */
   
#define AD7705_SPIx                                 SPI1  
#define AD7705_SPIx_CLK_ENABLE()                    __HAL_RCC_SPI1_CLK_ENABLE()

#define AD7705_SPIx_SCK_AF                          GPIO_AF5_SPI1
#define AD7705_SPIx_SCK_GPIO_PORT                   GPIOA
#define AD7705_SPIx_SCK_PIN                         GPIO_PIN_5
#define AD7705_SPIx_SCK_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOA_CLK_ENABLE()
#define AD7705_SPIx_SCK_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOA_CLK_DISABLE()

#define AD7705_SPIx_MISO_MOSI_AF                    GPIO_AF5_SPI1
#define AD7705_SPIx_MISO_MOSI_GPIO_PORT             GPIOA
#define AD7705_SPIx_MISO_MOSI_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOA_CLK_ENABLE()
#define AD7705_SPIx_MISO_MOSI_GPIO_CLK_DISABLE()    __HAL_RCC_GPIOA_CLK_DISABLE()
#define AD7705_SPIx_MISO_PIN                        GPIO_PIN_6
#define AD7705_SPIx_MOSI_PIN                        GPIO_PIN_7
   
#elif (AD7705_SPI_SELECTION==__SPI_2)  /* SPI2 */
   
#define AD7705_SPIx                                 SPI2  
#define AD7705_SPIx_CLK_ENABLE()                    __HAL_RCC_SPI2_CLK_ENABLE()

#define AD7705_SPIx_SCK_AF                          GPIO_AF5_SPI2
#define AD7705_SPIx_SCK_GPIO_PORT                   GPIOB
#define AD7705_SPIx_SCK_PIN                         GPIO_PIN_13
#define AD7705_SPIx_SCK_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOB_CLK_ENABLE()
#define AD7705_SPIx_SCK_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOB_CLK_DISABLE()

#define AD7705_SPIx_MISO_MOSI_AF                    GPIO_AF5_SPI2
#define AD7705_SPIx_MISO_MOSI_GPIO_PORT             GPIOB
#define AD7705_SPIx_MISO_MOSI_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOB_CLK_ENABLE()
#define AD7705_SPIx_MISO_MOSI_GPIO_CLK_DISABLE()    __HAL_RCC_GPIOB_CLK_DISABLE()
#define AD7705_SPIx_MISO_PIN                        GPIO_PIN_14
#define AD7705_SPIx_MOSI_PIN                        GPIO_PIN_15
   
#else
#error "must select the spi used to control AD7705"
#endif  /* AD7705_SPIx */

/* Maximum Timeout values for flags waiting loops. These timeouts are not based
   on accurate values, they just guarantee that the application will not remain
   stuck if the SPI communication is corrupted.
   You may modify these timeout values depending on CPU frequency and application
   conditions (interrupts routines ...). */   
#define AD7705_SPIx_TIMEOUT_MAX                   1000

/**
  * @brief  AD7705 Wait ms 
  */
#ifndef osCMSIS
#define AD7705_WAIT_MS(x)  HAL_Delay(x)
#else  /* use rtos */
#define AD7705_WAIT_MS(x)  osDelay(x)
#endif

/******************************************************************************/
/* AD7705                                                                   */
/******************************************************************************/

/*AD7705控制引脚IO*/
#define AD7705_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOA_CLK_ENABLE()
#define AD7705_GPIO_PORT          GPIOA
#define AD770_RDY                 GPIO_PIN_2
#define AD7705_RST                GPIO_PIN_3

/*AD7705 Registers*/
#define AD7705_REG_COMM	    0 /* Communications Register(WO, 8-bit) */
#define AD7705_REG_CONF	    1 /* 设置寄存器Configuration Register (RW, 8-bit)*/
#define AD7705_REG_CLOCK    2 /* Clock Register (RW, 8-bit)*/
#define AD7705_REG_DATA	    3 /* Data Register	     	(RO, 16-bit) */
#define AD7705_REG_TEST	    4 /* TEST Register	     	(RO, 8-bit) */
#define AD7705_REG_NONE	    5 /* UNUSED 	     	            */
#define AD7705_REG_OFFSET   6 /* Offset Register	    (RW, 24-bit */
#define AD7705_REG_GAIN	    7 /* GAIN Register	(RW, 24-bit */



/* 通讯寄存器Communications Register Bit Designations (AD7705_REG_COMM) */
#define AD7705_COMM_WEN		(1 << 7) 			/* Write Enable */
#define AD7705_COMM_RDY		(0 << 7)                        /* Ready */
#define AD7705_COMM_WRITE	(0 << 3) 			/* Write Operation */
#define AD7705_COMM_READ        (1 << 3) 			/* Read Operation */
#define AD7705_COMM_ADDR(x)	(((x) & 0x7) << 4)	        /* Register Address */
#define AD7705_COMM_STBY	(1 << 2) 			/* Standby Mode 等待掉电模式*/
#define AD7705_COMM_CHAN(x)	((x) & 0x3) 			/* Channel select */

/* AD7705_CONF_CHAN(x) options */
#define AD7705_CH_AIN1P_AIN1M	0 /* AIN1(+) - AIN1(-) */
#define AD7705_CH_AIN2P_AIN2M	1 /* AIN2(+) - AIN2(-) */
#define AD7705_CH_AIN1M_AIN1M	2 /* AIN1(-) - AIN1(-) */
#define AD7705_CH_AIN1M_AIN2M	3 /* AIN1(-) - AIN2(-) */



/* Clock Register Bit Designations (AD7705_REG_CLOCK) */
#define AD7705_CLK_CLKDIS	(1 << 4)    /* Master Clock Disable */
#define AD7705_CLK_CLKDIV	(1 << 3)    /* Clock Divider */
#define AD7705_CLK_RATE(x)	((x) & 0x7) /* Update Rate Selection */


/* AD7705_CLK_RATE(x) options */
#define AD7705_RATE_20HZ         0  /* 20HZ */
#define AD7705_RATE_25HZ         1  /*  25HZ */
#define AD7705_RATE_100HZ        2  /*  100HZ */
#define AD7705_RATE_200HZ        3  /*  200HZ */
#define AD7705_RATE_50HZ         4  /*  50HZ */
#define AD7705_RATE_60HZ         5  /*  60HZ */
#define AD7705_RATE_250HZ        6  /*  250HZ */
#define AD7705_RATE_500HZ        7  /*  500HZ */




/* Configuration Register Bit Designations (AD7705_REG_CONF) */
#define AD7705_CONF_UNIPOLAR      (1 << 2) 			/* Unipolar/Bipolar Enable */
#define AD7705_CONF_GAIN(x)	  (((x) & 0x7) << 3) 	        /* Gain Select */
#define AD7705_CONF_MODE(x)	  (((x) & 0x3) << 6) 	        /* Mode Select */
#define AD7705_CONF_BUF		  (1 << 1) 			/* Buffered Mode Enable */
#define AD7705_CONF_FSYNC	  (0 << 0) 			/* Filter Synchronization Enable */

/* AD7705_CONF_MODE(x) options 设置寄存器的工作模式*/
#define AD7705_MODE_NORMAL		 0 /* Normal Mode 正常模式*/
#define AD7705_MODE_CAL_SELF		 1 /* Self-Calibration Mode 自校准模式*/
#define AD7705_MODE_CAL_SYS_ZERO         2 /* System Zero-Scale Calibration 零标度系统校准*/
#define AD7705_MODE_CAL_SYS_FULL         3 /* System Full-Scale Calibration 满标度系统校准*/

/* AD7705_CONF_GAIN(x) options 设置寄存器的增益选择*/
#define AD7705_GAIN_1       0
#define AD7705_GAIN_2       1
#define AD7705_GAIN_4       2
#define AD7705_GAIN_8       3
#define AD7705_GAIN_16      4
#define AD7705_GAIN_32      5
#define AD7705_GAIN_64      6
#define AD7705_GAIN_128     7

/* AD7705_CONF_UNIPOLAR options 设置寄存器的工作极性选择*/
#define AD7705_UNIPOLAR   1	/* Unipolar. 单极性*/
#define AD7705_BIPOLAR    0	/* Bipolar. 双极性*/

/* AD7705_CONF_BUF options 设置寄存器的缓冲器控制*/
#define AD7705_BUF_ENABLE     1	/* Enable Buffer. 缓冲器与模拟输入串联，输入端允许处理高阻抗原，间接测量电压，直接测量电阻选择这个*/
#define AD7705_BUF_DISABLE    0	/* Disable Buffer. AD内部缓冲器短路，直接测量电压值选择这个*/





/******************************************************************************/
/* Functions Prototypes                                                       */
/******************************************************************************/
/* Initializes the SPI communication peripheral. */
void AD7705_SPI_Init(void);

/* Writes data to SPI. */
unsigned char AD7705_SPI_Write(unsigned char* data,
                        unsigned char bytesNumber);

/* Reads data from SPI. */
unsigned char AD7705_SPI_Read(unsigned char* data,
                       unsigned char bytesNumber);


/* Initialize AD7705 and check if the device is present*/
unsigned char AD7705_Init(void);
/* Sends 32 consecutive 1's on SPI in order to reset the part. */
void AD7705_Reset(void);
/* Reads the value of the selected register. */
unsigned long AD7705_GetRegisterValue(unsigned char regAddress, 
				      unsigned char size,
                                      unsigned char channel);
/* Writes a value to the register. */
void AD7705_SetRegisterValue(unsigned char regAddress,
			     unsigned long regValue, 
			     unsigned char size,
                             unsigned char channel);
/* Reads /RDY bit of Status register. */
unsigned char AD7705_Ready(unsigned char channel);              
/* Waits for RDY . */
void AD7705_WaitRdy(unsigned char channel);

/* AD7705 Single Measurement */
unsigned long AD7705_SingleMeasurement(unsigned long AD7705Channel,unsigned long gain,unsigned long buffer);
#ifdef __cplusplus
}
#endif
#endif /*__ AD7705_H */
