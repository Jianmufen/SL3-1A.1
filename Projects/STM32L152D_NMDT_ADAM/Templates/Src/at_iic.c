#include "at_iic.h"

//ģ��IIC����PCF8563   PB13=SCL,PB12=SDA,PA0=INT  ����Ϊ32.768KHz.


//��ʼ��IIC
void FM_IIC_Init(void)
{					     
	GPIO_InitTypeDef GPIO_InitStruct;
	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	/*Configure GPIO pins : PB14 PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	 /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_SET);//PB14,PB15�����
}

void FM_Delay(uint8_t i)   //us����ʱ
{
	uint8_t k,j;
	for(k=0;k<i;k++)
	{
	 for(j=0;j<32;j++)
		; //��ʱ1us=32*31.25ns=1000ns=1us
	}
}

//����SDAΪ���ģʽ
void FM_SDA_OUT()
{
	GPIO_InitTypeDef GPIO_InitStruct;
	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	/*Configure GPIO pins : PB12 */
  GPIO_InitStruct.Pin = GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

//����SDAΪ���ģʽ
void FM_SDA_IN()
{
	GPIO_InitTypeDef GPIO_InitStruct;
	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	/*Configure GPIO pins : PB12 */
  GPIO_InitStruct.Pin = GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT ;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}


//����IIC��ʼ�ź�
void FM_IIC_Start(void)
{
	FM_SDA_OUT();     //sda�����
	FM_IIC_SDA_H;	  	  
	FM_IIC_SCL_H;
	FM_Delay(4);
 	FM_IIC_SDA_L;//START:when CLK is high,DATA change form high to low 
	FM_Delay(4);
	FM_IIC_SCL_L;//ǯסI2C���ߣ�׼�����ͻ�������� 
}	  
//����IICֹͣ�ź�
void FM_IIC_Stop(void)
{
	FM_SDA_OUT();//sda�����
	FM_IIC_SCL_L;
	FM_IIC_SDA_L;//STOP:when CLK is high DATA change form low to high
	FM_Delay(4);	
	FM_IIC_SCL_H;
	FM_IIC_SDA_H;//����I2C���߽����ź�
	FM_Delay(4);							   	
}
//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
uint8_t FM_IIC_Wait_Ack(void)
{
	uint8_t ucErrTime=0;
	FM_SDA_IN();      //SDA����Ϊ����  
	FM_IIC_SDA_H; FM_Delay(1);	   
	FM_IIC_SCL_H;	FM_Delay(1); 
	while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14))
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			FM_IIC_Stop();
			return 1;
		}
	}
	FM_IIC_SCL_L;//ʱ�����0 	   
	return 0;  
} 
//����ACKӦ��
void FM_IIC_Ack(void)
{
	FM_IIC_SCL_L;
	FM_SDA_OUT();
	FM_IIC_SDA_L;
	FM_Delay(2);
	FM_IIC_SCL_H;
	FM_Delay(2);
	FM_IIC_SCL_L;
}
//������ACKӦ��		    
void FM_IIC_NAck(void)
{
	FM_IIC_SCL_L;
	FM_SDA_OUT();
	FM_IIC_SDA_H;
	FM_Delay(2);
	FM_IIC_SCL_H;
	FM_Delay(2);
	FM_IIC_SCL_L;
}					 				     
//IIC����һ���ֽ�
//���شӻ�����Ӧ��
//1����Ӧ��
//0����Ӧ��			  
void FM_IIC_Send_Byte(uint8_t txd)
{                        
  uint8_t t;   
	FM_SDA_OUT(); 	    
  FM_IIC_SCL_L;//����ʱ�ӿ�ʼ���ݴ���
  for(t=0;t<8;t++)
  {              
   //IIC_SDA=(txd&0x80)>>7;
	if((txd&0x80)>>7)
		FM_IIC_SDA_H;
	else
		FM_IIC_SDA_L;
		txd<<=1; 	  
		FM_Delay(2);
		FM_IIC_SCL_H;
		FM_Delay(2);
		FM_IIC_SCL_L;	
		FM_Delay(2);
   }	 
} 	    
//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK   
uint8_t FM_IIC_Read_Byte(uint8_t ack)
{
	unsigned char i,receive=0;
	FM_SDA_IN();//SDA����Ϊ����
  for(i=0;i<8;i++ )
	{
    FM_IIC_SCL_L; 
    FM_Delay(2);
		FM_IIC_SCL_H;
    receive<<=1;
    if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14))
			receive++;   
		FM_Delay(1);
  }					 
  if (ack==0)
      FM_IIC_NAck();//����nACK
  else
      FM_IIC_Ack(); //����ACK   
  return receive;
}


