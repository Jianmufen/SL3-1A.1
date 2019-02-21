/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __myiic_H
#define __myiic_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx_hal.h"

//IO��������	 
#define IIC_SCL_H    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET)        //SCL
#define IIC_SCL_L    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET)     //SCL

#define IIC_SDA_H    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET)     //SDA	 
#define IIC_SDA_L 	 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET)  //SCL
 
//��������
void IIC_Init(void);	 
void SDA_OUT();
void SDA_IN();	 
void IIC_Start(void);				//����IIC��ʼ�ź�
void IIC_Stop(void);	  			//����IICֹͣ�ź�
void IIC_Send_Byte(uint8_t txd);			//IIC����һ���ֽ�
uint8_t IIC_Read_Byte(void);//IIC��ȡһ���ֽ�
uint8_t IIC_Wait_Ack(void); 				//IIC�ȴ�ACK�ź�
void IIC_Ack(void);					//IIC����ACK�ź�
void IIC_NAck(void);				//IIC������ACK�ź�
void Delay(uint8_t i);

void IIC_Write_One_Byte(uint8_t daddr,uint8_t addr,uint8_t data);
uint8_t IIC_Read_One_Byte(uint8_t daddr,uint8_t addr);	  	 
	 
	 
#ifdef __cplusplus
}
#endif
#endif /*__ pinoutConfig_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
