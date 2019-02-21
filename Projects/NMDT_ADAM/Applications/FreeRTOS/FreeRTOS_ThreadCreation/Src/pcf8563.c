#include "pcf8563.h"

//ģ��IIC����PCF8563   PB13=SCL,PB12=SDA,PA0=INT  ����Ϊ32.768KHz.
PCF8563_Time_Typedef PCF_DataStruct_Time;
unsigned char Time_Buffer[7];

//��ʼ��IIC
HAL_StatusTypeDef IIC_Init(void)
{					     
	IIC_Start(); //I2C��������
	IIC_Send_Byte(0xA2);//����д����
	if(IIC_Wait_Ack()==HAL_ERROR)//�ȴ�I2C���߻ظ�
	{
		
		return HAL_ERROR;//�������е�����  �ͽ�����
	}
	IIC_Send_Byte(0x00);//ѡ���ַ0x00����״̬�Ĵ���1��֮���д��ַ�Զ���1
	if(IIC_Wait_Ack()==HAL_ERROR)//�ȴ�I2C���߻ظ�
	{
		
		return HAL_ERROR;
	}
	IIC_Send_Byte(0x00); //����STOP=0��оƬʱ������
	if(IIC_Wait_Ack()==HAL_ERROR)//�ȴ�I2C���߻ظ�
	{
		
		return HAL_ERROR;
	}
	IIC_Send_Byte(0x13);//�򿪱����жϺͶ�ʱ���жϣ����巽ʽ
	if(IIC_Wait_Ack()==HAL_ERROR)//�ȴ�I2C���߻ظ�
	{
		
		return HAL_ERROR;
	}
	IIC_Start(); //I2C��������
	IIC_Send_Byte(0xA2);//����д����
	if(IIC_Wait_Ack()==HAL_ERROR)//�ȴ�I2C���߻ظ�
	{
		
		return HAL_ERROR;
	}
	IIC_Send_Byte(0x09);//ѡ���ַ0x09���ӱ����Ĵ���
	if(IIC_Wait_Ack()==HAL_ERROR)//�ȴ�I2C���߻ظ�
	{
		
		return HAL_ERROR;
	}
	IIC_Send_Byte(0x80);//�رշ��ӱ���
	if(IIC_Wait_Ack()==HAL_ERROR)//�ȴ�I2C���߻ظ�
	{
		
		return HAL_ERROR;
	}
	IIC_Send_Byte(0x80);//�ر�Сʱ����
	if(IIC_Wait_Ack()==HAL_ERROR)//�ȴ�I2C���߻ظ�
	{
		
		return HAL_ERROR;
	}
	IIC_Send_Byte(0x80);//�ر��ձ���
	if(IIC_Wait_Ack()==HAL_ERROR)//�ȴ�I2C���߻ظ�
	{
		
		return HAL_ERROR;
	}
	IIC_Send_Byte(0x80);//�ر����ڱ���
	if(IIC_Wait_Ack()==HAL_ERROR)//�ȴ�I2C���߻ظ�
	{
		
		return HAL_ERROR;
	}
	IIC_Send_Byte(0x00);//����CLKOUT�������Ϊ���迹
	if(IIC_Wait_Ack()==HAL_ERROR)//�ȴ�I2C���߻ظ�
	{
		
		return HAL_ERROR;
	}
	IIC_Send_Byte(0x82);//��ʱ����Ч��Ƶ����Ϊ1Hz
	if(IIC_Wait_Ack()==HAL_ERROR)//�ȴ�I2C���߻ظ�
	{
		
		return HAL_ERROR;
	}
	IIC_Send_Byte(0x01);//��ʱ����Ч��Ƶ����Ϊ1Hz
	if(IIC_Wait_Ack()==HAL_ERROR)//�ȴ�I2C���߻ظ�
	{
		
		return HAL_ERROR;
		
	}
	IIC_Stop(); //I2C����ֹͣ
	return HAL_OK;
}


//����SDAΪ���ģʽ
void SDA_OUT(void)
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
void SDA_IN(void)
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


void I2C_delay(void)  //PCF8563��I2C����Ƶ��Ϊ400kHz�����ݲ�ͬ�������趨�ʵ��ȴ���
{
	unsigned char i=5;
	while(i)
	{
		i--;
	}
}

//����IIC��ʼ�ź�
void IIC_Start(void)
{
	SDA_OUT();     //sda�����
	IIC_SDA_H;	  	  
	IIC_SCL_H;
	I2C_delay();
 	IIC_SDA_L;//START:when CLK is high,DATA change form high to low 
	I2C_delay();
	IIC_SCL_L;//ǯסI2C���ߣ�׼�����ͻ�������� 
	I2C_delay();
}	  
//����IICֹͣ�ź�
void IIC_Stop(void)
{
	SDA_OUT();     //sda�����
	IIC_SCL_H;
	IIC_SDA_L;
 	I2C_delay();
	IIC_SDA_H;
	I2C_delay();	
  IIC_SCL_L; 	
	I2C_delay();
}
//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
HAL_StatusTypeDef IIC_Wait_Ack(void)
{
	SDA_IN();      //SDA����Ϊ����  
	IIC_SCL_L;
 	I2C_delay();
	IIC_SDA_H;//�ڱ�־λʱ���������ʱ SDA ��Ӧ���ֵ͵�ƽ
 	I2C_delay();   
	IIC_SCL_H;
	I2C_delay();
	if(SSDA)//���SDA�ĵ�ƽΪ�ͣ��ʹ�����մ�PCF8563������Ӧ���ź�
	{
		I2C_delay();  
		IIC_SCL_L;
		return HAL_ERROR;
	}
	I2C_delay();  
	IIC_SCL_L;//ʱ�����0 	   
	return HAL_OK;;  
} 
//����ACKӦ��
void IIC_Ack(void)
{
	SDA_OUT();     //sda�����
	IIC_SCL_L;
	IIC_SDA_L;
	I2C_delay();
	IIC_SCL_H;
	I2C_delay();
	IIC_SCL_L;
}
//������ACKӦ��		    
void IIC_NAck(void)
{
	SDA_OUT();     //sda�����
	IIC_SCL_L;
	IIC_SDA_H;//
	I2C_delay();
	IIC_SCL_H;
	I2C_delay();
	IIC_SCL_L;
}					 				     
//IIC����һ���ֽ�
//���شӻ�����Ӧ��
//1����Ӧ��
//0����Ӧ��			  
unsigned char IIC_Send_Byte(unsigned char RTC_Cmd)
{                        
  unsigned char i=8;  
SDA_OUT();     //sda�����  
  while(i--)
	{
		IIC_SCL_L;
		if(RTC_Cmd&0x80)
		{
			IIC_SDA_H;
		}
		else
		{
			IIC_SDA_L;
		}
		RTC_Cmd<<=1;
		I2C_delay();
		IIC_SCL_H;
		I2C_delay();
		IIC_SCL_L;
		I2C_delay();
	}
	IIC_SCL_L;
	return SSDA;//ÿ���յ�8λ����λ��PCF8563����һ��Ӧ���źţ���SDA���յ�
   	 
} 	    
//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK   
unsigned char IIC_Read_Byte(void)
{
	unsigned char  i=8;
	unsigned char Read_Byte=0;
	SDA_IN();//SDA����Ϊ����
  IIC_SDA_H;
	while(i--)
	{
		Read_Byte<<=1;
		IIC_SCL_L;
		I2C_delay();
		IIC_SCL_H;
		I2C_delay();
		if(SSDA)//���SDA�Ž��յ��ߵ�ƽ
		{
			Read_Byte|=0x01;
		}
	}
	IIC_SCL_L;
	return Read_Byte;
	
}



HAL_StatusTypeDef PCF8563_Set_Time(uint8_t year,uint8_t month,uint8_t day,uint8_t hour,uint8_t minute,uint8_t second)
{
	//��������������Чλ
	if (year > 99)      year    = 0;  //�ָ�00��
	if (month > 12)     month   = 1;  //�ָ�1��
	if (day > 31)       day     = 1;  //�ָ�1��
	if (hour > 23)      hour    = 0;  //�ָ�0ʱ
	if (minute > 59)    minute  = 0;  //�ָ�0����
	if (second > 59)    second  = 0;  //�ָ�0��
	PCF_DataStruct_Time.RTC_Year   =  RTC_ByteToBcd2(year);
	PCF_DataStruct_Time.RTC_Month  =  RTC_ByteToBcd2(month);
	PCF_DataStruct_Time.RTC_Day    =  RTC_ByteToBcd2(day);
	PCF_DataStruct_Time.RTC_Hour   =  RTC_ByteToBcd2(hour);
	PCF_DataStruct_Time.RTC_Minute =  RTC_ByteToBcd2(minute);
	PCF_DataStruct_Time.RTC_Second =  RTC_ByteToBcd2(second);
	IIC_Start(); //I2C��������
	IIC_Send_Byte(0xA2);//����д����
	if(IIC_Wait_Ack()==HAL_ERROR)
  {
		//printf("û�еȵ�Ӧ���ź�1\r\n");
		return  HAL_ERROR;
	}//�ȴ�I2C���߻ظ�
	
	IIC_Send_Byte(0x02);//ѡ����Ĵ�����ַ0x02
	if(IIC_Wait_Ack()==HAL_ERROR)
  {
		//printf("û�еȵ�Ӧ���ź�2\r\n");
		return  HAL_ERROR;
	}//�ȴ�I2C���߻ظ�
	
	IIC_Send_Byte(PCF_DataStruct_Time.RTC_Second);//д����  ��0x02��
	if(IIC_Wait_Ack()==HAL_ERROR)
  {
		//printf("û�еȵ�Ӧ���ź�3\r\n");
		return  HAL_ERROR;
	}//�ȴ�I2C���߻ظ�
	
	IIC_Send_Byte(PCF_DataStruct_Time.RTC_Minute);//д���  ��0x03��
	if(IIC_Wait_Ack()==HAL_ERROR)
  {
		//printf("û�еȵ�Ӧ���ź�4\r\n");
		return  HAL_ERROR;
	}//�ȴ�I2C���߻ظ�
	
	IIC_Send_Byte(PCF_DataStruct_Time.RTC_Hour);//д��ʱ  ��0x04��
	if(IIC_Wait_Ack()==HAL_ERROR)
  {
		//printf("û�еȵ�Ӧ���ź�5\r\n");
		return  HAL_ERROR;
	}//�ȴ�I2C���߻ظ�
	
	IIC_Send_Byte(PCF_DataStruct_Time.RTC_Day);//д����  ��0x05��
	if(IIC_Wait_Ack()==HAL_ERROR)
  {
		//printf("û�еȵ�Ӧ���ź�6\r\n");
		return  HAL_ERROR;
	}//�ȴ�I2C���߻ظ�
	
	IIC_Send_Byte(3);//д������  ��0x06��
	if(IIC_Wait_Ack()==HAL_ERROR)
  {
		//printf("û�еȵ�Ӧ���ź�7\r\n");
		return  HAL_ERROR;
	}//�ȴ�I2C���߻ظ�
	
	IIC_Send_Byte(PCF_DataStruct_Time.RTC_Month);//д����  ��0x07��
	if(IIC_Wait_Ack()==HAL_ERROR)
  {
		//printf("û�еȵ�Ӧ���ź�8\r\n");
		return  HAL_ERROR;
	}//�ȴ�I2C���߻ظ�
	
	IIC_Send_Byte(PCF_DataStruct_Time.RTC_Year);//д����  ��0x08��
	if(IIC_Wait_Ack()==HAL_ERROR)
  {
		//printf("û�еȵ�Ӧ���ź�9\r\n");
		return  HAL_ERROR;
	}//�ȴ�I2C���߻ظ�
	
	IIC_Stop();
	return HAL_OK;
}

HAL_StatusTypeDef PCF8563_Read_Time(void)
{
	IIC_Start(); //I2C��������
	IIC_Send_Byte(0xA2);//����д����
	if(IIC_Wait_Ack()==HAL_ERROR)
	{
		//printf("û�еȵ�Ӧ���ź�10\r\n");
		return HAL_ERROR;
	}
	//IIC_Wait_Ack();
	IIC_Send_Byte(0x02);//������ʼ��ַ
	if(IIC_Wait_Ack()==HAL_ERROR)
	{
		//printf("û�еȵ�Ӧ���ź�11\r\n");
		return HAL_ERROR;
	}
	IIC_Start();
	IIC_Send_Byte(0xA3);//���Ͷ�����
	if(IIC_Wait_Ack()==HAL_ERROR)
	{
		//printf("û�еȵ�Ӧ���ź�12\r\n");
		return HAL_ERROR;
	}
	Time_Buffer[0]=IIC_Read_Byte();//��ȡ��
	IIC_Ack();//�������ݵ�ʱ��Ҫ����Ӧ���źţ���PCF8563���յ���ȷ���ѽ��յ�����
	
	Time_Buffer[1]=IIC_Read_Byte();//��ȡ����
	IIC_Ack();
	
	Time_Buffer[2]=IIC_Read_Byte();//��ȡСʱ
	IIC_Ack();
	
	Time_Buffer[3]=IIC_Read_Byte();//��ȡ��
	IIC_Ack();
	
	Time_Buffer[4]=IIC_Read_Byte();//��ȡ����
	IIC_Ack();
	
	Time_Buffer[5]=IIC_Read_Byte();//��ȡ��
	IIC_Ack();
	
	Time_Buffer[6]=IIC_Read_Byte();//��ȡ��
	IIC_NAck();
	IIC_Stop();
	
	//���β���
	Time_Buffer[6] &= 0xff;//��
	Time_Buffer[5] &= 0x1f;//��
	Time_Buffer[3] &= 0x3f;//��
	Time_Buffer[2] &= 0x3f;//ʱ
	Time_Buffer[1] &= 0x7f;//��
	Time_Buffer[0] &= 0x7f;//��
	//ת����ʮ����
	PCF_DataStruct_Time.RTC_Year   =  RTC_Bcd2ToByte(Time_Buffer[6]);//��
	PCF_DataStruct_Time.RTC_Month  =  RTC_Bcd2ToByte(Time_Buffer[5]);//��
	PCF_DataStruct_Time.RTC_Day    =  RTC_Bcd2ToByte(Time_Buffer[3]);//��
	PCF_DataStruct_Time.RTC_Hour   =  RTC_Bcd2ToByte(Time_Buffer[2]);//ʱ
	PCF_DataStruct_Time.RTC_Minute =  RTC_Bcd2ToByte(Time_Buffer[1]);//��
	PCF_DataStruct_Time.RTC_Second =  RTC_Bcd2ToByte(Time_Buffer[0]);//��
	return HAL_OK;
}

