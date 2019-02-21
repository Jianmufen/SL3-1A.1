#ifndef __LCD_H
#define __LCD_H


#ifdef __cplusplus
 extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx_hal.h"  
#include "main.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������V3
//SSD1306 OLED ����IC��������
//������ʽ:8080����/4�ߴ���
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2015/1/14
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 


//OLEDģʽ����
//����8080ģʽ ��ģ���BS1��BS2����VCC��
		    						  
//---------------------------OLED�˿ڶ���--------------------------  					   
    
/* DC */
#define OLED_DC(x)        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9,x)
     
/* Write*/
#define OLED_Write(x)     HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10,x)

/* Read */
#define OLED_Read(x)      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11,x)
   
/* CS */
#define OLED_CS(x)        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8,x)
  

#define DATAOUT(x) GPIOB->ODR=(GPIOB->ODR&0xff00)|(x&0x00FF); //���
     
extern uint8_t number;
extern uint8_t number1;
extern uint8_t number2;
extern uint8_t number3;
extern uint8_t number4;
extern uint8_t number5;

extern uint8_t a1,a2,a3,a4,a5,a6,a7,a8;
extern uint8_t b1,b2,b3,b4,b5,b6,b7;
extern uint8_t c1,c2,c3,c4,c5,c6,c7;
extern uint8_t d4,e4;

extern char disp_buf[128];

void OLED_W_Command(unsigned char com);	
void OLED_W_Data(unsigned char dat); 		    
void OLED_Init(void);
void OLED_Clear(void);
void OLED_Char(unsigned char x,unsigned char y,unsigned char chr,uint8_t i);
void OLED_Refresh_Gram(void);
void OLED_DrawPoint(unsigned char x,unsigned char y,unsigned char t);
void OLED_ShowString(unsigned char x,unsigned char y,const char *p,uint8_t i);
void OLED_China(unsigned char x,unsigned char y,unsigned char n,uint8_t i);
void OLED_China1(unsigned char x,unsigned char y,uint8_t i);
void OLED_OpenMenu(void);
void OLED_ZhuMenu(void);
void OLED_SheZhiMenu(void);
void OLED_AverageMenu(void);
void OLED_YingJianMenu(void);
void OLED_Huatu(uint8_t x,uint8_t y,uint8_t i);
void OLED_DataMenu(void);
void OLED_DingZhengMenu(void);
void OLED_CunChuMenu(void);
#ifdef __cplusplus
}
#endif
#endif 

	 







 

