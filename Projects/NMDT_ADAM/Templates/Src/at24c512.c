#include "at24c512.h"
#include "at_iic.h"

/******************************************************************************
                            ������صı�������
******************************************************************************/

//unsigned char buffer[4];



/**
  *****************************************************************************
  * @Name   : ��FM24C256дһ���ֽ�����
  *
  * @Brief  : none
  *
  * @Input  : FM_ADD��FM24C256�ĵ�ַ��д����   0xa4����0xa0
	            RAM_ADD:�ڴ��ַ  ��Χ0x0000---0x7fff
  *           dat��    Ҫд�������
  *
  * @Output : none
  *
  * @Return : none
  *****************************************************************************
**/
void FM24C256_Write_Byte(uint8_t FM_ADD,uint16_t RAM_ADD, char dat)
{
	FM_IIC_Start();                //����IIC����
	FM_IIC_Send_Byte(FM_ADD);      //����д�����Լ�������ѡ��  0xa0=����1��д  0xa4����2��д
	FM_IIC_Wait_Ack();               //�ȴ�Ӧ��
	FM_IIC_Send_Byte(RAM_ADD>>8);  //�����ڴ��ַ�ĸ�8λ
	FM_IIC_Wait_Ack();
	FM_IIC_Send_Byte(RAM_ADD%256); //�����ڴ��ַ�ĵ�8λ
	FM_IIC_Wait_Ack();
	FM_IIC_Send_Byte(dat);         //����һ�ֽ�����
	FM_IIC_Wait_Ack(); 
	FM_IIC_Stop();                //ֹͣ  
	HAL_Delay(10);
}


/**
  *****************************************************************************
  * @Name   : ��AT24C512�����һ���ֽ�����
  *
  * @Brief  : none
  *
  * @Input  : FM_ADD��FM24C256�ĵ�ַ��д����   0xa0����0xa4
	            RAM_ADD:�ڴ��ַ  ��Χ0x0000---0x7fff
  *           
  *
  * @Output : none
  *
  * @Return : dat��    ����������
  *****************************************************************************
**/
uint8_t FM24C256_Read_Byte(uint8_t FM_ADD,uint16_t RAM_ADD)
{
	uint8_t temp=0;
	
	FM_IIC_Start();                //����IIC����
	FM_IIC_Send_Byte(FM_ADD);      //���Ͷ������Լ�������ѡ��  0xa0=����1��д  0xa4����2��д
	FM_IIC_Wait_Ack();               //�ȴ�Ӧ��
	FM_IIC_Send_Byte(RAM_ADD>>8);  //�����ڴ��ַ�ĸ�8λ
	FM_IIC_Wait_Ack();
	FM_IIC_Send_Byte(RAM_ADD%256); //�����ڴ��ַ�ĵ�8λ
	FM_IIC_Wait_Ack();
	FM_IIC_Start();                //����IIC����
	FM_IIC_Send_Byte(FM_ADD+1);      //���Ͷ�ָ��   0xa1=0xa0+1����1�Ķ�   0xa5=0x24+1����2�Ķ� 
	FM_IIC_Wait_Ack();
	temp=FM_IIC_Read_Byte(0);       //������
	FM_IIC_Stop();                //ֹͣ  
	return temp;                  //�����������ݷ���
}

/**
  *****************************************************************************
  * @Name   : ��AT24C512��д��N���ֽ�   Page Write ҳд
  *
  * @Brief  : none
  *
  * @Input  : FM_ADD��FM24C256�ĵ�ַ��д����   0xa0����0xa4
	            RAM_ADD:��ʼд����ڴ��ַ  ��Χ0x0000---0x7fff
  *           dat��    Ҫд�������16λ����32λ
*             len:     Ҫд�����ݵĳ��� 2��4
  * @Output : none
  *
  * @Return : none
  *****************************************************************************
**/
void FM24C256_Write_NByte(uint8_t FM_ADD,uint16_t RAM_ADD,uint8_t n, char nBuffer[n])
{
	uint8_t i;
  FM_IIC_Start();                //����IIC����
	FM_IIC_Send_Byte(FM_ADD);      //����д�����Լ�������ѡ��  0xa0=����1��д  0xa4����2��д
	FM_IIC_Wait_Ack();               //�ȴ�Ӧ��
	FM_IIC_Send_Byte(RAM_ADD>>8);  //�����ڴ��ַ1�ĸ�8λ
	FM_IIC_Wait_Ack();
	FM_IIC_Send_Byte(RAM_ADD%256); //�����ڴ��ַ1�ĵ�8λ
	FM_IIC_Wait_Ack();
	for(i=0;i<n;i++)
	{
		FM_IIC_Send_Byte(nBuffer[i]);         //����һ�ֽ�����
		FM_IIC_Wait_Ack(); 
		
	}
	FM_IIC_Stop();                //ֹͣ  
	HAL_Delay(10);
}


/**
  *****************************************************************************
  * @Name   : ��FM24C256������M���ֽ�����  Sequential   Read
  *
  * @Brief  : none
  *
  * @Input  : FM_ADD��FM24C256�ĵ�ַ��д����   0xa7����0xa3
	            RAM_ADD:��ʼ�������ڴ��ַ  ��Χ0x0000---0x7fff
  *           dat��    Ҫ����������16λ����32λ
*             len:     Ҫд�����ݵĳ��� 2��4
  * @Output : none
  *
  * @Return : none
  *****************************************************************************
**/
void FM24C256_Read_MByte(uint8_t FM_ADD,uint16_t RAM_ADD,uint8_t m,char *nBuffer)
{
	uint8_t i;
	FM_IIC_Start();                //����IIC����
	FM_IIC_Send_Byte(FM_ADD+1);      //���Ͷ�ָ��   0xa1=0xa0+1����1�Ķ�   0xa5=0xa4+1����2�Ķ� 
	FM_IIC_Wait_Ack();
	for(i=0;i<m;i++)
	{
		*nBuffer=FM_IIC_Read_Byte(0);       //������(FM_ADD);		//������
		FM_IIC_Wait_Ack();
		FM_ADD++;
		nBuffer++;
		
	}
	FM_IIC_NAck();
	FM_IIC_Stop(); 		//ֹͣ  
	
}
/**
  *****************************************************************************
  * @Name   : ���FM24C256�Ƿ�����
  *
  * @Brief  : none
  *
  * @Input  : FM_ADD��FM24C256�ĵ�ַ��д����   0xa7����0xa3
	            RAM_DD: �����һ����ַ0x7fff�洢0xff���ݣ�Ȼ���ڶ�����ַ
                      0x7fff�����ݺ�0xff�Ա�
  * @Output : none
  *
  * @Return : 3���ڴ涼�ɹ���2�ڴ�1�ɹ����ڴ�2ʧ�ܣ�  1�ڴ�2�ɹ����ڴ�1ʧ�ܣ�0��ʧ��
  *****************************************************************************
**/
uint8_t FM24C256_Check(void)
{
	uint8_t temp,data;
	temp=FM24C256_Read_Byte(0xa0,0x7fff);
	data=FM24C256_Read_Byte(0xa4,0x7fff);
	if(temp==0xff&data==0xff)
		return 1;
	else
	{
		FM24C256_Write_Byte(0xa0,0x7fff,0xff);
		FM24C256_Write_Byte(0xa4,0x7fff,0xff);
		
		temp=FM24C256_Read_Byte(0xa0,0x7fff);
		data=FM24C256_Read_Byte(0xa4,0x7fff);
		if(temp==0xff&data==0xff)
			return 1;
		else 
				return 0;
		}
	return 1;

}


//��AT24CXX�����ָ����ַ��ʼ����ָ������������
//ReadAddr :��ʼ�����ĵ�ַ ��24c02Ϊ0~255
//pBuffer  :���������׵�ַ
//NumToRead:Ҫ�������ݵĸ���
void AT24C512_Read(uint8_t FM_ADD,uint16_t ReadAddr,uint8_t *pBuffer,uint16_t NumToRead)
{
	while(NumToRead)
	{
		*pBuffer++=FM24C256_Read_Byte(FM_ADD,ReadAddr++);	
		NumToRead--;
	}
}  
//��AT24CXX�����ָ����ַ��ʼд��ָ������������
//WriteAddr :��ʼд��ĵ�ַ ��24c02Ϊ0~255
//pBuffer   :���������׵�ַ
//NumToWrite:Ҫд�����ݵĸ���
void AT24C512_Write(uint16_t FM_ADD,uint16_t WriteAddr,uint8_t *pBuffer,uint16_t NumToWrite)
{
	while(NumToWrite--)
	{
		FM24C256_Write_Byte(FM_ADD,WriteAddr,*pBuffer);
		//WriteAddr++;
		pBuffer++;
	}
}


