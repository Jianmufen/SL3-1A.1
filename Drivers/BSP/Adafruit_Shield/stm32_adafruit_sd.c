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

uint8_t  SD_Type = 0;//SD�������� ,0�����޿�
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
	
//��sd��д��һ�����ݰ������� 512�ֽ�
//buf:���ݻ�����
//cmd:ָ��
//����ֵ:0,�ɹ�;����,ʧ��;	
static uint8_t SD_SendBlock(uint8_t *buf, uint8_t cmd)
{	
	uint16_t t = 0;		  	  
	if(SD_WaitReady())return 1;//�ȴ�׼��ʧЧ
	SD_SPI_ReadWriteByte(cmd);
	if(cmd!=0XFD)//���ǽ���ָ��
	{
		for(t=0;t<512;t++)SPI1_ReadWriteByte(buf[t]);//����ٶ�,���ٺ�������ʱ��
	    SD_SPI_ReadWriteByte(0xFF);//����crc
	    SD_SPI_ReadWriteByte(0xFF);
		t=SD_SPI_ReadWriteByte(0xFF);//������Ӧ
		if((t&0x1F)!=0x05)return 2;//��Ӧ����									  					    
	}						 									  					    
    return 0;//д��ɹ�
}	
	
	
static uint8_t SD_RecvData(uint8_t *buf,uint16_t len)
{			  	  
	if(SD_GetResponse(0xFE))return 1;//�ȴ�SD������������ʼ����0xFE
    while(len--)//��ʼ��������
    {
        *buf=SPI1_ReadWriteByte(0xFF);
        buf++;
    }
    //������2��αCRC��dummy CRC��
    SD_SPI_ReadWriteByte(0xFF);
    SD_SPI_ReadWriteByte(0xFF);									  					    
    return 0;//��ȡ�ɹ�
}

static uint8_t SD_SPI_ReadWriteByte(uint8_t data)
{
			return SPI1_ReadWriteByte(data);
}

//�ȴ���׼����
//����ֵ:0,׼������;����,�������
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
	}while(t<0xFFFFFF);//�ȴ� 
	return 1;
}


//ȡ��ѡ��,�ͷ�SPI����
static void SD_DisSelect(void)
{
			SD_CS_HIGH();
			SPI1_ReadWriteByte(0xff);//�ṩ�����8��ʱ��
}


//ѡ��sd��,���ҵȴ���׼��OK
//����ֵ:0,�ɹ�;1,ʧ��;
static uint8_t SD_Select(void)
{
			SD_CS_LOW();
			if(SD_WaitReady()==0)
					return 0;//�ȴ��ɹ�
		
			SD_DisSelect();//ȡ���ϴ�Ƭѡ
			
			return 1;//�ȴ�ʧ��
}
	




//�ȴ�SD����Ӧ
//Response:Ҫ�õ��Ļ�Ӧֵ
//����ֵ:0,�ɹ��õ��˸û�Ӧֵ
//    ����,�õ���Ӧֵʧ��
static uint8_t SD_GetResponse(uint8_t Response)
{
	uint16_t Count=0xFFFF;//�ȴ�����	   						  
	while ((SD_SPI_ReadWriteByte(0XFF)!=Response)&&Count)
	{
				Count--;//�ȴ��õ�׼ȷ�Ļ�Ӧ  	  
	}
	if (Count==0)
	{
				return SD_RESPONSE_FAILURE;//�õ���Ӧʧ��   
	}
	else 
	{
				return SD_RESPONSE_NO_ERROR;//��ȷ��Ӧ
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
	uint16_t retry = 0;  // �������г�ʱ����
  unsigned int r1 = 0;      // ���SD���ķ���ֵ
	uint8_t buf[4];  
  /* Configure IO functionalities for SD pin */
  SD_IO_Init();//��ʼ��IO
	SD_SPI_SpeedLow();	//���õ�����ģʽ 
	
	for(i=0;i<10;i++)
		SD_SPI_ReadWriteByte(0xff);//��������74������
	
	retry=20;
	do
	{
			r1 = SD_SendCmd(SD_CMD_GO_IDLE_STATE, 0, 0x95); //����SD_CMD_GO_IDLE_STATE(CMD0)ʹ�����븴λ��������IDLE״̬
	}while((r1!=0X01) && retry--);
	
//	printf("����״̬=%d  retry=%d\r\n", r1, retry);
	SD_Type = 0; //Ĭ���޿�
	if(r1==0x01)		//�������븴λ״̬
	{
				/*����CMD8��鿨�Ƿ�֧��SD2.0Э��*/
				if(1 == SD_SendCmd(SD_CMD_SEND_CMD8, 0x1AA, 0x87))		/*SD��֧��2.0Э��*/
				{
							for(i = 0; i < 4; i++)
							{
										buf[i] = SD_SPI_ReadWriteByte(0XFF);	//Get trailing return value of R7 resp
//								printf("buf[%d]=%d\r\n", i, buf[i]);
							}
							if((buf[2]== 0x01) && (buf[3] == 0xAA))//���Ƿ�֧��2.7~3.6V
							{
										retry=0XFFFE;//���ó�ʱʱ��
										do
										{
													SD_SendCmd(SD_CMD_CMD55, 0, 0x01);		//����CMD55
													r1 = SD_SendCmd(SD_CMD_CMD41, 0x40000000, 0x01);	//����CMD41
//													printf("cmd41.r1=%d\r\n", r1);
										}while(r1&&retry--);
										
										r1 = SD_SendCmd(SD_CMD_CMD58, 0, 0x01);		//����CMD58
//										printf("cmd58.r1=%d\r\n", r1);//����SD2.0���汾��ʼ,����Ϊ�棬��SD��֧��2.0Э��
										if(retry && (r1 == 0))
										{
													for(i=0;i<4;i++)
													{
																buf[i] = SD_SPI_ReadWriteByte(0XFF);//�õ�OCRֵ
													}
													if(buf[0]&0x40)
													{
																SD_Type = SD_TYPE_V2HC;    //���CCS
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
				else		/*SD����֧��2.0Э�飬֧��V1.X����MMC V3Э��*/
				{
							SD_SendCmd(SD_CMD_CMD55, 0, 0x01);		//����CMD55
							r1 = SD_SendCmd(SD_CMD_CMD41, 0, 0x01);//����CMD41
							if(r1 <= 1)
							{
										SD_Type=SD_TYPE_V1;
										retry=0XFFFE;
										do //�ȴ��˳�IDLEģʽ
										{
													SD_SendCmd(SD_CMD_CMD55, 0, 0x01);		//����CMD55
													r1 = SD_SendCmd(SD_CMD_CMD41, 0, 0x01);//����CMD41
//													printf("cmd41.r1=%d\r\n", r1);
										}while(r1&&retry--);
							}
							else		//MMC����֧��CMD55+CMD41ʶ��
							{
										SD_Type = SD_TYPE_MMC;//MMC V3
//										printf("SD_TYPE_MMC\r\n");
										retry=0XFFFE;
										do //�ȴ��˳�IDLEģʽ
										{											    
													r1 = SD_SendCmd(SD_CMD_SEND_OP_COND, 0, 0x01) ;	//����CMD1
//													printf("cmd1.r1=%d\r\n", r1);
										}while(r1&&retry--);  
							}
							
							r1 = SD_SendCmd(SD_CMD_SET_BLOCKLEN, 512, 0x01) ;	//����CMD16
//							printf("cmd16.r1=%d\r\n", r1);
							if((retry == 0) || (r1 != 0))
							{
										SD_Type = SD_TYPE_ERR;//����Ŀ�
										printf("SD_TYPE_ERR\r\n");
							}
				}
				
	}
	
	/*ȡ��ѡ���ͷ�SPI����*/
	SD_DisSelect();
	
	/*��ʼ����ɣ���������SPIΪ����ģʽ*/
	SD_SPI_SpeedHigh();
	
	if(SD_Type)
	{
				return MSD_OK;		/*ʶ�𿨣�����0*/
	}
	else if(r1)
	{
				return MSD_ERROR;	/*�����󣬷��ط���CMD16ʱ�����ķ���ֵ*/
	}
	else
	{
				return 0xaa;//�������󷵻�0xaa
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
  * @param  ReadAddr: Address from where data is to be read  ������512��������
  * @param  BlockSize: SD card data block size, that should be 512
  * @param  NumOfBlocks: Number of SD blocks to read ��Ҫ��ȡ��������
  * @retval SD status
  */
uint8_t BSP_SD_ReadBlocks(uint32_t* pData, uint32_t ReadAddr, uint16_t BlockSize, uint32_t NumberOfBlocks)
{
  uint32_t counter = 0, offset = 0;
  uint8_t rvalue = MSD_ERROR;
  uint8_t *ptr = (uint8_t*) pData;
	uint8_t r1 = 0;
	
	/*�ȼ���Ƿ�Ϊ512��������*/
	if((ReadAddr % 512) != 0)
	{
				printf("ReadAddr is not 512 * N\r\n");
				return MSD_ERROR;
	}
	
	/*�Ƚ��ֽڵ�ַת��Ϊ������*/
	ReadAddr >>= 9;			//�ȼ���ReadAddr = ReadAddr >> 9;
	
	if(SD_Type!=SD_TYPE_V2HC)	//�����SD����С��������ת�����ֽڵ�ַ
	{
				ReadAddr <<= 9;//ת��Ϊ�ֽڵ�ַ
	}
	
	if(NumberOfBlocks == 1)		//ֻ��һ������
	{
				r1 = SD_SendCmd(SD_CMD_READ_SINGLE_BLOCK, ReadAddr, 0X01);//����CM17������
				if(r1==0)//ָ��ͳɹ�
				{
							r1 = SD_RecvData(ptr, 512);//����512���ֽ�	   
				}
	}
	else		//��ȡ�����������NumberOfBlocks>1
	{
				r1 = SD_SendCmd(SD_CMD_READ_MULT_BLOCK, ReadAddr, 0X01);//����CMD18����������
				do
				{
					r1 = SD_RecvData(ptr, 512);//����512���ֽ�	 
					ptr += 512;  
				}while(--NumberOfBlocks && r1==0); 	
				SD_SendCmd(SD_CMD_STOP_TRANSMISSION, 0, 0X01);	//����CMD12ֹͣ����
	}
	SD_DisSelect();//ȡ��Ƭѡ
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

	
	/*�ȼ���Ƿ�Ϊ512��������*/
	if((WriteAddr % 512) != 0)
	{
				printf("WriteAddr is not 512 * N\r\n");
				return MSD_ERROR;
	}
	
	/*�Ƚ��ֽڵ�ַת��Ϊ������*/
	WriteAddr >>= 9;			//��WriteAddr = WriteAddr >> 9;
	
	if(SD_Type!=SD_TYPE_V2HC)	//�����SD����С��������ת�����ֽڵ�ַ
	{
				WriteAddr <<= 9;//ת��Ϊ�ֽڵ�ַ
	}
	
	if(NumberOfBlocks == 1)		//ֻдһ������
	{
				r1 = SD_SendCmd(SD_CMD_WRITE_SINGLE_BLOCK, WriteAddr, 0X01);//����CM17д����
				if(r1 == 0)//ָ��ͳɹ�
				{
							r1 = SD_SendBlock(ptr, 0xfe);//д512���ֽ�	   
				}
	}
	else		//��ȡ�����������NumberOfBlocks>1
	{
				if(SD_Type!=SD_TYPE_MMC)
				{
					SD_SendCmd(SD_CMD_CMD55, 0, 0X01);	
					SD_SendCmd(SD_CMD_SET_BLOCK_COUNT, NumberOfBlocks, 0X01);//����ָ��	
				}
				r1 = SD_SendCmd(SD_CMD_WRITE_MULT_BLOCK, WriteAddr, 0X01);//����CMD18����������
				if(r1 == 0)
				{
							do
							{
								r1 = SD_SendBlock(ptr, 0xFC);//����512���ֽ�	 
								ptr += 512;  
							}while(--NumberOfBlocks && r1==0); 	
							r1 = SD_SendBlock(0, 0xFD);
				}	
	}
	
	SD_DisSelect();//ȡ��Ƭѡ
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
	uint32_t Capacity = 0;  	//SD������
	uint16_t csize = 0;
	
	len = 16;

	r1 = SD_SendCmd(SD_CMD_SEND_CSD,0,0x01);//��CMD9�����CSD
	if(r1 == 0)		//�õ�SD���Ļ�Ӧ���ȴ�����SD��������������
	{
				if(SD_GetResponse(0xFE))	//�ȴ�SD������������ʼ����0xFE
				{
							printf("û�н��յ�SD���ظ�����ʼ����do not wait 0xFE\r\n");
							return 1;
				}
				while(len--)//��ʼ��������
				{
						CSD_Tab[counter] = SPI1_ReadWriteByte(0xFF);
						counter++;
				}
				//������2��αCRC��dummy CRC��
				SD_SPI_ReadWriteByte(0xFF);
				SD_SPI_ReadWriteByte(0xFF);									  					    
					
	}
	
	if(counter == 16)
	{
				rvalue = SD_RESPONSE_NO_ERROR;
				if((CSD_Tab[0] & 0xC0) == 0x40)//���ΪSDHC�����������淽ʽ����
				{
							csize = CSD_Tab[9] + ((uint16_t)CSD_Tab[8] << 8) + 1;
							Capacity = (uint32_t)csize << 10;//�õ�������	 
				}
				else
				{
							n = (CSD_Tab[5] & 15) + ((CSD_Tab[10] & 128) >> 7) + ((CSD_Tab[9] & 3) << 1) + 2;
							csize = (CSD_Tab[8] >> 6) + ((uint16_t)CSD_Tab[7] << 2) + ((uint16_t)(CSD_Tab[6] & 3) << 10) + 1;
							Capacity= (uint32_t)csize << (n - 9);//�õ�������  
				}
	}
	else
	{
				rvalue = SD_RESPONSE_FAILURE;
				printf("counter=%d\r\n", counter);
	}
	
//	printf("������blocks=%d\r\n", Capacity);
//	printf("����Capacity=%dG\r\n", (Capacity * 512) >> 29);
	
  /* Send dummy byte: 8 Clock pulses of delay */
  SD_IO_WriteDummy();

	/*SD���ظ��޴�����ṹ��*/
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
		SD_DisSelect();//ȡ���ϴ�Ƭѡ
		if(SD_Select())return 0XFF;//ƬѡʧЧ 
		//����
			SD_SPI_ReadWriteByte(Cmd | 0x40);//�ֱ�д������
			SD_SPI_ReadWriteByte(Arg >> 24);
			SD_SPI_ReadWriteByte(Arg >> 16);
			SD_SPI_ReadWriteByte(Arg >> 8);
			SD_SPI_ReadWriteByte(Arg);	  
			SD_SPI_ReadWriteByte(Crc); 
		if(Cmd == 12)SD_SPI_ReadWriteByte(0xff);//Skip a stuff byte when stop reading
			//�ȴ���Ӧ����ʱ�˳�
		Retry=0X1F;
		do
		{
			r1=SD_SPI_ReadWriteByte(0xFF);
		}while((r1&0X80) && Retry--);	 
		//����״ֵ̬
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
//			/*SD(С������)����������,���յ�����CMD1���Եȵ�SD_RESPONSE_NO_ERROR�����Ӧ������SDHC��(��������)���������ˣ���Ϊ�Ȳ��������Ӧ*/
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
