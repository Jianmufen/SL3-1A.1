#include "lcd.h"
#include "lcdfont.h"
#include "rtc.h"
#include "stdio.h"
#include "main.h"

//static void GET_RTC_Time(void);
//static char disp_buf[128];//��ʾ����
//OLED���Դ�
//��Ÿ�ʽ����.
// [0]0 1 2 3 ... 16	
// [1]0 1 2 3 ... 16	
// [2]0 1 2 3 ... 16	
// [3]0 1 2 3 ... 16	
// [4]0 1 2 3 ... 16	
// [5]0 1 2 3 ... 16	
// [6]0 1 2 3 ... 16	
// [7]0 1 2 3 ... 16 
//.................
//[64]0 1 2 3 ... 16 
uint8_t OLED_GRAM[64][16];   //����һ������  ��д����  �ٰ�����д��LCD
char TAB5[32] = {0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,\
									0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff};

char TAB6[128] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,\
									0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,\
									0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,\
									0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,\
									0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,\
									0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,\
									0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,\
									0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};

char TAB7[128] = {0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,\
									0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,\
									0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,\
									0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,\
									0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,\
									0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,\
									0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,\
									0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00};

char TAB8[128] = {0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,\
									0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,\
									0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,\
									0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,\
									0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,\
									0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,\
									0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,\
									0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff};

/*��������*/
static void OLED_Buzy(void);
/**************************ʵ�ֺ���******************************************** 
*����ԭ��:		void Lcd_Char(uint8_t x,uint8_t y,unsigned char chr)
*��������:		�ڶ���дһ��8*16��ASCII�벻ʹ��ˢ������OLED_Refresh_Gramˢ��
*��ڲ���:x��(0<x<64)  yҳ��0<y<128��  highlight=ture���ԣ�highlight=false����
*******************************************************************************/ 
void Lcd_Char(unsigned char x,unsigned char y,unsigned char chr,unsigned char highlight)
{
			uint8_t i = 0, temp = 0;
			x=x&0x3f;   //��������  ����x>63  y>127
			y=y&0x7f;
			chr=chr - ' ';//�õ�ƫ�ƺ��ֵ
			if(x < 32)		/*�ϰ���*/
			{
						for(i=0;i<16;i++)
						{
									OLED_W_Command(0x34);							/*ʹ����չָ�*/
									OLED_W_Command(0x80 + x + i);			/*������ʾҳ��ַ*/
									OLED_W_Command(0x80 + y);					/*�����е�ַ*/
									OLED_W_Command(0x30);							/*ʹ�û���ָ�*/
									if(highlight)											/*����*/
									{
												OLED_W_Data(asc2_1608[chr][i]);
												OLED_W_Data(0x20);
									}
									else															/*����*/
									{
												OLED_W_Data(asc2_1608[chr][i]);
												OLED_W_Data(0x20);
									}
						}
			}
			else				/*�°���*/
			{
						for(i=0;i<16;i++)
						{
									OLED_W_Command(0x34);							/*ʹ����չָ�*/
									OLED_W_Command(0x80 + x + i);			/*������ʾҳ��ַ*/
									OLED_W_Command(0x88 + y);					/*�����е�ַ*/
									OLED_W_Command(0x30);							/*ʹ�û���ָ�*/
							
									if(highlight)											/*����*/
									{
												OLED_W_Data(asc2_1608[chr][i]);
												OLED_W_Data(0x20);
									}
									else															/*����*/
									{
												OLED_W_Data(asc2_1608[chr][i]);
												OLED_W_Data(0x00);
									}
						}
			}
			
			OLED_W_Command(0x36);
			OLED_W_Command(0x30);
}	


/**************************ʵ�ֺ���******************************************** 
*����ԭ��:		void OLED_ShowString(uint8_t x,uint8_t y,const uint8_t *p)
*��������:		��ʾ�ַ���
*��ڲ���:x��(0=<x=<63)  y�У�0=<y=<127��  highlight=ture���ԣ�highlight=false����
*******************************************************************************/ 
void Lcd_ShowString(unsigned char x,unsigned char y,const char *p,unsigned char highlight)
{
			uint8_t i = 0, j = 0,len = 0;
			uint8_t chr[16] = {0};
			len = strlen(p);				/*�õ��ַ����ĳ���*/
			for(j=0;j<len;j++)
			{
						chr[j] = *p - ' ';
						p++;
			}
			if(x < 32)		/*�ϰ���*/
			{
						for(i=0;i<16;i++)
						{
									OLED_W_Command(0x34);							/*ʹ����չָ�*/
									OLED_W_Command(0x80 + x + i);			/*������ʾҳ��ַ*/
									OLED_W_Command(0x80 + y);					/*�����е�ַ*/
									OLED_W_Command(0x30);							/*ʹ�û���ָ�*/
									if(highlight)											/*����*/
									{
												for(j=0;j<len;j++)
												{
															OLED_W_Data(asc2_1608[chr[j]][i]);
												}
									}
									else															/*����*/
									{
												for(j=0;j<len;j++)
												{
															OLED_W_Data(asc2_1608[chr[j]][i]);
												}
									}
						}
			}
			else				/*�°���*/
			{
						for(i=0;i<16;i++)
						{
									OLED_W_Command(0x34);							/*ʹ����չָ�*/
									OLED_W_Command(0x80 + x + i);			/*������ʾҳ��ַ*/
									OLED_W_Command(0x88 + y);					/*�����е�ַ*/
									OLED_W_Command(0x30);							/*ʹ�û���ָ�*/
									
									if(highlight)											/*����*/
									{
												for(j=0;j<len;j++)
												{
															OLED_W_Data(asc2_1608[chr[j]][i]);
												}
									}
									else															/*����*/
									{
											for(j=0;j<len;j++)
												{
															OLED_W_Data(asc2_1608[chr[j]][i]);
												}
									}
						}
			}
			
			OLED_W_Command(0x36);
			OLED_W_Command(0x30);
}


//�����Դ浽LCD		 
void OLED_Refresh_Gram(void)
{
			volatile unsigned int i=0,j=0,k=0,x=0,y=0,m=0,n=0;
			char *p;
	
			x=0x80;
			
			for(i=0;i<2;i++)											/*��д�ϰ�������д�°���*/
			{
						y=0x80;
						for(j=0;j<32;j++)								/*д32�У����ϰ���*/
						{
									OLED_W_Command(0x34);
									OLED_W_Command(y + j);
									OLED_W_Command(x);
									OLED_W_Command(0x30);
									for(k=0;k<16;k++)					/*ÿ��дһ�У�һ��128���㣬��16���ֽ�*/
									{
												if(i==0)
												{
															OLED_W_Data(OLED_GRAM[j][k]);
												}
												else if(i==1)
												{
															OLED_W_Data(OLED_GRAM[j + 32][k]);
												}
									}
						}
						x=0x88;
			}
			OLED_W_Command(0x36);
			OLED_W_Command(0x30);
}



/*RT12864M-1��ʾ��һ�е���ʾ
 *unsigned char x����ʾ�������ڵ�������x=1�����һ�У�x=2����ڶ��У�x=3��������У�x=4���������
 *char *pҪ��ʾ��������һ��16�ֽڵ��ַ�����Ʃ��"0123456789012345"��"1234��ʾ��567089"
 */							
void rt12864m_show(unsigned char x, char *p)
{
			uint8_t i = 0;
	
			if(1 ==x)			/*��һ����ʾ*/
			{
						for(i = 0; i < 8; i++)
						{
									OLED_W_Command(0x80 + i);
									OLED_W_Data(*p);
									p++;
									OLED_W_Data(*p);
									p++;
						}
			}
			else if(2 ==x)			/*�ڶ�����ʾ*/
			{
						for(i = 0; i < 8; i++)
						{
									OLED_W_Command(0x90 + i);
									OLED_W_Data(*p);
									p++;
									OLED_W_Data(*p);
									p++;
						}
			}
			else if(3 ==x)			/*�ڶ�����ʾ*/
			{
						for(i = 0; i < 8; i++)
						{
									OLED_W_Command(0x88 + i);
									OLED_W_Data(*p);
									p++;
									OLED_W_Data(*p);
									p++;
						}
			}
			else if(4 ==x)			/*�ڶ�����ʾ*/
			{
						for(i = 0; i < 8; i++)
						{
									OLED_W_Command(0x98 + i);
									OLED_W_Data(*p);
									p++;
									OLED_W_Data(*p);
									p++;
						}
			}
			else
			{
						for(i = 0; i < 8; i++)
						{
									OLED_W_Command(0x80 + i);
									OLED_W_Data(*p);
									p++;
									OLED_W_Data(*p);
									p++;
						}
			}
}
/**************************ʵ�ֺ���******************************************** 
*����ԭ��:		void OLED_OpenMenu_SL3(void)
*��������:		������������ǵĿ����˵�
*��ڲ���:��
*******************************************************************************/ 
void OLED_OpenMenu_SL3(void)
{
			char a[] = {"                "};
			char b[] = {"SL3-1A          "};
			char c[] = {"  ������ⱨ����"};
			char d[] = {"                "};
			char *p = NULL;
			uint8_t i = 0, j = 0;
			
			p = a;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x80 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = b;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x90 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = c;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x88 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = d;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x98 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			
//		OLED_ShowString(16, 0, "SL3-1A", 0);
//		OLED_China(32, 16, 19, 0);//��
//		OLED_China(32, 32, 20, 0);//��
//		OLED_China(32, 48, 21, 0);//��
//		OLED_China(32, 64, 22, 0);//��
//		OLED_China(32, 80, 23, 0);//��
//		OLED_China(32, 96, 24, 0);//��
//		OLED_China(32, 112, 25, 0);//��
//	
//		OLED_Refresh_Gram();
}



/**************************ʵ�ֺ���******************************************** 
*����ԭ��:		void OLED_OpenMenu_SL3(void)
*��������:		������������ǵĿ����˵�
*��ڲ���:��
*******************************************************************************/ 
void OLED_OpenMenu_EN2(void)
{
			char a[] = {"                "};
			char b[] = {"EN2-B           "};
			char c[] = {"      ���������"};
			char d[] = {"                "};
			char *p = NULL;
			uint8_t i = 0, j = 0;
			
			p = a;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x80 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = b;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x90 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = c;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x88 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = d;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x98 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			
//		OLED_ShowString(16, 0, "SL3-1A", 0);
//		OLED_China(32, 16, 19, 0);//��
//		OLED_China(32, 32, 20, 0);//��
//		OLED_China(32, 48, 21, 0);//��
//		OLED_China(32, 64, 22, 0);//��
//		OLED_China(32, 80, 23, 0);//��
//		OLED_China(32, 96, 24, 0);//��
//		OLED_China(32, 112, 25, 0);//��
//	
//		OLED_Refresh_Gram();
}

//��ʼ��KS0108	������������RT12864M-1��ʾ��			    
void OLED_Init(void)
{
			OLED_CS_H();							//ѡ�񲢿ڷ�ʽ
			HAL_Delay(1000);						//��ʱ800ms
	
			OLED_W_Command(0x30); 		//8λ���ڣ�ʹ�û���ָ�
			OLED_Buzy();							/*�ȴ���ʾ����æ*/
	
			OLED_W_Command(0x30); 		//8λ���ڣ�ʹ�û���ָ�
			OLED_Buzy();							/*�ȴ���ʾ����æ*/
	
			OLED_W_Command(0x0c); 		//����ʾ
			OLED_Buzy();							/*�ȴ���ʾ����æ*/
	
			OLED_W_Command(0x01); 		//�����ʾ
			HAL_Delay(2);							//��ʱ2����
	
			OLED_W_Command(0x06); 		//��ʾ���ݲ��ƶ�
			OLED_Buzy();							/*�ȴ���ʾ����æ*/
	
			OLED_W_Command(0x02); 		//��ַ��Ϊ����ַ���㿪ʼ
			OLED_Buzy();							/*�ȴ���ʾ����æ*/
	
			OLED_W_Command(0x14); 		//��ʾ�ƶ�
			OLED_Buzy();							/*�ȴ���ʾ����æ*/
			
			OLED_W_Command(0x30); 		//8λ���ڣ�ʹ�û���ָ�
			OLED_Buzy();							/*�ȴ���ʾ����æ*/
}  


/*��ͼ��ʽ�ӳ���*/
void pic_u(unsigned int lon,unsigned int hight,unsigned int addx, unsigned int addy,char *p)
{
			uint8_t i=0,j=0;
			
			for(i=0;i<hight;i++)
			{
						OLED_W_Command(0x34);
						OLED_W_Command(addy);
						OLED_W_Command(addx);
						OLED_W_Command(0x30);
						for(j=0;j<lon;j++)
						{
									OLED_W_Data(*p);
									p++;
									OLED_W_Data(*p);
									p++;
						}
						addy++;
			}
			
			OLED_W_Command(0x36);
			OLED_W_Command(0x30);
}

/*�ϰ�����ͼ�����ӳ���*/
void pic_clear_u(void)
{
			uint8_t i=0,j=0,k=0;
	
			k=0x80;
			for(i=0;i<32;i++)
			{
						OLED_W_Command(0x34);
						OLED_W_Command(k);
						OLED_W_Command(0x80);
						OLED_W_Command(0x30);
						for(j=0;j<32;j++)
						{
									OLED_W_Data(0x00);
						}
						k++;
			}
			
			k=0x90;
			for(i=0;i<32;i++)
			{
						OLED_W_Command(0x34);
						OLED_W_Command(k);
						OLED_W_Command(0x80);
						OLED_W_Command(0x30);
						for(j=0;j<32;j++)
						{
									OLED_W_Data(0x00);
						}
						k++;
			}
			
			OLED_W_Command(0x36);
			OLED_W_Command(0x30);
}


/*�ϰ�����ͼ�����ӳ���*/
void pic_clear_d(void)
{
			uint8_t i=0,j=0,k=0;
	
			k=0x88;
			for(i=0;i<32;i++)
			{
						OLED_W_Command(0x34);
						OLED_W_Command(k);
						OLED_W_Command(0x80);
						OLED_W_Command(0x30);
						for(j=0;j<32;j++)
						{
									OLED_W_Data(0x00);
						}
						k++;
			}
			
			k=0x98;
			for(i=0;i<32;i++)
			{
						OLED_W_Command(0x34);
						OLED_W_Command(k);
						OLED_W_Command(0x80);
						OLED_W_Command(0x30);
						for(j=0;j<32;j++)
						{
									OLED_W_Data(0x00);
						}
						k++;
			}
			
			OLED_W_Command(0x36);
			OLED_W_Command(0x30);
}
void pic_d(unsigned int lon,unsigned int hight,unsigned int addx, unsigned int addy,char *p)
{
			uint8_t i=0,j=0,k=0;
	
			for(i=0;i<hight;i++)
			{
						OLED_W_Command(0x34);
						OLED_W_Command(addy);
						OLED_W_Command(addx);
						OLED_W_Command(0x30);
						for(j=0;j<lon;j++)
						{
									OLED_W_Data(*p);
									p++;
									OLED_W_Data(*p);
									p++;
						}
						addy++;
			}
			OLED_W_Command(0x36);
			OLED_W_Command(0x30);
}
/*��ʾ�Զ���16*16����*/
void dz_d(unsigned int command1, char *p)
{
			uint8_t i=0,j=0,k=0;
			
			for(i=0;i<16;i++)
			{
						OLED_W_Command(command1);
						for(j=0;j<2;j++)
						{
									OLED_W_Data(*p);
									p++;
						}
						command1++;
			}
}

void OLED_Clear(void) 
{ 
			OLED_W_Command(0x01); 		//�����ʾ
			HAL_Delay(2);							//��ʱ2����
	
//			uint8_t i,n; 
//			for(i=0;i<64;i++) 
//				 for(n=0;n<16;n++)
//						OLED_GRAM[i][n]=0x00;
//				OLED_Refresh_Gram();//������ʾ
} 

/*�޷���ȡæ��־��ֻ����ʱһ��ʱ�����ȴ���ʾ����æ��д����һ������ʱ��Ϊ72us��������ʱʱ��>=72us*/
static void OLED_Buzy(void) 
{
			volatile unsigned int i = 0;
			for(i = 0; i < 100; i++)			/*169�϶����У�170����*/
			{
						__nop();
			}
}

//ȫ����ʾ�ַ�2,һ����ʾ16���ַ���������ַ�����ʾ
void test_oled2(void)
{
			char a[] = "�Ϻ�ShangHai�Ϻ�";
			char *p = NULL;
			uint8_t i = 0;
			p = a;
	
			for(i=0;i<8;i++)
			{
						OLED_W_Command(0x80 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			p=a;
			for(i=0;i<8;i++)
			{
						OLED_W_Command(0x90 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			p=a;
			for(i=0;i<8;i++)
			{
						OLED_W_Command(0x88 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			p=a;
			for(i=0;i<8;i++)
			{
						OLED_W_Command(0x98 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			pic_clear_u();				
			pic_u(0x01, 0x10, 0x82, 0x80, TAB8);
}

//ȫ����ʾ�ַ�
void test_oled1(void)
{
			uint8_t i = 0;
			
			for(i=0;i<8;i++)
			{
						OLED_W_Command(0x80 + i);
						OLED_W_Data(0x30);
						OLED_W_Data(0x31);
			}
			
			for(i=0;i<8;i++)
			{
						OLED_W_Command(0x90 + i);
						OLED_W_Data(0x30);
						OLED_W_Data(0x31);
			}
			
			for(i=0;i<8;i++)
			{
						OLED_W_Command(0x88 + i);
						OLED_W_Data(0x30);
						OLED_W_Data(0x31);
			}
			
			for(i=0;i<8;i++)
			{
						OLED_W_Command(0x98 + i);
						OLED_W_Data(0x30);
						OLED_W_Data(0x31);
			}
}

/*��ʾ����*/
void test_oled(void)
{		
			uint8_t i = 0, j = 0, k = 0, l = 0;
			char a[] = {"�Ϻ�����������SH"};
			char *p = NULL;
			p=a;
			
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x80 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			HAL_Delay(5);							//��ʱ1����
			
			
			p=a;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x90 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			HAL_Delay(5);							//��ʱ1����
			
			
			p=a;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x88 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			HAL_Delay(5);							//��ʱ1����
			
			
			p=a;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x98 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			HAL_Delay(5);							//��ʱ1����
			
			
			OLED_W_Command(0x30);
			OLED_W_Command(0x34);
			OLED_W_Command(0x04);
			OLED_W_Command(0x30);
}

/*������ʾ*/
void test_oled3(void)
{
			uint8_t i=0;
//			OLED_W_Command(0x06);		/*������ƣ���ʾ����*/
//			OLED_W_Command(0x0e);		/*��꿪����������˸*/
	
			OLED_W_Command(0x80);		/*��*/
			OLED_W_Data(0xb7);
			OLED_W_Data(0xb4);
//			HAL_Delay(800);
	
			OLED_W_Command(0x81);		/*��*/
			OLED_W_Data(0xb0);
			OLED_W_Data(0xd7);
//			HAL_Delay(800);
	
			OLED_W_Command(0x82);		/*��*/
			OLED_W_Data(0xcf);
			OLED_W_Data(0xd4);
//			HAL_Delay(800);
	
			OLED_W_Command(0x83);		/*ʾ*/
			OLED_W_Data(0xca);
			OLED_W_Data(0xbe);
//			HAL_Delay(800);
			
			OLED_W_Command(0x84);		/*��*/
			OLED_W_Data(0xd1);
			OLED_W_Data(0xdd);
//			HAL_Delay(800);
			
			OLED_W_Command(0x85);		/*ʾ*/
			OLED_W_Data(0xca);
			OLED_W_Data(0xbe);
//			HAL_Delay(800);
			
			OLED_W_Command(0x86);		/*˵*/
			OLED_W_Data(0xcb);
			OLED_W_Data(0xb5);
//			HAL_Delay(800);
			
			OLED_W_Command(0x87);		/*��*/
			OLED_W_Data(0xc3);
			OLED_W_Data(0xf7);
//			HAL_Delay(800);
			
//			OLED_W_Command(0x88);		/*��*/
//			OLED_W_Data(0xc3);
//			OLED_W_Data(0xf7);
//			HAL_Delay(800);
			
//			OLED_W_Command(0x0c);		/*����*/
			
			OLED_W_Command(0x90);		/*��*/
			OLED_W_Data(0xc8);
			OLED_W_Data(0xf0);
			
			OLED_W_Command(0x91);		/*��*/
			OLED_W_Data(0xcc);
			OLED_W_Data(0xd8);
//			HAL_Delay(1600);
			
//			OLED_W_Command(0x92);		/*+*/
//			OLED_W_Data(0xa3);
//			OLED_W_Data(0xab);
//			HAL_Delay(1600);
			
			pic_clear_u();				/*�ϰ�������*/
			pic_u(0x02, 0x10, 0x92, 0x90, TAB6);
//			HAL_Delay(1600);
			
//			OLED_W_Command(0x92);		/*=*/
//			OLED_W_Data(0xa3);
//			OLED_W_Data(0xbd);
//			HAL_Delay(1600);
			
			OLED_W_Command(0x92);		/*��*/
			OLED_W_Data(0xc8);
			OLED_W_Data(0xf0);
			
			OLED_W_Command(0x93);		/*��*/
			OLED_W_Data(0xcc);
			OLED_W_Data(0xd8);
//			HAL_Delay(1600);
			
			pic_u(0x02, 0x10, 0x95, 0x90, TAB6);
//			HAL_Delay(1600);
			
			OLED_W_Command(0x94);		/*��*/
			OLED_W_Data(0xbb);
			OLED_W_Data(0xe6);
//			HAL_Delay(1600);
			
			OLED_W_Command(0x95);		/*ͼ*/
			OLED_W_Data(0xcd);
			OLED_W_Data(0xbc);
//			HAL_Delay(1600);
			
			OLED_W_Command(0x96);		/*��*/
			OLED_W_Data(0xd1);
			OLED_W_Data(0xdd);
			HAL_Delay(1600);
//			
//			OLED_W_Command(0x91);		/*ʾ*/
//			OLED_W_Data(0xca);
//			OLED_W_Data(0xbe);
//			HAL_Delay(1600);
			
			
}

/*��ʾ�Զ��������*/
void test_oled4(void)
{
			uint8_t i,n;
	
			dz_d(0x50, TAB5);
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x80 + i);
						OLED_W_Data(0x00);
						OLED_W_Data(0x02);
			}
			
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x90 + i);
						OLED_W_Data(0x00);
						OLED_W_Data(0x02);
			}
}

/*������ʾ*/
void test_oled5(void)
{
			uint8_t i=0,j=0,k=0,x=0,y=0;
			char *p;
	
			x=0x80;
			
			for(i=0;i<2;i++)
			{
						y=0x80;
						for(j=0;j<32;j++)
						{
									OLED_W_Command(0x34);
									OLED_W_Command(y);
									OLED_W_Command(x);
									OLED_W_Command(0x30);
									for(k=0;k<16;k++)
									{
												OLED_W_Data(*p);
												p++;
									}
									y++;
						}
						x=0x88;
			}
			OLED_W_Command(0x36);
			OLED_W_Command(0x30);
}

/*���㺯����ʾ*/
void shou_point(void)
{
			OLED_DrawPoint(0, 8, 1);
			OLED_DrawPoint(1, 8, 1);
			OLED_DrawPoint(2, 8, 1);
			OLED_DrawPoint(3, 8, 1);
			OLED_DrawPoint(4, 8, 1);
			OLED_DrawPoint(5, 8, 1);
			OLED_DrawPoint(6, 8, 1);
			OLED_DrawPoint(7, 8, 1);
			OLED_DrawPoint(8, 8, 1);
			OLED_DrawPoint(9, 8, 1);
	
			OLED_Char(16, 0, 'A', 0);
			OLED_Char(16, 8, 'B', 0);
			OLED_Char(16, 16, 'C', 0);
			OLED_ShowString(16, 24, "EFGHIJKLMOPQR", 0);
			OLED_China(32, 0, 0, 0);
			OLED_China(48, 0, 3, 0);
	
			OLED_Refresh_Gram();
}
/**************************ʵ�ֺ���******************************************** 
*����ԭ��:		unsigned char OLED_W_Command(void); 
*��������:		д���OLED�� 
*******************************************************************************/ 
void OLED_W_Command(unsigned int com)
{
		unsigned int i;
		
//		OLED_Buzy();							/*�ȴ���ʾ����æ*/
		OLED_RS_L();						/*RS�͵�ƽΪд�������*/
	
		com &= 0xff;
		com <<= 1;
		DATAOUT(com);

		OLED_E_H();
		OLED_Buzy();							/*�ȴ���ʾ����æ*/
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//	  __nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
		OLED_E_L();
		OLED_Buzy();							/*�ȴ���ʾ����æ*/
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
} 


/**************************ʵ�ֺ���******************************************** 
*����ԭ��:		void OLED_W_Data(unsigned dat); 
*��������:		д���ݵ�oled��ʾ���� 
*******************************************************************************/ 
void OLED_W_Data(unsigned int dat)
{
		unsigned int i;
	
//		OLED_Buzy();						/*�ȴ���ʾ����æ*/

		OLED_RS_H();							/*RS�ߵ�ƽΪд���ݲ���*/
		
		dat &= 0x000000ff;
		dat <<= 1;
		DATAOUT(dat);

		OLED_E_H();
		OLED_Buzy();							/*�ȴ���ʾ����æ*/
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
		OLED_E_L();
		OLED_Buzy();							/*�ȴ���ʾ����æ*/
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
}


/**************************ʵ�ֺ���******************************************** 
*����ԭ��:		void OLED_DrawPoint(uint8_t x,uint8_t y,uint8_t t) 
*��������:		��x�� ��y�л�һ����
*��ڲ�����x:0~63��y:0~127;t:1��䣬0���
*******************************************************************************/ 
void OLED_DrawPoint(unsigned char x,unsigned char y,unsigned char t) 
{ 
	uint8_t pos,bx,temp=0;
	x=x&0x3f;   //��������  ����x>64  y>128
	y=y&0x7f;
	pos=y/8;   //��Ӧ��OLED_GRAM�е�ҳ
	bx=y%8;
	temp=1<<(bx);			/*˳����temp=1<<(7-bx)Ϊ����*/
	if(t)
		OLED_GRAM[x][pos]|=temp;
	else 
		OLED_GRAM[x][pos]&=~temp;	    
} 


/**************************ʵ�ֺ���******************************************** 
*����ԭ��:		void OLED_Char(uint8_t x,uint8_t y,unsigned char chr)
*��������:		�ڶ���дһ��8*16��ASCII��
*��ڲ���:x��(0<x<64)  yҳ��0<y<128��  highlight=ture���ԣ�highlight=false����
*******************************************************************************/ 
void OLED_Char(unsigned char x,unsigned char y,unsigned char chr,unsigned char highlight)
{
			uint8_t t,temp,t1;
			uint8_t y0=y;
			x=x&0x3f;   //��������  ����x>63  y>127
			y=y&0x7f;
			chr=chr - ' ';//�õ�ƫ�ƺ��ֵ
			for(t=0;t<16;t++)             
			{
						temp=asc2_1608[chr][t];
						for(t1=0;t1<8;t1++)
						{
							if(highlight == false)//����
							{
										if(temp&0x01)
											OLED_DrawPoint(x,y,1);
										else 
											OLED_DrawPoint(x,y,0);
										temp>>=1;
										y++;
										if((y-y0)==8)
										{
											y=y0;
											x++;
											break;
										}
							}
							else//����
							{
									if(temp&0x01)
										OLED_DrawPoint(x,y,0);
									else 
										OLED_DrawPoint(x,y,1);
									temp>>=1;
									y++;
									if((y-y0)==8)
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
*��ڲ���:x��(0=<x=<63)  y�У�0=<y=<127��  highlight=ture���ԣ�highlight=false����
*******************************************************************************/ 
void OLED_ShowString(unsigned char x,unsigned char y,const char *p,unsigned char highlight)
{	
    while((*p<='~')&&(*p>=' '))//�ж��ǲ��ǷǷ��ַ�!
    {       
        if(y>120)
        {
					y=0;
					x+=16;
				}
        if(x>48)
        {
				  y=x=0;
					OLED_Clear();
				}
        OLED_Char(x,y,*p,highlight);	 
        y+=8;
        p++;
    }  
	
}	   


/**************************ʵ�ֺ���******************************************** 
*����ԭ��:		void OLED_China(uint8_t x,uint8_t y,unsigned char cha)
*��������:		�ڶ���дһ��16*16�ĺ���
*��ڲ���:x��(0=<x=<63)  yҳ��0=<y=<127��  highlight=ture���ԣ�highlight=false����
*******************************************************************************/ 
void OLED_China(unsigned char x,unsigned char y,unsigned char n,unsigned char highlight)
{
	uint8_t t,t1,temp;
	uint8_t y0=y,x0=x;
	x=x&0x3f;   //��������  ����x>63  y>127
	y=y&0x7f;
	for(t=0;t<32;t++)             
	{
		temp=china_1616[n][t];
		for(t1=0;t1<8;t1++)    //���㺯��8����һ��
		{
			if(highlight==false)//����
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
//	x = x0;
//	y0 += 8;
//	y = y0;
//	for(t=16;t<32;t++)             
//	{
//		temp=china_1616[n][t];
//		for(t1=0;t1<8;t1++)    //���㺯��8����һ��
//		{
//			if(highlight==false)//����
//			{
//						if(temp&0x01)
//							OLED_DrawPoint(x,y,1);
//						else 
//							OLED_DrawPoint(x,y,0);
//						temp>>=1;
//						y++;
//						if((y-y0)==16)
//						{
//							y=y0;
//							x++;
//							break;
//						}
//		  }
//			else
//			{
//				if(temp&0x01)
//				  OLED_DrawPoint(x,y,0);
//			  else 
//			 	  OLED_DrawPoint(x,y,1);
//			  temp>>=1;
//			  y++;
//			  if((y-y0)==16)
//			  {
//				  y=y0;
//				  x++;
//				  break;
//		  	}
//			}
//		}
//	}
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
	for(t=0;t<48;t++)             
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
	
	  OLED_China(1,40,0,false);
		OLED_China(17,40,1,false);
		OLED_China(33,40,2,false);
		OLED_China(49,40,3,false);
		OLED_China(65,40,4,false);
		OLED_China(81,40,5,false);
		OLED_China(97,40,6,false);
		OLED_China(113,40,7,false);
	
		OLED_Refresh_Gram();		//������ʾ��OLED 
} 	    


//void OLED_Clear(void) 
//{ 
//			uint8_t i,n; 
//	
//			for(i=0;i<8;i++) 
//			{
//						for(n=0;n<132;n++)
//						{
//									OLED_GRAM[n][i]=0X00;
//						}
//			}
//				
//					
//			OLED_Refresh_Gram();//������ʾ
//} 

////��ʼ��KS0108	������������RT12864J-1��ʾ��			    
//void OLED_Init(void)
//{
//			OLED_CS_H();								/*ѡ�������*/
//			OLED_W_Command(0x3e); 		//display off��ʾ��
//			OLED_CS_L();								/*ѡ���Ұ���*/
//			OLED_W_Command(0x3e); 		//display off��ʾ��
//			HAL_Delay(1);						//��ʱ1����
//	
//			OLED_CS_H();								/*ѡ�������*/
//			OLED_W_Command(0xc0); 		//������ʾ��ʼ��Ϊ0
//			OLED_CS_L();								/*ѡ���Ұ���*/
//			OLED_W_Command(0xc0); 		//������ʾ��ʼ��Ϊ0
//			HAL_Delay(1);						//��ʱ1����
//	
//			OLED_CS_H();								/*ѡ�������*/
//			OLED_W_Command(0xb8); 		//������ʼҳ��ַΪ0
//			OLED_CS_L();								/*ѡ���Ұ���*/
//			OLED_W_Command(0xb8); 		//������ʼҳ��ַΪ0
//			HAL_Delay(1);	

//			OLED_CS_H();								/*ѡ�������*/
//			OLED_W_Command(0x40); 		//������ʼ�е�ַΪ0
//			OLED_CS_L();								/*ѡ���Ұ���*/
//			OLED_W_Command(0x40); 		//������ʼ�е�ַΪ0
//			HAL_Delay(1);						//��ʱ1����
//			
//			OLED_CS_H();								/*ѡ�������*/
//			OLED_W_Command(0x3f); 		//display on��ʾ��
//			OLED_CS_L();								/*ѡ���Ұ���*/
//			OLED_W_Command(0x3f); 		//display on��ʾ��
//			HAL_Delay(1);						//��ʱ1����
//}  


////�����Դ浽LCD		 
//void OLED_Refresh_Gram(void)
//{
//		uint8_t i,n;		    
//		for(i = 0; i < 8; i++)  
//		{
//					OLED_CS_H();								/*ѡ�������*/
//					OLED_W_Command (0xb8+i);    //����ҳ��ַ��0~7��
//					OLED_W_Command (0x40);      //������ʾ�е�ַ
//			
//					OLED_CS_L();								/*ѡ���Ұ���*/
//					OLED_W_Command (0xb8+i);    //����ҳ��ַ��0~7��
//					OLED_W_Command (0x40);      //������ʾ�е�ַ
//			
//					for(n = 0; n < 128; n++)
//					{
//								if((n < 64) && (n >= 0))
//								{
//											OLED_CS_H();								/*ѡ�������*/
//								}
//								else if((n < 128) && (n > 63))
//								{
//											OLED_CS_L();								/*ѡ���Ұ���*/
//								}
//								OLED_W_Data(OLED_GRAM[n][i]); 
//					}
//						
//		}
//}


///**************************ʵ�ֺ���******************************************** 
//*����ԭ��:		unsigned char OLED_W_Command(void); 
//*��������:		д���OLED�� 
//*******************************************************************************/ 
//void OLED_W_Command(unsigned int com)
//{
//		unsigned int i;
//		
////		OLED_CS_H();						/*ѡ��2������*/
//		OLED_RS_L();						/*RS�͵�ƽΪд�������*/
//		
//		com &= 0xff;
//		com <<= 1;
//		DATAOUT(com);

//		OLED_E_H();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		OLED_E_L();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//} 


///**************************ʵ�ֺ���******************************************** 
//*����ԭ��:		void OLED_W_Data(unsigned dat); 
//*��������:		д���ݵ�oled��ʾ���� 
//*******************************************************************************/ 
//void OLED_W_Data(unsigned int dat)
//{
//		unsigned int i;
//	
////		OLED_CS_H();						/*ѡ��2������*/
//		OLED_RS_H();						/*RS�ߵ�ƽΪд���ݲ���*/
//		
//		dat &= 0x000000ff;
//		dat <<= 1;
//		DATAOUT(dat);

//		OLED_E_H();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		OLED_E_L();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();__nop();
//}



