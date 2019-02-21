#include "myiic.h"

//ģ��IIC����PCF8563   PB13=SCL,PB12=SDA,PA0=INT  ����Ϊ32.768KHz.


//��ʼ��IIC
void IIC_Init(void)
{					     
	GPIO_InitTypeDef GPIO_InitStruct;
	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	/*Configure GPIO pins : PB12 PB13 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	 /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12|GPIO_PIN_13, GPIO_PIN_SET);//PB12,PB13�����
}

void Delay(uint8_t i)   //us����ʱ
{
	uint8_t k;
	uint8_t j=32;
	uint8_t temp=i*j;
	for(k=0;k<temp;k++);
}

//����SDAΪ���ģʽ
void SDA_OUT()
{
	GPIO_InitTypeDef GPIO_InitStruct;
	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	/*Configure GPIO pins : PB12 */
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

//����SDAΪ���ģʽ
void SDA_IN()
{
	GPIO_InitTypeDef GPIO_InitStruct;
	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	/*Configure GPIO pins : PB12 */
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT ;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}


//����IIC��ʼ�ź�
void IIC_Start(void)
{
	SDA_OUT();     //sda�����
	IIC_SDA_H;	  	  
	IIC_SCL_H;
	Delay(4);
 	IIC_SDA_L;//START:when CLK is high,DATA change form high to low 
	Delay(4);
	IIC_SCL_L;//ǯסI2C���ߣ�׼�����ͻ�������� 
}	  
//����IICֹͣ�ź�
void IIC_Stop(void)
{
	SDA_OUT();//sda�����
	IIC_SCL_L;
	IIC_SDA_L;//STOP:when CLK is high DATA change form low to high
 	Delay(4);
	IIC_SCL_H; 
	IIC_SDA_H;//����I2C���߽����ź�
	Delay(4);							   	
}
//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
uint8_t IIC_Wait_Ack(void)
{
	uint8_t ucErrTime=0;
	SDA_IN();      //SDA����Ϊ����  
	IIC_SDA_H;
	Delay(1);	   
	IIC_SCL_H;
	Delay(1); 
	while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12))
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL_L;//ʱ�����0 	   
	return 0;  
} 
//����ACKӦ��
void IIC_Ack(void)
{
	IIC_SCL_L;
	SDA_OUT();
	IIC_SDA_L;
	Delay(2);
	IIC_SCL_H;
	Delay(2);
	IIC_SCL_L;
}
//������ACKӦ��		    
void IIC_NAck(void)
{
	IIC_SCL_L;
	SDA_OUT();
	IIC_SDA_H;
	Delay(2);
	IIC_SCL_H;
	Delay(2);
	IIC_SCL_L;
}					 				     
//IIC����һ���ֽ�
//���شӻ�����Ӧ��
//1����Ӧ��
//0����Ӧ��			  
void IIC_Send_Byte(uint8_t txd)
{                        
  uint8_t t;   
	SDA_OUT(); 	    
  IIC_SCL_L;//����ʱ�ӿ�ʼ���ݴ���
  for(t=0;t<8;t++)
  {              
   //IIC_SDA=(txd&0x80)>>7;
	if((txd&0x80)>>7)
		IIC_SDA_H;
	else
		IIC_SDA_L;
		txd<<=1; 	  
		Delay(2);  //��TEA5767��������ʱ���Ǳ����
		IIC_SCL_H;
		Delay(2);
		IIC_SCL_L;	
		Delay(2);
   }	 
} 	    
//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK   
uint8_t IIC_Read_Byte(void)
{
	unsigned char i,receive=0;
	SDA_IN();//SDA����Ϊ����
  for(i=0;i<8;i++ )
	{
    IIC_SCL_L; 
    Delay(2);
		IIC_SCL_H;
    receive<<=1;
    if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12))
			receive++;   
		Delay(1);
  }					 
//  if (!ack)
//      IIC_NAck();//����nACK
//  else
//      IIC_Ack(); //����ACK   
  return receive;
}




