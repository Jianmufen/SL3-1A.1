#ifndef _at24c512_h_ 
#define _at24c512_h_


/**
  ****************************** Support C++ **********************************
**/
#ifdef __cplusplus
	extern "C"{
#endif		
/**
  *****************************************************************************
**/
		

/******************************************************************************
                             �ⲿ����ͷ�ļ�
                  Ӧ�õ���ͬ������ͷ�ļ����������޸ļ���                        
******************************************************************************/

#include "at_iic.h"

/******************************************************************************
                                 �����궨��
******************************************************************************/		
#define FM24C256_Check_Data                       (unsigned char)0xFF  //����ã�����������ֵ

#define FM24C256_Write1                            (unsigned char)0xa4  //�ڴ�1��д����
#define FM24C256_Read1                             (unsigned char)0xa5  //�ڴ�1�Ķ�����
	
#define FM24C256_Write2                            (unsigned char)0xa0  //�ڴ�2��д����
#define FM24C256_Read2                             (unsigned char)0xa1  //�ڴ�2�Ķ�����
	

void FM24C256_Write_NByte(uint8_t FM_ADD,uint16_t RAM_ADD,uint8_t n, char nBuffer[n]);
uint8_t FM24C256_Read_Byte(uint8_t FM_ADD,uint16_t RAM_ADD);
void FM24C256_Write_Byte(uint8_t FM_ADD,uint16_t RAM_ADD, char dat);
int8_t FM24C256_Check(void);
void FM24C256_Read_MByte(uint8_t FM_ADD,uint16_t RAM_ADD,uint8_t m,char *nBuffer);

void AT24C512_Read(uint8_t FM_ADD,uint16_t ReadAddr,uint8_t *pBuffer,uint16_t NumToRead);
void AT24C512_Write(uint16_t FM_ADD,uint16_t WriteAddr,uint8_t *pBuffer,uint16_t NumToWrite);




#ifdef __cplusplus
	}
#endif
/**
  *****************************************************************************
**/


#endif
