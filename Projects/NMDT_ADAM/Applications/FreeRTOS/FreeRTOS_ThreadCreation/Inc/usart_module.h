/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART_MODULE_H
#define __USART_MODULE_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx_hal.h"
#include "main.h"
#include "time.h"
#include "usart.h"

/*ȫ�ֱ���*/
extern char power_flag;
extern char disp_buf[20];
extern char data_buf[256];
//������Ϣ���Ʊ���
extern uint8_t usart_debug;
extern int download_number;				/*��Ҫ���ݵ�����*/
extern uint8_t download_flag ;					/*��Ҫ���ݵı�־*/
extern unsigned int year2, month2, day2, hour2, minute2;
/*����洢��ַ*/
#define EEPROM_ADDR							  	(FLASH_EEPROM_BASE + 4)							//����洢д��־�ʹ洢��EEPROM��ͷ��ַ
#define STATION_ADDR								(EEPROM_ADDR + 4)										//̨վ�žʹ洢��EEPROM��ͷ��ַ
#if (SENSOR_ID == SENSOR_R)																								//����������
#define WARN_60M_ADDR								(STATION_ADDR + 4)									//����60������������ֵ�ʹ洢��EEPROM��ͷ��ַ
#define WARN_1D_ADDR								(WARN_60M_ADDR + 4)									//һ����������ֵ�ʹ洢��EEPROM��ͷ��ַ
#endif

#if (SENSOR_ID == SENSOR_W)
#define WARN_WINDSPEED_ADDR					(STATION_ADDR + 4)									//���ٱ���ֵ�ʹ洢��EEPROM��ͷ��ַ.
#define WINDSPEED_K_ADDR						(WARN_WINDSPEED_ADDR + 4)						//���ٵĲ���Ƶ��ϵ��K
#define WINDSPEED_C_ADDR						(WINDSPEED_K_ADDR + 8)							//���ٵĲ���Ƶ�ʳ�����C
#define WINDSPEED_CORRECT_A_ADDR		(WINDSPEED_C_ADDR + 8)							//���ٶ���ϵ���Ķ�����A
#define WINDSPEED_CORRECT_B_ADDR		(WINDSPEED_CORRECT_A_ADDR + 8)			//���ٶ���ϵ����һ����B
#define WINDSPEED_CORRECT_C_ADDR		(WINDSPEED_CORRECT_B_ADDR + 8)			//���ٶ���ϵ���ĳ�����C
#endif
//typedef struct
//{
//		uint8_t usart_command_set;                     /*ͨ�����ڷ����������ò�����ӡ��־*/
//		uint8_t sd_state;                              /*CPU���й����� SD����״̬*/
//		uint8_t usart_wind;                            /*��ļ�����������ӡ*/
//		uint8_t wind_array;                            /*���������ݴ�ӡ*/
//		uint8_t usart_display;                         /*��ʾģ����Դ�ӡ*/
//}Debug;
//extern Debug debug;


//��������
int32_t init_usart_module(void);
	 
#ifdef __cplusplus
}
#endif
#endif /*__STORAGE_MODULE_H */
