/****************************************Copyright (c)****************************************************
**
**
**
**--------------File Info---------------------------------------------------------------------------------
** File name:				LCM_DRIVE.h
** Latest modified Date:    2008-03-07
** Latest Version:          1.0
** Descriptions:            ��������KS0107B��ȡģ��ʽ�������ֽڵ���
**
**--------------------------------------------------------------------------------------------------------
** Created by:              JGW
** Created date:            2008-10-27
** Version:                 1.0
** Descriptions:            The original version
**
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Descriptions:
**
*********************************************************************************************************/

#ifndef __LCD_DRIVE_H
#define __LCD_DRIVE_H


#ifdef __cplusplus
 extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx_hal.h"
#include "main.h"

   
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/


/**
  * @brief  LCD Control Interface pins
  */
/* LCD -- BL:lcd backlight */
#define LCD_BL_PIN                              GPIO_PIN_9
#define LCD_BL_PORT                             GPIOB
#define LCD_BL_GPIO_CLK_ENABLE()                __HAL_RCC_GPIOB_CLK_ENABLE()
#define LCD_BL_GPIO_CLK_DISABLE()               __HAL_RCC_GPIOB_CLK_DISABLE()

/* LCD -- BUSY */   /* unused */
#define LCD_BUSY_PIN                            GPIO_PIN_7
#define LCD_BUSY_PORT                           GPIOE
#define LCD_BUSY_GPIO_CLK_ENABLE()              __HAL_RCC_GPIOE_CLK_ENABLE()
#define LCD_BUSY_GPIO_CLK_DISABLE()             __HAL_RCC_GPIOE_CLK_DISABLE()
   
/* LCD -- EN */
#define LCD_EN_PIN                              GPIO_PIN_11
#define LCD_EN_PORT                             GPIOE
#define LCD_EN_GPIO_CLK_ENABLE()                __HAL_RCC_GPIOE_CLK_ENABLE()
#define LCD_EN_GPIO_CLK_DISABLE()               __HAL_RCC_GPIOE_CLK_DISABLE()
   
/* LCD -- DI */
#define LCD_DI_PIN                              GPIO_PIN_8
#define LCD_DI_PORT                             GPIOE
#define LCD_DI_GPIO_CLK_ENABLE()                __HAL_RCC_GPIOE_CLK_ENABLE()
#define LCD_DI_GPIO_CLK_DISABLE()               __HAL_RCC_GPIOE_CLK_DISABLE()
   
/* LCD -- RW */  /* unused */
#define LCD_RW_PIN                              GPIO_PIN_11
#define LCD_RW_PORT                             GPIOE
#define LCD_RW_GPIO_CLK_ENABLE()                __HAL_RCC_GPIOE_CLK_ENABLE()
#define LCD_RW_GPIO_CLK_DISABLE()               __HAL_RCC_GPIOE_CLK_DISABLE()
   
/* LCD -- CS1 */
#define LCD_CS1_PIN                             GPIO_PIN_9
#define LCD_CS1_PORT                            GPIOE
#define LCD_CS1_GPIO_CLK_ENABLE()               __HAL_RCC_GPIOE_CLK_ENABLE()
#define LCD_CS1_GPIO_CLK_DISABLE()              __HAL_RCC_GPIOE_CLK_DISABLE()
   
/* LCD -- CS2 */
#define LCD_CS2_PIN                             GPIO_PIN_10
#define LCD_CS2_PORT                            GPIOE
#define LCD_CS2_GPIO_CLK_ENABLE()               __HAL_RCC_GPIOE_CLK_ENABLE()
#define LCD_CS2_GPIO_CLK_DISABLE()              __HAL_RCC_GPIOE_CLK_DISABLE()

   
/* LCD -- DATA */
#define LCD_DATA_PINS                           ( GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 )
#define LCD_DATA_PORT                            GPIOE
#define LCD_DATA_GPIO_CLK_ENABLE()               __HAL_RCC_GPIOE_CLK_ENABLE()
#define LCD_DATA_GPIO_CLK_DISABLE()              __HAL_RCC_GPIOE_CLK_DISABLE()
   
   
/**
  * @brief  LCD Control Lines management
  */  
    
/* BL */
#define LCD_BL_ON()                           HAL_GPIO_WritePin(LCD_BL_PORT, LCD_BL_PIN, GPIO_PIN_RESET)
#define LCD_BL_OFF()                          HAL_GPIO_WritePin(LCD_BL_PORT, LCD_BL_PIN, GPIO_PIN_SET)
    
/* EN */
#define LCD_EN_LOW()                           HAL_GPIO_WritePin(LCD_EN_PORT, LCD_EN_PIN, GPIO_PIN_RESET)
#define LCD_EN_HIGH()                          HAL_GPIO_WritePin(LCD_EN_PORT, LCD_EN_PIN, GPIO_PIN_SET)
   
/* DI */
#define LCD_DI_LOW()                           HAL_GPIO_WritePin(LCD_DI_PORT, LCD_DI_PIN, GPIO_PIN_RESET)
#define LCD_DI_HIGH()                          HAL_GPIO_WritePin(LCD_DI_PORT, LCD_DI_PIN, GPIO_PIN_SET)

/* RW */  /* unused */
#define LCD_RW_LOW()                           /*HAL_GPIO_WritePin(LCD_RW_PORT, LCD_RW_PIN, GPIO_PIN_RESET)*/
#define LCD_RW_HIGH()                          /*HAL_GPIO_WritePin(LCD_RW_PORT, LCD_RW_PIN, GPIO_PIN_SET)*/
   
/* CS1 */
#define LCD_CS1_LOW()                          HAL_GPIO_WritePin(LCD_CS1_PORT, LCD_CS1_PIN, GPIO_PIN_RESET)
#define LCD_CS1_HIGH()                         HAL_GPIO_WritePin(LCD_CS1_PORT, LCD_CS1_PIN, GPIO_PIN_SET)
   
/* CS2 */
#define LCD_CS2_LOW()                          HAL_GPIO_WritePin(LCD_CS2_PORT, LCD_CS2_PIN, GPIO_PIN_RESET)
#define LCD_CS2_HIGH()                         HAL_GPIO_WritePin(LCD_CS2_PORT, LCD_CS2_PIN, GPIO_PIN_SET)
   
   
/**
  * @}
  */

/**
  * @}
  */

extern unsigned char const CHSStr1[];

/***********************************************************************
* ���ƣ�	LCD_Check_Busy()
* ���ܣ�	�ȴ�LCD�������
* ��ڲ�����
***********************************************************************/
void LCD_Check_Busy(void);

/***********************************************************************
* ���ƣ�LCM_WriteByte()
* ���ܣ���ָ����д����(һ�ֽ�)��
* ��ڲ�����x 		x����ֵ(0-127)
*	        y       y����ֵ(0-8)
*          wrdata	��Ҫд������
* ���ڲ�������
* ˵���������disp_buf��Ӧ�洢��Ԫ
***********************************************************************/
void  LCM_WriteByte(unsigned char x, unsigned char  y, unsigned char wrdata);

/***********************************************************************
* ���ƣ�LCM_DispFill()
* ���ܣ�����ʾ���������
* ��ڲ�����filldata  	Ҫд��LCM���������
* ���ڲ�������
* ˵���������disp_buf��Ӧ�洢��Ԫ
***********************************************************************/
void  LCM_DispFill(unsigned char filldata);

/***********************************************************************
* ���ƣ�LCM_DispIni()
* ���ܣ�LCM��ʾ��ʼ����ʹ����ʾ��������ʾ��ʼ��Ϊ0��������
* ��ڲ�������
* ���ڲ�������
***********************************************************************/
void  LCM_DispInit(void);

/***********************************************************************
* ���ƣ�LCM_DispChar()
* ���ܣ�ָ����ַ��ʾ�ַ���
* ��ڲ�����disp_cy		��ʾ��ֵ(0-7)
*	   disp_cx		��ʾ��ֵ(0-15)
*	   dispdata		��Ҫ��ʾ���ַ�(ASCII��)
* ע��֧����ʾ�ַ�0-9��A-Z��a-z���ո��ַ���ʾ��ʽΪ5*7��ģΪ8*8������
*     ��Ļ��ʾΪ8*16(��8�У�ÿ��16���ַ�)��
***********************************************************************/
void LCM_DispChar(unsigned char disp_cy, unsigned char disp_cx, char dispdata);

/***********************************************************************
* ���ƣ�LCM_DispStr()
* ���ܣ��ַ�����ʾ�����
* ��ڲ�����disp_cy 	��ʾ��ʼ��(0-7)
*	   disp_cx      ��ʾ��ʼ��(0-15)
*	   disp_str	�ִ�ָ��
* ���ڲ�������
* ע��֧����ʾ�ַ�0-9��A-Z��a-z���ո��ַ���ʾ��ʽΪ5*7��ģΪ8*8��������Ļ��ʾ
*     Ϊ8*16(��8�У�ÿ��16���ַ�)��
***********************************************************************/
void  LCM_DispStr(unsigned char disp_cy, unsigned char disp_cx, const char *disp_str);

/***********************************************************************
* ���ƣ�LCM_DispCHS()
* ���ܣ��ַ�����ʾ�����
* ��ڲ�����DisCHS_y    ��ʾ��ʼ��(0-3)
*	        DisCHS_x    ��ʾ��ʼ��(0-7)
*	        pStr	    �ִ�ָ��
* ���ڲ�������
* ע��֧����ʾ�����ַ���ģΪ16*16��������Ļ��ʾ
*     Ϊ4*8(��4�У�ÿ��8���ַ�)����Ӹ�����ʾ��
***********************************************************************/
void LCM_DispCHS(unsigned char DisCHS_y, unsigned char DisCHS_x,const unsigned char *pStr,unsigned char highlight);

/***********************************************************************
* ���ƣ�LCM_DispCHSStr()
* ���ܣ��ַ�����ʾ�����
* ��ڲ�����DisCHS_y    ��ʾ��ʼ��(0-3)
*	        DisCHS_x    ��ʾ��ʼ��(0-7)
*	        pStr	    �ִ�ָ��
* ���ڲ�������
* ע��֧����ʾ�����ַ���ģΪ16*16��������Ļ��ʾ
*     Ϊ4*8(��4�У�ÿ��8���ַ�)����Ӹ�����ʾ��
***********************************************************************/
void LCM_DispCHSStr(unsigned char DisCHS_y, unsigned char DisCHS_x,unsigned char Num,const unsigned char *pStr,unsigned char highlight);


/***********************************************************************
* ���ƣ�LCM_DispChar8_16()
* ���ܣ�ָ����ַ��ʾ�ַ���
* ��ڲ�����disp_cy		��ʾ��ֵ(0-3)
*	   disp_cx		��ʾ��ֵ(0-15)
*	   dispdata		��Ҫ��ʾ���ַ�(ASCII��)
* ע��֧����ʾ�ַ�0-9��A-Z��a-z���ո��ַ���ʾ��ʽΪ5*7��ģΪ8*16������
*     ��Ļ��ʾΪ4*16(��4�У�ÿ��16���ַ�)��֧�ָ�����ʾ��
***********************************************************************/
void LCM_DispChar8_16(unsigned char disp_cy,unsigned char disp_cx,char dispdata,unsigned char highlight);
/***********************************************************************
* ���ƣ�LCM_DispStr8_16()
* ���ܣ��ַ�����ʾ�����
* ��ڲ�����disp_cy 	��ʾ��ʼ��(0-3)
*	   disp_cx      ��ʾ��ʼ��(0-15)
*	   disp_str	�ִ�ָ��
* ���ڲ�������
* ע��֧����ʾ�ַ�0-9��A-Z��a-z���ո��ַ���ʾ��ʽΪ5*7��ģΪ8*16��������Ļ��ʾ
*     Ϊ4*16(��4�У�ÿ��16���ַ�)��
***********************************************************************/
void  LCM_DispStr8_16(unsigned char disp_cy, unsigned char disp_cx, const char *disp_str,unsigned char highlight);




#ifdef __cplusplus
}
#endif
#endif  /* __LCD_DRIVE_H */
