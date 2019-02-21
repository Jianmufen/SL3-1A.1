/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __pcf8563_H
#define __pcf8563_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx_hal.h"
#include "stdio.h"


//IO��������	 
#define IIC_SCL_H    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET)        //SCL
#define IIC_SCL_L    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET)     //SCL

#define IIC_SDA_H    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET)     //SDA	 
#define IIC_SDA_L 	 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET)  //SCL
#define SSDA         HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_12)
	 
typedef struct
{
	unsigned char RTC_Year;      //��
	unsigned char RTC_Month;     //��
	unsigned char RTC_WeekDay;   //����
	unsigned char RTC_Day;       //��
	unsigned char RTC_Hour;      //ʱ
	unsigned char RTC_Minute;    //��
	unsigned char RTC_Second;    //��
}PCF8563_Time_Typedef;


extern unsigned char Time_Buffer[7];	
extern PCF8563_Time_Typedef PCF_DataStruct_Time	 ;
//typedef enum
//{
//	FALSE=0,
//	TURE=1,
//}bool;




//��������
void SDA_OUT(void);
void SDA_IN(void);
void I2C_delay(void);
void IIC_Init(void);	 
void IIC_Start(void);				//����IIC��ʼ�ź�
void IIC_Stop(void);	  			//����IICֹͣ�ź�
unsigned char IIC_Send_Byte(unsigned char RTC_Cmd);
unsigned char IIC_Read_Byte(void);//IIC��ȡһ���ֽ�
uint8_t IIC_Wait_Ack(void); 				//IIC�ȴ�ACK�ź�
void IIC_Ack(void);					//IIC����ACK�ź�
void IIC_NAck(void);				//IIC������ACK�ź�

uint8_t PCF8563_Read_Time(void);
uint8_t PCF8563_Set_Time(uint8_t Year,uint8_t Month,uint8_t Day,uint8_t Hour,uint8_t Minute,uint8_t Second);
	 
	 
#ifdef __cplusplus
}
#endif
#endif /*__ pinoutConfig_H */

