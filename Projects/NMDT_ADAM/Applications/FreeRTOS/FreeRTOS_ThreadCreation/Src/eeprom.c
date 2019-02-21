/**
  ******************************************************************************
  * File Name          : data_eeprom.h
  * Description        : This file provides code for the dataeeprom
  *                      read/write process.
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
#include "eeprom.h"


/* Private function prototypes -----------------------------------------------*/


/**
  * @brief  Write data to DATAEEPROM at a specified address
  * @param  Address:  specifies the address to be programmed.
  * @param  Data: specifies the data to be programmed
  * @param  Count:  Indicate the number of Data to be write.
  * 
  * @retval HAL_StatusTypeDef HAL Status
  */
HAL_StatusTypeDef data_eeprom_write(uint32_t Address,uint8_t *Data,uint32_t Count)
{
  HAL_StatusTypeDef status = HAL_ERROR;
  uint32_t dat=0;
  
  /* Check the parameters */
  assert_param(IS_FLASH_DATA_ADDRESS(Address)); 
  assert_param(IS_FLASH_DATA_ADDRESS(Address+Count-1));

  
  /* Unlocks the data memory and FLASH_PECR register access */
  HAL_FLASHEx_DATAEEPROM_Unlock();
  
  /* Clear FLASH Write protected error flag if it's set,in case of write fail */
  if(__HAL_FLASH_GET_FLAG(FLASH_FLAG_WRPERR))
  {
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_WRPERR);
  }
  /*dat=FLASH->SR;
  printf("wflash->sr:%#x\r\n",dat);*/
  
  /* Write DATA to the DATAEEPROM */
  while(Count>=sizeof(uint32_t))   /* write with a WORD copy */
  {
    dat=*((uint32_t *)Data);
    status=HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD,Address,dat);
    if(status!=HAL_OK)
    {
      HAL_FLASHEx_DATAEEPROM_Lock();
      return status;
    }
    Count-=sizeof(uint32_t);
    Address+=sizeof(uint32_t);
    Data+=sizeof(uint32_t);
  }
  while(Count>=sizeof(uint16_t))   /* write with a HalfWORD copy */
  {
    dat=*((uint16_t *)Data);
    status=HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_HALFWORD,Address,dat);
    if(status!=HAL_OK)
    {
      HAL_FLASHEx_DATAEEPROM_Lock();
      return status;
    }
    Count-=sizeof(uint16_t);
    Address+=sizeof(uint16_t);
    Data+=sizeof(uint16_t);
  }
  while(Count>0)  /* write the rest bytes with a byte-to-byte copy */
  {
    dat=*((uint8_t *)Data);
    status=HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE,Address,dat);
    if(status!=HAL_OK)
    {
      HAL_FLASHEx_DATAEEPROM_Lock();
      return status;
    }
    Count-=1;
    Address+=1;
    Data+=1;
  }
  
  /* Locks the Data memory and FLASH_PECR register access */
  HAL_FLASHEx_DATAEEPROM_Lock();
  
  return status;
}


/**
  * @brief  Read data from DATAEEPROM at a specified address
  * @param  Address:  specifies the address to read from.
  * @param  Data: specifies the buffer to save the data read
  * @param  Count:  Indicate the number of Data to read.
  * 
  * @retval HAL_StatusTypeDef HAL Status
  */
HAL_StatusTypeDef data_eeprom_read(uint32_t Address,uint8_t *Data,uint32_t Count)
{
  
  /* Check the parameters */
  assert_param(IS_FLASH_DATA_ADDRESS(Address)); 
  assert_param(IS_FLASH_DATA_ADDRESS(Address+Count-1));
  
  
  /* Read DATA from the DATAEEPROM */
  while(Count>=sizeof(uint32_t))   /* read with a WORD copy */
  {
    *((uint32_t *)Data)=*((uint32_t *)Address);
    Count-=sizeof(uint32_t);
    Address+=sizeof(uint32_t);
    Data+=sizeof(uint32_t);
  }
  while(Count>=sizeof(uint16_t))   /* read with a HalfWORD copy */
  {
    *((uint16_t *)Data)=*((uint16_t *)Address);
    Count-=sizeof(uint16_t);
    Address+=sizeof(uint16_t);
    Data+=sizeof(uint16_t);
  }
  while(Count>0)
  {
    *Data=*((uint8_t *)Address);
    Count-=1;
    Address+=1;
    Data+=1;
  }
  

  return HAL_OK;
}




/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
