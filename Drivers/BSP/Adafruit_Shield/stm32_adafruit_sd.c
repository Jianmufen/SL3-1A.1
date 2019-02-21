/**
  ******************************************************************************
  * @file    stm32_adafruit_sd.c
  * @author  MCD Application Team
  * @version V1.1.1
  * @date    21-November-2014
  * @brief   This file provides a set of functions needed to manage the SD card
  *          mounted on the Adafruit 1.8" TFT LCD shield (reference ID 802),
  *          that is used with the STM32 Nucleo board through SPI interface.
  *          It implements a high level communication layer for read and write 
  *          from/to this memory. The needed STM32XXxx hardware resources (SPI and 
  *          GPIO) are defined in stm32XXxx_nucleo.h file, and the initialization is 
  *          performed in SD_IO_Init() function declared in stm32XXxx_nucleo.c 
  *          file.
  *          You can easily tailor this driver to any other development board, 
  *          by just adapting the defines for hardware resources and 
  *          SD_IO_Init() function.
  *            
  *          +-------------------------------------------------------+
  *          |                     Pin assignment                    |
  *          +-------------------------+---------------+-------------+
  *          |  STM32XXxx SPI Pins     |     SD        |    Pin      |
  *          +-------------------------+---------------+-------------+
  *          | SD_SPI_CS_PIN           |   ChipSelect  |    1        |
  *          | SD_SPI_MOSI_PIN / MOSI  |   DataIn      |    2        |
  *          |                         |   GND         |    3 (0 V)  |
  *          |                         |   VDD         |    4 (3.3 V)|
  *          | SD_SPI_SCK_PIN / SCLK   |   Clock       |    5        |
  *          |                         |   GND         |    6 (0 V)  |
  *          | SD_SPI_MISO_PIN / MISO  |   DataOut     |    7        |
  *          +-------------------------+---------------+-------------+
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
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

/* File Info : -----------------------------------------------------------------
                                   User NOTES
1. How to use this driver:
--------------------------
   - This driver does not need a specific component driver for the micro SD device
     to be included with.

2. Driver description:
---------------------
  + Initialization steps:
     o Initialize the micro SD card using the BSP_SD_Init() function. 
     o Checking the SD card presence is not managed because SD detection pin is
       not physically mapped on the Adafruit shield.
     o The function BSP_SD_GetCardInfo() is used to get the micro SD card information 
       which is stored in the structure "SD_CardInfo".
  
  + Micro SD card operations
     o The micro SD card can be accessed with read/write block(s) operations once 
       it is ready for access. The access can be performed in polling 
       mode by calling the functions BSP_SD_ReadBlocks()/BSP_SD_WriteBlocks()
       
     o The SD erase block(s) is performed using the function BSP_SD_Erase() with 
       specifying the number of blocks to erase.
     o The SD runtime status is returned when calling the function BSP_SD_GetStatus().
     
@note This driver's version interfacing SD card using SPI protocol, supports only
      SDSC (Secure Digital Standard Capacity) cards (capacity up to 4GB).
------------------------------------------------------------------------------*/ 

/* Includes ------------------------------------------------------------------*/
#include "stm32_adafruit_sd.h"
#include "usart.h"
#include "sd_io_low.h"
/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32_ADAFRUIT
  * @{
  */ 
  
/** @defgroup STM32_ADAFRUIT_SD
  * @{
  */ 
  
/* Private typedef -----------------------------------------------------------*/

/** @defgroup STM32_ADAFRUIT_SD_Private_Types_Definitions
  * @{
  */ 

/**
  * @}
  */
  
/* Private define ------------------------------------------------------------*/

/** @defgroup STM32_ADAFRUIT_SD_Private_Defines
  * @{
  */
#define SD_DUMMY_BYTE           0xFF
#define SD_NO_RESPONSE_EXPECTED 0x80

uint8_t  SD_Type = 0;//SD卡的类型 ,0代表无卡
/**
  * @}
  */
  
/* Private macro -------------------------------------------------------------*/

/** @defgroup STM32_ADAFRUIT_SD_Private_Macros
  * @{
  */  

/**
  * @}
  */
  
/* Private variables ---------------------------------------------------------*/

/** @defgroup STM32_ADAFRUIT_SD_Private_Variables
  * @{
  */       
__IO uint8_t SdStatus = SD_PRESENT;

/**
  * @}
  */ 

/* Private function prototypes -----------------------------------------------*/
static uint8_t SD_GetCIDRegister(SD_CID* Cid);
static uint8_t SD_GetCSDRegister(SD_CSD* Csd);
static uint8_t SD_GetDataResponse(void);
//static uint8_t SD_GoIdleState(void);
static uint8_t SD_SendCmd(uint8_t Cmd, uint32_t Arg, uint8_t Crc);

static uint8_t 		SD_GetResponse(uint8_t Response);
static uint8_t 		SD_Select(void);
static uint8_t 		SD_WaitReady(void);
static void 			SD_DisSelect(void);
static uint8_t 		SD_SPI_ReadWriteByte(uint8_t data);
static uint8_t SD_RecvData(uint8_t *buf,uint16_t len);
static uint8_t SD_SendBlock(uint8_t *buf, uint8_t cmd);
/** @defgroup STM32_ADAFRUIT_SD_Private_Function_Prototypes
  * @{
  */ 
/**
  * @}
  */
	
//向sd卡写入一个数据包的内容 512字节
//buf:数据缓存区
//cmd:指令
//返回值:0,成功;其他,失败;	
static uint8_t SD_SendBlock(uint8_t *buf, uint8_t cmd)
{	
	uint16_t t = 0;		  	  
	if(SD_WaitReady())return 1;//等待准备失效
	SD_SPI_ReadWriteByte(cmd);
	if(cmd!=0XFD)//不是结束指令
	{
		for(t=0;t<512;t++)SPI1_ReadWriteByte(buf[t]);//提高速度,减少函数传参时间
	    SD_SPI_ReadWriteByte(0xFF);//忽略crc
	    SD_SPI_ReadWriteByte(0xFF);
		t=SD_SPI_ReadWriteByte(0xFF);//接收响应
		if((t&0x1F)!=0x05)return 2;//响应错误									  					    
	}						 									  					    
    return 0;//写入成功
}	
	
	
static uint8_t SD_RecvData(uint8_t *buf,uint16_t len)
{			  	  
	if(SD_GetResponse(0xFE))return 1;//等待SD卡发回数据起始令牌0xFE
    while(len--)//开始接收数据
    {
        *buf=SPI1_ReadWriteByte(0xFF);
        buf++;
    }
    //下面是2个伪CRC（dummy CRC）
    SD_SPI_ReadWriteByte(0xFF);
    SD_SPI_ReadWriteByte(0xFF);									  					    
    return 0;//读取成功
}

static uint8_t SD_SPI_ReadWriteByte(uint8_t data)
{
			return SPI1_ReadWriteByte(data);
}

//等待卡准备好
//返回值:0,准备好了;其他,错误代码
static uint8_t SD_WaitReady(void)
{
	uint32_t t=0;
	do
	{
		if(SPI1_ReadWriteByte(0xFF) == 0xFF)
		{
				return 0;		//OK
		}
		t++;		  	
	}while(t<0xFFFFFF);//等待 
	return 1;
}


//取消选择,释放SPI总线
static void SD_DisSelect(void)
{
			SD_CS_HIGH();
			SPI1_ReadWriteByte(0xff);//提供额外的8个时钟
}


//选择sd卡,并且等待卡准备OK
//返回值:0,成功;1,失败;
static uint8_t SD_Select(void)
{
			SD_CS_LOW();
			if(SD_WaitReady()==0)
					return 0;//等待成功
		
			SD_DisSelect();//取消上次片选
			
			return 1;//等待失败
}
	




//等待SD卡回应
//Response:要得到的回应值
//返回值:0,成功得到了该回应值
//    其他,得到回应值失败
static uint8_t SD_GetResponse(uint8_t Response)
{
	uint16_t Count=0xFFFF;//等待次数	   						  
	while ((SD_SPI_ReadWriteByte(0XFF)!=Response)&&Count)
	{
				Count--;//等待得到准确的回应  	  
	}
	if (Count==0)
	{
				return SD_RESPONSE_FAILURE;//得到回应失败   
	}
	else 
	{
				return SD_RESPONSE_NO_ERROR;//正确回应
	}
}


 
/* Private functions ---------------------------------------------------------*/
    
/** @defgroup STM32_ADAFRUIT_SD_Private_Functions
  * @{
  */ 
  
/**
  * @brief  Initializes the SD/SD communication.
  * @param  None
  * @retval The SD Response: 
  *         - MSD_ERROR: Sequence failed
  *         - MSD_OK: Sequence succeed
  */
uint8_t BSP_SD_Init(void)
{ 
	uint16_t i = 0;
	uint16_t retry = 0;  // 用来进行超时计数
  unsigned int r1 = 0;      // 存放SD卡的返回值
	uint8_t buf[4];  
  /* Configure IO functionalities for SD pin */
  SD_IO_Init();//初始化IO
	SD_SPI_SpeedLow();	//设置到低速模式 
	
	for(i=0;i<10;i++)
		SD_SPI_ReadWriteByte(0xff);//发送最少74个脉冲
	
	retry=20;
	do
	{
			r1 = SD_SendCmd(SD_CMD_GO_IDLE_STATE, 0, 0x95); //发送SD_CMD_GO_IDLE_STATE(CMD0)使卡进入复位，即进入IDLE状态
	}while((r1!=0X01) && retry--);
	
//	printf("卡的状态=%d  retry=%d\r\n", r1, retry);
	SD_Type = 0; //默认无卡
	if(r1==0x01)		//若卡进入复位状态
	{
				/*发送CMD8检查卡是否支持SD2.0协议*/
				if(1 == SD_SendCmd(SD_CMD_SEND_CMD8, 0x1AA, 0x87))		/*SD卡支持2.0协议*/
				{
							for(i = 0; i < 4; i++)
							{
										buf[i] = SD_SPI_ReadWriteByte(0XFF);	//Get trailing return value of R7 resp
//								printf("buf[%d]=%d\r\n", i, buf[i]);
							}
							if((buf[2]== 0x01) && (buf[3] == 0xAA))//卡是否支持2.7~3.6V
							{
										retry=0XFFFE;//设置超时时间
										do
										{
													SD_SendCmd(SD_CMD_CMD55, 0, 0x01);		//发送CMD55
													r1 = SD_SendCmd(SD_CMD_CMD41, 0x40000000, 0x01);	//发送CMD41
//													printf("cmd41.r1=%d\r\n", r1);
										}while(r1&&retry--);
										
										r1 = SD_SendCmd(SD_CMD_CMD58, 0, 0x01);		//发送CMD58
//										printf("cmd58.r1=%d\r\n", r1);//鉴别SD2.0卡版本开始,条件为真，则SD卡支持2.0协议
										if(retry && (r1 == 0))
										{
													for(i=0;i<4;i++)
													{
																buf[i] = SD_SPI_ReadWriteByte(0XFF);//得到OCR值
													}
													if(buf[0]&0x40)
													{
																SD_Type = SD_TYPE_V2HC;    //检查CCS
//																printf("SD_TYPE_V2HC\r\n");
													}	
													else 
													{
																SD_Type = SD_TYPE_V2;
//																printf("SD_TYPE_V2\r\n");
													} 
										}
							}
				}
				else		/*SD卡不支持2.0协议，支持V1.X或者MMC V3协议*/
				{
							SD_SendCmd(SD_CMD_CMD55, 0, 0x01);		//发送CMD55
							r1 = SD_SendCmd(SD_CMD_CMD41, 0, 0x01);//发送CMD41
							if(r1 <= 1)
							{
										SD_Type=SD_TYPE_V1;
										retry=0XFFFE;
										do //等待退出IDLE模式
										{
													SD_SendCmd(SD_CMD_CMD55, 0, 0x01);		//发送CMD55
													r1 = SD_SendCmd(SD_CMD_CMD41, 0, 0x01);//发送CMD41
//													printf("cmd41.r1=%d\r\n", r1);
										}while(r1&&retry--);
							}
							else		//MMC卡不支持CMD55+CMD41识别
							{
										SD_Type = SD_TYPE_MMC;//MMC V3
//										printf("SD_TYPE_MMC\r\n");
										retry=0XFFFE;
										do //等待退出IDLE模式
										{											    
													r1 = SD_SendCmd(SD_CMD_SEND_OP_COND, 0, 0x01) ;	//发送CMD1
//													printf("cmd1.r1=%d\r\n", r1);
										}while(r1&&retry--);  
							}
							
							r1 = SD_SendCmd(SD_CMD_SET_BLOCKLEN, 512, 0x01) ;	//发送CMD16
//							printf("cmd16.r1=%d\r\n", r1);
							if((retry == 0) || (r1 != 0))
							{
										SD_Type = SD_TYPE_ERR;//错误的卡
										printf("SD_TYPE_ERR\r\n");
							}
				}
				
	}
	
	/*取消选择，释放SPI总线*/
	SD_DisSelect();
	
	/*初始化完成，可以设置SPI为高速模式*/
	SD_SPI_SpeedHigh();
	
	if(SD_Type)
	{
				return MSD_OK;		/*识别卡，返回0*/
	}
	else if(r1)
	{
				return MSD_ERROR;	/*卡错误，返回发送CMD16时，卡的返回值*/
	}
	else
	{
				return 0xaa;//其他错误返回0xaa
	}
}

/**
  * @brief  Returns information about specific card.
  * @param  pCardInfo: Pointer to a SD_CardInfo structure that contains all SD 
  *         card information.
  * @retval The SD Response:
  *         - MSD_ERROR: Sequence failed
  *         - MSD_OK: Sequence succeed
  */
uint8_t BSP_SD_GetCardInfo(SD_CardInfo *pCardInfo)
{
  uint8_t status = MSD_ERROR;

  SD_GetCSDRegister(&(pCardInfo->Csd));
  status = SD_GetCIDRegister(&(pCardInfo->Cid));
  pCardInfo->CardCapacity = (pCardInfo->Csd.DeviceSize + 1) ;
  pCardInfo->CardCapacity *= (1 << (pCardInfo->Csd.DeviceSizeMul + 2));
  pCardInfo->CardBlockSize = 1 << (pCardInfo->Csd.RdBlockLen);
  pCardInfo->CardCapacity *= pCardInfo->CardBlockSize;

  /* Returns the response */
  if (status == SD_RESPONSE_NO_ERROR)
  {
    return MSD_OK;
  }
  else
  {
    return MSD_ERROR;
  }
}

/**
  * @brief  Reads block(s) from a specified address in the SD card, in polling mode. 
  * @param  pData: Pointer to the buffer that will contain the data to transmit
  * @param  ReadAddr: Address from where data is to be read  必须是512的整数倍
  * @param  BlockSize: SD card data block size, that should be 512
  * @param  NumOfBlocks: Number of SD blocks to read 想要读取的扇区数
  * @retval SD status
  */
uint8_t BSP_SD_ReadBlocks(uint32_t* pData, uint32_t ReadAddr, uint16_t BlockSize, uint32_t NumberOfBlocks)
{
  uint32_t counter = 0, offset = 0;
  uint8_t rvalue = MSD_ERROR;
  uint8_t *ptr = (uint8_t*) pData;
	uint8_t r1 = 0;
	
	/*先检查是否为512的整数倍*/
	if((ReadAddr % 512) != 0)
	{
				printf("ReadAddr is not 512 * N\r\n");
				return MSD_ERROR;
	}
	
	/*先将字节地址转换为扇区号*/
	ReadAddr >>= 9;			//等价于ReadAddr = ReadAddr >> 9;
	
	if(SD_Type!=SD_TYPE_V2HC)	//如果是SD卡即小容量卡，转换成字节地址
	{
				ReadAddr <<= 9;//转换为字节地址
	}
	
	if(NumberOfBlocks == 1)		//只读一块扇区
	{
				r1 = SD_SendCmd(SD_CMD_READ_SINGLE_BLOCK, ReadAddr, 0X01);//发送CM17读命令
				if(r1==0)//指令发送成功
				{
							r1 = SD_RecvData(ptr, 512);//接收512个字节	   
				}
	}
	else		//读取多个扇区，即NumberOfBlocks>1
	{
				r1 = SD_SendCmd(SD_CMD_READ_MULT_BLOCK, ReadAddr, 0X01);//发送CMD18连续读命令
				do
				{
					r1 = SD_RecvData(ptr, 512);//接收512个字节	 
					ptr += 512;  
				}while(--NumberOfBlocks && r1==0); 	
				SD_SendCmd(SD_CMD_STOP_TRANSMISSION, 0, 0X01);	//发送CMD12停止命令
	}
	SD_DisSelect();//取消片选
	return r1;//
}

/**
  * @brief  Writes block(s) to a specified address in the SD card, in polling mode. 
  * @param  pData: Pointer to the buffer that will contain the data to transmit
  * @param  WriteAddr: Address from where data is to be written  
  * @param  BlockSize: SD card data block size, that should be 512
  * @param  NumOfBlocks: Number of SD blocks to write
  * @retval SD status
  */
uint8_t BSP_SD_WriteBlocks(uint32_t* pData, uint32_t WriteAddr, uint16_t BlockSize, uint32_t NumberOfBlocks)
{
  uint32_t counter = 0, offset = 0;
  uint8_t rvalue = MSD_ERROR;
  uint8_t *ptr = (uint8_t*) pData;
	uint8_t r1 = 0;

	
	/*先检查是否为512的整数倍*/
	if((WriteAddr % 512) != 0)
	{
				printf("WriteAddr is not 512 * N\r\n");
				return MSD_ERROR;
	}
	
	/*先将字节地址转换为扇区号*/
	WriteAddr >>= 9;			//等WriteAddr = WriteAddr >> 9;
	
	if(SD_Type!=SD_TYPE_V2HC)	//如果是SD卡即小容量卡，转换成字节地址
	{
				WriteAddr <<= 9;//转换为字节地址
	}
	
	if(NumberOfBlocks == 1)		//只写一块扇区
	{
				r1 = SD_SendCmd(SD_CMD_WRITE_SINGLE_BLOCK, WriteAddr, 0X01);//发送CM17写命令
				if(r1 == 0)//指令发送成功
				{
							r1 = SD_SendBlock(ptr, 0xfe);//写512个字节	   
				}
	}
	else		//读取多个扇区，即NumberOfBlocks>1
	{
				if(SD_Type!=SD_TYPE_MMC)
				{
					SD_SendCmd(SD_CMD_CMD55, 0, 0X01);	
					SD_SendCmd(SD_CMD_SET_BLOCK_COUNT, NumberOfBlocks, 0X01);//发送指令	
				}
				r1 = SD_SendCmd(SD_CMD_WRITE_MULT_BLOCK, WriteAddr, 0X01);//发送CMD18连续读命令
				if(r1 == 0)
				{
							do
							{
								r1 = SD_SendBlock(ptr, 0xFC);//接收512个字节	 
								ptr += 512;  
							}while(--NumberOfBlocks && r1==0); 	
							r1 = SD_SendBlock(0, 0xFD);
				}	
	}
	
	SD_DisSelect();//取消片选
	return r1;//
}

/**
  * @brief  Returns the SD status.
  * @param  None
  * @retval The SD status.
  */
uint8_t BSP_SD_GetStatus(void)
{
#if !defined (SD_GET_STATUS_WORKAROUND)
  uint16_t status = 0;
  
  /* Send CMD13 (SD_SEND_STATUS) to get SD status */
  SD_SendCmd(SD_CMD_SEND_STATUS, 0, 0xFF, SD_NO_RESPONSE_EXPECTED);
  
  status = SD_IO_ReadByte();
  status |= (uint16_t)(SD_IO_ReadByte() << 8);
  
  /* Send Dummy Byte */
  SD_IO_WriteDummy();
  
  /* Find SD status according to card state */
  if (status == SD_RESPONSE_NO_ERROR)
  {
    return MSD_OK;
  }
  else
  {
    return MSD_ERROR;
  }
#else
  /* This is a temporary workaround for this issue: on some STM32 Nucleo boards 
     reading the SD card status will return an error */
  return MSD_OK;
#endif /* SD_GET_STATUS_WORKAROUND */
}

/**
  * @brief  Erases the specified memory area of the given SD card. 
  * @param  StartAddr: Start byte address
  * @param  EndAddr: End byte address
  * @retval SD status
  */
//uint8_t BSP_SD_Erase(uint32_t StartAddr, uint32_t EndAddr)
//{
//  uint8_t rvalue = SD_RESPONSE_FAILURE;

//  /* Send CMD32 (Erase group start) and check if the SD acknowledged the erase command: R1 response (0x00: no errors) */
//  if (!SD_SendCmd(SD_CMD_SD_ERASE_GRP_START, StartAddr, 0xFF, SD_RESPONSE_NO_ERROR))
//  {
//    /* Send CMD33 (Erase group end) and Check if the SD acknowledged the erase command: R1 response (0x00: no errors) */
//    if (!SD_SendCmd(SD_CMD_SD_ERASE_GRP_END, EndAddr, 0xFF, SD_RESPONSE_NO_ERROR))
//    {
//      /* Send CMD38 (Erase) and Check if the SD acknowledged the erase command: R1 response (0x00: no errors) */
//      if (!SD_SendCmd(SD_CMD_ERASE, 0, 0xFF, SD_RESPONSE_NO_ERROR))
//      {
//        /* Verify that SD card is ready to use after the specific command ERASE */
//        rvalue = (uint8_t)SD_IO_WaitResponse(SD_RESPONSE_NO_ERROR);
//      }
//    }
//  }
//  
//  /* Return the response */
//  if (rvalue == SD_RESPONSE_NO_ERROR)
//  {
//    return MSD_OK;
//  }
//  else
//  {
//    return MSD_ERROR;
//  }
//}

/**
  * @brief  Reads the SD card SCD register.
  *         Reading the contents of the CSD register in SPI mode is a simple 
  *         read-block transaction.
  * @param  Csd: pointer on an SCD register structure
  * @retval SD status
  */
static uint8_t SD_GetCSDRegister(SD_CSD* Csd)
{
  uint32_t counter = 0, len = 0;
  uint8_t rvalue = SD_RESPONSE_FAILURE;
  uint8_t CSD_Tab[16];
	uint8_t r1 = 0, n = 0;
	uint32_t Capacity = 0;  	//SD卡容量
	uint16_t csize = 0;
	
	len = 16;

	r1 = SD_SendCmd(SD_CMD_SEND_CSD,0,0x01);//发CMD9命令，读CSD
	if(r1 == 0)		//得到SD卡的回应，等待接收SD卡发过来的数据
	{
				if(SD_GetResponse(0xFE))	//等待SD卡发回数据起始令牌0xFE
				{
							printf("没有接收到SD卡回复的起始令牌do not wait 0xFE\r\n");
							return 1;
				}
				while(len--)//开始接收数据
				{
						CSD_Tab[counter] = SPI1_ReadWriteByte(0xFF);
						counter++;
				}
				//下面是2个伪CRC（dummy CRC）
				SD_SPI_ReadWriteByte(0xFF);
				SD_SPI_ReadWriteByte(0xFF);									  					    
					
	}
	
	if(counter == 16)
	{
				rvalue = SD_RESPONSE_NO_ERROR;
				if((CSD_Tab[0] & 0xC0) == 0x40)//如果为SDHC卡，按照下面方式计算
				{
							csize = CSD_Tab[9] + ((uint16_t)CSD_Tab[8] << 8) + 1;
							Capacity = (uint32_t)csize << 10;//得到扇区数	 
				}
				else
				{
							n = (CSD_Tab[5] & 15) + ((CSD_Tab[10] & 128) >> 7) + ((CSD_Tab[9] & 3) << 1) + 2;
							csize = (CSD_Tab[8] >> 6) + ((uint16_t)CSD_Tab[7] << 2) + ((uint16_t)(CSD_Tab[6] & 3) << 10) + 1;
							Capacity= (uint32_t)csize << (n - 9);//得到扇区数  
				}
	}
	else
	{
				rvalue = SD_RESPONSE_FAILURE;
				printf("counter=%d\r\n", counter);
	}
	
//	printf("扇区数blocks=%d\r\n", Capacity);
//	printf("容量Capacity=%dG\r\n", (Capacity * 512) >> 29);
	
  /* Send dummy byte: 8 Clock pulses of delay */
  SD_IO_WriteDummy();

	/*SD卡回复无错就填充结构体*/
  if(rvalue == SD_RESPONSE_NO_ERROR)
  {
    /* Byte 0 */
    Csd->CSDStruct = (CSD_Tab[0] & 0xC0) >> 6;
    Csd->SysSpecVersion = (CSD_Tab[0] & 0x3C) >> 2;
    Csd->Reserved1 = CSD_Tab[0] & 0x03;

    /* Byte 1 */
    Csd->TAAC = CSD_Tab[1];

    /* Byte 2 */
    Csd->NSAC = CSD_Tab[2];

    /* Byte 3 */
    Csd->MaxBusClkFrec = CSD_Tab[3];

    /* Byte 4 */
    Csd->CardComdClasses = CSD_Tab[4] << 4;

    /* Byte 5 */
    Csd->CardComdClasses |= (CSD_Tab[5] & 0xF0) >> 4;
    Csd->RdBlockLen = CSD_Tab[5] & 0x0F;

    /* Byte 6 */
    Csd->PartBlockRead = (CSD_Tab[6] & 0x80) >> 7;
    Csd->WrBlockMisalign = (CSD_Tab[6] & 0x40) >> 6;
    Csd->RdBlockMisalign = (CSD_Tab[6] & 0x20) >> 5;
    Csd->DSRImpl = (CSD_Tab[6] & 0x10) >> 4;
    Csd->Reserved2 = 0; /*!< Reserved */

    Csd->DeviceSize = (CSD_Tab[6] & 0x03) << 10;

    /* Byte 7 */
    Csd->DeviceSize |= (CSD_Tab[7]) << 2;

    /* Byte 8 */
    Csd->DeviceSize |= (CSD_Tab[8] & 0xC0) >> 6;

    Csd->MaxRdCurrentVDDMin = (CSD_Tab[8] & 0x38) >> 3;
    Csd->MaxRdCurrentVDDMax = (CSD_Tab[8] & 0x07);

    /* Byte 9 */
    Csd->MaxWrCurrentVDDMin = (CSD_Tab[9] & 0xE0) >> 5;
    Csd->MaxWrCurrentVDDMax = (CSD_Tab[9] & 0x1C) >> 2;
    Csd->DeviceSizeMul = (CSD_Tab[9] & 0x03) << 1;
    /* Byte 10 */
    Csd->DeviceSizeMul |= (CSD_Tab[10] & 0x80) >> 7;
      
    Csd->EraseGrSize = (CSD_Tab[10] & 0x40) >> 6;
    Csd->EraseGrMul = (CSD_Tab[10] & 0x3F) << 1;

    /* Byte 11 */
    Csd->EraseGrMul |= (CSD_Tab[11] & 0x80) >> 7;
    Csd->WrProtectGrSize = (CSD_Tab[11] & 0x7F);

    /* Byte 12 */
    Csd->WrProtectGrEnable = (CSD_Tab[12] & 0x80) >> 7;
    Csd->ManDeflECC = (CSD_Tab[12] & 0x60) >> 5;
    Csd->WrSpeedFact = (CSD_Tab[12] & 0x1C) >> 2;
    Csd->MaxWrBlockLen = (CSD_Tab[12] & 0x03) << 2;

    /* Byte 13 */
    Csd->MaxWrBlockLen |= (CSD_Tab[13] & 0xC0) >> 6;
    Csd->WriteBlockPaPartial = (CSD_Tab[13] & 0x20) >> 5;
    Csd->Reserved3 = 0;
    Csd->ContentProtectAppli = (CSD_Tab[13] & 0x01);

    /* Byte 14 */
    Csd->FileFormatGrouop = (CSD_Tab[14] & 0x80) >> 7;
    Csd->CopyFlag = (CSD_Tab[14] & 0x40) >> 6;
    Csd->PermWrProtect = (CSD_Tab[14] & 0x20) >> 5;
    Csd->TempWrProtect = (CSD_Tab[14] & 0x10) >> 4;
    Csd->FileFormat = (CSD_Tab[14] & 0x0C) >> 2;
    Csd->ECC = (CSD_Tab[14] & 0x03);

    /* Byte 15 */
    Csd->CSD_CRC = (CSD_Tab[15] & 0xFE) >> 1;
    Csd->Reserved4 = 1;
  }
  
  /* Return the response */
  return rvalue;
}

/**
  * @brief  Reads the SD card CID register.
  *         Reading the contents of the CID register in SPI mode is a simple 
  *         read-block transaction.
  * @param  Cid: pointer on an CID register structure
  * @retval SD status
  */
static uint8_t SD_GetCIDRegister(SD_CID* Cid)
{
  uint32_t counter = 0;
  uint8_t rvalue = SD_RESPONSE_FAILURE;
  uint8_t CID_Tab[16];
  
  /* Send CMD10 (CID register) and Wait for response in the R1 format (0x00 is no errors) */
  if (!SD_IO_WriteCmd(SD_CMD_SEND_CID, 0, 0xFF, SD_RESPONSE_NO_ERROR))
  {
    if (!SD_IO_WaitResponse(SD_START_DATA_SINGLE_BLOCK_READ))
    {
      /* Store CID register value on CID_Tab */
      for (counter = 0; counter < 16; counter++)
      {
        CID_Tab[counter] = SD_IO_ReadByte();
      }

      /* Get CRC bytes (not really needed by us, but required by SD) */
      SD_IO_WriteByte(SD_DUMMY_BYTE);
      SD_IO_WriteByte(SD_DUMMY_BYTE);

      /* Set response value to success */
      rvalue = SD_RESPONSE_NO_ERROR;
    }
  }

  /* Send dummy byte: 8 Clock pulses of delay */
  SD_IO_WriteDummy();

  if(rvalue == SD_RESPONSE_NO_ERROR)
  {
    /* Byte 0 */
    Cid->ManufacturerID = CID_Tab[0];

    /* Byte 1 */
    Cid->OEM_AppliID = CID_Tab[1] << 8;

    /* Byte 2 */
    Cid->OEM_AppliID |= CID_Tab[2];

    /* Byte 3 */
    Cid->ProdName1 = CID_Tab[3] << 24;

    /* Byte 4 */
    Cid->ProdName1 |= CID_Tab[4] << 16;

    /* Byte 5 */
    Cid->ProdName1 |= CID_Tab[5] << 8;

    /* Byte 6 */
    Cid->ProdName1 |= CID_Tab[6];

    /* Byte 7 */
    Cid->ProdName2 = CID_Tab[7];

    /* Byte 8 */
    Cid->ProdRev = CID_Tab[8];

    /* Byte 9 */
    Cid->ProdSN = CID_Tab[9] << 24;

    /* Byte 10 */
    Cid->ProdSN |= CID_Tab[10] << 16;

    /* Byte 11 */
    Cid->ProdSN |= CID_Tab[11] << 8;

    /* Byte 12 */
    Cid->ProdSN |= CID_Tab[12];

    /* Byte 13 */
    Cid->Reserved1 |= (CID_Tab[13] & 0xF0) >> 4;
    Cid->ManufactDate = (CID_Tab[13] & 0x0F) << 8;

    /* Byte 14 */
    Cid->ManufactDate |= CID_Tab[14];

    /* Byte 15 */
    Cid->CID_CRC = (CID_Tab[15] & 0xFE) >> 1;
    Cid->Reserved2 = 1;
  }
  /* Return the response */
  return rvalue;
}

/**
  * @brief  Sends 5 bytes command to the SD card and get response
  * @param  Cmd: The user expected command to send to SD card.
  * @param  Arg: The command argument.
  * @param  Crc: The CRC.
  * @param  Response: Expected response from the SD card
  * @retval SD status
  */
static uint8_t SD_SendCmd(uint8_t Cmd, uint32_t Arg, uint8_t Crc)
{
		unsigned int r1 = 0;	
		uint8_t Retry = 0; 
		SD_DisSelect();//取消上次片选
		if(SD_Select())return 0XFF;//片选失效 
		//发送
			SD_SPI_ReadWriteByte(Cmd | 0x40);//分别写入命令
			SD_SPI_ReadWriteByte(Arg >> 24);
			SD_SPI_ReadWriteByte(Arg >> 16);
			SD_SPI_ReadWriteByte(Arg >> 8);
			SD_SPI_ReadWriteByte(Arg);	  
			SD_SPI_ReadWriteByte(Crc); 
		if(Cmd == 12)SD_SPI_ReadWriteByte(0xff);//Skip a stuff byte when stop reading
			//等待响应，或超时退出
		Retry=0X1F;
		do
		{
			r1=SD_SPI_ReadWriteByte(0xFF);
		}while((r1&0X80) && Retry--);	 
		//返回状态值
    return r1;
}

/**
  * @brief  Gets the SD card data response.
  * @param  None
  * @retval The SD status: Read data response xxx0<status>1
  *         - status 010: Data accecpted
  *         - status 101: Data rejected due to a crc error
  *         - status 110: Data rejected due to a Write error.
  *         - status 111: Data rejected due to other error.
  */
static uint8_t SD_GetDataResponse(void)
{
  uint32_t counter = 0;
  uint8_t response = SD_RESPONSE_FAILURE;
  uint8_t rvalue = SD_DATA_OTHER_ERROR;

  while (counter <= 64)
  {
    /* Read response */
    response = SD_IO_ReadByte();
    
    /* Mask unused bits */
    response &= 0x1F;
    switch (response)
    {
      case SD_DATA_OK:
      {
        rvalue = SD_DATA_OK;
        break;
      }
      case SD_DATA_CRC_ERROR:
        return SD_DATA_CRC_ERROR;
      case SD_DATA_WRITE_ERROR:
        return SD_DATA_WRITE_ERROR;
      default:
      {
        rvalue = SD_DATA_OTHER_ERROR;
        break;
      }
    }
    /* Exit loop in case of data ok */
    if (rvalue == SD_DATA_OK)
      break;
    
    /* Increment loop counter */
    counter++;
  }

  /* Wait null data */
  while (SD_IO_ReadByte() == 0);

  /* Return response */
  return response;
}



/**
  * @brief  Put the SD in Idle state.
  * @param  None
  * @retval SD status
  */
//static uint8_t SD_GoIdleState(void)
//{
//			SD_Error res;
//			/* Send CMD0 (SD_CMD_GO_IDLE_STATE) to put SD in SPI mode and 
//				 wait for In Idle State Response (R1 Format) equal to 0x01 */
//			if (SD_SendCmd(SD_CMD_GO_IDLE_STATE, 0, 0x95, SD_IN_IDLE_STATE) != SD_RESPONSE_NO_ERROR)
//			{
//				
//				/* No Idle State Response: return response failure */
//				return SD_RESPONSE_FAILURE;
//			}
//			/*----------Activates the card initialization process-----------*/
//			/* Send CMD1 (Activates the card process) until response equal to 0x0 and
//				 Wait for no error Response (R1 Format) equal to 0x00 */
//			/*SD(小容量卡)在这里正常,接收到命令CMD1可以等到SD_RESPONSE_NO_ERROR这个回应。但是SDHC卡(大容量卡)死在这里了，因为等不到这个回应*/
//			while (SD_SendCmd(SD_CMD_SEND_OP_COND, 0, 0xFF, SD_RESPONSE_NO_ERROR) != SD_RESPONSE_NO_ERROR);
//			return SD_RESPONSE_NO_ERROR;
//}

/**
  * @}
  */  

/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
