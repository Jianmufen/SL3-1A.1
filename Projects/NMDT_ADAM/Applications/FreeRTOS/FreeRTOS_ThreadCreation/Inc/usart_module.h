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

/*全局变量*/
extern char power_flag;
extern char disp_buf[20];
extern char data_buf[256];
//调试信息控制变量
extern uint8_t usart_debug;
extern int download_number;				/*补要数据的条数*/
extern uint8_t download_flag ;					/*补要数据的标志*/
extern unsigned int year2, month2, day2, hour2, minute2;
/*记忆存储地址*/
#define EEPROM_ADDR							  	(FLASH_EEPROM_BASE + 4)							//记忆存储写标志就存储在EEPROM的头地址
#define STATION_ADDR								(EEPROM_ADDR + 4)										//台站号就存储在EEPROM的头地址
#if (SENSOR_ID == SENSOR_R)																								//雨量报警仪
#define WARN_60M_ADDR								(STATION_ADDR + 4)									//连续60分钟雨量报警值就存储在EEPROM的头地址
#define WARN_1D_ADDR								(WARN_60M_ADDR + 4)									//一天雨量报警值就存储在EEPROM的头地址
#endif

#if (SENSOR_ID == SENSOR_W)
#define WARN_WINDSPEED_ADDR					(STATION_ADDR + 4)									//风速报警值就存储在EEPROM的头地址.
#define WINDSPEED_K_ADDR						(WARN_WINDSPEED_ADDR + 4)						//风速的采样频率系数K
#define WINDSPEED_C_ADDR						(WINDSPEED_K_ADDR + 8)							//风速的采样频率常数项C
#define WINDSPEED_CORRECT_A_ADDR		(WINDSPEED_C_ADDR + 8)							//风速订正系数的二次项A
#define WINDSPEED_CORRECT_B_ADDR		(WINDSPEED_CORRECT_A_ADDR + 8)			//风速订正系数的一次项B
#define WINDSPEED_CORRECT_C_ADDR		(WINDSPEED_CORRECT_B_ADDR + 8)			//风速订正系数的常数项C
#endif
//typedef struct
//{
//		uint8_t usart_command_set;                     /*通过串口发送命令设置参数打印标志*/
//		uint8_t sd_state;                              /*CPU运行过程中 SD卡的状态*/
//		uint8_t usart_wind;                            /*风的计算结果参数打印*/
//		uint8_t wind_array;                            /*风数组数据打印*/
//		uint8_t usart_display;                         /*显示模块调试打印*/
//}Debug;
//extern Debug debug;


//函数声明
int32_t init_usart_module(void);
	 
#ifdef __cplusplus
}
#endif
#endif /*__STORAGE_MODULE_H */
