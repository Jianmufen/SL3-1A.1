/**
  ******************************************************************************
  * @file    sd_io_low.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    31-March-2015
  * @brief   This file provides set of firmware functions to manage:
  *          - sd spi low level io init to use \Drivers\BSP\Adafruit_Shield\'s sd driver
  *          - microSD available on Adafruit 1.8" TFT LCD 
  *            shield (reference ID 802)
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
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
#include "sd_io_low.h"

/** @addtogroup BSP
  * @{
  */ 


/**
  * @brief LINK SD Card
  */
#define SD_DUMMY_BYTE            0xFF    
#define SD_NO_RESPONSE_EXPECTED  0x80
   
/**
  * @}
  */ 


/**
 * @brief BUS variables
 */

#ifdef HAL_SPI_MODULE_ENABLED
uint32_t SpixTimeout = SD_SPIx_TIMEOUT_MAX;        /*<! Value of Timeout when SPI communication fails */
static SPI_HandleTypeDef hsd_Spi;
#endif /* HAL_SPI_MODULE_ENABLED */



/** @defgroup STM32L1XX_SD_Private_Functions Private Functions
  * @{
  */ 
#ifdef HAL_SPI_MODULE_ENABLED
static void               SPIx_Init(void);
static void               SPIx_Write(uint8_t Value);
static uint32_t           SPIx_Read(void);
static void               SPIx_Error (void);
static void               SPIx_MspInit(void);
#endif /* HAL_SPI_MODULE_ENABLED */


#ifdef HAL_SPI_MODULE_ENABLED
/* SD IO functions */
void                      SD_IO_Init(void);
HAL_StatusTypeDef         SD_IO_WriteCmd(uint8_t Cmd, uint32_t Arg, uint8_t Crc, uint8_t Response);
HAL_StatusTypeDef         SD_IO_WaitResponse(uint8_t Response);
void                      SD_IO_WriteDummy(void);
void                      SD_IO_WriteByte(uint8_t Data);
uint8_t                   SD_IO_ReadByte(void);
#endif /* HAL_SPI_MODULE_ENABLED */
/**
  * @}
  */ 



#ifdef HAL_SPI_MODULE_ENABLED
/******************************************************************************
                            BUS OPERATIONS
*******************************************************************************/
/**
  * @brief  Initializes SPI MSP.
  * @retval None
  */
static void SPIx_MspInit(void)
{
  GPIO_InitTypeDef  gpioinitstruct = {0};
  
  /*** Configure the GPIOs ***/  
  /* Enable GPIO clock */
  SD_SPIx_SCK_GPIO_CLK_ENABLE();
  SD_SPIx_MISO_MOSI_GPIO_CLK_ENABLE();

  /* Configure SPI SCK */
  gpioinitstruct.Pin        = SD_SPIx_SCK_PIN;
  gpioinitstruct.Mode       = GPIO_MODE_AF_PP;
  gpioinitstruct.Pull       = GPIO_PULLUP;
  gpioinitstruct.Speed      = GPIO_SPEED_HIGH;
  gpioinitstruct.Alternate  = SD_SPIx_SCK_AF;
  HAL_GPIO_Init(SD_SPIx_SCK_GPIO_PORT, &gpioinitstruct);

  /* Configure SPI MISO and MOSI */ 
  gpioinitstruct.Pin        = SD_SPIx_MOSI_PIN;
  gpioinitstruct.Alternate  = SD_SPIx_MISO_MOSI_AF;
  gpioinitstruct.Pull       = GPIO_PULLDOWN;
  HAL_GPIO_Init(SD_SPIx_MISO_MOSI_GPIO_PORT, &gpioinitstruct);
  
  gpioinitstruct.Pin        = SD_SPIx_MISO_PIN;
  HAL_GPIO_Init(SD_SPIx_MISO_MOSI_GPIO_PORT, &gpioinitstruct);

  /*** Configure the SPI peripheral ***/ 
  /* Enable SPI clock */
  SD_SPIx_CLK_ENABLE();
}

/**
  * @brief  Initializes SPI HAL.
  * @retval None
  */
static void SPIx_Init(void)
{
  if(HAL_SPI_GetState(&hsd_Spi) == HAL_SPI_STATE_RESET)
  {
    /* SPI Config */
    hsd_Spi.Instance = SD_SPIx;//SPI1
      /* SPI baudrate is set to 8 MHz maximum (PCLK2/SPI_BaudRatePrescaler = 32/4 = 8 MHz) 
       to verify these constraints:
          - ST7735 LCD SPI interface max baudrate is 15MHz for write and 6.66MHz for read
            Since the provided driver doesn't use read capability from LCD, only constraint 
            on write baudrate is considered.
          - SD card SPI interface max baudrate is 25MHz for write/read
          - PCLK2 max frequency is 32 MHz 
       */
    //hsd_Spi.Init.BaudRatePrescaler  = SPI_BAUDRATEPRESCALER_4;
    hsd_Spi.Init.BaudRatePrescaler  = SPI_BAUDRATEPRESCALER_128;//定义波特率预分频的值:波特率预分频值为128
    hsd_Spi.Init.Direction          = SPI_DIRECTION_2LINES; //设置SPI单向或者双向的数据模式:SPI设置为双线模式
    hsd_Spi.Init.CLKPhase           = SPI_PHASE_2EDGE;//串行同步时钟的第二个跳变沿（上升或下降）数据被采样
    hsd_Spi.Init.CLKPolarity        = SPI_POLARITY_HIGH;//串行同步时钟的空闲状态为高电平
    hsd_Spi.Init.CRCCalculation     = SPI_CRCCALCULATION_DISABLE;//关闭硬件CRC校验
    hsd_Spi.Init.CRCPolynomial      = 7;//CRC值计算的多项式
    hsd_Spi.Init.DataSize           = SPI_DATASIZE_8BIT;//设置SPI的数据大小:SPI发送接收8位帧结构
    hsd_Spi.Init.FirstBit           = SPI_FIRSTBIT_MSB; //指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
    hsd_Spi.Init.NSS                = SPI_NSS_SOFT;//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
    hsd_Spi.Init.TIMode             = SPI_TIMODE_DISABLE;//关闭TI模式
    hsd_Spi.Init.Mode               = SPI_MODE_MASTER;//设置SPI工作模式，设置为主模式
    
    SPIx_MspInit();
    HAL_SPI_Init(&hsd_Spi);
  }
}

/**
  * @brief  SPI Read 4 bytes from device
  * @retval Read data
*/
static uint32_t SPIx_Read(void)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t readvalue = 0;
  uint32_t writevalue = 0xFFFFFFFF;
  
  status = HAL_SPI_TransmitReceive(&hsd_Spi, (uint8_t*) &writevalue, (uint8_t*) &readvalue, 1, SpixTimeout);
  
  /* Check the communication status */
  if(status != HAL_OK)
  {
    /* Execute user timeout callback */
    SPIx_Error();
  }

  return readvalue;
}

/**
  * @brief  SPI Write a byte to device
  * @param  Value: value to be written
  * @retval None
  */
static void SPIx_Write(uint8_t Value)
{
  HAL_StatusTypeDef status = HAL_OK;

  status = HAL_SPI_Transmit(&hsd_Spi, (uint8_t*) &Value, 1, SpixTimeout);

  /* Check the communication status */
  if(status != HAL_OK)
  {
    /* Execute user timeout callback */
    SPIx_Error();
  }
}

/**
  * @brief  SPI error treatment function
  * @retval None
  */
static void SPIx_Error (void)
{
  /* De-initialize the SPI communication BUS */
  HAL_SPI_DeInit(&hsd_Spi);

  /* Re-Initiaize the SPI communication BUS */
  SPIx_Init();
}




/******************************************************************************
                            LINK OPERATIONS
*******************************************************************************/

/********************************* LINK SD ************************************/
/**
  * @brief  Initializes the SD Card and put it into StandBy State (Ready for 
  *         data transfer).
  * @retval None
  */
void SD_IO_Init(void)
{
  GPIO_InitTypeDef  gpioinitstruct = {0};
  uint8_t counter = 0;

  /* SD_CS_GPIO Periph clock enable */
  SD_CS_GPIO_CLK_ENABLE();

  /* Configure SD_CS_PIN pin: SD Card CS pin */
  gpioinitstruct.Pin    = SD_CS_PIN;
  gpioinitstruct.Mode   = GPIO_MODE_OUTPUT_PP;
  gpioinitstruct.Pull   = GPIO_PULLUP;
  gpioinitstruct.Speed  = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(SD_CS_GPIO_PORT, &gpioinitstruct);

  /*------------Put SD in SPI mode--------------*/
  /* SD SPI Config */
  SPIx_Init();

  /* SD chip select high */
  SD_CS_HIGH();
  
  /* SD chip select low */
//  SD_CS_LOW();
  
  /* Send dummy byte 0xFF, 10 times with CS high */
  /* Rise CS and MOSI for 80 clocks cycles */
  for (counter = 0; counter <= 9; counter++)
  {
    /* Send dummy byte 0xFF */
    SD_IO_WriteByte(SD_DUMMY_BYTE);	 //启动传输
  }
}

/**
  * @brief  Writes a byte on the SD.
  * @param  Data: byte to send.
  * @retval None
  */
void SD_IO_WriteByte(uint8_t Data)
{
  /* Send the byte */
  SPIx_Write(Data);
}
 
/**
  * @brief  Reads a byte from the SD.
  * @retval The received byte.
  */
uint8_t SD_IO_ReadByte(void)
{
			uint8_t data = 0;
			
			/* Get the received data */
			data = SPIx_Read();

//			printf("从SD卡读出来的data= %d\r\n", data);
			/* Return the shifted data */
			return data;
}

/**
  * @brief  Sends 5 bytes command to the SD card and get response
  * @param  Cmd: The user expected command to send to SD card.
  * @param  Arg: The command argument.
  * @param  Crc: The CRC.
  * @param  Response: Expected response from the SD card
  * @retval HAL_StatusTypeDef HAL Status
  */
HAL_StatusTypeDef SD_IO_WriteCmd(uint8_t Cmd, uint32_t Arg, uint8_t Crc, uint8_t Response)
{
			uint32_t counter = 0x00;
			uint8_t frame[6] = {0};
//			printf("5Response = %d\r\n", Response);
			/* Prepare Frame to send */
			frame[0] = (Cmd | 0x40);         /* Construct byte 1 */
			frame[1] = (uint8_t)(Arg >> 24); /* Construct byte 2 */
			frame[2] = (uint8_t)(Arg >> 16); /* Construct byte 3 */
			frame[3] = (uint8_t)(Arg >> 8);  /* Construct byte 4 */
			frame[4] = (uint8_t)(Arg);       /* Construct byte 5 */
			frame[5] = (Crc);                /* Construct byte 6 */
			
			/* SD chip select low */
			SD_CS_LOW();
//			printf("6Response = %d\r\n", Response);
			/* Send Frame */
			for (counter = 0; counter < 6; counter++)
			{
					SD_IO_WriteByte(frame[counter]); /* Send the Cmd bytes */
			}

//			printf("1Response = %d\r\n", Response);
//			if(Response != SD_NO_RESPONSE_EXPECTED)			/*SD_NO_RESPONSE_EXPECTED = 0x80*/
//			{
//					printf("2Response = %d\r\n", Response);
//					return SD_IO_WaitResponse(Response);
//			}  
			
			return HAL_OK;
}

/**
  * @brief  Waits response from the SD card
  * @param  Response: Expected response from the SD card
  * @retval HAL_StatusTypeDef HAL Status
  */
HAL_StatusTypeDef SD_IO_WaitResponse(uint8_t Response)
{
  uint32_t timeout = 0xFFFF;

  /* Check if response is got or a timeout is happen */
  while ((SD_IO_ReadByte() != Response) && timeout)
  {
    timeout--;
  }

  if (timeout == 0)
  {
//			printf("wait sd Response timeout\r\n");
			/* After time out */
			return HAL_TIMEOUT;
  }   
  else
  {
    /* Right response got */
    return HAL_OK;
  }
  }  
 
/**
  * @brief  Sends dummy byte with CS High
  * @retval None
  */
void SD_IO_WriteDummy(void)
{
  /* SD chip select high */
  SD_CS_HIGH();
  
  /* Send Dummy byte 0xFF */
  SD_IO_WriteByte(SD_DUMMY_BYTE);
}

void SD_SPI_SpeedLow(void)//设置到低速模式 
{
    __HAL_SPI_DISABLE(&hsd_Spi);            //关闭SPI
    hsd_Spi.Instance->CR1&=0XFFC7;          //位3-5清零，用来设置波特率
    hsd_Spi.Instance->CR1|=SPI_BAUDRATEPRESCALER_128;//设置SPI速度
    __HAL_SPI_ENABLE(&hsd_Spi);             //使能SPI
}

void SD_SPI_SpeedHigh(void)//设置到高速模式 
{
    __HAL_SPI_DISABLE(&hsd_Spi);            //关闭SPI
    hsd_Spi.Instance->CR1&=0XFFC7;          //位3-5清零，用来设置波特率
    hsd_Spi.Instance->CR1|=SPI_BAUDRATEPRESCALER_2;//设置SPI速度
    __HAL_SPI_ENABLE(&hsd_Spi);             //使能SPI
}

//SPI1 读写一个字节
//TxData:要写入的字节
//返回值:读取到的字节
uint8_t SPI1_ReadWriteByte(uint8_t TxData)
{
    uint8_t Rxdata;
    HAL_SPI_TransmitReceive(&hsd_Spi, &TxData, &Rxdata, 1, 1000);       
 	return Rxdata;          		    //返回收到的数据		
}




#endif /* HAL_SPI_MODULE_ENABLED */



