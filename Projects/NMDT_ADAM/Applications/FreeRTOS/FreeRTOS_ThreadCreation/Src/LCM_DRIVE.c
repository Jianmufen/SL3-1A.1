/***********************************************************************
* �ļ�����LCM_DRIVE.C
* ���ܣ�ͼ��Һ��TG12864B-2��������
* ˵������LCM_DRIVE.H�ļ��ж�����LCM������ַ���������д���������ַΪ2004H��д
*      ���ݲ�����ַΪ2005H���Ұ�����д���������ַΪ2000H��д���ݲ�����ַΪ2001H��
*      ����GRAPHICS.C��ʹ����disp_buf��Ϊ��ͼ������������LCM_WriteByte()��LCM_
*      DispFill()��Ҫ����disp_buf��
***********************************************************************/
#include	"LCM_DRIVE.h"
#include        "Font8_16.h"    /* 8*16�����ַ� */



#define LCM_L  	1
#define LCM_R 	2

/* ����LCM�������� */
#define  LCM_XMAX	(128)
#define  LCM_YMAX	(64)

/*#define DIS_DLY()	do{ \
asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop"); \
  asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop"); \
    asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop"); \
      asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop"); \
        asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop"); \
          asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop"); \
} while(0)*/


/* ����LCM������������ */
#define	LCM_DISPON	0x3f									/* ��LCM��ʾ 							*/
#define LCM_STARTROW	0xc0									/* ��ʾ��ʼ��0��������LCM_STARTROW+x������ʼ�С�(x<64) 		*/
#define	LCM_ADDRSTRY	0xb8									/* ҳ��ʼ��ַ��������LCM_ADDRSTRX+x���õ�ǰҳ(����)��(x<8) 	*/
#define	LCM_ADDRSTRX	0x40									/* ����ʼ��ַ��������LCM_ADDRSTRY+x���õ�ǰ��(����)��(x<64) */




/** @addtogroup Private_Functions
  * @{
  */
static void LCD_IO_Init(void);
__STATIC_INLINE void DIS_DLY(void);
__STATIC_INLINE void LCD_IO_Write(unsigned char command);
static void LCM_Wr1Command(unsigned char wrdata);
static void LCM_Wr2Command(unsigned char wrdata);
static void LCM_Wr1Data(unsigned char wrdata);
static void LCM_Wr2Data(unsigned char wrdata);



/***********************************************************************
* ���ƣ�	LCD_IO_Init()
* ���ܣ�	��ʼ��LCD�õ���IO��
* ��ڲ�����
***********************************************************************/
static void LCD_IO_Init(void)
{
  GPIO_InitTypeDef  gpioinitstruct = {0};

  /* LCD_BL Periph clock enable */
  LCD_BL_GPIO_CLK_ENABLE();
  /* Configure LCD_BL_PIN pin */
  gpioinitstruct.Pin    = LCD_BL_PIN;
  gpioinitstruct.Mode   = GPIO_MODE_OUTPUT_PP;
  gpioinitstruct.Pull   = GPIO_PULLUP;
  gpioinitstruct.Speed  = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(LCD_BL_PORT, &gpioinitstruct);
  
  
  /* LCD_EN Periph clock enable */
  LCD_EN_GPIO_CLK_ENABLE();
  /* Configure LCD_EN_PIN pin */
  gpioinitstruct.Pin    = LCD_EN_PIN;
  HAL_GPIO_Init(LCD_EN_PORT, &gpioinitstruct);
  
  
  /* LCD_DI Periph clock enable */
  LCD_DI_GPIO_CLK_ENABLE();
  /* Configure LCD_EN_PIN pin */
  gpioinitstruct.Pin    = LCD_DI_PIN;
  HAL_GPIO_Init(LCD_DI_PORT, &gpioinitstruct);
  
  
  /* LCD_RW Periph clock enable */  /* unused */
  //LCD_RW_GPIO_CLK_ENABLE();
  /* Configure LCD_EN_PIN pin */
  //gpioinitstruct.Pin    = LCD_RW_PIN;
  //HAL_GPIO_Init(LCD_RW_PORT, &gpioinitstruct);
  
  
  /* LCD_CS1 Periph clock enable */
  LCD_CS1_GPIO_CLK_ENABLE();
  /* Configure LCD_EN_PIN pin */
  gpioinitstruct.Pin    = LCD_CS1_PIN;
  HAL_GPIO_Init(LCD_CS1_PORT, &gpioinitstruct);
  
  
  /* LCD_CS2 Periph clock enable */
  LCD_CS2_GPIO_CLK_ENABLE();
  /* Configure LCD_EN_PIN pin */
  gpioinitstruct.Pin    = LCD_CS2_PIN;
  HAL_GPIO_Init(LCD_CS2_PORT, &gpioinitstruct);
  
  
  /* LCD_DATA Periph clock enable */
  LCD_DATA_GPIO_CLK_ENABLE();
  /* Configure LCD_DATA_PINS pin */
  gpioinitstruct.Pin    = LCD_DATA_PINS;
  HAL_GPIO_Init(LCD_DATA_PORT, &gpioinitstruct);
  
  
}

__STATIC_INLINE void DIS_DLY(void)
{
  /*asm("nop");
  asm("nop");
  return;*/
  
  //�������Ż��ٶȣ���ʱ��С��ʾ����������free rtos����Ҫ����һ��
  volatile uint32_t i=50;
  for(i=0;i<50;i++);
	
//  while(i--)
//  {
//    asm("nop");
//  }
}


/***********************************************************************
* ���ƣ�	LCD_IO_Write()
* ���ܣ�	��ʼ��LCD�õ���IO��
* ��ڲ�����command  	Ҫд��LCM��������
***********************************************************************/
__STATIC_INLINE void LCD_IO_Write(unsigned char command)
{
  /*GPIOE->ODR&=0xFF00;
  GPIOE->ODR|=command;
  return;*/
  
  
  /*command=~command;
  uint32_t bsrr=(((uint32_t)command << 16)&0x00FF0000) |
                        ((~((uint32_t)command))&0x00FF);*/
  uint32_t bsrr=((~(uint32_t)command << 16)&0x00FF0000) | (command&0x00FF);
  //printf("cmd:0x%02X,bsrr:0x%08X\r\n",command,bsrr);
  
  LCD_DATA_PORT->BSRR = bsrr;
}

/**
  * @}
  */

/**
  * @}
  */





/***********************************************************************
* ���ƣ�	LCD_Check_Busy()
* ���ܣ�	�ȴ�LCD�������
* ��ڲ�����
***********************************************************************/
void LCD_Check_Busy(void)
{
	/*while(1) {
		LCD_EN_LOW();
		LCD_DI_LOW();
		LCD_RW = 1;
		P0 = 0xff;
		LCD_EN_HIGH();
		DIS_DLY();
		if(!LCD_Busy) break;
    }
    LCD_EN = 0;*/
}

/***********************************************************************
* ���ƣ�LCM_Wr1Command()
* ���ܣ�д�����ӳ�����ѡ��Ϊ�����(CS1)��
* ��ڲ�����command  	Ҫд��LCM��������
***********************************************************************/
static void LCM_Wr1Command(unsigned char wrdata)
{
    LCD_CS1_HIGH();
    LCD_CS2_LOW();
    
    /*DIS_DLY();*/

    LCD_EN_LOW();           										/* ʹ���ź�Ϊ�� 					*/
    LCD_DI_LOW();           										/* �üĴ���ѡ��Ϊ�� 				*/
    LCD_RW_LOW();           										/* �ö���дѡ��Ϊ�� 				*/
    LCD_IO_Write(wrdata);   										/* ������ָ���������������ݶ˿� 	        */
    LCD_EN_HIGH();           										/* ʹ���ź�Ϊ�� 					*/
    DIS_DLY();          										/* ��ʱ2us 							*/
    LCD_EN_LOW();           										/* ʹ���ź�Ϊ�� 					*/
}

/***********************************************************************
* ���ƣ�LCM_Wr2Command()
* ���ܣ�д�����ӳ�����ѡ��Ϊ�Ұ���(CS2)��
* ��ڲ�����command  	Ҫд��LCM��������
***********************************************************************/
static void LCM_Wr2Command(unsigned char wrdata)
{
    LCD_CS1_LOW();
    LCD_CS2_HIGH();
    
    /*DIS_DLY();*/

    LCD_EN_LOW();           										/* ʹ���ź�Ϊ�� 					*/
    LCD_DI_LOW();           										/* �üĴ���ѡ��Ϊ�� 				*/
    LCD_RW_LOW();           										/* �ö���дѡ��Ϊ�� 				*/
    LCD_IO_Write(wrdata);   										/* ������ָ���������������ݶ˿� 	        */
    LCD_EN_HIGH();           										/* ʹ���ź�Ϊ�� 					*/
    DIS_DLY();          										/* ��ʱ2us 							*/
    LCD_EN_LOW();           										/* ʹ���ź�Ϊ�� 					*/
}

/***********************************************************************
* ���ƣ�LCM_Wr1Data()
* ���ܣ�д�����ӳ�����ѡ��Ϊ�����(CS1)��
* ��ڲ�����wrdata  	Ҫд��LCM������
***********************************************************************/
static void LCM_Wr1Data(unsigned char wrdata)
{
	LCD_CS1_HIGH();
	LCD_CS2_LOW();
        
        /*DIS_DLY();*/

	LCD_Check_Busy();
	LCD_EN_LOW();
	LCD_DI_HIGH();             									/* �üĴ���ѡ��Ϊ�� 				*/
	LCD_RW_LOW();             									/* �ö���дѡ��Ϊ�� 				*/
	LCD_IO_Write(wrdata);   									/* ������ָ���������������ݶ˿� 	        */
	LCD_EN_HIGH();             									/* ʹ���ź�Ϊ�� 					*/
	DIS_DLY();            										/* ��ʱ2us 							*/
	LCD_EN_LOW();             									/* ʹ���ź�Ϊ�� 					*/
}

/***********************************************************************
* ���ƣ�LCM_Wr2Data()
* ���ܣ�д�����ӳ�����ѡ��Ϊ�Ұ���(CS2)��
* ��ڲ�����wrdata  	Ҫд��LCM������
***********************************************************************/
static void LCM_Wr2Data(unsigned char wrdata)
{
	LCD_CS1_LOW();
	LCD_CS2_HIGH();
        
        /*DIS_DLY();*/

	LCD_Check_Busy();
	LCD_EN_LOW();
	LCD_DI_HIGH();             									/* �üĴ���ѡ��Ϊ�� 				*/
	LCD_RW_LOW();             									/* �ö���дѡ��Ϊ�� 				*/
	LCD_IO_Write(wrdata);   									/* ������ָ���������������ݶ˿� 	        */
	LCD_EN_HIGH();             									/* ʹ���ź�Ϊ�� 					*/
	DIS_DLY();            										/* ��ʱ2us 							*/
	LCD_EN_LOW();             									/* ʹ���ź�Ϊ�� 					*/
}

/***********************************************************************
* ���ƣ�LCM_WriteByte()
* ���ܣ���ָ����д����(һ�ֽ�)��
* ��ڲ�����x 		x����ֵ(0-127)
*	        y       y����ֵ(0-8)
*          wrdata	��Ҫд������
* ���ڲ�������
* ˵���������disp_buf��Ӧ�洢��Ԫ
***********************************************************************/
void  LCM_WriteByte(unsigned char x, unsigned char  y, unsigned char wrdata)
{
    x &= 0x7f;                                                  // ��������
    y &= 0x07;
//   y = y>>3;
    if(x<64) {													// ѡ��Һ������оƬ(��CS1--����ǰ64���㣬CS2--���ƺ�64����)
        LCM_Wr1Command(LCM_ADDRSTRX+x);                         // ���õ�ǰ�е�ַ����x����
        LCM_Wr1Command(LCM_ADDRSTRY+y);							// ���õ�ǰҳ��ַ����y����
//        for(x=0; x<5; x++);									// ����ʱ
        LCM_Wr1Data(wrdata);
    }
    else {
        x = x-64;                                               // ����x����ֵ
        LCM_Wr2Command(LCM_ADDRSTRX+x);
        LCM_Wr2Command(LCM_ADDRSTRY+y);
//        for(x=0; x<5; x++);
        LCM_Wr2Data(wrdata);
    }
}

/***********************************************************************
* ���ƣ�LCM_DispFill()
* ���ܣ�����ʾ���������
* ��ڲ�����filldata  	Ҫд��LCM���������
* ���ڲ�������
* ˵���������disp_buf��Ӧ�洢��Ԫ
***********************************************************************/
void  LCM_DispFill(unsigned char filldata)
{
	unsigned char  x, y;

    LCM_Wr1Command(LCM_STARTROW);								// ������ʾ��ʼ��Ϊ0
    LCM_Wr2Command(LCM_STARTROW);

    for(y=0; y<8; y++) {
        LCM_Wr1Command(LCM_ADDRSTRY+y);                         // ����ҳ(��)��ַ
        LCM_Wr1Command(LCM_ADDRSTRX);							// �����е�ַ
        LCM_Wr2Command(LCM_ADDRSTRY+y);
        LCM_Wr2Command(LCM_ADDRSTRX);

        for(x=0; x<64; x++) {
            LCM_Wr1Data(filldata);
            LCM_Wr2Data(filldata);
        }
    }
}
/***********************************************************************
* ���ƣ�LCM_DispInit()
* ���ܣ�LCM��ʾ��ʼ����ʹ����ʾ��������ʾ��ʼ��Ϊ0��������
* ��ڲ�������
* ���ڲ�������
***********************************************************************/
void  LCM_DispInit(void)
{
	/*LCM_RST = 0;*/									/*	��λ����оƬ						*/
	/*DIS_DLY();*/
	/*LCM_RST = 1;*/
  
        LCD_IO_Init();                                                                          /*      ��ʼ��LCD�õ���IO��                                      */

	LCM_Wr1Command(LCM_DISPON);								/*	����ʾ							*/
	LCM_Wr1Command(LCM_STARTROW);								/*	������ʾ��ʼ��Ϊ0					*/
	LCM_Wr2Command(LCM_DISPON);
        LCM_Wr2Command(LCM_STARTROW);

	LCM_DispFill(0);									/*	����								*/

	LCM_Wr1Command(LCM_ADDRSTRY+0);								/*	����ҳ(��)��ַ						*/
	LCM_Wr1Command(LCM_ADDRSTRX+0);								/*	�����е�ַ������					*/
	LCM_Wr2Command(LCM_ADDRSTRY+0);
	LCM_Wr2Command(LCM_ADDRSTRX+0);
}

/* ASCII���Ӧ�ĵ������ݱ� */
unsigned char const  ASCII_TAB20[80] = {
				0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x9e, 0x00, 0x00,
         		0x00, 0x0e, 0x00, 0x0e, 0x00,
         		0x28, 0xfe, 0x28, 0xfe, 0x28,
         		0x48, 0x54, 0xfe, 0x54, 0x24,
         		0x46, 0x26, 0x10, 0xc8, 0xc4,
         		0x6c, 0x92, 0xaa, 0x44, 0xa0,
         		0x00, 0x0a, 0x06, 0x00, 0x00,
			 	0x00, 0x38, 0x44, 0x82, 0x00,
			 	0x00, 0x82, 0x44, 0x38, 0x00,
			 	0x28, 0x10, 0x7c, 0x10, 0x28,
			 	0x10, 0x10, 0x7c, 0x10, 0x10,
			 	0x00, 0xa0, 0x60, 0x00, 0x00,
			 	0x10, 0x10, 0x10, 0x10, 0x10,
			 	0x00, 0xc0, 0xc0, 0x00, 0x00,
			 	0x40, 0x20, 0x10, 0x08, 0x04
		               };

unsigned char const  ASCII_TAB30[80] = {
				0x7C, 0xA2, 0x92, 0x8A, 0x7C,
				0x00, 0x84, 0xFE, 0x80, 0x00,
				0x84, 0xC2, 0xA2, 0x92, 0x8C,
				0x42, 0x82, 0x8A, 0x96, 0x62,
				0x30, 0x28, 0x24, 0xFE, 0x20,
				0x4E, 0x8A, 0x8A, 0x8A, 0x72,
				0x78, 0x94, 0x92, 0x92, 0x60,
				0x02, 0xE2, 0x12, 0x0A, 0x06,
				0x6C, 0x92, 0x92, 0x92, 0x6C,
 				0x0C, 0x92, 0x92, 0x52, 0x3C,
				0x00, 0x6C, 0x6C, 0x00, 0x00,
				0x00, 0xAC, 0x6C, 0x00, 0x00,
				0x10, 0x28, 0x44, 0x82, 0x00,
				0x28, 0x28, 0x28, 0x28, 0x28,
				0x00, 0x82, 0x44, 0x28, 0x10,
				0x04, 0x02, 0xA2, 0x12, 0x0C
		       	       };

unsigned char const  ASCII_TAB40[80] = {
				0x64, 0x92, 0xF2, 0x82, 0x7C,
				0xFC, 0x22, 0x22, 0x22, 0xFC,
		 		0xFE, 0x92, 0x92, 0x92, 0x6C,
				0x7C, 0x82, 0x82, 0x82, 0x44,
				0xFE, 0x82, 0x82, 0x44, 0x38,
				0xFE, 0x92, 0x92, 0x92, 0x82,
				0xFE, 0x12, 0x12, 0x12, 0x02,
				0x7C, 0x82, 0x92, 0x92, 0xF4,
				0xFE, 0x10, 0x10, 0x10, 0xFE,
				0x00, 0x82, 0xFE, 0x82, 0x00,
				0x40, 0x80, 0x82, 0x7E, 0x02,
				0xFE, 0x10, 0x28, 0x44, 0x82,
				0xFE, 0x80, 0x80, 0x80, 0x80,
				0xFE, 0x04, 0x18, 0x04, 0xFE,
				0xFE, 0x08, 0x10, 0x20, 0xFE,
				0x7C, 0x82, 0x82, 0x82, 0x7C
		       	       };

unsigned char const  ASCII_TAB50[80] = {
				0xFE, 0x12, 0x12, 0x12, 0x0C,
				0x7C, 0x82, 0xA2, 0x42, 0xBC,
				0xFE, 0x12, 0x32, 0x52, 0x8C,
				0x8C, 0x92, 0x92, 0x92, 0x62,
				0x02, 0x02, 0xFE, 0x02, 0x02,
				0x7E, 0x80, 0x80, 0x80, 0x7E,
				0x3E, 0x40, 0x80, 0x40, 0x3E,
				0x7E, 0x80, 0x70, 0x80, 0x7E,
				0xC6, 0x28, 0x10, 0x28, 0xC6,
				0x0E, 0x10, 0xE0, 0x10, 0x0E,
				0xC2, 0xA2, 0x92, 0x8A, 0x86,
				0x00, 0xFE, 0x82, 0x82, 0x00,
				0x04, 0x08, 0x10, 0x20, 0x40,
				0x00, 0x82, 0x82, 0xFE, 0x00,
				0x08, 0x04, 0x02, 0x04, 0x08,
				0x80, 0x80, 0x80, 0x80, 0x80
		      	       };

unsigned char const  ASCII_TAB60[80] = {
				0x00, 0x02, 0x04, 0x08, 0x00,
				0x40, 0xA8, 0xA8, 0xA8, 0xF0,
				0xFE, 0x90, 0x88, 0x88, 0x70,
				0x70, 0x88, 0x88, 0x88, 0x40,
				0x70, 0x88, 0x88, 0x90, 0xFE,
				0x70, 0xA8, 0xA8, 0xA8, 0x30,
				0x10, 0xFC, 0x12, 0x02, 0x04,
				0x18, 0xA4, 0xA4, 0xA4, 0x7C,
				0xFE, 0x10, 0x08, 0x08, 0xF0,
				0x00, 0x88, 0xFA, 0x80, 0x00,
				0x40, 0x80, 0x88, 0x7A, 0x00,
				0xFE, 0x20, 0x50, 0x88, 0x00,
				0x00, 0x82, 0xFE, 0x80, 0x00,
				0xF8, 0x08, 0x30, 0x08, 0xF8,
				0xF8, 0x10, 0x08, 0x08, 0xF0,
				0x70, 0x88, 0x88, 0x88, 0x70
		               };

unsigned char const  ASCII_TAB70[80] = {
				0xF8, 0x28, 0x28, 0x28, 0x10,
				0x10, 0x28, 0x28, 0x30, 0xF8,
				0xF8, 0x10, 0x08, 0x08, 0x10,
				0x90, 0xA8, 0xA8, 0xA8, 0x40,
				0x08, 0x7E, 0x88, 0x80, 0x40,
				0x78, 0x80, 0x80, 0x40, 0xF8,
				0x38, 0x40, 0x80, 0x40, 0x38,
				0x78, 0x80, 0x60, 0x80, 0x78,
				0x88, 0x50, 0x20, 0x50, 0x88,
				0x18, 0xA0, 0xA0, 0xA0, 0x78,
				0x88, 0xC8, 0xA8, 0x98, 0x88,
				0x00, 0x10, 0x6C, 0x82, 0x00,
				0x00, 0x00, 0xFE, 0x00, 0x00,
				0x00, 0x82, 0x6C, 0x10, 0x00,
				0x10, 0x10, 0x54, 0x38, 0x10,
				0x10, 0x38, 0x54, 0x10, 0x10
		       	       };
/***********************************************************************
* ���ƣ�LCM_DispChar()
* ���ܣ�ָ����ַ��ʾ�ַ���
* ��ڲ�����disp_cy		��ʾ��ֵ(0-7)
*	   disp_cx		��ʾ��ֵ(0-15)
*	   dispdata		��Ҫ��ʾ���ַ�(ASCII��)
* ע��֧����ʾ�ַ�0-9��A-Z��a-z���ո��ַ���ʾ��ʽΪ5*7��ģΪ8*8������
*     ��Ļ��ʾΪ8*16(��8�У�ÿ��16���ַ�)��
***********************************************************************/
void LCM_DispChar(unsigned char disp_cy, unsigned char disp_cx, char dispdata)
{
    unsigned char  const  *pchardata;
    unsigned char  i;

    disp_cy = disp_cy&0x07;										// ��������
    disp_cx = disp_cx&0x0f;

    /*
    *	��Ҫ�ҳ���ʾ���ݵ����ͣ���������Ӧ�ĵ������ݱ�Ȼ������ָ�룬��ȡ�ö�Ӧ�ĵ������ݡ�
    */

    switch(dispdata&0xf0) {
        case 0x20:
            dispdata = (dispdata&0x0f)*5;
            pchardata = &ASCII_TAB20[dispdata];
            break;
        case 0x30:
            dispdata = (dispdata&0x0f)*5;
            pchardata = &ASCII_TAB30[dispdata];
            break;
        case 0x40:
            dispdata = (dispdata&0x0f)*5;
            pchardata = &ASCII_TAB40[dispdata];
            break;
        case 0x50:
            dispdata = (dispdata&0x0f)*5;
            pchardata = &ASCII_TAB50[dispdata];
            break;
        case 0x60:
            dispdata = (dispdata&0x0f)*5;
            pchardata = &ASCII_TAB60[dispdata];
            break;
        case 0x70:
            dispdata = (dispdata&0x0f)*5;
            pchardata = &ASCII_TAB70[dispdata];
            break;
        default:
            pchardata = &ASCII_TAB20[0];
            break;
   } 															// end of switch(dispdata&0xf0)...


   if( (disp_cx&0x08) == 0 ) {									// ѡ��Һ������оƬ(��CS1--����ǰ8���ַ���CS2--���ƺ�8���ַ�)
        i = disp_cx << 3;
        LCM_Wr1Command(LCM_ADDRSTRX+i);							// ���õ�ǰ�е�ַ������
        LCM_Wr1Command(LCM_ADDRSTRY+disp_cy);					// ���õ�ǰҳ��ַ������

        LCM_Wr1Data(0x00);										// ��ʾһ�пո�
        for(i=0; i<5; i++) {
            LCM_Wr1Data(*pchardata);							// ��������
            pchardata++;
        }

        LCM_Wr1Data(0x00);
        LCM_Wr1Data(0x00);
    }
    else {
        i = (disp_cx&0x07)<<3;									// ��Y>7,��ѡȡ��CS2���ҵ�ֵַҪ�ȼ�ȥ8���ٳ���8
        LCM_Wr2Command(LCM_ADDRSTRX+i);
        LCM_Wr2Command(LCM_ADDRSTRY+disp_cy);					// ���õ�ǰҳ��ַ������

        LCM_Wr2Data(0x00);										// ��ʾһ�пո�
        for(i=0; i<5; i++) {
            LCM_Wr2Data(*pchardata);							// ��������
            pchardata++;
        }
        LCM_Wr2Data(0x00);
        LCM_Wr2Data(0x00);
   }
}


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
void  LCM_DispStr(unsigned char disp_cy, unsigned char disp_cx, const char *disp_str)
{
    while( *disp_str != '\0') {
        disp_cy = (disp_cy & 0x07);								// ��������
        disp_cx = (disp_cx & 0x3f);
        LCM_DispChar(disp_cy, disp_cx, *disp_str); 				// ��ʾ�ַ�

        disp_str++;												// ָ����һ�ַ�����
        disp_cx++;
        if (disp_cx > 15) {										// ָ����һ��ʾ��
            disp_cx = 0;
            disp_cy++;
        }
    }
}
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
void LCM_DispCHS(unsigned char DisCHS_y, unsigned char DisCHS_x,const unsigned char *pStr,unsigned char highlight)
{
    unsigned char i = 0;//, j = 0;
    const unsigned char *pStrTemp = pStr;
    unsigned char disp_cx = DisCHS_x;
    unsigned char disp_cy = DisCHS_y;
    unsigned char disp=0;

    if (DisCHS_x > 7) DisCHS_x = 7;
    if (DisCHS_y > 3) DisCHS_y = 3;

    if (DisCHS_x <= 3) {                                        //  �����
        disp_cx <<= 4;
        disp_cy <<= 1;
        LCM_Wr1Command(LCM_ADDRSTRX + disp_cx);                 // ���õ�ǰ�е�ַ������
        LCM_Wr1Command(LCM_ADDRSTRY + disp_cy);                 // ���õ�ǰҳ��ַ������
        for(i=0; i<16; i++) {
            disp = highlight ? ~(*(pStrTemp + i)) : (*(pStrTemp + i));  /* ���� */
          
            LCM_Wr1Data(disp);						// ��������
        }

        LCM_Wr1Command(LCM_ADDRSTRX + disp_cx); 				// ���õ�ǰ�е�ַ������
        LCM_Wr1Command(LCM_ADDRSTRY + disp_cy + 1);				// ���õ�ǰҳ��ַ������
        for(i=16; i<32; i++) {
            disp = highlight ? ~(*(pStrTemp + i)) : (*(pStrTemp + i));  /* ���� */
          
            LCM_Wr1Data(disp);						// ��������
        }
    }
    else {                             							//  �Ұ���
        disp_cx -= 4;
        disp_cx <<= 4;
        disp_cy <<= 1;
        LCM_Wr2Command(LCM_ADDRSTRX+disp_cx); 					// ���õ�ǰ�е�ַ������
        LCM_Wr2Command(LCM_ADDRSTRY+disp_cy);					// ���õ�ǰҳ��ַ������
        for(i=0; i<16; i++) {
            disp = highlight ? ~(*(pStrTemp + i)) : (*(pStrTemp + i));  /* ���� */
          
            LCM_Wr2Data(disp);						// ��������
        }

        LCM_Wr2Command(LCM_ADDRSTRX+disp_cx); 					// ���õ�ǰ�е�ַ������
        LCM_Wr2Command(LCM_ADDRSTRY+disp_cy + 1); 				// ���õ�ǰҳ��ַ������
        for(i=16; i<32; i++) {
            disp = highlight ? ~(*(pStrTemp + i)) : (*(pStrTemp + i));  /* ���� */
          
            LCM_Wr2Data(disp);						// ��������
        }
    }

}
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
void LCM_DispCHSStr(unsigned char DisCHS_y, unsigned char DisCHS_x,unsigned char Num,const unsigned char *pStr,unsigned char highlight)
{
    unsigned char j, ChaNum = Num;
    const unsigned char *pStrTemp = pStr;
    unsigned char disp_cx = DisCHS_x;
    unsigned char disp_cy = DisCHS_y;

    if (DisCHS_x > 7) DisCHS_x = 7;
    if (DisCHS_y > 3) DisCHS_y = 3;

    for(j = 0; j < ChaNum; j++) {
        if(disp_cx >= 8) {
            disp_cx = 0;
            disp_cy++;
            if (disp_cy >= 4) {
                disp_cy = 0;
            }
        }
        LCM_DispCHS(disp_cy, disp_cx, pStrTemp + (j << 5),highlight);
        disp_cx++;
    }
}

// ��(0) ��(1) ��(2) ��(3) ��(4) ��(5) ˮ(6)
extern unsigned char const CHSStr1[]=
{
	0x00,0x40,0x42,0x42,0x42,0x42,0x42,0xFE,0x42,0x42,0x42,0x42,0x42,0x42,0x40,0x00,
	0x00,0x80,0x40,0x20,0x10,0x08,0x06,0x01,0x02,0x04,0x08,0x10,0x30,0x60,0x20,0x00,	/*"��",0*/
	
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0x40,0x40,0x40,0x40,0x40,0x00,0x00,0x00,
	0x00,0x40,0x40,0x40,0x40,0x40,0x40,0x7F,0x40,0x40,0x40,0x40,0x40,0x60,0x40,0x00,	/*"��",1*/
	
	0x00,0x02,0x02,0x02,0x02,0x02,0x02,0xFE,0x22,0x62,0xC2,0x82,0x02,0x03,0x02,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,	/*"��",2*/
	
	0x02,0x02,0xF2,0x32,0x52,0x92,0x12,0xFE,0x32,0x52,0x92,0x12,0xF2,0x02,0x02,0x00,
	0x00,0x00,0xFF,0x01,0x02,0x04,0x00,0x7F,0x01,0x02,0x44,0x80,0x7F,0x00,0x00,0x00,	/*"��",3*/
	
	0x00,0x02,0x02,0x02,0x02,0x82,0x42,0xFE,0x06,0x42,0xC2,0x82,0x02,0x03,0x02,0x00,
	0x00,0x08,0x04,0x02,0x01,0x00,0x00,0x7F,0x00,0x00,0x00,0x01,0x07,0x02,0x00,0x00,	/*"��",4*/
	
	0x00,0x02,0x02,0x02,0x02,0x02,0x02,0xFE,0x22,0x62,0xC2,0x82,0x02,0x03,0x02,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,	/*"��",5*/
	
	0x00,0x10,0x10,0x10,0x90,0x70,0x00,0xFF,0x20,0x60,0x90,0x08,0x04,0x00,0x00,0x00	/*"ˮ",6*//
	0x10,0x10,0x08,0x06,0x01,0x40,0x80,0x7F,0x00,0x00,0x01,0x06,0x0C,0x18,0x08,0x0

};	


/***********************************************************************
* ���ƣ�LCM_DispChar8_16()
* ���ܣ�ָ����ַ��ʾ�ַ���
* ��ڲ�����disp_cy		��ʾ��ֵ(0-3)
*	   disp_cx		��ʾ��ֵ(0-15)
*	   dispdata		��Ҫ��ʾ���ַ�(ASCII��)
* ע��֧����ʾ�ַ�0-9��A-Z��a-z���ո��ַ���ʾ��ʽΪ5*7��ģΪ8*16������
*     ��Ļ��ʾΪ4*16(��4�У�ÿ��16���ַ�)��֧�ָ�����ʾ��
***********************************************************************/
void LCM_DispChar8_16(unsigned char disp_cy,unsigned char disp_cx,char dispdata,unsigned char highlight)
{
	unsigned char i;
        unsigned char disp=0;
        
	disp_cy&=0x07;
	disp_cx&=0x0F;
	if(disp_cx<8)
	{
		disp_cx<<=3;
		disp_cy<<=1;
		LCM_Wr1Command(LCM_ADDRSTRX+disp_cx);
		LCM_Wr1Command(LCM_ADDRSTRY+disp_cy);
		for(i=0;i<8;i++)
		{
                  disp = highlight ? ~ASCII_8_16[dispdata-0x20][i] : ASCII_8_16[dispdata-0x20][i];  /* ���� */
                  
                  LCM_Wr1Data(disp);
		}
		LCM_Wr1Command(LCM_ADDRSTRX+disp_cx);
		LCM_Wr1Command(LCM_ADDRSTRY+disp_cy+1);
		for(i=8;i<16;i++)
		{
                  disp = highlight ? ~ASCII_8_16[dispdata-0x20][i] : ASCII_8_16[dispdata-0x20][i]; /* ���� */
                  
                  LCM_Wr1Data(disp);
		}
	}
	else
	{
		disp_cx-=8;
		disp_cx<<=3;
		disp_cy<<=1;
		LCM_Wr2Command(LCM_ADDRSTRX+disp_cx);
		LCM_Wr2Command(LCM_ADDRSTRY+disp_cy);
		for(i=0;i<8;i++)
		{
                  disp = highlight ? ~ASCII_8_16[dispdata-0x20][i] : ASCII_8_16[dispdata-0x20][i]; /* ���� */
                  
                  LCM_Wr2Data(disp);
		}
		LCM_Wr2Command(LCM_ADDRSTRX+disp_cx);
		LCM_Wr2Command(LCM_ADDRSTRY+disp_cy+1);
		for(i=8;i<16;i++)
		{
                  disp = highlight ? ~ASCII_8_16[dispdata-0x20][i] : ASCII_8_16[dispdata-0x20][i]; /* ���� */
                  
                  LCM_Wr2Data(disp);
		}
	}
}

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
void  LCM_DispStr8_16(unsigned char disp_cy, unsigned char disp_cx, const char *disp_str,unsigned char highlight)
{
    while( *disp_str != '\0') {
        disp_cy = (disp_cy & 0x07);								// ��������
        disp_cx = (disp_cx & 0x3f);
        LCM_DispChar8_16(disp_cy, disp_cx, *disp_str, highlight); 				// ��ʾ�ַ�

        disp_str++;												// ָ����һ�ַ�����
        disp_cx++;
        if (disp_cx > 15) {										// ָ����һ��ʾ��
            disp_cx = 0;
            disp_cy++;
        }
    }
}





