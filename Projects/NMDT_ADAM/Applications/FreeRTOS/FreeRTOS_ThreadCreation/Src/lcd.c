#include "lcd.h"
#include "lcdfont.h"
#include "rtc.h"
#include "stdio.h"
#include "main.h"

//static void GET_RTC_Time(void);
//static char disp_buf[128];//显示缓存
//OLED的显存
//存放格式如下.
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
uint8_t OLED_GRAM[64][16];   //定义一个数组  先写数组  再把数组写到LCD
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

/*函数声明*/
static void OLED_Buzy(void);
/**************************实现函数******************************************** 
*函数原型:		void Lcd_Char(uint8_t x,uint8_t y,unsigned char chr)
*功　　能:		在定点写一个8*16的ASCII码不使用刷屏函数OLED_Refresh_Gram刷屏
*入口参数:x行(0<x<64)  y页（0<y<128）  highlight=ture反显，highlight=false正显
*******************************************************************************/ 
void Lcd_Char(unsigned char x,unsigned char y,unsigned char chr,unsigned char highlight)
{
			uint8_t i = 0, temp = 0;
			x=x&0x3f;   //参数过滤  避免x>63  y>127
			y=y&0x7f;
			chr=chr - ' ';//得到偏移后的值
			if(x < 32)		/*上半屏*/
			{
						for(i=0;i<16;i++)
						{
									OLED_W_Command(0x34);							/*使用扩展指令集*/
									OLED_W_Command(0x80 + x + i);			/*设置显示页地址*/
									OLED_W_Command(0x80 + y);					/*设置列地址*/
									OLED_W_Command(0x30);							/*使用基本指令集*/
									if(highlight)											/*反显*/
									{
												OLED_W_Data(asc2_1608[chr][i]);
												OLED_W_Data(0x20);
									}
									else															/*正显*/
									{
												OLED_W_Data(asc2_1608[chr][i]);
												OLED_W_Data(0x20);
									}
						}
			}
			else				/*下半屏*/
			{
						for(i=0;i<16;i++)
						{
									OLED_W_Command(0x34);							/*使用扩展指令集*/
									OLED_W_Command(0x80 + x + i);			/*设置显示页地址*/
									OLED_W_Command(0x88 + y);					/*设置列地址*/
									OLED_W_Command(0x30);							/*使用基本指令集*/
							
									if(highlight)											/*反显*/
									{
												OLED_W_Data(asc2_1608[chr][i]);
												OLED_W_Data(0x20);
									}
									else															/*正显*/
									{
												OLED_W_Data(asc2_1608[chr][i]);
												OLED_W_Data(0x00);
									}
						}
			}
			
			OLED_W_Command(0x36);
			OLED_W_Command(0x30);
}	


/**************************实现函数******************************************** 
*函数原型:		void OLED_ShowString(uint8_t x,uint8_t y,const uint8_t *p)
*功　　能:		显示字符串
*入口参数:x行(0=<x=<63)  y列（0=<y=<127）  highlight=ture反显，highlight=false正显
*******************************************************************************/ 
void Lcd_ShowString(unsigned char x,unsigned char y,const char *p,unsigned char highlight)
{
			uint8_t i = 0, j = 0,len = 0;
			uint8_t chr[16] = {0};
			len = strlen(p);				/*得到字符串的长度*/
			for(j=0;j<len;j++)
			{
						chr[j] = *p - ' ';
						p++;
			}
			if(x < 32)		/*上半屏*/
			{
						for(i=0;i<16;i++)
						{
									OLED_W_Command(0x34);							/*使用扩展指令集*/
									OLED_W_Command(0x80 + x + i);			/*设置显示页地址*/
									OLED_W_Command(0x80 + y);					/*设置列地址*/
									OLED_W_Command(0x30);							/*使用基本指令集*/
									if(highlight)											/*反显*/
									{
												for(j=0;j<len;j++)
												{
															OLED_W_Data(asc2_1608[chr[j]][i]);
												}
									}
									else															/*正显*/
									{
												for(j=0;j<len;j++)
												{
															OLED_W_Data(asc2_1608[chr[j]][i]);
												}
									}
						}
			}
			else				/*下半屏*/
			{
						for(i=0;i<16;i++)
						{
									OLED_W_Command(0x34);							/*使用扩展指令集*/
									OLED_W_Command(0x80 + x + i);			/*设置显示页地址*/
									OLED_W_Command(0x88 + y);					/*设置列地址*/
									OLED_W_Command(0x30);							/*使用基本指令集*/
									
									if(highlight)											/*反显*/
									{
												for(j=0;j<len;j++)
												{
															OLED_W_Data(asc2_1608[chr[j]][i]);
												}
									}
									else															/*正显*/
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


//更新显存到LCD		 
void OLED_Refresh_Gram(void)
{
			volatile unsigned int i=0,j=0,k=0,x=0,y=0,m=0,n=0;
			char *p;
	
			x=0x80;
			
			for(i=0;i<2;i++)											/*先写上半屏，再写下半屏*/
			{
						y=0x80;
						for(j=0;j<32;j++)								/*写32行，即上半屏*/
						{
									OLED_W_Command(0x34);
									OLED_W_Command(y + j);
									OLED_W_Command(x);
									OLED_W_Command(0x30);
									for(k=0;k<16;k++)					/*每次写一行，一共128个点，即16个字节*/
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



/*RT12864M-1显示屏一行的显示
 *unsigned char x：显示内容所在的行数，x=1代表第一行，x=2代表第二行，x=3代表第三行，x=4代表第四行
 *char *p要显示的内容是一个16字节的字符串，譬如"0123456789012345"、"1234显示屏567089"
 */							
void rt12864m_show(unsigned char x, char *p)
{
			uint8_t i = 0;
	
			if(1 ==x)			/*第一行显示*/
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
			else if(2 ==x)			/*第二行显示*/
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
			else if(3 ==x)			/*第二行显示*/
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
			else if(4 ==x)			/*第二行显示*/
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
/**************************实现函数******************************************** 
*函数原型:		void OLED_OpenMenu_SL3(void)
*功　　能:		雨量报警监测仪的开机菜单
*入口参数:无
*******************************************************************************/ 
void OLED_OpenMenu_SL3(void)
{
			char a[] = {"                "};
			char b[] = {"SL3-1A          "};
			char c[] = {"  雨量监测报警仪"};
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
//		OLED_China(32, 16, 19, 0);//雨
//		OLED_China(32, 32, 20, 0);//量
//		OLED_China(32, 48, 21, 0);//报
//		OLED_China(32, 64, 22, 0);//警
//		OLED_China(32, 80, 23, 0);//监
//		OLED_China(32, 96, 24, 0);//测
//		OLED_China(32, 112, 25, 0);//仪
//	
//		OLED_Refresh_Gram();
}



/**************************实现函数******************************************** 
*函数原型:		void OLED_OpenMenu_SL3(void)
*功　　能:		雨量报警监测仪的开机菜单
*入口参数:无
*******************************************************************************/ 
void OLED_OpenMenu_EN2(void)
{
			char a[] = {"                "};
			char b[] = {"EN2-B           "};
			char c[] = {"      风向风速仪"};
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
//		OLED_China(32, 16, 19, 0);//雨
//		OLED_China(32, 32, 20, 0);//量
//		OLED_China(32, 48, 21, 0);//报
//		OLED_China(32, 64, 22, 0);//警
//		OLED_China(32, 80, 23, 0);//监
//		OLED_China(32, 96, 24, 0);//测
//		OLED_China(32, 112, 25, 0);//仪
//	
//		OLED_Refresh_Gram();
}

//初始化KS0108	驱动器驱动的RT12864M-1显示屏			    
void OLED_Init(void)
{
			OLED_CS_H();							//选择并口方式
			HAL_Delay(1000);						//延时800ms
	
			OLED_W_Command(0x30); 		//8位并口，使用基本指令集
			OLED_Buzy();							/*等待显示屏不忙*/
	
			OLED_W_Command(0x30); 		//8位并口，使用基本指令集
			OLED_Buzy();							/*等待显示屏不忙*/
	
			OLED_W_Command(0x0c); 		//打开显示
			OLED_Buzy();							/*等待显示屏不忙*/
	
			OLED_W_Command(0x01); 		//清除显示
			HAL_Delay(2);							//延时2毫秒
	
			OLED_W_Command(0x06); 		//显示内容不移动
			OLED_Buzy();							/*等待显示屏不忙*/
	
			OLED_W_Command(0x02); 		//地址归为，地址从零开始
			OLED_Buzy();							/*等待显示屏不忙*/
	
			OLED_W_Command(0x14); 		//显示移动
			OLED_Buzy();							/*等待显示屏不忙*/
			
			OLED_W_Command(0x30); 		//8位并口，使用基本指令集
			OLED_Buzy();							/*等待显示屏不忙*/
}  


/*绘图方式子程序*/
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

/*上半屏绘图清屏子程序*/
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


/*上半屏绘图清屏子程序*/
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
/*显示自定义16*16点阵*/
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
			OLED_W_Command(0x01); 		//清除显示
			HAL_Delay(2);							//延时2毫秒
	
//			uint8_t i,n; 
//			for(i=0;i<64;i++) 
//				 for(n=0;n<16;n++)
//						OLED_GRAM[i][n]=0x00;
//				OLED_Refresh_Gram();//更新显示
} 

/*无法读取忙标志，只有延时一段时间来等待显示屏不忙，写命令一次所需时间为72us，所以延时时间>=72us*/
static void OLED_Buzy(void) 
{
			volatile unsigned int i = 0;
			for(i = 0; i < 100; i++)			/*169肯定不行，170可以*/
			{
						__nop();
			}
}

//全屏显示字符2,一共显示16个字符，多出的字符不显示
void test_oled2(void)
{
			char a[] = "上海ShangHai上海";
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

//全屏显示字符
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

/*显示汉字*/
void test_oled(void)
{		
			uint8_t i = 0, j = 0, k = 0, l = 0;
			char a[] = {"上海气象仪器厂SH"};
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
			HAL_Delay(5);							//延时1毫秒
			
			
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
			HAL_Delay(5);							//延时1毫秒
			
			
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
			HAL_Delay(5);							//延时1毫秒
			
			
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
			HAL_Delay(5);							//延时1毫秒
			
			
			OLED_W_Command(0x30);
			OLED_W_Command(0x34);
			OLED_W_Command(0x04);
			OLED_W_Command(0x30);
}

/*反白显示*/
void test_oled3(void)
{
			uint8_t i=0;
//			OLED_W_Command(0x06);		/*光标右移，显示右移*/
//			OLED_W_Command(0x0e);		/*光标开，不反白闪烁*/
	
			OLED_W_Command(0x80);		/*反*/
			OLED_W_Data(0xb7);
			OLED_W_Data(0xb4);
//			HAL_Delay(800);
	
			OLED_W_Command(0x81);		/*白*/
			OLED_W_Data(0xb0);
			OLED_W_Data(0xd7);
//			HAL_Delay(800);
	
			OLED_W_Command(0x82);		/*显*/
			OLED_W_Data(0xcf);
			OLED_W_Data(0xd4);
//			HAL_Delay(800);
	
			OLED_W_Command(0x83);		/*示*/
			OLED_W_Data(0xca);
			OLED_W_Data(0xbe);
//			HAL_Delay(800);
			
			OLED_W_Command(0x84);		/*演*/
			OLED_W_Data(0xd1);
			OLED_W_Data(0xdd);
//			HAL_Delay(800);
			
			OLED_W_Command(0x85);		/*示*/
			OLED_W_Data(0xca);
			OLED_W_Data(0xbe);
//			HAL_Delay(800);
			
			OLED_W_Command(0x86);		/*说*/
			OLED_W_Data(0xcb);
			OLED_W_Data(0xb5);
//			HAL_Delay(800);
			
			OLED_W_Command(0x87);		/*明*/
			OLED_W_Data(0xc3);
			OLED_W_Data(0xf7);
//			HAL_Delay(800);
			
//			OLED_W_Command(0x88);		/*：*/
//			OLED_W_Data(0xc3);
//			OLED_W_Data(0xf7);
//			HAL_Delay(800);
			
//			OLED_W_Command(0x0c);		/*光标关*/
			
			OLED_W_Command(0x90);		/*瑞*/
			OLED_W_Data(0xc8);
			OLED_W_Data(0xf0);
			
			OLED_W_Command(0x91);		/*特*/
			OLED_W_Data(0xcc);
			OLED_W_Data(0xd8);
//			HAL_Delay(1600);
			
//			OLED_W_Command(0x92);		/*+*/
//			OLED_W_Data(0xa3);
//			OLED_W_Data(0xab);
//			HAL_Delay(1600);
			
			pic_clear_u();				/*上班屏清屏*/
			pic_u(0x02, 0x10, 0x92, 0x90, TAB6);
//			HAL_Delay(1600);
			
//			OLED_W_Command(0x92);		/*=*/
//			OLED_W_Data(0xa3);
//			OLED_W_Data(0xbd);
//			HAL_Delay(1600);
			
			OLED_W_Command(0x92);		/*瑞*/
			OLED_W_Data(0xc8);
			OLED_W_Data(0xf0);
			
			OLED_W_Command(0x93);		/*特*/
			OLED_W_Data(0xcc);
			OLED_W_Data(0xd8);
//			HAL_Delay(1600);
			
			pic_u(0x02, 0x10, 0x95, 0x90, TAB6);
//			HAL_Delay(1600);
			
			OLED_W_Command(0x94);		/*绘*/
			OLED_W_Data(0xbb);
			OLED_W_Data(0xe6);
//			HAL_Delay(1600);
			
			OLED_W_Command(0x95);		/*图*/
			OLED_W_Data(0xcd);
			OLED_W_Data(0xbc);
//			HAL_Delay(1600);
			
			OLED_W_Command(0x96);		/*演*/
			OLED_W_Data(0xd1);
			OLED_W_Data(0xdd);
			HAL_Delay(1600);
//			
//			OLED_W_Command(0x91);		/*示*/
//			OLED_W_Data(0xca);
//			OLED_W_Data(0xbe);
//			HAL_Delay(1600);
			
			
}

/*显示自定义点阵函数*/
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

/*点阵显示*/
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

/*画点函数演示*/
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
/**************************实现函数******************************************** 
*函数原型:		unsigned char OLED_W_Command(void); 
*功　　能:		写命令到OLED上 
*******************************************************************************/ 
void OLED_W_Command(unsigned int com)
{
		unsigned int i;
		
//		OLED_Buzy();							/*等待显示屏不忙*/
		OLED_RS_L();						/*RS低电平为写命令操作*/
	
		com &= 0xff;
		com <<= 1;
		DATAOUT(com);

		OLED_E_H();
		OLED_Buzy();							/*等待显示屏不忙*/
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
		OLED_Buzy();							/*等待显示屏不忙*/
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


/**************************实现函数******************************************** 
*函数原型:		void OLED_W_Data(unsigned dat); 
*功　　能:		写数据到oled显示屏。 
*******************************************************************************/ 
void OLED_W_Data(unsigned int dat)
{
		unsigned int i;
	
//		OLED_Buzy();						/*等待显示屏不忙*/

		OLED_RS_H();							/*RS高电平为写数据操作*/
		
		dat &= 0x000000ff;
		dat <<= 1;
		DATAOUT(dat);

		OLED_E_H();
		OLED_Buzy();							/*等待显示屏不忙*/
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
		OLED_Buzy();							/*等待显示屏不忙*/
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


/**************************实现函数******************************************** 
*函数原型:		void OLED_DrawPoint(uint8_t x,uint8_t y,uint8_t t) 
*功　　能:		在x行 ，y列画一个点
*入口参数：x:0~63；y:0~127;t:1填充，0清空
*******************************************************************************/ 
void OLED_DrawPoint(unsigned char x,unsigned char y,unsigned char t) 
{ 
	uint8_t pos,bx,temp=0;
	x=x&0x3f;   //参数过滤  避免x>64  y>128
	y=y&0x7f;
	pos=y/8;   //对应在OLED_GRAM中的页
	bx=y%8;
	temp=1<<(bx);			/*顺向，若temp=1<<(7-bx)为逆向*/
	if(t)
		OLED_GRAM[x][pos]|=temp;
	else 
		OLED_GRAM[x][pos]&=~temp;	    
} 


/**************************实现函数******************************************** 
*函数原型:		void OLED_Char(uint8_t x,uint8_t y,unsigned char chr)
*功　　能:		在定点写一个8*16的ASCII码
*入口参数:x行(0<x<64)  y页（0<y<128）  highlight=ture反显，highlight=false正显
*******************************************************************************/ 
void OLED_Char(unsigned char x,unsigned char y,unsigned char chr,unsigned char highlight)
{
			uint8_t t,temp,t1;
			uint8_t y0=y;
			x=x&0x3f;   //参数过滤  避免x>63  y>127
			y=y&0x7f;
			chr=chr - ' ';//得到偏移后的值
			for(t=0;t<16;t++)             
			{
						temp=asc2_1608[chr][t];
						for(t1=0;t1<8;t1++)
						{
							if(highlight == false)//正显
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
							else//反显
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

/**************************实现函数******************************************** 
*函数原型:		void OLED_ShowString(uint8_t x,uint8_t y,const uint8_t *p)
*功　　能:		显示字符串
*入口参数:x行(0=<x=<63)  y列（0=<y=<127）  highlight=ture反显，highlight=false正显
*******************************************************************************/ 
void OLED_ShowString(unsigned char x,unsigned char y,const char *p,unsigned char highlight)
{	
    while((*p<='~')&&(*p>=' '))//判断是不是非法字符!
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


/**************************实现函数******************************************** 
*函数原型:		void OLED_China(uint8_t x,uint8_t y,unsigned char cha)
*功　　能:		在定点写一个16*16的汉字
*入口参数:x行(0=<x=<63)  y页（0=<y=<127）  highlight=ture反显，highlight=false正显
*******************************************************************************/ 
void OLED_China(unsigned char x,unsigned char y,unsigned char n,unsigned char highlight)
{
	uint8_t t,t1,temp;
	uint8_t y0=y,x0=x;
	x=x&0x3f;   //参数过滤  避免x>63  y>127
	y=y&0x7f;
	for(t=0;t<32;t++)             
	{
		temp=china_1616[n][t];
		for(t1=0;t1<8;t1++)    //画点函数8个点一画
		{
			if(highlight==false)//正显
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
//		for(t1=0;t1<8;t1++)    //画点函数8个点一画
//		{
//			if(highlight==false)//正显
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

/**************************实现函数******************************************** 
*函数原型:		void OLED_China(uint8_t x,uint8_t y,uint8_t i)
*功　　能:		在定点写一个24*24的汉字
*入口参数:x列(0<x<127)  y页（0<y<63）
*******************************************************************************/ 
void OLED_China1(uint8_t x,uint8_t y,uint8_t i)
{
	uint8_t t,t1,temp;
	uint8_t y0=y;
	x=x&0x7f;   //参数过滤  避免x>127  y>63
	y=y&0x3f;
//	chr=chr-'便';//得到偏移后的值
	for(t=0;t<72;t++)             
	{
		temp=china_2424[i][t];
		for(t1=0;t1<8;t1++)    //画点函数8个点一画
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


/**************************实现函数******************************************** 
*函数原型:		void OLED_Huatu(uint8_t x,uint8_t y,uint8_t i)
*功　　能:		在定点画一个图形
*入口参数:x列(0<x<127)  y页（0<y<63）
*******************************************************************************/ 
void OLED_Huatu(uint8_t x,uint8_t y,uint8_t i)
{
	uint8_t t,t1,temp;
	uint8_t y0=y;
	x=x&0x7f;   //参数过滤  避免x>127  y>63
	y=y&0x3f;
//	chr=chr-'便';//得到偏移后的值
	for(t=0;t<48;t++)             
	{
		temp=DianLiang[i][t];
		for(t1=0;t1<8;t1++)    //画点函数8个点一画
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

/**************************实现函数******************************************** 
*函数原型:		void OLED_OpenMenu(void)
*功　　能:		开机菜单
*入口参数:无
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
	
		OLED_Refresh_Gram();		//更新显示到OLED 
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
//			OLED_Refresh_Gram();//更新显示
//} 

////初始化KS0108	驱动器驱动的RT12864J-1显示屏			    
//void OLED_Init(void)
//{
//			OLED_CS_H();								/*选中左半屏*/
//			OLED_W_Command(0x3e); 		//display off显示关
//			OLED_CS_L();								/*选中右半屏*/
//			OLED_W_Command(0x3e); 		//display off显示关
//			HAL_Delay(1);						//延时1毫秒
//	
//			OLED_CS_H();								/*选中左半屏*/
//			OLED_W_Command(0xc0); 		//设置显示起始行为0
//			OLED_CS_L();								/*选中右半屏*/
//			OLED_W_Command(0xc0); 		//设置显示起始行为0
//			HAL_Delay(1);						//延时1毫秒
//	
//			OLED_CS_H();								/*选中左半屏*/
//			OLED_W_Command(0xb8); 		//设置起始页地址为0
//			OLED_CS_L();								/*选中右半屏*/
//			OLED_W_Command(0xb8); 		//设置起始页地址为0
//			HAL_Delay(1);	

//			OLED_CS_H();								/*选中左半屏*/
//			OLED_W_Command(0x40); 		//设置起始列地址为0
//			OLED_CS_L();								/*选中右半屏*/
//			OLED_W_Command(0x40); 		//设置起始列地址为0
//			HAL_Delay(1);						//延时1毫秒
//			
//			OLED_CS_H();								/*选中左半屏*/
//			OLED_W_Command(0x3f); 		//display on显示开
//			OLED_CS_L();								/*选中右半屏*/
//			OLED_W_Command(0x3f); 		//display on显示开
//			HAL_Delay(1);						//延时1毫秒
//}  


////更新显存到LCD		 
//void OLED_Refresh_Gram(void)
//{
//		uint8_t i,n;		    
//		for(i = 0; i < 8; i++)  
//		{
//					OLED_CS_H();								/*选中左半屏*/
//					OLED_W_Command (0xb8+i);    //设置页地址（0~7）
//					OLED_W_Command (0x40);      //设置显示列地址
//			
//					OLED_CS_L();								/*选中右半屏*/
//					OLED_W_Command (0xb8+i);    //设置页地址（0~7）
//					OLED_W_Command (0x40);      //设置显示列地址
//			
//					for(n = 0; n < 128; n++)
//					{
//								if((n < 64) && (n >= 0))
//								{
//											OLED_CS_H();								/*选中左半屏*/
//								}
//								else if((n < 128) && (n > 63))
//								{
//											OLED_CS_L();								/*选中右半屏*/
//								}
//								OLED_W_Data(OLED_GRAM[n][i]); 
//					}
//						
//		}
//}


///**************************实现函数******************************************** 
//*函数原型:		unsigned char OLED_W_Command(void); 
//*功　　能:		写命令到OLED上 
//*******************************************************************************/ 
//void OLED_W_Command(unsigned int com)
//{
//		unsigned int i;
//		
////		OLED_CS_H();						/*选中2个半屏*/
//		OLED_RS_L();						/*RS低电平为写命令操作*/
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


///**************************实现函数******************************************** 
//*函数原型:		void OLED_W_Data(unsigned dat); 
//*功　　能:		写数据到oled显示屏。 
//*******************************************************************************/ 
//void OLED_W_Data(unsigned int dat)
//{
//		unsigned int i;
//	
////		OLED_CS_H();						/*选中2个半屏*/
//		OLED_RS_H();						/*RS高电平为写数据操作*/
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



