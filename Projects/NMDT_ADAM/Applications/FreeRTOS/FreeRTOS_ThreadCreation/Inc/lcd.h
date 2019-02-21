#ifndef __LCD_H
#define __LCD_H


#ifdef __cplusplus
 extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx_hal.h"  
#include "main.h"

		    						  
//---------------------------OLED端口定义--------------------------  					   
    
/* RS 命令数据选择位，高电平写数据，低电平写命令*/
#define OLED_RS_H()        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET)
#define OLED_RS_L()        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET)
     
/* E为高电平时读，E为下降沿时写操作有效*/
#define OLED_E_H()     HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET)
#define OLED_E_L()     HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET)

/* CS 片选信号 高电平有效 选中左右两个半屏*/
#define OLED_CS_H()        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET)
#define OLED_CS_L()        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET)

#define DATAOUT(x) GPIOB->ODR = ((GPIOB->ODR & 0xfffffe01)|((x) & 0x1fe)); //输出
     
extern uint8_t OLED_GRAM[64][16];
extern char TAB6[128];
extern char TAB7[128];
extern char TAB8[128];

void OLED_W_Command(unsigned int com);	
void OLED_W_Data(unsigned int dat); 		    
void OLED_Init(void);
void OLED_Clear(void);
void OLED_Char(unsigned char x,unsigned char y,unsigned char chr,unsigned char highlight);
void OLED_Refresh_Gram(void);
void OLED_DrawPoint(unsigned char x,unsigned char y,unsigned char t);
void OLED_ShowString(unsigned char x,unsigned char y,const char *p,unsigned char highlight);
void OLED_China(unsigned char x,unsigned char y,unsigned char n, unsigned char highlight);
void OLED_China1(unsigned char x,unsigned char y, unsigned char highlight);
void OLED_OpenMenu(void);
void OLED_Huatu(uint8_t x,uint8_t y,uint8_t i);
void OLED_OpenMenu_SL3(void);
void OLED_OpenMenu_EN2(void);
void test_oled(void);
void test_oled1(void);
void test_oled2(void);
void test_oled3(void);
void test_oled4(void);
void shou_point(void);
void dz_d(unsigned int command1, char *p);

void rt12864m_show(unsigned char x, char *p);
void pic_clear_u(void);
void pic_u(unsigned int lon,unsigned int hight,unsigned int addx, unsigned int addy,char *p);
void pic_clear_d(void);
void pic_d(unsigned int lon,unsigned int hight,unsigned int addx, unsigned int addy,char *p);
void Lcd_Char(unsigned char x,unsigned char y,unsigned char chr,unsigned char highlight);
void Lcd_ShowString(unsigned char x,unsigned char y,const char *p,unsigned char highlight);

#ifdef __cplusplus
}
#endif
#endif 

	 







 

