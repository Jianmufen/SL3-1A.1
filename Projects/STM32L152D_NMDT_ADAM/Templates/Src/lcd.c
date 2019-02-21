#include "lcd.h"
#include "lcdfont.h"
#include "rtc.h"
#include "stdio.h"
#include "main.h"

//static void GET_RTC_Time(void);
//static char disp_buf[128];//��ʾ����
//OLED���Դ�
//��Ÿ�ʽ����.
//[0]0 1 2 3 ... 127	
//[1]0 1 2 3 ... 127	
//[2]0 1 2 3 ... 127	
//[3]0 1 2 3 ... 127	
//[4]0 1 2 3 ... 127	
//[5]0 1 2 3 ... 127	
//[6]0 1 2 3 ... 127	
//[7]0 1 2 3 ... 127 		   
uint8_t OLED_GRAM[132][8];   //����һ������  ��д����  �ٰ�����д��LCD
uint8_t number=0;
uint8_t number1=0;
uint8_t number2=0;
uint8_t number3=0;
uint8_t number4=0;
uint8_t number5=0;

 uint8_t a1=0,a2=0,a3=0,a4=0,a5=0,a6=0,a7=0,a8=0;
 uint8_t b1=0,b2=0,b3=0,b4=0,b5=0,b6=0,b7=0;
 uint8_t c1=0,c2=0,c3=0,c4=0,c5=0,c6=0,c7=0;
 uint8_t d4=0,e4=0;
 
char disp_buf[128];
///* RTC Time*/
//static RTC_TimeTypeDef Time;
//static RTC_DateTypeDef Date;

///**************************ʵ�ֺ���******************************************** 
//*����ԭ��:		void GET_RTC_Time(void) 
//*��������:		�õ�RTC��ʱ��
//*******************************************************************************/ 
//void GET_RTC_Time(void)
//{
//	RTC_DateTypeDef sdatestructureget;
//  RTC_TimeTypeDef stimestructureget;
//  /* ��ȡ��ǰʱ�� */
//  HAL_RTC_GetTime(&hrtc, &stimestructureget, RTC_FORMAT_BIN);
//  /* ��ȡ��ǰ���� */
//  HAL_RTC_GetDate(&hrtc, &sdatestructureget, RTC_FORMAT_BIN);
//	
//}



//�����Դ浽LCD		 
void OLED_Refresh_Gram(void)
{
	uint8_t i,n;		    
	for(i=0;i<8;i++)  
	{  
		OLED_W_Command (0xb0+i);    //����ҳ��ַ��0~7��
		OLED_W_Command (0x00);      //������ʾλ�á��е͵�ַ
		OLED_W_Command (0x10);      //������ʾλ�á��иߵ�ַ   
		for(n=0;n<132;n++)
		OLED_W_Data(OLED_GRAM[n][i]); 
	}   
}

/**************************ʵ�ֺ���******************************************** 
*����ԭ��:		unsigned char OLED_W_Command(void); 
*��������:		д���OLED�� 
*******************************************************************************/ 
void OLED_W_Command(unsigned char com)
{
	unsigned char j;
	OLED_CS(1);
  for(j=0;j<3;j++);
	OLED_DC(0);
	OLED_Read(1);
	OLED_Write(0);
	com=com&0xFF;
	DATAOUT(com);
  OLED_CS(0);
	for(j=0;j<3;j++);
	OLED_CS(1);
	
} 	    

/**************************ʵ�ֺ���******************************************** 
*����ԭ��:		void OLED_W_Data(unsigned dat); 
*��������:		д���ݵ�oled��ʾ���� 
*******************************************************************************/ 
void OLED_W_Data(unsigned char dat)
{
	unsigned char j;
	OLED_CS(1);
	for(j=0;j<3;j++);
	OLED_DC(1);
	OLED_Read(1);
	OLED_Write(0);
	dat=dat&0xff;
	DATAOUT(dat)
	OLED_CS(0);
	for(j=0;j<3;j++);
	OLED_CS(1);
} 	    


/**************************ʵ�ֺ���******************************************** 
*����ԭ��:		void OLED_Clear(void) 
*��������:		���� 
*******************************************************************************/ 
void OLED_Clear(void) 
{ 
	uint8_t i,n; 
	for(i=0;i<8;i++) 
     for(n=0;n<132;n++)
        OLED_GRAM[n][i]=0X00;
   	OLED_Refresh_Gram();//������ʾ
} 


/**************************ʵ�ֺ���******************************************** 
*����ԭ��:		void OLED_DrawPoint(uint8_t x,uint8_t y,uint8_t t) 
*��������:		��x�� ��y�л�һ����
*��ڲ�����x:0~~127��y:0~~63;t:1��䣬0���
*******************************************************************************/ 
void OLED_DrawPoint(unsigned char x,unsigned char y,unsigned char t) 
{ 
	uint8_t pos,bx,temp=0;
	x=x&0x7f;   //��������  ����x>127  y>63
	y=y&0x3f;
	pos=y/8;   //��Ӧ��OLED_GRAM�е�ҳ
	bx=y%8;
	temp=1<<(7-bx);
	if(t)
		OLED_GRAM[x][pos]|=temp;
	else 
		OLED_GRAM[x][pos]&=~temp;	    
} 


/**************************ʵ�ֺ���******************************************** 
*����ԭ��:		void OLED_Char(uint8_t x,uint8_t y,unsigned char chr)
*��������:		�ڶ���дһ��8*16��ASCII��
*��ڲ���:x��(0<x<127)  yҳ��0<y<63��  i=1������ʾ��i=0����
*******************************************************************************/ 
void OLED_Char(unsigned char x,unsigned char y,unsigned char chr,uint8_t i)
{
	uint8_t t,temp,t1;
	uint8_t y0=y;
	x=x&0x7f;   //��������  ����x>127  y>63
	y=y&0x3f;
	chr=chr-' ';//�õ�ƫ�ƺ��ֵ
	for(t=0;t<16;t++)             
	{
		temp=asc2_1608[chr][t];
		for(t1=0;t1<8;t1++)
		{
			if(i==1)
			{
			if(temp&0x01)
				OLED_DrawPoint(x,y,1);
			else 
				OLED_DrawPoint(x,y,0);
			temp>>=1;
			y++;
			if((y-y0)==16)
			{
				y=y0;
				x++;
				break;
			}
		}
			else
			{
				if(temp&0x01)
				OLED_DrawPoint(x,y,0);
			else 
				OLED_DrawPoint(x,y,1);
			temp>>=1;
			y++;
			if((y-y0)==16)
			{
				y=y0;
				x++;
				break;
			}
				
			}

	}
} 	    
}

/**************************ʵ�ֺ���******************************************** 
*����ԭ��:		void OLED_ShowString(uint8_t x,uint8_t y,const uint8_t *p)
*��������:		��ʾ�ַ���
*��ڲ���:x��(0<x<127)  yҳ��0<y<63��    i=1������ʾ��i=0����
*******************************************************************************/ 
void OLED_ShowString(unsigned char x,unsigned char y,const char *p,uint8_t i)
{	
    while((*p<='~')&&(*p>=' '))//�ж��ǲ��ǷǷ��ַ�!
    {       
        if(x>120)
        {
					x=0;
					y+=16;
				}
        if(y>48)
        {
				  y=x=0;
					OLED_Clear();
				}
        OLED_Char(x,y,*p,i);	 
        x+=8;
        p++;
    }  
	
}	   


/**************************ʵ�ֺ���******************************************** 
*����ԭ��:		void OLED_China(uint8_t x,uint8_t y,unsigned char cha)
*��������:		�ڶ���дһ��16*16�ĺ���
*��ڲ���:x��(0<x<127)  yҳ��0<y<63��    i=1������ʾ��i=0����
*******************************************************************************/ 
void OLED_China(unsigned char x,unsigned char y,unsigned char n,uint8_t i)
{
	uint8_t t,t1,temp;
	uint8_t y0=y;
	x=x&0x7f;   //��������  ����x>127  y>63
	y=y&0x3f;
//	chr=chr-'��';//�õ�ƫ�ƺ��ֵ
	for(t=0;t<32;t++)             
	{
		temp=china_1616[n][t];
		for(t1=0;t1<8;t1++)    //���㺯��8����һ��
		{
			if(i==1)
			{
			  if(temp&0x01)
				  OLED_DrawPoint(x,y,1);
			  else 
				  OLED_DrawPoint(x,y,0);
			  temp>>=1;
			  y++;
			  if((y-y0)==16)
			  {
				  y=y0;
				  x++;
				  break;
			  }
		  }
			else
			{
				if(temp&0x01)
				  OLED_DrawPoint(x,y,0);
			  else 
			 	  OLED_DrawPoint(x,y,1);
			  temp>>=1;
			  y++;
			  if((y-y0)==16)
			  {
				  y=y0;
				  x++;
				  break;
		  	}
			}
		}
	}
} 	    

/**************************ʵ�ֺ���******************************************** 
*����ԭ��:		void OLED_China(uint8_t x,uint8_t y,uint8_t i)
*��������:		�ڶ���дһ��24*24�ĺ���
*��ڲ���:x��(0<x<127)  yҳ��0<y<63��
*******************************************************************************/ 
void OLED_China1(uint8_t x,uint8_t y,uint8_t i)
{
	uint8_t t,t1,temp;
	uint8_t y0=y;
	x=x&0x7f;   //��������  ����x>127  y>63
	y=y&0x3f;
//	chr=chr-'��';//�õ�ƫ�ƺ��ֵ
	for(t=0;t<72;t++)             
	{
		temp=china_2424[i][t];
		for(t1=0;t1<8;t1++)    //���㺯��8����һ��
		{
			if(temp&0x01)
				OLED_DrawPoint(x,y,1);
			else 
				OLED_DrawPoint(x,y,0);
			temp>>=1;
			y++;
			if((y-y0)==24)
			{
				y=y0;
				x++;
				break;
			}
		}
	}
} 	    


/**************************ʵ�ֺ���******************************************** 
*����ԭ��:		void OLED_Huatu(uint8_t x,uint8_t y,uint8_t i)
*��������:		�ڶ��㻭һ��ͼ��
*��ڲ���:x��(0<x<127)  yҳ��0<y<63��
*******************************************************************************/ 
void OLED_Huatu(uint8_t x,uint8_t y,uint8_t i)
{
	uint8_t t,t1,temp;
	uint8_t y0=y;
	x=x&0x7f;   //��������  ����x>127  y>63
	y=y&0x3f;
//	chr=chr-'��';//�õ�ƫ�ƺ��ֵ
	for(t=0;t<64;t++)             
	{
		temp=DianLiang[i][t];
		for(t1=0;t1<8;t1++)    //���㺯��8����һ��
		{
			if(temp&0x01)
				OLED_DrawPoint(x,y,1);
			else 
				OLED_DrawPoint(x,y,0);
			temp>>=1;
			y++;
			if((y-y0)==16)
			{
				y=y0;
				x++;
				break;
			}
		}
	}
} 	    

/**************************ʵ�ֺ���******************************************** 
*����ԭ��:		void OLED_OpenMenu(void)
*��������:		�����˵�
*��ڲ���:��
*******************************************************************************/ 
void OLED_OpenMenu(void)
{
	  OLED_China1(17,8,0);
		OLED_China1(41,8,1);
		OLED_China1(65,8,2);
		OLED_China1(89,8,3);
	
	  OLED_China(1,40,0,1);
		OLED_China(17,40,1,1);
		OLED_China(33,40,2,1);
		OLED_China(49,40,3,1);
		OLED_China(65,40,4,1);
		OLED_China(81,40,5,1);
		OLED_China(97,40,6,1);
		OLED_China(113,40,7,1);
	
		OLED_Refresh_Gram();		//������ʾ��OLED 
} 	    


/**************************ʵ�ֺ���******************************************** 
*����ԭ��:		void OLED_ZhuMenu(void)
*��������:		���˵�
*��ڲ���:��number
*******************************************************************************/ 
void OLED_ZhuMenu(void)
{
	OLED_China(17,0,8,1);//��
  OLED_China(49,0,9,1);//��
  OLED_China(81,0,10,1);//��
	OLED_ShowString(104,0,"///",1);

	OLED_ShowString(8,16,"00",1);//ʱ
	OLED_Char(24,16,':',1);
	OLED_ShowString(32,16,"00",1);//��
	OLED_Char(48,16,':',1);
	OLED_ShowString(56,16,"00",1);//��
	OLED_ShowString(96,16,"S00M",1);
	
	if(number==0)
	{
	OLED_China(48,32,11,1);//ƽ��
	OLED_China(64,32,12,1);
	
	OLED_China(16,48,13,0);//˲ʱ
	OLED_China(32,48,14,0);
	OLED_China(80,48,15,1);//����
	OLED_China(96,48,16,1);
	}
	else if(number==1)
	{
		OLED_China(48,32,11,1);//ƽ��
	OLED_China(64,32,12,1);
	
	OLED_China(16,48,13,1);//˲ʱ
	OLED_China(32,48,14,1);
	OLED_China(80,48,15,0);//����
	OLED_China(96,48,16,0);
	}
	else if(number==2)
	{
		OLED_China(48,32,11,0);//ƽ��
	OLED_China(64,32,12,0);
	
	OLED_China(16,48,13,1);//˲ʱ
	OLED_China(32,48,14,1);
	OLED_China(80,48,15,1);//����
	OLED_China(96,48,16,1);
	}
	
	OLED_Refresh_Gram();		//������ʾ��OLED 	
} 	    

/**************************ʵ�ֺ���******************************************** 
*����ԭ��:		void OLED_DataMenu(void)
*��������:		�¶Ȳ˵�
*��ڲ���:��number1
*******************************************************************************/ 
void OLED_DataMenu(void)
{
	if(number1==0)
	{
	OLED_China(1,16,17,1);//��
	OLED_China(17,16,18,1);//��
	OLED_Char(33,16,':',1);
	OLED_ShowString(40,16,"     //./",1);
	OLED_China(113,16,61,1);//��
	
	OLED_China(1,32,19,1);//ʪ
	OLED_China(17,32,20,1);//��
	OLED_Char(33,32,':',1);
	OLED_ShowString(40,32,"    /// %RH",1);
	
	OLED_China(1,48,17,1);//��
	OLED_China(17,48,21,1);//ѹ
	OLED_Char(33,48,':',1);
	OLED_ShowString(40,48,"  ////./hpa",1);
	}
	else if(number1==1)
	{
		OLED_China(1,16,22,1);//��
	OLED_China(17,16,23,1);//��
	OLED_China(33,16,23,1);//��
	OLED_China(49,16,24,1);//��
	OLED_Char(65,16,':',1);
	OLED_ShowString(73,16,"  ///",1);
	OLED_China(113,16,62,1);//��
	
  OLED_China(1,32,22,1);//��
	OLED_China(17,32,23,1);//��
	OLED_China(33,32,23,1);//��
	OLED_China(49,32,25,1);//��
	OLED_Char(65,32,':',1);
	OLED_ShowString(72,32,"//./m/s",1);
	
	OLED_China(1,48,26,1);//��
	OLED_ShowString(17,48,"    ",1);
	OLED_China(49,48,18,1);//��
	OLED_Char(65,48,':',1);
	OLED_ShowString(73,48," //./",1);
	OLED_China(113,48,61,1);//��
	}
	else if(number1==2)
	{
		OLED_China(1,16,27,1);//��
	OLED_China(17,16,28,1);//��
	OLED_China(33,16,29,1);//��
	OLED_Char(49,16,':',1);
	OLED_ShowString(56,16,"    0.0mm",1);

  OLED_China(1,32,14,1);//ʱ
	OLED_China(17,32,28,1);//��
	OLED_China(33,32,29,1);//��
	OLED_Char(49,32,':',1);
	OLED_ShowString(56,32,"    0.0mm",1);
	
	OLED_China(1,48,10,1);//��
	OLED_China(17,48,28,1);//��
	OLED_China(33,48,29,1);//��
	OLED_Char(49,48,':',1);
	OLED_ShowString(56,48,"    0.0mm",1);
	}
	
	OLED_Refresh_Gram();		//������ʾ��OLED 
} 	    

/**************************ʵ�ֺ���******************************************** 
*����ԭ��:		void OLED_AverageMenu(void)
*��������:		ƽ���˵�
*��ڲ���:��
*******************************************************************************/ 
void OLED_AverageMenu(void)
{
	OLED_China(1,0,59,1);//��
	OLED_China(17,0,27,1);//��
	OLED_China(33,0,23,1);//��
	OLED_China(49,0,24,1);//��
	OLED_Char(65,0,':',1);
	OLED_ShowString(88,0,"///",1);
	OLED_China(112,0,62,1);//��

	
  OLED_China(1,16,59,1);//��
	OLED_China(17,16,27,1);//��
	OLED_China(33,16,23,1);//��
	OLED_China(49,16,25,1);//��
	OLED_Char(65,16,':',1);
	OLED_ShowString(72,16,"//./m/s",1);
	
	
	OLED_China(1,32,60,1);//ʮ
	OLED_China(17,32,27,1);//��
	OLED_China(33,32,23,1);//��
	OLED_China(49,32,24,1);//��
	OLED_Char(65,32,':',1);
	OLED_ShowString(88,32,"///",1);
	OLED_China(112,32,62,1);//��
	
	OLED_China(1,48,60,1);//ʮ
	OLED_China(17,48,27,1);//��
	OLED_China(33,48,23,1);//��
	OLED_China(49,48,25,1);//��
	OLED_Char(65,48,':',1);
	OLED_ShowString(72,48,"//./m/s",1);
	
	OLED_Refresh_Gram();		//������ʾ��OLED 
} 	    


/**************************ʵ�ֺ���******************************************** 
*����ԭ��:		void OLED_SheZhiMenu(void)
*��������:		ƽ���˵�
*��ڲ���:number5
*******************************************************************************/ 
void OLED_SheZhiMenu(void)
{
	if(number5==0)
	{
		OLED_China(1,0,30,0);//��
	OLED_China(17,0,31,0);//��
	OLED_China(33,0,32,0);//��
	OLED_China(49,0,2,0);//ʽ
	OLED_ShowString(65,0,"...",1);

  OLED_China(1,16,14,1);//ʱ
	OLED_China(17,16,40,1);//��
	OLED_China(33,16,15,1);//��
	OLED_China(49,16,16,1);//��
	OLED_ShowString(65,16,"...",1);
	
	OLED_China(1,32,42,1);//Ӳ
	OLED_China(17,32,43,1);//��
	OLED_China(33,32,15,1);//��
	OLED_China(49,32,16,1);//��
	OLED_ShowString(65,32,"...",1);
	
	OLED_China(1,48,55,1);//ϵ
	OLED_China(17,48,56,1);//ͳ
	OLED_China(33,48,57,1);//��
	OLED_China(49,48,58,1);//��
	OLED_ShowString(65,48," ***",1);
	}
	else if(number5==1)
	{
		OLED_China(1,0,30,1);//��
	OLED_China(17,0,31,1);//��
	OLED_China(33,0,32,1);//��
	OLED_China(49,0,2,1);//ʽ
	OLED_ShowString(65,1,"...",1);

  OLED_China(1,16,14,0);//ʱ
	OLED_China(17,16,40,0);//��
	OLED_China(33,16,15,0);//��
	OLED_China(49,16,16,0);//��
	OLED_ShowString(65,16,"...",1);
	
	OLED_China(1,32,42,1);//Ӳ
	OLED_China(17,32,43,1);//��
	OLED_China(33,32,15,1);//��
	OLED_China(49,32,16,1);//��
	OLED_ShowString(65,32,"...",1);
	
	OLED_China(1,48,55,1);//ϵ
	OLED_China(17,48,56,1);//ͳ
	OLED_China(33,48,57,1);//��
	OLED_China(49,48,58,1);//��
	OLED_ShowString(65,48," ***",1);
	}
	else if(number5==2)
	{
		OLED_China(1,0,30,1);//��
	OLED_China(17,0,31,1);//��
	OLED_China(33,0,32,1);//��
	OLED_China(49,0,2,1);//ʽ
	OLED_ShowString(65,0,"...",1);

  OLED_China(1,16,14,1);//ʱ
	OLED_China(17,16,40,1);//��
	OLED_China(33,16,15,1);//��
	OLED_China(49,16,16,1);//��
	OLED_ShowString(65,16,"...",1);
	
	OLED_China(1,32,42,0);//Ӳ
	OLED_China(17,32,43,0);//��
	OLED_China(33,32,15,0);//��
	OLED_China(49,32,16,0);//��
	OLED_ShowString(65,32,"...",1);
	
	OLED_China(1,48,55,1);//ϵ
	OLED_China(17,48,56,1);//ͳ
	OLED_China(33,48,57,1);//��
	OLED_China(49,48,58,1);//��
	OLED_ShowString(65,48," ***",1);
	}
	else if(number5==3)
	{
		OLED_China(1,0,30,1);//��
	OLED_China(17,0,31,1);//��
	OLED_China(33,0,32,1);//��
	OLED_China(49,0,2,1);//ʽ
	OLED_ShowString(65,0,"...",1);

  OLED_China(1,16,14,1);//ʱ
	OLED_China(17,16,40,1);//��
	OLED_China(33,16,15,1);//��
	OLED_China(49,16,16,1);//��
	OLED_ShowString(65,16,"...",1);
	
	OLED_China(1,32,42,1);//Ӳ
	OLED_China(17,32,43,1);//��
	OLED_China(33,32,15,1);//��
	OLED_China(49,32,16,1);//��
	OLED_ShowString(65,32,"...",1);
	
	OLED_China(1,48,55,0);//ϵ
	OLED_China(17,48,56,0);//ͳ
	OLED_China(33,48,57,0);//��
	OLED_China(49,48,58,0);//��
	OLED_ShowString(65,48," ***",1);
	}
	OLED_Refresh_Gram();		//������ʾ��OLED 
}
/**************************ʵ�ֺ���******************************************** 
*����ԭ��:		void OLED_YingJianMenu(void)
*��������:		Ӳ���˵�
*��ڲ���:number2
*******************************************************************************/ 
void OLED_YingJianMenu(void)
{
	if(number2==0)
	{
  OLED_China(1,0,44,0);//��
	OLED_China(17,0,45,0);//ʼ
	OLED_China(33,0,42,0);//Ӳ
	OLED_China(49,0,46,0);//��
	OLED_China(65,0,47,0);//��
	OLED_China(81,0,48,0);//��
	
	OLED_China(1,16,49,1);//��
	OLED_China(17,16,50,1);//��
	OLED_China(33,16,42,1);//Ӳ
	OLED_China(49,16,46,1);//��
	OLED_China(65,16,47,1);//��
	OLED_China(81,16,48,1);//��
	
	OLED_China(1,32,51,1);//��
	OLED_China(17,32,52,1);//��
	OLED_China(33,32,53,1);//У
	OLED_China(49,32,51,1);//��
	OLED_ShowString(65,32," ***",1);
	
	OLED_China(1,48,54,1);//��
	OLED_ShowString(17,48,"PAMS",1);
	OLED_China(49,48,35,1);//��
	OLED_China(65,48,36,1);//��
	OLED_China(81,48,37,1);//��
	OLED_ShowString(97,48," ***",1);
}
	if(number2==1)
	{
  OLED_China(1,0,44,1);//��
	OLED_China(17,0,45,1);//ʼ
	OLED_China(33,0,42,1);//Ӳ
	OLED_China(49,0,46,1);//��
	OLED_China(65,0,47,1);//��
	OLED_China(81,0,48,1);//��
	
	OLED_China(1,16,49,0);//��
	OLED_China(17,16,50,0);//��
	OLED_China(33,16,42,0);//Ӳ
	OLED_China(49,16,46,0);//��
	OLED_China(65,16,47,0);//��
	OLED_China(81,16,48,0);//��
	
	OLED_China(1,32,51,1);//��
	OLED_China(17,32,52,1);//��
	OLED_China(33,32,53,1);//У
	OLED_China(49,32,51,1);//��
	OLED_ShowString(65,32," ***",1);
	
	OLED_China(1,48,54,1);//��
	OLED_ShowString(17,48,"PAMS",1);
	OLED_China(49,48,35,1);//��
	OLED_China(65,48,36,1);//��
	OLED_China(81,48,37,1);//��
	OLED_ShowString(97,48," ***",1);
}
	if(number2==2)
	{
  OLED_China(1,0,44,1);//��
	OLED_China(17,0,45,1);//ʼ
	OLED_China(33,0,42,1);//Ӳ
	OLED_China(49,0,46,1);//��
	OLED_China(65,0,47,1);//��
	OLED_China(81,0,48,1);//��
	
	OLED_China(1,16,49,1);//��
	OLED_China(17,16,50,1);//��
	OLED_China(33,16,42,1);//Ӳ
	OLED_China(49,16,46,1);//��
	OLED_China(65,16,47,1);//��
	OLED_China(81,16,48,1);//��
	
	OLED_China(1,32,51,0);//��
	OLED_China(17,32,52,0);//��
	OLED_China(33,32,53,0);//У
	OLED_China(49,32,51,0);//��
	OLED_ShowString(65,32," ***",1);
	
	OLED_China(1,48,54,1);//��
	OLED_ShowString(17,48,"PAMS",1);
	OLED_China(49,48,35,1);//��
	OLED_China(65,48,36,1);//��
	OLED_China(81,48,37,1);//��
	OLED_ShowString(97,48," ***",1);
}
	if(number2==3)
	{
  OLED_China(1,0,44,1);//��
	OLED_China(17,0,45,1);//ʼ
	OLED_China(33,0,42,1);//Ӳ
	OLED_China(49,0,46,1);//��
	OLED_China(65,0,47,1);//��
	OLED_China(81,0,48,1);//��
	
	OLED_China(1,16,49,1);//��
	OLED_China(17,16,50,1);//��
	OLED_China(33,16,42,1);//Ӳ
	OLED_China(49,16,46,1);//��
	OLED_China(65,16,47,1);//��
	OLED_China(81,16,48,1);//��
	
	OLED_China(1,32,51,1);//��
	OLED_China(17,32,52,1);//��
	OLED_China(33,32,53,1);//У
	OLED_China(49,32,51,1);//��
	OLED_ShowString(65,32," ***",1);
	
	OLED_China(1,48,54,0);//��
	OLED_ShowString(17,48,"PAMS",0);
	OLED_China(49,48,35,0);//��
	OLED_China(65,48,36,0);//��
	OLED_China(81,48,37,0);//��
	OLED_ShowString(97,48," ***",1);
}


	OLED_Refresh_Gram();		//������ʾ��OLED 
} 	    


/**************************ʵ�ֺ���******************************************** 
*����ԭ��:		void OLED_CunChuMenu(void)
*��������:		�洢�˵�
*��ڲ���:number3
*******************************************************************************/ 
void OLED_CunChuMenu(void)
{
	if(number3==0)
	{
  OLED_China(1,0,33,0);//��
	OLED_China(17,0,34,0);//��
	OLED_China(33,0,35,0);//��
	OLED_China(49,0,36,0);//��
	OLED_Char(65,0,':',1);
	OLED_ShowString(104,0,"00M",1);
	
	OLED_ShowString(1,16,"PAMS",1);
	OLED_China(33,16,35,1);//��
	OLED_China(49,16,36,1);//��
	OLED_Char(65,16,':',1);
	OLED_ShowString(104,16,"//M",1);
	
	OLED_ShowString(1,32,"PAMS",1);
	OLED_China(33,32,3,1);//��
	OLED_China(49,32,4,1);//��
	OLED_China(65,32,38,1);//��
	OLED_China(81,32,39,1);//��
	OLED_Char(97,32,':',1);
	OLED_ShowString(104,32," //",1);
	}
	if(number3==1)
	{
  OLED_China(1,0,33,1);//��
	OLED_China(17,0,34,1);//��
	OLED_China(33,0,35,1);//��
	OLED_China(49,0,36,1);//��
	OLED_Char(65,0,':',1);
	OLED_ShowString(104,0,"00M",1);
	
	OLED_ShowString(1,16,"PAMS",0);
	OLED_China(33,16,35,0);//��
	OLED_China(49,16,36,0);//��
	OLED_Char(65,16,':',1);
	OLED_ShowString(104,16,"//M",1);
	
	OLED_ShowString(1,32,"PAMS",1);
	OLED_China(33,32,3,1);//��
	OLED_China(49,32,4,1);//��
	OLED_China(65,32,38,1);//��
	OLED_China(81,32,39,1);//��
	OLED_Char(97,32,':',1);
	OLED_ShowString(104,32," //",1);
	}
	if(number3==2)
	{
  OLED_China(1,0,33,1);//��
	OLED_China(17,0,34,1);//��
	OLED_China(33,0,35,1);//��
	OLED_China(49,0,36,1);//��
	OLED_Char(65,0,':',1);
	OLED_ShowString(104,0,"00M",1);
	
	OLED_ShowString(1,16,"PAMS",1);
	OLED_China(33,16,35,1);//��
	OLED_China(49,16,36,1);//��
	OLED_Char(65,16,':',1);
	OLED_ShowString(104,16,"//M",1);
	
	OLED_ShowString(1,32,"PAMS",0);
	OLED_China(33,32,3,0);//��
	OLED_China(49,32,4,0);//��
	OLED_China(65,32,38,0);//��
	OLED_China(81,32,39,0);//��
	OLED_Char(97,32,':',1);
	OLED_ShowString(104,32," //",1);
	}
	
	OLED_Refresh_Gram();		//������ʾ��OLED 
} 

/**************************ʵ�ֺ���******************************************** 
*����ԭ��:		void OLED_DingZhengMenu(void)
*��������:		�洢�˵�
*��ڲ���:��number4
*******************************************************************************/ 
void OLED_DingZhengMenu(void)
{
	if(number4==0)
	{
		OLED_ShowString(64,0,": */./",1);
	OLED_China(1,0,17,0);//���¶���
	OLED_China(17,0,18,0);
	OLED_China(33,0,64,0);
	OLED_China(49,0,51,0);
	OLED_China(112,0,61,1);
	
	OLED_ShowString(64,16,": *//%RH",1);
	OLED_China(1,16,19,1);//ʪ�ȶ���
	OLED_China(17,16,20,1);
	OLED_China(33,16,64,1);
	OLED_China(49,16,51,1);
	
	
	OLED_ShowString(64,32,":*/./m/s",1);
	OLED_China(1,32,23,1);//���ٶ���
	OLED_China(17,32,25,1);
	OLED_China(33,32,64,1);
	OLED_China(49,32,51,1);
	
	OLED_ShowString(64,48,": */./",1);
	OLED_China(1,48,26,1);//���¶���
	OLED_China(17,48,18,1);
	OLED_China(33,48,64,1);
	OLED_China(49,48,51,1);
	if((a1!=0)||(b1!=0)||(c1!=0))
	{
		if(a1==0)
				OLED_ShowString(80,0,"+",1);
			else if(a1==1)
				OLED_ShowString(80,0,"-",1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",b1);
			OLED_ShowString(88,0,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",c1);
			OLED_ShowString(104,0,disp_buf,1);
	}
	if((a2!=0)||(b2!=0)||(c2!=0))
	{
		if(a2==0)
				OLED_ShowString(80,16,"+",1);
			else if(a2==1)
				OLED_ShowString(80,16,"-",1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",b2);
			OLED_ShowString(88,16,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",c2);
			OLED_ShowString(96,16,disp_buf,1);
	}
	if((a3!=0)||(b3!=0)||(c3!=0))
	{
		if(a3==0)
				OLED_ShowString(72,32,"+",1);
			else if(a3==1)
				OLED_ShowString(72,32,"-",1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",b3);
			OLED_ShowString(80,32,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",c3);
			OLED_ShowString(96,32,disp_buf,1);
	}
	}
	else if(number4==1)
	{
		OLED_ShowString(64,0,": */./",1);
	OLED_China(1,0,17,1);//���¶���
	OLED_China(17,0,18,1);
	OLED_China(33,0,64,1);
	OLED_China(49,0,51,1);
	OLED_China(112,0,61,1);
	
	OLED_ShowString(64,16,": *//%RH",1);
	OLED_China(1,16,19,0);//ʪ�ȶ���
	OLED_China(17,16,20,0);
	OLED_China(33,16,64,0);
	OLED_China(49,16,51,0);
	
	
	OLED_ShowString(64,32,":*/./m/s",1);
	OLED_China(1,32,23,1);//���ٶ���
	OLED_China(17,32,25,1);
	OLED_China(33,32,64,1);
	OLED_China(49,32,51,1);
	
	OLED_ShowString(64,48,": */./",1);
	OLED_China(1,48,26,1);//���¶���
	OLED_China(17,48,18,1);
	OLED_China(33,48,64,1);
	OLED_China(49,48,51,1);
	OLED_China(112,48,61,1);
	if((a1!=0)||(b1!=0)||(c1!=0))
	{
		if(a1==0)
				OLED_ShowString(80,0,"+",1);
			else if(a1==1)
				OLED_ShowString(80,0,"-",1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",b1);
			OLED_ShowString(88,0,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",c1);
			OLED_ShowString(104,0,disp_buf,1);
	}
	if((a2!=0)||(b2!=0)||(c2!=0))
	{
		if(a2==0)
				OLED_ShowString(80,16,"+",1);
			else if(a2==1)
				OLED_ShowString(80,16,"-",1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",b2);
			OLED_ShowString(88,16,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",c2);
			OLED_ShowString(96,16,disp_buf,1);
	}
	if((a3!=0)||(b3!=0)||(c3!=0))
	{
		if(a3==0)
				OLED_ShowString(72,32,"+",1);
			else if(a3==1)
				OLED_ShowString(72,32,"-",1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",b3);
			OLED_ShowString(80,32,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",c3);
			OLED_ShowString(96,32,disp_buf,1);
	}
	}
	else if(number4==2)
	{
		OLED_ShowString(64,0,": */./",1);
	OLED_China(1,0,17,1);//���¶���
	OLED_China(17,0,18,1);
	OLED_China(33,0,64,1);
	OLED_China(49,0,51,1);
	OLED_China(112,0,61,1);
	
	OLED_ShowString(64,16,": *//%RH",1);
	OLED_China(1,16,19,1);//ʪ�ȶ���
	OLED_China(17,16,20,1);
	OLED_China(33,16,64,1);
	OLED_China(49,16,51,1);
	
	
	OLED_ShowString(64,32,":*/./m/s",1);
	OLED_China(1,32,23,0);//���ٶ���
	OLED_China(17,32,25,0);
	OLED_China(33,32,64,0);
	OLED_China(49,32,51,0);
	
	OLED_ShowString(64,48,": */./",1);
	OLED_China(1,48,26,1);//���¶���
	OLED_China(17,48,18,1);
	OLED_China(33,48,64,1);
	OLED_China(49,48,51,1);
	OLED_China(112,48,61,1);
	if((a1!=0)||(b1!=0)||(c1!=0))
	{
		if(a1==0)
				OLED_ShowString(80,0,"+",1);
			else if(a1==1)
				OLED_ShowString(80,0,"-",1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",b1);
			OLED_ShowString(88,0,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",c1);
			OLED_ShowString(104,0,disp_buf,1);
	}
	if((a2!=0)||(b2!=0)||(c2!=0))
	{
		if(a2==0)
				OLED_ShowString(80,16,"+",1);
			else if(a2==1)
				OLED_ShowString(80,16,"-",1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",b2);
			OLED_ShowString(88,16,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",c2);
			OLED_ShowString(96,16,disp_buf,1);
	}
	if((a3!=0)||(b3!=0)||(c3!=0))
	{
		if(a3==0)
				OLED_ShowString(72,32,"+",1);
			else if(a3==1)
				OLED_ShowString(72,32,"-",1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",b3);
			OLED_ShowString(80,32,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",c3);
			OLED_ShowString(96,32,disp_buf,1);
	}
	
	}
	else if(number4==3)
	{
		OLED_ShowString(64,0,": */./",1);
	OLED_China(1,0,17,1);//���¶���
	OLED_China(17,0,18,1);
	OLED_China(33,0,64,1);
	OLED_China(49,0,51,1);
	OLED_China(112,0,61,1);
	
	OLED_ShowString(64,16,": *//%RH",1);
	OLED_China(1,16,19,1);//ʪ�ȶ���
	OLED_China(17,16,20,1);
	OLED_China(33,16,64,1);
	OLED_China(49,16,51,1);
	//OLED_China(112,16,61,1);
	
	OLED_ShowString(64,32,":*/./m/s",1);
	OLED_China(1,32,23,1);//���ٶ���
	OLED_China(17,32,25,1);
	OLED_China(33,32,64,1);
	OLED_China(49,32,51,1);
	
	OLED_ShowString(64,48,": */./",1);
	OLED_China(1,48,26,1);//���¶���
	OLED_China(17,48,18,0);
	OLED_China(33,48,64,0);
	OLED_China(49,48,51,0);
	OLED_China(112,48,61,0);
	
	
	}
	else if(number4==4)
	{
		OLED_ShowString(64,0,": //.///",1);
	OLED_China(1,0,17,0);//��ѹ����
	OLED_China(17,0,21,0);
	OLED_China(33,0,63,0);
	OLED_China(49,0,58,0);
	
	OLED_ShowString(64,16,":*/./hpa",1);
	OLED_China(1,16,17,1);//��ѹ����
	OLED_China(17,16,21,1);
	OLED_China(33,16,64,1);
	OLED_China(49,16,51,1);
	
	OLED_ShowString(64,32,"+//./hpa",1);
	OLED_China(1,32,63,1);//��������
	OLED_China(17,32,58,1);
	OLED_China(33,32,64,1);
	OLED_China(49,32,51,1);
	
	OLED_ShowString(64,48,":/.//hpa",1);
	OLED_China(1,48,18,1);//�¶�ϵ��
	OLED_China(17,48,20,1);
	OLED_China(33,48,64,1);
	OLED_China(49,48,55,1);
	OLED_China(112,48,58,1);
	if((a4!=0)||(b4!=0)||(c4!=0)||(d4!=0)||(e4!=0))
	{
		snprintf(disp_buf,sizeof(disp_buf),"%01u",a4);
			OLED_ShowString(80,0,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",b4);
			OLED_ShowString(88,0,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",c4);
			OLED_ShowString(104,0,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",d4);
			OLED_ShowString(112,0,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",e4);
			OLED_ShowString(120,0,disp_buf,1);
	}
	if((a5!=0)||(b5!=0)||(c5!=0))
	{
		if(a5==0)
				OLED_ShowString(72,16,"+",1);
			else if(a5==1)
				OLED_ShowString(72,16,"-",1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",b5);
			OLED_ShowString(80,16,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",c5);
			OLED_ShowString(96,16,disp_buf,1);
	}
	if((a6!=0)||(b6!=0)||(c6!=0))
	{
		snprintf(disp_buf,sizeof(disp_buf),"%01u",a6);
			OLED_ShowString(72,32,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",b6);
			OLED_ShowString(80,32,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",c6);
			OLED_ShowString(96,32,disp_buf,1);
	}
	if((a7!=0)||(b7!=0)||(c7!=0))
	{
		snprintf(disp_buf,sizeof(disp_buf),"%01u",a7);
			OLED_ShowString(72,48,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",b7);
			OLED_ShowString(88,48,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",c7);
			OLED_ShowString(96,48,disp_buf,1);
	}
	}
	else if(number4==5)
	{
			OLED_ShowString(64,0,": //.///",1);
	OLED_China(1,0,17,1);//��ѹ����
	OLED_China(17,0,21,1);
	OLED_China(33,0,63,1);
	OLED_China(49,0,58,1);
	
	OLED_ShowString(64,16,":*/./hpa",1);
	OLED_China(1,16,17,0);//��ѹ����
	OLED_China(17,16,21,0);
	OLED_China(33,16,64,0);
	OLED_China(49,16,51,0);
	
	OLED_ShowString(64,32,"+//./hpa",1);
	OLED_China(1,32,63,1);//��������
	OLED_China(17,32,58,1);
	OLED_China(33,32,64,1);
	OLED_China(49,32,51,1);
	
	OLED_ShowString(64,48,":/.//hpa",1);
	OLED_China(1,48,18,1);//�¶�ϵ��
	OLED_China(17,48,20,1);
	OLED_China(33,48,64,1);
	OLED_China(49,48,55,1);
	OLED_China(112,48,58,1);
	if((a4!=0)||(b4!=0)||(c4!=0)||(d4!=0)||(e4!=0))
	{
		snprintf(disp_buf,sizeof(disp_buf),"%01u",a4);
			OLED_ShowString(80,0,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",b4);
			OLED_ShowString(88,0,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",c4);
			OLED_ShowString(104,0,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",d4);
			OLED_ShowString(112,0,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",e4);
			OLED_ShowString(120,0,disp_buf,1);
	}
	if((a5!=0)||(b5!=0)||(c5!=0))
	{
		if(a5==0)
				OLED_ShowString(72,16,"+",1);
			else if(a5==1)
				OLED_ShowString(72,16,"-",1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",b5);
			OLED_ShowString(80,16,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",c5);
			OLED_ShowString(96,16,disp_buf,1);
	}
	if((a6!=0)||(b6!=0)||(c6!=0))
	{
		snprintf(disp_buf,sizeof(disp_buf),"%01u",a6);
			OLED_ShowString(72,32,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",b6);
			OLED_ShowString(80,32,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",c6);
			OLED_ShowString(96,32,disp_buf,1);
	}
	if((a7!=0)||(b7!=0)||(c7!=0))
	{
		snprintf(disp_buf,sizeof(disp_buf),"%01u",a7);
			OLED_ShowString(72,48,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",b7);
			OLED_ShowString(88,48,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",c7);
			OLED_ShowString(96,48,disp_buf,1);
	}
	
	}
	else if(number4==6)
	{
			OLED_ShowString(64,0,": //.///",1);
	OLED_China(1,0,17,1);//��ѹ����
	OLED_China(17,0,21,1);
	OLED_China(33,0,63,1);
	OLED_China(49,0,58,1);
	
	OLED_ShowString(64,16,":*/./hpa",1);
	OLED_China(1,16,17,1);//��ѹ����
	OLED_China(17,16,21,1);
	OLED_China(33,16,64,1);
	OLED_China(49,16,51,1);
	
	OLED_ShowString(64,32,"+//./hpa",1);
	OLED_China(1,32,63,0);//��������
	OLED_China(17,32,58,0);
	OLED_China(33,32,64,0);
	OLED_China(49,32,51,0);
	
	OLED_ShowString(64,48,":/.//hpa",1);
	OLED_China(1,48,18,1);//�¶�ϵ��
	OLED_China(17,48,20,1);
	OLED_China(33,48,64,1);
	OLED_China(49,48,55,1);
	OLED_China(112,48,58,1);
	if((a4!=0)||(b4!=0)||(c4!=0)||(d4!=0)||(e4!=0))
	{
		snprintf(disp_buf,sizeof(disp_buf),"%01u",a4);
			OLED_ShowString(80,0,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",b4);
			OLED_ShowString(88,0,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",c4);
			OLED_ShowString(104,0,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",d4);
			OLED_ShowString(112,0,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",e4);
			OLED_ShowString(120,0,disp_buf,1);
	}
	if((a5!=0)||(b5!=0)||(c5!=0))
	{
		if(a5==0)
				OLED_ShowString(72,16,"+",1);
			else if(a5==1)
				OLED_ShowString(72,16,"-",1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",b5);
			OLED_ShowString(80,16,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",c5);
			OLED_ShowString(96,16,disp_buf,1);
	}
	if((a6!=0)||(b6!=0)||(c6!=0))
	{
		snprintf(disp_buf,sizeof(disp_buf),"%01u",a6);
			OLED_ShowString(72,32,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",b6);
			OLED_ShowString(80,32,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",c6);
			OLED_ShowString(96,32,disp_buf,1);
	}
	if((a7!=0)||(b7!=0)||(c7!=0))
	{
		snprintf(disp_buf,sizeof(disp_buf),"%01u",a7);
			OLED_ShowString(72,48,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",b7);
			OLED_ShowString(88,48,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",c7);
			OLED_ShowString(96,48,disp_buf,1);
	}

	}
	else if(number4==7)
	{
			OLED_ShowString(64,0,": //.///",1);
	OLED_China(1,0,17,1);//��ѹ����
	OLED_China(17,0,21,1);
	OLED_China(33,0,63,1);
	OLED_China(49,0,58,1);
	
	OLED_ShowString(64,16,":*/./hpa",1);
	OLED_China(1,16,17,1);//��ѹ����
	OLED_China(17,16,21,1);
	OLED_China(33,16,64,1);
	OLED_China(49,16,51,1);
	
	OLED_ShowString(64,32,"+//./hpa",1);
	OLED_China(1,32,63,1);//��������
	OLED_China(17,32,58,1);
	OLED_China(33,32,64,1);
	OLED_China(49,32,51,1);
	
	OLED_ShowString(64,48,":/.//hpa",1);
	OLED_China(1,48,18,0);//�¶�ϵ��
	OLED_China(17,48,20,0);
	OLED_China(33,48,64,0);
	OLED_China(49,48,55,0);
	OLED_China(112,48,58,0);
	if((a4!=0)||(b4!=0)||(c4!=0)||(d4!=0)||(e4!=0))
	{
		snprintf(disp_buf,sizeof(disp_buf),"%01u",a4);
			OLED_ShowString(80,0,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",b4);
			OLED_ShowString(88,0,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",c4);
			OLED_ShowString(104,0,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",d4);
			OLED_ShowString(112,0,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",e4);
			OLED_ShowString(120,0,disp_buf,1);
	}
	if((a5!=0)||(b5!=0)||(c5!=0))
	{
		if(a5==0)
				OLED_ShowString(72,16,"+",1);
			else if(a5==1)
				OLED_ShowString(72,16,"-",1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",b5);
			OLED_ShowString(80,16,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",c5);
			OLED_ShowString(96,16,disp_buf,1);
	}
	if((a6!=0)||(b6!=0)||(c6!=0))
	{
		snprintf(disp_buf,sizeof(disp_buf),"%01u",a6);
			OLED_ShowString(72,32,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",b6);
			OLED_ShowString(80,32,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",c6);
			OLED_ShowString(96,32,disp_buf,1);
	}
	if((a7!=0)||(b7!=0)||(c7!=0))
	{
		snprintf(disp_buf,sizeof(disp_buf),"%01u",a7);
			OLED_ShowString(72,48,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",b7);
			OLED_ShowString(88,48,disp_buf,1);
			snprintf(disp_buf,sizeof(disp_buf),"%01u",c7);
			OLED_ShowString(96,48,disp_buf,1);
	}
	
	}
	OLED_Refresh_Gram();		//������ʾ��OLED 
}

//��ʼ��SSD1303				    
void OLED_Init(void)
{
	OLED_W_Command(0xae); //display off��ʾ��
        OLED_W_Command(0x40); //display start line 00000��ʾ��ʼ��00000
	      OLED_W_Command(0xb0);
        OLED_W_Command(0x81); //contrast�Աȶ�
        OLED_W_Command(0xff);
        OLED_W_Command(0x82); //brightness����
        OLED_W_Command(0x20);
        OLED_W_Command(0xa0); //no remap���ػ�
        OLED_W_Command(0xa4); //intire display offȫ����ʾ��
        OLED_W_Command(0xa6); //normal display������ʾ
        OLED_W_Command(0xa8); //39 mux
        OLED_W_Command(0x3f);
        OLED_W_Command(0xad); //DCDC off
        OLED_W_Command(0x8b);
        OLED_W_Command(0xc8); //scan from COM[N-1] to COM0��COM[N-1] �� COM0ɨ��
        OLED_W_Command(0xd3); //row 0->com 62
        OLED_W_Command(0x00);
        OLED_W_Command(0xd5);    //
        OLED_W_Command(0x70);    //
        OLED_W_Command(0xd8); //mono mode,normal power mode monoģʽ,������Դģʽ
        OLED_W_Command(0x00);  //?
        OLED_W_Command(0xda); //alternative COM pin configuration��һ��com��������
        OLED_W_Command(0x12);
        OLED_W_Command(0xaf); //display on��ʾ��
	OLED_Clear();
	
}  
