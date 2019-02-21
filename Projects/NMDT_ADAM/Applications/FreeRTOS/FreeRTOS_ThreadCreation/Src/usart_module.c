/* Includes ------------------------------------------------------------------*/
#include "usart_module.h"
#include "cmsis_os.h"

#include "sys_time_module.h"
#include "display_module.h"
#include "storage_module.h"
#include "string.h"
#include "stdlib.h"
#include "eeprom.h"

/* Private define ------------------------------------------------------------*/
#define UART_RX_BUF_SIZE  (512) 	
#define usartPRIORITY     osPriorityNormal
#define usartSTACK_SIZE   (512)

/* RTC Timeͨ����������ʱ��*/
static RTC_TimeTypeDef Usart_Time;
static RTC_DateTypeDef Usart_Date;

/*����1����*/		
static char rx1_buffer[UART_RX_BUF_SIZE]={0};  /* USART1 receiving buffer */
static uint32_t rx1_count	=	0;     /* receiving counter */
static uint8_t cr1_begin	=	false;        /* '\r'  received */ 
static uint8_t rx1_cplt		=	false;   /* received a frame of data ending with '<'and'>' */

/*����2����*/		
static char rx2_buffer[UART_RX_BUF_SIZE]={0};  /* USART1 receiving buffer */
static uint32_t rx2_count	=	0;     /* receiving counter */
static uint8_t cr2_begin	=	false;        /* '\r'  received */ 
static uint8_t rx2_cplt		=	false;   /* received a frame of data ending with '\r'and'\n' */

/*ȫ�ֱ���*/
char power_flag = 1;					/*��һ���ϵ��־Ϊ1   ����Ϊ0*/
char disp_buf[20];
char data_buf[256];
int download_number = 0;				/*��Ҫ���ݵ�����*/
uint8_t download_flag = 0;					/*��Ҫ���ݵı�־*/
unsigned int year2 = 0, month2 = 0, day2 = 0, hour2 = 0, minute2 = 0;

//������Ϣ��ӡ�Ŀ��Ʊ���
uint8_t usart_debug = 0;

/*eepromд��־*/
static uint8_t eepromm_write_flag = 0;

/* os relative ����1*/
static osThreadId Usart1ThreadHandle;
static osSemaphoreId semaphore_usart1;
static void Usart1_Thread(void const *argument);

/* os relative ����2*/
static osThreadId Usart2ThreadHandle;
static osSemaphoreId semaphore_usart2;
static void Usart2_Thread(void const *argument);

/**
  * @brief  Init Storage Module. 
  * @retval 0:success;-1:failed
  */
int32_t init_usart_module(void)
{
			USART1_UART_Init(19200);
			USART2_UART_Init(19200);
//			printf("Hello World\r\n");
//			HAL_UART_Transmit(&huart2, (uint8_t *)"Hello World!\r\n", 15, 0xFF);
			
			
	
			/*����������ʼֵ*/
			download_number = 0;
			download_flag = 0;
			power_flag 		=	1;
			#if (SENSOR_ID == SENSOR_R) 
			/*����Ҫ����洢�����ݴ洢��CPU��flash���棬Ʃ��̨վ�ţ�������ֵ*/
			if(data_eeprom_read(EEPROM_ADDR, (uint8_t *)&eepromm_write_flag, 1) == HAL_OK)
			{
//					printf("eepromm_write_flag = %d\r\n", eepromm_write_flag);
					if(eepromm_write_flag == 1)					/*����eeprom����д������*/
					{
							if(data_eeprom_read(STATION_ADDR, (uint8_t *)&rain_sample.station, 4) == HAL_OK)
							{
//									printf("station = %d\r\n", rain_sample.station);
							}
							else
							{
//									printf("read STATION_ADDR error\r\n");
							}
							if(data_eeprom_read(WARN_60M_ADDR, (uint8_t *)&rain_sample.rain_60m_warn, 2) == HAL_OK)
							{
//									printf("rain_60m_warn = %d\r\n", rain_sample.rain_60m_warn);
							}
							else
							{
//									printf("read WARN_60M_ADDR error\r\n");
							}
							if(data_eeprom_read(WARN_1D_ADDR, (uint8_t *)&rain_sample.rain_d_warn, 2) == HAL_OK)
							{
//									printf("rain_d_warn = %d\r\n", rain_sample.rain_d_warn);
							}
							else
							{
//									printf("read WARN_1D_ADDR error\r\n");
							}
					}
					else
					{
							/*����������ʼֵ*/
							eepromm_write_flag 				= 1;
							rain_sample.station 			= 00001;
							rain_sample.rain_60m_warn = 200;
							rain_sample.rain_d_warn 	= 500;
//							printf("eepromm_write_flag = %d		%d\r\n", eepromm_write_flag, sizeof(eepromm_write_flag));
							if(data_eeprom_write(EEPROM_ADDR, 		(uint8_t *)&eepromm_write_flag, 				1) == HAL_OK)
							{
//									printf("write EEPROM_ADDR ok\r\n");
							}
							else
							{
//									printf("write EEPROM_ADDR failed\r\n");
							}
							if(data_eeprom_write(STATION_ADDR, 	(uint8_t *)&rain_sample.station,				4) == HAL_OK)
							{
//								printf("write STATION_ADDR ok\r\n");
							}
							else
							{
//									printf("write STATION_ADDR failed\r\n");
							}
							if(data_eeprom_write(WARN_60M_ADDR, 	(uint8_t *)&rain_sample.rain_60m_warn, 	2) == HAL_OK)
							{
//									printf("write WARN_60M_ADDR ok\r\n");
							}
							else
							{
//									printf("write WARN_60M_ADDR failed\r\n");
							}
							if(data_eeprom_write(WARN_1D_ADDR, 	(uint8_t *)&rain_sample.rain_d_warn, 		2) == HAL_OK)
							{
//									printf("write WARN_1D_ADDR ok\r\n");
							}
							else
							{
//									printf("write WARN_1D_ADDR failed\r\n");
							}
		//					data_eeprom_write(STATION_ADDR, 	(uint8_t *)&rain_sample.station,				4);
		//					data_eeprom_write(WARN_60M_ADDR, 	(uint8_t *)&rain_sample.rain_60m_warn, 	4);
		//					data_eeprom_write(WARN_1D_ADDR, 	(uint8_t *)&rain_sample.rain_d_warn, 		4);
					}
			}
			else
			{
					/*����������ʼֵ*/
					eepromm_write_flag 				= 1;
					rain_sample.station 			= 00001;
					rain_sample.rain_60m_warn = 200;
					rain_sample.rain_d_warn 	= 500;
			}
			#endif
			
			#if (SENSOR_ID == SENSOR_W) 
			/*����Ҫ����洢�����ݴ洢��CPU��flash���棬Ʃ��̨վ�ţ�������ֵ*/
			if(data_eeprom_read(EEPROM_ADDR, (uint8_t *)&eepromm_write_flag, 1) == HAL_OK)
			{
//					printf("eepromm_write_flag = %d\r\n", eepromm_write_flag);
					if(eepromm_write_flag == 4)					/*����eeprom����д������*/
					{
							if(data_eeprom_read(STATION_ADDR, (uint8_t *)&wind_sample.station, 4) == HAL_OK)
							{
//									printf("station = %d\r\n", wind_sample.station);
							}
							else
							{
//									printf("read STATION_ADDR error\r\n");
							}
							if(data_eeprom_read(WARN_WINDSPEED_ADDR, (uint8_t *)&wind_sample.ws_3s_warn, 2) == HAL_OK)
							{
//									printf("ws_3s_warn = %d\r\n", wind_sample.ws_3s_warn);
							}
							else
							{
//									printf("read WARN_WINDSPEED_ADDR error\r\n");
							}
							if(data_eeprom_read(WINDSPEED_K_ADDR, (uint8_t *)&wind_sample.ws_rate_k, 8) == HAL_OK)
							{
//									printf("ws_rate_k = %lf\r\n", wind_sample.ws_rate_k);
							}
							else
							{
//									printf("read WINDSPEED_K_ADDR error\r\n");
							}
							if(data_eeprom_read(WINDSPEED_C_ADDR, (uint8_t *)&wind_sample.ws_rate_c, 8) == HAL_OK)
							{
//									printf("ws_rate_c = %lf\r\n", wind_sample.ws_rate_c);
							}
							else
							{
//									printf("read WINDSPEED_C_ADDR error\r\n");
							}
							if(data_eeprom_read(WINDSPEED_CORRECT_A_ADDR, (uint8_t *)&wind_sample.ws_a, 8) == HAL_OK)
							{
//									printf("ws_a = %lf\r\n", wind_sample.ws_a);
							}
							else
							{
//									printf("read WINDSPEED_CORRECT_A_ADDR error\r\n");
							}
							if(data_eeprom_read(WINDSPEED_CORRECT_B_ADDR, (uint8_t *)&wind_sample.ws_b, 8) == HAL_OK)
							{
//									printf("ws_b = %lf\r\n", wind_sample.ws_b);
							}
							else
							{
//									printf("read WINDSPEED_CORRECT_B_ADDR error\r\n");
							}
							if(data_eeprom_read(WINDSPEED_CORRECT_C_ADDR, (uint8_t *)&wind_sample.ws_c, 8) == HAL_OK)
							{
//									printf("ws_c = %lf\r\n", wind_sample.ws_c);
							}
							else
							{
//									printf("read WINDSPEED_CORRECT_C_ADDR error\r\n");
							}
					}
					else
					{
							/*����������ʼֵ*/
							eepromm_write_flag 				= 4;					/*�Ƿ�д���ı�־*/
							wind_sample.station 			= 00001;
							wind_sample.ws_3s_warn 		= 10.0;				/*˲ʱ���ٴ���10.0�ͱ���*/
							wind_sample.ws_rate_k 		= 0.0415;			/*   ��������=ws_rate_k*Ƶ�� + ws_rate_c   */
							wind_sample.ws_rate_c 		= 0.3;
						
							wind_sample.ws_a 					= 0.0;			/*   ˲ʱ����=ws_a*�������ٵ�ƽ�� + ws_b*�������� + ws_c   */
							wind_sample.ws_b 					= 1.0;
							wind_sample.ws_c 					= 0.0;
						
//							printf("eepromm_write_flag = %d		%d\r\n", eepromm_write_flag, sizeof(eepromm_write_flag));
							if(data_eeprom_write(EEPROM_ADDR, 		(uint8_t *)&eepromm_write_flag, 				1) == HAL_OK)
							{
//									printf("write EEPROM_ADDR ok\r\n");
							}
							else
							{
//									printf("write EEPROM_ADDR failed\r\n");
							}
							if(data_eeprom_write(STATION_ADDR, 	(uint8_t *)&wind_sample.station,				4) == HAL_OK)
							{
//									printf("write STATION_ADDR ok\r\n");
							}
							else
							{
//									printf("write STATION_ADDR failed\r\n");
							}
							if(data_eeprom_write(WARN_WINDSPEED_ADDR, 	(uint8_t *)&wind_sample.ws_3s_warn, 	2) == HAL_OK)
							{
//									printf("write WARN_WINDSPEED_ADDR ok\r\n");
							}
							else
							{
//									printf("write WARN_WINDSPEED_ADDR failed\r\n");
							}
							if(data_eeprom_write(WINDSPEED_K_ADDR, 	(uint8_t *)&wind_sample.ws_rate_k, 		8) == HAL_OK)
							{
//									printf("write WINDSPEED_K_ADDR ok\r\n");
							}
							else
							{
//									printf("write WINDSPEED_K_ADDR failed\r\n");
							}
							if(data_eeprom_write(WINDSPEED_C_ADDR, 	(uint8_t *)&wind_sample.ws_rate_c, 		8) == HAL_OK)
							{
//									printf("write WINDSPEED_C_ADDR ok\r\n");
							}
							else
							{
//									printf("write WINDSPEED_C_ADDR failed\r\n");
							}
							if(data_eeprom_write(WINDSPEED_CORRECT_A_ADDR, 	(uint8_t *)&wind_sample.ws_a, 		8) == HAL_OK)
							{
//									printf("write WINDSPEED_CORRECT_A_ADDR ok\r\n");
							}
							else
							{
//									printf("write WINDSPEED_CORRECT_A_ADDR failed\r\n");
							}
							if(data_eeprom_write(WINDSPEED_CORRECT_B_ADDR, 	(uint8_t *)&wind_sample.ws_b, 		8) == HAL_OK)
							{
//									printf("write WINDSPEED_CORRECT_B_ADDR ok\r\n");
							}
							else
							{
//									printf("write WINDSPEED_CORRECT_B_ADDR failed\r\n");
							}
							if(data_eeprom_write(WINDSPEED_CORRECT_C_ADDR, 	(uint8_t *)&wind_sample.ws_c, 		8) == HAL_OK)
							{
//									printf("write WINDSPEED_CORRECT_C_ADDR ok\r\n");
							}
							else
							{
//									printf("write WINDSPEED_CORRECT_C_ADDR failed\r\n");
							}
					}
			}
			else
			{
					/*����������ʼֵ*/
					eepromm_write_flag 				= 4;					/*�Ƿ�д���ı�־*/
					wind_sample.station 			= 00001;
					wind_sample.ws_3s_warn 		= 20;				/*˲ʱ���ٴ���20�ͱ���*/
					wind_sample.ws_rate_k 		= 0.5;			/*   ��������=ws_rate_k*Ƶ�� + ws_rate_c   */
					wind_sample.ws_rate_c 		= 0;
				
					wind_sample.ws_a 					= 0.0;			/*   ˲ʱ����=ws_a*�������ٵ�ƽ�� + ws_b*�������� + ws_c   */
					wind_sample.ws_b 					= 1.0;
					wind_sample.ws_c 					= 0.0;
//					printf("read eeprom failed\r\n");		/*������ȡEEPROMʧ�ܾ�ʹ��Ĭ��ֵ*/
			}
			#endif

			/* Define used semaphore ��������1���ź���*/
			osSemaphoreDef(SEM_USART1);
			/* Create the semaphore used by the two threads */
			semaphore_usart1=osSemaphoreCreate(osSemaphore(SEM_USART1), 1);
			if(semaphore_usart1 == NULL)
			{
//				printf("Create Semaphore_USART1 failed!\r\n");
				return -1;
			}
			
			/* Define used semaphore ��������2���ź���*/
			osSemaphoreDef(SEM_USART2);
			/* Create the semaphore used by the two threads */
			semaphore_usart2=osSemaphoreCreate(osSemaphore(SEM_USART2), 1);
			if(semaphore_usart2 == NULL)
			{
//				printf("Create Semaphore_USART2 failed!\r\n");
				return -1;
			}
		 
			/* Create a thread to read historical data��������1����洢�������� */
			osThreadDef(Usart1, Usart1_Thread, usartPRIORITY, 0, usartSTACK_SIZE);
			Usart1ThreadHandle=osThreadCreate(osThread(Usart1), NULL);
			if(Usart1ThreadHandle == NULL)
			{
//				printf("Create Usart1 Thread failed!\r\n");
				return -1;
			}
			
			/* Create a thread to read historical data��������2����洢�������� */
			osThreadDef(Usart2, Usart2_Thread, usartPRIORITY, 0, usartSTACK_SIZE);
			Usart2ThreadHandle=osThreadCreate(osThread(Usart2), NULL);
			if(Usart2ThreadHandle == NULL)
			{
//				printf("Create Usart2 Thread failed!\r\n");
				return -1;
			}
			
			return 0;
}


/*����1������������*/
static void Usart1_Thread(void const *argument)
{
	uint32_t i = 0;
	unsigned long tatal_seconds2 = 0, tatal_seconds1 = 0;
	unsigned int year1 = 0, month1 = 0, day1 = 0, hour1 = 0, minute1 = 0;
	static struct tm usart_tm = {0};
	
	if(init_storage_module()<0)
  {
//			printf("init storage module failed!\r\n");
  }
  else
  {
//			printf("init storage module ok!\r\n");
  }
	
	if(init_display_module()<0)
  {
//			printf("init display module failed!\r\n");
  }
  else
  {
//			printf("init display module ok!\r\n");
  }
	
	if(init_sys_time_module()<0)
  {
//			printf("init sys_time module failed!\r\n");
  }
  else
  {
//			printf("init sys_time module ok!\r\n");
  }
	
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);     /*��ʼ��PA1Ϊ�͵�ƽ 485�����ڽ���ģʽ*/
	
	while(osSemaphoreWait(semaphore_usart1, 1)	==	osOK);			/*���ĵ��մ����Ĵ����ź�������Դ���ź����մ��������ͷž��ǿ���ʹ�õ���Դ*/
	while(1)
	{
		
		if(osSemaphoreWait(semaphore_usart1,osWaitForever)==osOK)
		{
				get_sys_time(&Usart_Date, &Usart_Time);
				usart_tm.tm_year 		= Usart_Date.Year;
				usart_tm.tm_mon 		= Usart_Date.Month;
				usart_tm.tm_mday 		= Usart_Date.Date;
				usart_tm.tm_hour 		= Usart_Time.Hours;
				usart_tm.tm_min 		= Usart_Time.Minutes;
				usart_tm.tm_sec 		= Usart_Time.Seconds;
				if((strlen(rx1_buffer) == 7) && (strcasecmp("<debug>", rx1_buffer) == 0))
				{
						printf("T\r\n");
						if(usart_debug)
						{
								usart_debug = 0;
						}
						else
						{
								usart_debug	= 1;
						}
				}
				else if((strlen(rx1_buffer) == 8) && (strcasecmp("<memory>", rx1_buffer) == 0))
				{
						i = xPortGetFreeHeapSize();
						printf("FreeHeapSize = %d\r\n", i);
						i = xPortGetMinimumEverFreeHeapSize();
						printf("Minimum = %d\r\n", i);
				}
				else if((strlen(rx1_buffer) == 9) && (strcasecmp("<version>", rx1_buffer) == 0))
				{
						printf("version:1.1\r\n");
				}
				else if((strlen(rx1_buffer) == 9) && (strcasecmp("<restart>", rx1_buffer) == 0))
				{
						HAL_NVIC_SystemReset();  /*����CPU*/
				}
				else if((strlen(rx1_buffer) == 6) && (strcasecmp("<time>", rx1_buffer) == 0))
				{
							get_sys_time(&Usart_Date, &Usart_Time);
							printf("time:20%02d-%02d-%02d %02d:%02d:%02d\r\n", Usart_Date.Year , Usart_Date.Month , Usart_Date.Date , Usart_Time.Hours , Usart_Time.Minutes , Usart_Time.Seconds );
				}
				else if((strstr(rx1_buffer,"<TIME ") != 0)	&&(strlen(rx1_buffer) == 19))	// <TIME 180313143940>
				{
						sscanf(rx1_buffer+6,"%02u",&Usart_Date.Year);
						sscanf(rx1_buffer+8,"%02u",&Usart_Date.Month);
						sscanf(rx1_buffer+10,"%02u",&Usart_Date.Date);
						sscanf(rx1_buffer+12,"%02u",&Usart_Time.Hours);
						sscanf(rx1_buffer+14,"%02u",&Usart_Time.Minutes);
						sscanf(rx1_buffer+16,"%02u",&Usart_Time.Seconds);
						Usart_Date.Year = (rx1_buffer[6] - 48) * 10 + (rx1_buffer[7] - 48);
						
						if(usart_debug)
						{
								printf("%02u-%02u-%02u %02u:%02u:%02u\r\n", Usart_Date.Year, Usart_Date.Month, Usart_Date.Date, Usart_Time.Hours, Usart_Time.Minutes, Usart_Time.Seconds);
						}

						if(PCF8563_Set_Time(Usart_Date.Year, Usart_Date.Month, Usart_Date.Date, Usart_Time.Hours, Usart_Time.Minutes, Usart_Time.Seconds) != HAL_OK)
						{
								printf("set time error\r\n");
						}
						else
						{
								printf("set time ok\r\n");
								sync_time();			/*���óɹ�֮��ͬ��һ��ʱ��*/
						}	
				}
				#if (SENSOR_ID == SENSOR_R) 
				/*SL1-3S*/
				else if((strstr(rx1_buffer,"<DATA ") != 0)	&&(strlen(rx1_buffer) == 24))	//<DATA 18040416 18040417>
			  {
							printf("T\r\n");
							/*��ʼʱ������ʱ*/
							sscanf(rx1_buffer+6,"%02u", &year2);
							sscanf(rx1_buffer+8,"%02u", &month2);
							sscanf(rx1_buffer+10,"%02u", &day2);
							sscanf(rx1_buffer+12,"%02u", &hour2);
							/*��ֹʱ������ʱ*/
							sscanf(rx1_buffer+15,"%02u", &year1);
							sscanf(rx1_buffer+17,"%02u", &month1);
							sscanf(rx1_buffer+19,"%02u", &day1);
							sscanf(rx1_buffer+21,"%02u", &hour1);

							
							tatal_seconds2 = l_mktime(year2, month2, day2, hour2, 0, 0);
							tatal_seconds1 = l_mktime(year1, month1, day1, hour1, 0, 0);
							
							download_number = (tatal_seconds1 - tatal_seconds2) / 3600;
							download_flag = 1;			/*��Ҫ���ݱ�־*/
							if(usart_debug)
							{
										printf("%02u%02u%02u   %02u%02u%02u\r\n", month2, day2, hour2, month1, day1,hour1);
										printf("��Ҫ�����ݸ���:%d\r\n", download_number);
							}
							
				}
				else if((strstr(rx1_buffer,"<R001") != 0)	&&(rx1_count == 45) && (rx1_buffer[19] == '1') && (rx1_buffer[39] == '0'))	//����ʱ��<R001201804051554371001234500000000000000000>
				{
						sscanf(rx1_buffer+7,"%02u",&Usart_Date.Year);
						sscanf(rx1_buffer+9,"%02u",&Usart_Date.Month);
						sscanf(rx1_buffer+11,"%02u",&Usart_Date.Date);
						sscanf(rx1_buffer+13,"%02u",&Usart_Time.Hours);
						sscanf(rx1_buffer+15,"%02u",&Usart_Time.Minutes);
						sscanf(rx1_buffer+17,"%02u",&Usart_Time.Seconds);
						Usart_Date.Year = (rx1_buffer[7] - 48) * 10 + (rx1_buffer[8] - 48);
					
						if(usart_debug)
						{
								printf("%02u-%02u-%02u %02u:%02u:%02u\r\n", Usart_Date.Year, Usart_Date.Month, Usart_Date.Date, Usart_Time.Hours, Usart_Time.Minutes, Usart_Time.Seconds);
						}

						if(PCF8563_Set_Time(Usart_Date.Year, Usart_Date.Month, Usart_Date.Date, Usart_Time.Hours, Usart_Time.Minutes, Usart_Time.Seconds) != HAL_OK)
						{
//								get_sys_time(&Usart_Date, &Usart_Time);
								printf("<R%05u20%02u%02u%02u%02u%02u%02u0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000%04u>}", 	\
														rain_sample.station, Usart_Date.Year, Usart_Date.Month, Usart_Date.Date,	Usart_Time.Hours, Usart_Time.Minutes, Usart_Time.Seconds, rain_sample.rain_60m_max_t);
						}
						else
						{
//								printf("<R%05u20%02u%02u%02u%02u%02u%02u0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000%04u>}", 	\
//														rain_sample.station, Usart_Date.Year, Usart_Date.Month, Usart_Date.Date,	Usart_Time.Hours, Usart_Time.Minutes, Usart_Time.Seconds, rain_sample.rain_60m_max_t);
								memset(data_buf, 0, sizeof(data_buf));									/*���㻺��*/
								i = rain_hour_fill(&usart_tm, (struct rain_hour_data *)data_buf);
								if(usart_debug)
								{
										printf("i=%d\r\n",i);
								}
								if(i == 171)
								{
										if(usart_debug)
										{
												printf("i = %d\r\n", i);
												printf("%s\r\n", data_buf);
										}
										if((data_buf[0] == '<') && (data_buf[161] == '>'))
										{
												data_buf[162] = '}';
												data_buf[163] = '\0';
												printf("%s", data_buf);
										}
										else
										{
												if(usart_debug)
												{
														printf("data_buf = %s\r\n", data_buf);
												}
												get_sys_time(&Usart_Date, &Usart_Time);
												printf("<R%05u20%02u%02u%02u%02u%02u%02u0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000%04u>}", 	\
																rain_sample.station, Usart_Date.Year, Usart_Date.Month, Usart_Date.Date,	Usart_Time.Hours, Usart_Time.Minutes, Usart_Time.Seconds, rain_sample.rain_60m_max_t);
										}
								}
								else
								{
										get_sys_time(&Usart_Date, &Usart_Time);
										printf("<R%05u20%02u%02u%02u%02u%02u%02u0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000%04u>}", 	\
														rain_sample.station, Usart_Date.Year, Usart_Date.Month, Usart_Date.Date,	Usart_Time.Hours, Usart_Time.Minutes, Usart_Time.Seconds,rain_sample.rain_60m_max_t);
								}
								sync_time();			/*���óɹ�֮��ͬ��һ��ʱ��*/
						}	
				}
				else if((strstr(rx1_buffer,"<R001") != 0)	&&(rx1_count == 45) && (rx1_buffer[39] == '1') && (rx1_buffer[19] == '0'))	//���ñ���ֵ��̨վ��
				{
						rain_sample.station = (rx1_buffer[22] - 48) * 10000 + (rx1_buffer[23] - 48) * 1000 + (rx1_buffer[24] - 48) * 100 + (rx1_buffer[25] - 48) * 10 + (rx1_buffer[26] - 48);
						rain_sample.rain_60m_warn = (rx1_buffer[40] - 48) * 100 + (rx1_buffer[41] - 48) * 10;
						rain_sample.rain_d_warn 	= (rx1_buffer[42] - 48) * 100 + (rx1_buffer[43] - 48) * 10;
						eepromm_write_flag 				= 1;
						
						/*д��eeprom����*/
						if(data_eeprom_write(EEPROM_ADDR, 		(uint8_t *)&eepromm_write_flag, 				1) == HAL_OK)
						{
								if(usart_debug)
								{
										printf("write EEPROM_ADDR ok\r\n");
								}
						}
						else
						{
								if(usart_debug)
								{
										printf("write EEPROM_ADDR failed\r\n");
								}	
						}
						if(data_eeprom_write(STATION_ADDR, 	(uint8_t *)&rain_sample.station,				4) == HAL_OK)
						{
								if(usart_debug)
								{
										printf("write STATION_ADDR ok\r\n");
								}	
						}
						else
						{
								if(usart_debug)
								{
										printf("write STATION_ADDR failed\r\n");
								}	
						}
						if(data_eeprom_write(WARN_60M_ADDR, 	(uint8_t *)&rain_sample.rain_60m_warn, 	2) == HAL_OK)
						{
								if(usart_debug)
								{
										printf("write WARN_60M_ADDR ok\r\n");
								}	
						}
						else
						{
								if(usart_debug)
								{
										printf("write WARN_60M_ADDR failed\r\n");
								}		
						}
						if(data_eeprom_write(WARN_1D_ADDR, 	(uint8_t *)&rain_sample.rain_d_warn, 		2) == HAL_OK)
						{
								if(usart_debug)
								{
										printf("write WARN_1D_ADDR ok\r\n");
								}		
						}
						else
						{
								if(usart_debug)
								{
										printf("write WARN_1D_ADDR failed\r\n");
								}		
						}
						memset(data_buf, 0, sizeof(data_buf));									/*���㻺��*/
						i = rain_hour_fill(&usart_tm, (struct rain_hour_data *)data_buf);
						if(i == 172)
						{
								if(usart_debug)
								{
										printf("i = %d\r\n", i);
								}
								printf("%s\r\n", data_buf);
								if((data_buf[0] == '<') && (data_buf[161] == '>'))
								{
										data_buf[162] = '}';
										data_buf[163] = '\0';
										printf("%s", data_buf);
								}
								else
								{
										if(usart_debug)
										{
												printf("data_buf = %s\r\n", data_buf);
										}
										get_sys_time(&Usart_Date, &Usart_Time);
										printf("<R%05u20%02u%02u%02u%02u%02u%02u0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000%04u>}", 	\
														rain_sample.station, Usart_Date.Year, Usart_Date.Month, Usart_Date.Date,	Usart_Time.Hours, Usart_Time.Minutes, Usart_Time.Seconds,rain_sample.rain_60m_max_t);
								}
						}
						else
						{
								get_sys_time(&Usart_Date, &Usart_Time);
								printf("<R%05u20%02u%02u%02u%02u%02u%02u0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000%04u>}", 	\
												rain_sample.station, Usart_Date.Year, Usart_Date.Month, Usart_Date.Date,	Usart_Time.Hours, Usart_Time.Minutes, Usart_Time.Seconds, rain_sample.rain_60m_max_t);
						}
				}
				else if((strstr(rx1_buffer,"<R001") != 0)	&&(rx1_count == 45) && (rx1_buffer[39] == '0') && (rx1_buffer[19] == '0'))	//��ȡ˲ʱ����
				{
						memset(data_buf, 0, sizeof(data_buf));									/*���㻺��*/
						i = rain_hour_fill(&usart_tm, (struct rain_hour_data *)data_buf);
						if(usart_debug)
						{
								printf("i=%d\r\n",i);
						}
						if(i == 171)
						{
								if(usart_debug)
								{
										printf("i = %d\r\n", i);
										printf("%s\r\n", data_buf);
								}
								if((data_buf[0] == '<') && (data_buf[161] == '>'))
								{
										data_buf[162] = '}';
										data_buf[163] = '\0';
										printf("%s", data_buf);
								}
								else
								{
										if(usart_debug)
										{
												printf("data_buf = %s\r\n", data_buf);
										}
										get_sys_time(&Usart_Date, &Usart_Time);
										printf("<R%05u20%02u%02u%02u%02u%02u%02u0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000%04u>}", 	\
														rain_sample.station, Usart_Date.Year, Usart_Date.Month, Usart_Date.Date,	Usart_Time.Hours, Usart_Time.Minutes, Usart_Time.Seconds, rain_sample.rain_60m_max_t);
								}
						}
						else
						{
								get_sys_time(&Usart_Date, &Usart_Time);
								printf("<R%05u20%02u%02u%02u%02u%02u%02u0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000%04u>}", 	\
												rain_sample.station, Usart_Date.Year, Usart_Date.Month, Usart_Date.Date,	Usart_Time.Hours, Usart_Time.Minutes, Usart_Time.Seconds,rain_sample.rain_60m_max_t);
						}
				}
				else if((strstr(rx1_buffer,"<R002") != 0)	&&(rx1_count == 45) && (rx1_buffer[39] == '0') && (rx1_buffer[19] == '0'))	//��ҪСʱ����
				{
							if(usart_debug)
							{
										printf("rx1_buffer=%s\r\n", rx1_buffer);
										printf("download_number=%d\r\n", download_number);
							}
							if(0 == download_number)											/*��ֹ��һ�β�Ҫ����û��ִ����ϣ��ֽ��յ��ڶ�����Ҫ����*/
							{
										/*��ʼʱ������ʱ*/
										year2 = 0;
										sscanf(rx1_buffer+27,"%02u", &month2);
										sscanf(rx1_buffer+29,"%02u", &day2);
										sscanf(rx1_buffer+31,"%02u", &hour2);
										/*��ֹʱ������ʱ*/
										year1 = 0;
										sscanf(rx1_buffer+33,"%02u", &month1);
										sscanf(rx1_buffer+35,"%02u", &day1);
										sscanf(rx1_buffer+37,"%02u", &hour1);

										
										tatal_seconds2 = l_mktime(year2, month2, day2, hour2, 0, 0);
										tatal_seconds1 = l_mktime(year1, month1, day1, hour1, 0, 0);
										
										download_number = (tatal_seconds1 - tatal_seconds2) / 3600;
										download_flag = 1;			/*��Ҫ���ݱ�־*/
										download_number += 1;		/*15��-17�㲹Ҫ�����Բ���3�����ݣ�15��16��17��������1 ֻ�ܲ�Ҫ15��16����������*/
										if(usart_debug)
										{
													printf("%02u%02u%02u   %02u%02u%02u\r\n", month2, day2, hour2, month1, day1,hour1);
													printf("��Ҫ�����ݸ���:%d\r\n", download_number);
										}
							  }
							
				}
				#endif
				
				#if (SENSOR_ID == SENSOR_W) 
				/*EN2-B*/
				else if((strstr(rx1_buffer,"<F001") != 0)	&&(rx1_count == 45) && (rx1_buffer[19] == '1') && (rx1_buffer[39] == '0'))	//����ʱ��<F001201804071345211001234501000001000000000>
				{
						sscanf(rx1_buffer+7,"%02u",&Usart_Date.Year);
						sscanf(rx1_buffer+9,"%02u",&Usart_Date.Month);
						sscanf(rx1_buffer+11,"%02u",&Usart_Date.Date);
						sscanf(rx1_buffer+13,"%02u",&Usart_Time.Hours);
						sscanf(rx1_buffer+15,"%02u",&Usart_Time.Minutes);
						sscanf(rx1_buffer+17,"%02u",&Usart_Time.Seconds);
						Usart_Date.Year = (rx1_buffer[7] - 48) * 10 + (rx1_buffer[8] - 48);
					
						if(usart_debug)
						{
								printf("%02u-%02u-%02u %02u:%02u:%02u\r\n", Usart_Date.Year, Usart_Date.Month, Usart_Date.Date, Usart_Time.Hours, Usart_Time.Minutes, Usart_Time.Seconds);
						}

						if(PCF8563_Set_Time(Usart_Date.Year, Usart_Date.Month, Usart_Date.Date, Usart_Time.Hours, Usart_Time.Minutes, Usart_Time.Seconds) != HAL_OK)
						{
								memset(data_buf, 0, sizeof(data_buf));				/*��������*/
								i = WindOutputMinDataFill(&usart_tm, (struct wind_minute *)data_buf); 
//								get_sys_time(&Usart_Date, &Usart_Time);
								data_buf[70] = '}';
								data_buf[71] = '\r';
								data_buf[72] = '\n';
								data_buf[73] = '\0';
								printf("%s", data_buf);		/*����ʧ�ܣ���ʱ�仹�ǲ��Ե�*/
						}
						else
						{
								usart_tm.tm_year 		= Usart_Date.Year;
								usart_tm.tm_mon 		= Usart_Date.Month;
								usart_tm.tm_mday 		= Usart_Date.Date;
								usart_tm.tm_hour 		= Usart_Time.Hours;
								usart_tm.tm_min 		= Usart_Time.Minutes;
								usart_tm.tm_sec 		= Usart_Time.Seconds;
							
								memset(data_buf, 0, sizeof(data_buf));				/*��������*/
								i = WindOutputMinDataFill(&usart_tm, (struct wind_minute *)data_buf); 
//								get_sys_time(&Usart_Date, &Usart_Time);
								data_buf[70] = '}';
								data_buf[71] = '\0';
								printf("%s", data_buf);			/*���óɹ���ʱ��Ĺ�����*/
								sync_time();			/*���óɹ�֮��ͬ��һ��ʱ��*/
						}	
				}
				else if((strstr(rx1_buffer,"<F001") != 0)	&&(rx1_count == 45) && (rx1_buffer[39] == '1') && (rx1_buffer[19] == '0'))	//���ñ���ֵ��̨վ��<F001000000000000000001234501000001000012300>
				{
						wind_sample.station = (rx1_buffer[22] - 48) * 10000 + (rx1_buffer[23] - 48) * 1000 + (rx1_buffer[24] - 48) * 100 + (rx1_buffer[25] - 48) * 10 + (rx1_buffer[26] - 48);
						wind_sample.ws_3s_warn = (rx1_buffer[40] - 48) * 10 + (rx1_buffer[41] - 48);
						eepromm_write_flag 				= 4;					/*�Ƿ�д���ı�־*/
					
						/*��Ҫ���õ�����д��EEPROM����ȥ*/
						if(data_eeprom_write(EEPROM_ADDR, 		(uint8_t *)&eepromm_write_flag, 				1) == HAL_OK)
							{
									if(usart_debug)
										{
												printf("write EEPROM_ADDR ok\r\n");
										}
							}
							else
							{
									if(usart_debug)
										{
												printf("write EEPROM_ADDR failed\r\n");
										}
							}
							if(data_eeprom_write(STATION_ADDR, 	(uint8_t *)&wind_sample.station,				4) == HAL_OK)
							{
									if(usart_debug)
										{
												printf("write STATION_ADDR ok\r\n");
										}
							}
							else
							{
									if(usart_debug)
										{
												printf("write STATION_ADDR failed\r\n");
										}
							}
							if(data_eeprom_write(WARN_WINDSPEED_ADDR, 	(uint8_t *)&wind_sample.ws_3s_warn, 	2) == HAL_OK)
							{
										if(usart_debug)
										{
												printf("write WARN_WINDSPEED_ADDR ok\r\n");
										}
							}
							else
							{
									if(usart_debug)
									{
											printf("write WARN_WINDSPEED_ADDR failed\r\n");
									}
							}
						
							/*���óɹ����ظ���λ����*/
							memset(data_buf, 0, sizeof(data_buf));				/*��������*/
							i = WindOutputMinDataFill(&usart_tm, (struct wind_minute *)data_buf); 
							data_buf[70] = '}';
							data_buf[71] = '\0';
							printf("%s", data_buf);		
				}
				else if((strstr(rx1_buffer,"<F001") != 0)	&&(rx1_count == 45) && (rx1_buffer[39] == '0') && (rx1_buffer[19] == '0'))	//��λ������Ҫ����<F001000000000000000001234501000001000000000>
				{
							memset(data_buf, 0, sizeof(data_buf));				/*��������*/
							i = WindOutputMinDataFill(&usart_tm, (struct wind_minute *)data_buf); 
							data_buf[70] = '}';
							data_buf[71] = '\0';
							printf("%s", data_buf);			/*˲ʱ��������*/
				}
				else if((strstr(rx1_buffer,"<F002") != 0)	&&(rx1_count == 45) && (rx1_buffer[39] == '0') && (rx1_buffer[19] == '0'))	//<F002000000000000000001234501061401071300000>
				{
							if(0 ==download_number)
							{
										/*��ʼʱ������ʱ*/
										year2 = 1;
										sscanf(rx1_buffer+29,"%02u", &day2);
										sscanf(rx1_buffer+31,"%02u", &hour2);
										/*��ֹʱ������ʱ*/
										year1 = 1;
										sscanf(rx1_buffer+35,"%02u", &day1);
										sscanf(rx1_buffer+37,"%02u", &hour1);

										if((day2 <= usart_tm.tm_mday) && (day1 <= usart_tm.tm_mday) && (day1 >= day2))/*����Ҫ����ʼʱ�����ֹʱ�䶼С�ڵ��ڵ�ǰ������*/
										{
												month2 = usart_tm.tm_mon;
												month1 = usart_tm.tm_mon;
												year2  = usart_tm.tm_year;
												year1  = usart_tm.tm_year;
										}
										else if((day2 < usart_tm.tm_mday) && (day1 < usart_tm.tm_mday) && (day1 >= day2))/*���м�*/
										{
													month2 = usart_tm.tm_mon;
													month1 = usart_tm.tm_mon;
													year2  = usart_tm.tm_year;
													year1  = usart_tm.tm_year;
										}
										else if((day2 > usart_tm.tm_mday) && (day1 > usart_tm.tm_mday) && (day1 >= day2))/*����Ҫ����ʼʱ�����ֹʱ�䶼���ڵ�ǰ������*/
										{
												year2  = usart_tm.tm_year;
												year1  = usart_tm.tm_year;
												month1 = usart_tm.tm_mon - 1;
												month2 = usart_tm.tm_mon - 1;
												if(month2 <= 0)
												{
														month2 = 12;
														year2 -= 1;
												}
												
												if(month1 <= 0)
												{
														month1 = 12;
														month1 -= 1;
												}
										 }
										else if (day1 < day2)		/*��ֹʱ������С����ʼʱ�����ڣ������²�Ҫ����*/
										{
													month2 = usart_tm.tm_mon - 1;
													month1 = usart_tm.tm_mon;
													year2  = usart_tm.tm_year;
													year1  = usart_tm.tm_year;
											
													if(month2 <= 0)
														{
																month2 = 12;
																year2 -= 1;
														}
										}

										
										tatal_seconds2 = l_mktime(year2, month2, day2, hour2, 0, 0);
										tatal_seconds1 = l_mktime(year1, month1, day1, hour1, 0, 0);
										
										if(tatal_seconds1 > tatal_seconds2)
										{
													download_number = (tatal_seconds1 - tatal_seconds2) / 3600;
													download_flag = 1;			/*��Ҫ���ݱ�־*/
													download_number += 1;		/*15��-17�㲹Ҫ�����Բ���3�����ݣ�15��16��17��������1 ֻ�ܲ�Ҫ15��16����������*/
													//printf("%02u-%02u-%02u   %02u-%02u-%02u\r\n", month2, day2, hour2, month1, day1,hour1);
													if(usart_debug)
													{
																printf("%02u%02u%02u   %02u%02u%02u\r\n", month2, day2, hour2, month1, day1,hour1);
																printf("��Ҫ�����ݸ���:%d\r\n", download_number);
													}
										}
							}							
				}
				else if((strlen(rx1_buffer) == 29) && (strncasecmp("<frequency ", rx1_buffer, 11) == 0))		/*����Ƶ�ʻ���ɷ��ٵ�ϵ��ws_rate_k��ws_rate_c  <frequency 000.2032 100.0001>*/
				{
						wind_sample.ws_rate_k = atof(rx1_buffer + 11);
						wind_sample.ws_rate_c = atof(rx1_buffer + 21);
					
						printf("ws_rate_k=%f	ws_rate_c=%f\r\n", wind_sample.ws_rate_k, wind_sample.ws_rate_c);
						
						if(data_eeprom_write(WINDSPEED_K_ADDR, 	(uint8_t *)&wind_sample.ws_rate_k, 		8) == HAL_OK)
						{
								printf("write WINDSPEED_K_ADDR ok\r\n");
						}
						else
						{
								printf("write WINDSPEED_K_ADDR failed\r\n");
						}
						if(data_eeprom_write(WINDSPEED_C_ADDR, 	(uint8_t *)&wind_sample.ws_rate_c, 		8) == HAL_OK)
						{
								printf("write WINDSPEED_C_ADDR ok\r\n");
						}
						else
						{
								printf("write WINDSPEED_C_ADDR failed\r\n");
						}
				}
				else if((strlen(rx1_buffer) == 36) && (strncasecmp("<correct ", rx1_buffer, 9) == 0))		//<correct 000.0232 100.0001 111.1234>
				{
						wind_sample.ws_a = atof(rx1_buffer + 10);
						wind_sample.ws_b = atof(rx1_buffer + 19);
						wind_sample.ws_c = atof(rx1_buffer + 28);
						printf("ws_a=%f	ws_b=%f	ws_c=%f\r\n", wind_sample.ws_a, wind_sample.ws_b, wind_sample.ws_c);
						
						if(data_eeprom_write(WINDSPEED_CORRECT_A_ADDR, 	(uint8_t *)&wind_sample.ws_a, 		8) == HAL_OK)
						{
								printf("write WINDSPEED_CORRECT_A_ADDR ok\r\n");
						}
						else
						{
								printf("write WINDSPEED_CORRECT_A_ADDR failed\r\n");
						}
						if(data_eeprom_write(WINDSPEED_CORRECT_B_ADDR, 	(uint8_t *)&wind_sample.ws_b, 		8) == HAL_OK)
						{
								printf("write WINDSPEED_CORRECT_B_ADDR ok\r\n");
						}
						else
						{
								printf("write WINDSPEED_CORRECT_B_ADDR failed\r\n");
						}
						if(data_eeprom_write(WINDSPEED_CORRECT_C_ADDR, 	(uint8_t *)&wind_sample.ws_c, 		8) == HAL_OK)
						{
								printf("write WINDSPEED_CORRECT_C_ADDR ok\r\n");
						}
						else
						{
								printf("write WINDSPEED_CORRECT_C_ADDR failed\r\n");
						}
				}
				#endif
				else
				{
						if(usart_debug)
							{
										printf("F:%s\r\n", rx1_buffer);
										Processing_FileManagement_Command((uint8_t *)rx1_buffer, rx1_count);
							}
				}
				
				rx1_count = 0;
				rx1_cplt = false;                                              /* clear the flag of receive */
				memset(rx1_buffer, 0, sizeof(rx1_buffer));                      /* clear the register of receive */
		}
	}
}

/*����2������������*/
static void Usart2_Thread(void const *argument)
{
	uint32_t i = 0;
	unsigned long tatal_seconds2 = 0, tatal_seconds1 = 0;
	unsigned int year1 = 0, month1 = 0, day1 = 0, hour1 = 0, minute1 = 0;
	static struct tm usart_tm = {0};
	
	while(osSemaphoreWait(semaphore_usart2, 1)	==	osOK);			/*���ĵ��մ����Ĵ����ź�������Դ���ź����մ��������ͷž��ǿ���ʹ�õ���Դ*/
	while(1)
	{
		
		if(osSemaphoreWait(semaphore_usart2, osWaitForever)==osOK)
		{
				get_sys_time(&Usart_Date, &Usart_Time);
				usart_tm.tm_year 		= Usart_Date.Year;
				usart_tm.tm_mon 		= Usart_Date.Month;
				usart_tm.tm_mday 		= Usart_Date.Date;
				usart_tm.tm_hour 		= Usart_Time.Hours;
				usart_tm.tm_min 		= Usart_Time.Minutes;
				usart_tm.tm_sec 		= Usart_Time.Seconds;
				if((strlen(rx2_buffer) == 7) && (strcasecmp("<debug>", rx2_buffer) == 0))
				{
						printf("T\r\n");
						if(usart_debug)
						{
								usart_debug = 0;
						}
						else
						{
								usart_debug	= 1;
						}
				}
				else if((strlen(rx2_buffer) == 8) && (strcasecmp("<memory>", rx2_buffer) == 0))
				{
						i = xPortGetFreeHeapSize();
						printf("FreeHeapSize = %d\r\n", i);
						i = xPortGetMinimumEverFreeHeapSize();
						printf("Minimum = %d\r\n", i);
				}
				else if((strlen(rx2_buffer) == 9) && (strcasecmp("<version>", rx2_buffer) == 0))
				{
						printf("version:1.1\r\n");
				}
				else if((strlen(rx2_buffer) == 9) && (strcasecmp("<restart>", rx2_buffer) == 0))
				{
						HAL_NVIC_SystemReset();  /*����CPU*/
				}
				else if((strlen(rx2_buffer) == 6) && (strcasecmp("<time>", rx2_buffer) == 0))
				{
							get_sys_time(&Usart_Date, &Usart_Time);
							printf("time:20%02d-%02d-%02d %02d:%02d:%02d\r\n", Usart_Date.Year , Usart_Date.Month , Usart_Date.Date , Usart_Time.Hours , Usart_Time.Minutes , Usart_Time.Seconds );
				}
				else if((strstr(rx2_buffer,"<TIME ") != 0)	&&(strlen(rx2_buffer) == 19))	// <TIME 180313143940>
				{
						sscanf(rx2_buffer+6,"%02u",&Usart_Date.Year);
						sscanf(rx2_buffer+8,"%02u",&Usart_Date.Month);
						sscanf(rx2_buffer+10,"%02u",&Usart_Date.Date);
						sscanf(rx2_buffer+12,"%02u",&Usart_Time.Hours);
						sscanf(rx2_buffer+14,"%02u",&Usart_Time.Minutes);
						sscanf(rx2_buffer+16,"%02u",&Usart_Time.Seconds);
						Usart_Date.Year = (rx2_buffer[6] - 48) * 10 + (rx2_buffer[7] - 48);
						
						if(usart_debug)
						{
								printf("%02u-%02u-%02u %02u:%02u:%02u\r\n", Usart_Date.Year, Usart_Date.Month, Usart_Date.Date, Usart_Time.Hours, Usart_Time.Minutes, Usart_Time.Seconds);
						}

						if(PCF8563_Set_Time(Usart_Date.Year, Usart_Date.Month, Usart_Date.Date, Usart_Time.Hours, Usart_Time.Minutes, Usart_Time.Seconds) != HAL_OK)
						{
								printf("set time error\r\n");
						}
						else
						{
								printf("set time ok\r\n");
								sync_time();			/*���óɹ�֮��ͬ��һ��ʱ��*/
						}	
				}
				#if (SENSOR_ID == SENSOR_R) 
				/*SL1-3S*/
				else if((strstr(rx2_buffer,"<DATA ") != 0)	&&(strlen(rx2_buffer) == 24))	//<DATA 18040416 18040417>
			  {
//							printf("T\r\n");
							/*��ʼʱ������ʱ*/
							sscanf(rx2_buffer+6,"%02u", &year2);
							sscanf(rx2_buffer+8,"%02u", &month2);
							sscanf(rx2_buffer+10,"%02u", &day2);
							sscanf(rx2_buffer+12,"%02u", &hour2);
							/*��ֹʱ������ʱ*/
							sscanf(rx2_buffer+15,"%02u", &year1);
							sscanf(rx2_buffer+17,"%02u", &month1);
							sscanf(rx2_buffer+19,"%02u", &day1);
							sscanf(rx2_buffer+21,"%02u", &hour1);

							
							tatal_seconds2 = l_mktime(year2, month2, day2, hour2, 0, 0);
							tatal_seconds1 = l_mktime(year1, month1, day1, hour1, 0, 0);
							
							download_number = (tatal_seconds1 - tatal_seconds2) / 3600;
							download_flag = 1;			/*��Ҫ���ݱ�־*/
							if(usart_debug)
							{
										printf("%02u%02u%02u   %02u%02u%02u\r\n", month2, day2, hour2, month1, day1,hour1);
										printf("��Ҫ�����ݸ���:%d\r\n", download_number);
							}
							
				}
				else if((strstr(rx2_buffer,"<R001") != 0)	&&(rx2_count == 45) && (rx2_buffer[19] == '1') && (rx2_buffer[39] == '0'))	//����ʱ��<R001201810221451371001234500000000000000000>
				{
						sscanf(rx2_buffer+7,"%02u",&Usart_Date.Year);
						sscanf(rx2_buffer+9,"%02u",&Usart_Date.Month);
						sscanf(rx2_buffer+11,"%02u",&Usart_Date.Date);
						sscanf(rx2_buffer+13,"%02u",&Usart_Time.Hours);
						sscanf(rx2_buffer+15,"%02u",&Usart_Time.Minutes);
						sscanf(rx2_buffer+17,"%02u",&Usart_Time.Seconds);
						Usart_Date.Year = (rx2_buffer[7] - 48) * 10 + (rx2_buffer[8] - 48);
					
						if(usart_debug)
						{
								printf("%02u-%02u-%02u %02u:%02u:%02u\r\n", Usart_Date.Year, Usart_Date.Month, Usart_Date.Date, Usart_Time.Hours, Usart_Time.Minutes, Usart_Time.Seconds);
						}

						if(PCF8563_Set_Time(Usart_Date.Year, Usart_Date.Month, Usart_Date.Date, Usart_Time.Hours, Usart_Time.Minutes, Usart_Time.Seconds) != HAL_OK)
						{
//								get_sys_time(&Usart_Date, &Usart_Time);
								snprintf(data_buf, 164, "<R%05u20%02u%02u%02u%02u%02u%02u0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000%04u>}", \
														rain_sample.station, Usart_Date.Year, Usart_Date.Month, Usart_Date.Date,	Usart_Time.Hours, Usart_Time.Minutes, Usart_Time.Seconds,rain_sample.rain_60m_max_t);
								//
								HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);     /*����PA1Ϊ�ߵ�ƽ 485�����ڷ���ģʽ*/
								//HAL_UART_Transmit(&huart2, (uint8_t *)data_buf, 164, 0xFFFFFFFF);
								//osDelay(3);
								printf("%s", 	data_buf);
								//printf("<R%05u20%02u%02u%02u%02u%02u%02u0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000%04u>}", 	\
														rain_sample.station, Usart_Date.Year, Usart_Date.Month, Usart_Date.Date,	Usart_Time.Hours, Usart_Time.Minutes, Usart_Time.Seconds, rain_sample.rain_60m_max_t);
								HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);     /*����PA1Ϊ�͵�ƽ 485�����ڽ���ģʽ*/
								
						}
						else
						{
								snprintf(data_buf, 164, "<R%05u20%02u%02u%02u%02u%02u%02u0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000%04u>}", \
														rain_sample.station, Usart_Date.Year, Usart_Date.Month, Usart_Date.Date,	Usart_Time.Hours, Usart_Time.Minutes, Usart_Time.Seconds,rain_sample.rain_60m_max_t);
								//printf("%s", 	data_buf);
								HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);     /*����PA1Ϊ�ߵ�ƽ 485�����ڷ���ģʽ*/
								printf("%s", 	data_buf);
								//HAL_UART_Transmit(&huart1, (uint8_t *)data_buf, 164, 0xFFFFFFFF);
								//printf("<R%05u20%02u%02u%02u%02u%02u%02u0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000%04u>}", 	\
														rain_sample.station, Usart_Date.Year, Usart_Date.Month, Usart_Date.Date,	Usart_Time.Hours, Usart_Time.Minutes, Usart_Time.Seconds, rain_sample.rain_60m_max_t);
								//osDelay(3);
								HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);     /*����PA1Ϊ�͵�ƽ 485�����ڽ���ģʽ*/
								sync_time();			/*���óɹ�֮��ͬ��һ��ʱ��*/
						}	
				}
				else if((strstr(rx2_buffer,"<R001") != 0)	&&(rx2_count == 45) && (rx2_buffer[39] == '1') && (rx2_buffer[19] == '0'))	//���ñ���ֵ��̨վ��<R001000000000000000001111100000000000012050>
				{
						rain_sample.station = (rx2_buffer[22] - 48) * 10000 + (rx2_buffer[23] - 48) * 1000 + (rx2_buffer[24] - 48) * 100 + (rx2_buffer[25] - 48) * 10 + (rx2_buffer[26] - 48);
						rain_sample.rain_60m_warn = (rx2_buffer[40] - 48) * 100 + (rx2_buffer[41] - 48) * 10;
						rain_sample.rain_d_warn 	= (rx2_buffer[42] - 48) * 100 + (rx2_buffer[43] - 48) * 10;
						eepromm_write_flag 				= 1;
						
						/*д��eeprom����*/
						if(data_eeprom_write(EEPROM_ADDR, 		(uint8_t *)&eepromm_write_flag, 				1) == HAL_OK)
						{
								if(usart_debug)
								{
										printf("write EEPROM_ADDR ok\r\n");
								}
						}
						else
						{
								if(usart_debug)
								{
										printf("write EEPROM_ADDR failed\r\n");
								}	
						}
						if(data_eeprom_write(STATION_ADDR, 	(uint8_t *)&rain_sample.station,				4) == HAL_OK)
						{
								if(usart_debug)
								{
										printf("write STATION_ADDR ok\r\n");
								}	
						}
						else
						{
								if(usart_debug)
								{
										printf("write STATION_ADDR failed\r\n");
								}	
						}
						if(data_eeprom_write(WARN_60M_ADDR, 	(uint8_t *)&rain_sample.rain_60m_warn, 	2) == HAL_OK)
						{
								if(usart_debug)
								{
										printf("write WARN_60M_ADDR ok\r\n");
								}	
						}
						else
						{
								if(usart_debug)
								{
										printf("write WARN_60M_ADDR failed\r\n");
								}		
						}
						if(data_eeprom_write(WARN_1D_ADDR, 	(uint8_t *)&rain_sample.rain_d_warn, 		2) == HAL_OK)
						{
								if(usart_debug)
								{
										printf("write WARN_1D_ADDR ok\r\n");
								}		
						}
						else
						{
								if(usart_debug)
								{
										printf("write WARN_1D_ADDR failed\r\n");
								}		
						}
						memset(data_buf, 0, sizeof(data_buf));									/*���㻺��*/
						i = rain_hour_fill(&usart_tm, (struct rain_hour_data *)data_buf);
						if(i == 172)
						{
								if(usart_debug)
								{
										printf("i = %d\r\n", i);
								}
								printf("%s\r\n", data_buf);
								if((data_buf[0] == '<') && (data_buf[161] == '>'))
								{
										data_buf[162] = '}';
										data_buf[163] = '\0';
	
										HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);     /*����PA1Ϊ�ߵ�ƽ 485�����ڷ���ģʽ*/
										//HAL_UART_Transmit(&huart2, (uint8_t *)data_buf, 164, 0xFFFF);
										printf("%s", data_buf);
										HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);     /*����PA1Ϊ�͵�ƽ 485�����ڽ���ģʽ*/
								}
								else
								{
										if(usart_debug)
										{
												printf("data_buf = %s\r\n", data_buf);
										}
										get_sys_time(&Usart_Date, &Usart_Time);
										HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);     /*����PA1Ϊ�ߵ�ƽ 485�����ڷ���ģʽ*/
										snprintf(data_buf, 164, "<R%05u20%02u%02u%02u%02u%02u%02u0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000%04u>}", \
														rain_sample.station, Usart_Date.Year, Usart_Date.Month, Usart_Date.Date,	Usart_Time.Hours, Usart_Time.Minutes, Usart_Time.Seconds,rain_sample.rain_60m_max_t);
										printf("%s", 	data_buf);
										//HAL_UART_Transmit(&huart2, (uint8_t *)data_buf, 164, 0xFFFF);
										HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);     /*����PA1Ϊ�͵�ƽ 485�����ڽ���ģʽ*/
								}
						}
						else
						{
								get_sys_time(&Usart_Date, &Usart_Time);
								//printf("<R%05u20%02u%02u%02u%02u%02u%02u0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000%04u>}", 	\
												rain_sample.station, Usart_Date.Year, Usart_Date.Month, Usart_Date.Date,	Usart_Time.Hours, Usart_Time.Minutes, Usart_Time.Seconds, rain_sample.rain_60m_max_t);
								HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);     /*����PA1Ϊ�ߵ�ƽ 485�����ڷ���ģʽ*/
								snprintf(data_buf, 164, "<R%05u20%02u%02u%02u%02u%02u%02u0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000%04u>}", \
												rain_sample.station, Usart_Date.Year, Usart_Date.Month, Usart_Date.Date,	Usart_Time.Hours, Usart_Time.Minutes, Usart_Time.Seconds,rain_sample.rain_60m_max_t);
								printf("%s", 	data_buf);
								//HAL_UART_Transmit(&huart2, (uint8_t *)data_buf, 164, 0xFFFF);
								HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);     /*����PA1Ϊ�͵�ƽ 485�����ڽ���ģʽ*/
						}
				}
				else if((strstr(rx2_buffer,"<R001") != 0)	&&(rx2_count == 45) && (rx2_buffer[39] == '0') && (rx2_buffer[19] == '0'))	//��ȡ˲ʱ����<R001000000000000000001111100000000000000000>
				{
						memset(data_buf, 0, sizeof(data_buf));									/*���㻺��*/
						i = rain_hour_fill(&usart_tm, (struct rain_hour_data *)data_buf);
						if(usart_debug)
						{
								printf("i=%d\r\n",i);
						}
						if(i == 171)
						{
								if(usart_debug)
								{
										printf("i = %d\r\n", i);
										printf("%s\r\n", data_buf);
								}
								if((data_buf[0] == '<') && (data_buf[161] == '>'))
								{
										data_buf[162] = '}';
										data_buf[163] = '\0';
										HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);     /*����PA1Ϊ�ߵ�ƽ 485�����ڷ���ģʽ*/
										//HAL_UART_Transmit(&huart2, (uint8_t *)data_buf, 164, 0xFFFF);
										printf("%s", data_buf);			/*˲ʱ��������*/
										HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);     /*����PA1Ϊ�͵�ƽ 485�����ڽ���ģʽ*/
								}
								else
								{
										if(usart_debug)
										{
												printf("data_buf = %s\r\n", data_buf);
										}
										get_sys_time(&Usart_Date, &Usart_Time);
										HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);     /*����PA1Ϊ�ߵ�ƽ 485�����ڷ���ģʽ*/
										snprintf(data_buf, 164, "<R%05u20%02u%02u%02u%02u%02u%02u0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000%04u>}", \
														rain_sample.station, Usart_Date.Year, Usart_Date.Month, Usart_Date.Date,	Usart_Time.Hours, Usart_Time.Minutes, Usart_Time.Seconds,rain_sample.rain_60m_max_t);
										printf("%s", 	data_buf);
										//HAL_UART_Transmit(&huart2, (uint8_t *)data_buf, 164, 0xFFFF);
										//printf("<R%05u20%02u%02u%02u%02u%02u%02u0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000%04u>}", 	\
														rain_sample.station, Usart_Date.Year, Usart_Date.Month, Usart_Date.Date,	Usart_Time.Hours, Usart_Time.Minutes, Usart_Time.Seconds, rain_sample.rain_60m_max_t);
										HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);     /*����PA1Ϊ�͵�ƽ 485�����ڽ���ģʽ*/
								}
						}
						else
						{
								get_sys_time(&Usart_Date, &Usart_Time);
								HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);     /*����PA1Ϊ�ߵ�ƽ 485�����ڷ���ģʽ*/
								snprintf(data_buf, 164, "<R%05u20%02u%02u%02u%02u%02u%02u0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000%04u>}", \
														rain_sample.station, Usart_Date.Year, Usart_Date.Month, Usart_Date.Date,	Usart_Time.Hours, Usart_Time.Minutes, Usart_Time.Seconds,rain_sample.rain_60m_max_t);
								printf("%s", 	data_buf);
								//HAL_UART_Transmit(&huart2, (uint8_t *)data_buf, 164, 0xFFFF);
								//printf("<R%05u20%02u%02u%02u%02u%02u%02u0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000%04u>}", 	\
												rain_sample.station, Usart_Date.Year, Usart_Date.Month, Usart_Date.Date,	Usart_Time.Hours, Usart_Time.Minutes, Usart_Time.Seconds,rain_sample.rain_60m_max_t);
								HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);     /*����PA1Ϊ�͵�ƽ 485�����ڽ���ģʽ*/
						}
				}
				else if((strstr(rx2_buffer,"<R002") != 0)	&&(rx2_count == 45) && (rx2_buffer[39] == '0') && (rx2_buffer[19] == '0'))	//��ҪСʱ����<R002000000000000000001111104051004051500000>
				{
							if(usart_debug)
							{
										printf("rx2_buffer=%s\r\n", rx2_buffer);
										printf("download_number=%d\r\n", download_number);
							}
							//download_number = 0;
							if(0 == download_number)											/*��ֹ��һ�β�Ҫ����û��ִ����ϣ��ֽ��յ��ڶ�����Ҫ����*/
							{
										/*��ʼʱ������ʱ*/
										year2 = 0;
										sscanf(rx2_buffer+27,"%02u", &month2);
										sscanf(rx2_buffer+29,"%02u", &day2);
										sscanf(rx2_buffer+31,"%02u", &hour2);
										/*��ֹʱ������ʱ*/
										year1 = 0;
										sscanf(rx2_buffer+33,"%02u", &month1);
										sscanf(rx2_buffer+35,"%02u", &day1);
										sscanf(rx2_buffer+37,"%02u", &hour1);

										
										tatal_seconds2 = l_mktime(year2, month2, day2, hour2, 0, 0);
										tatal_seconds1 = l_mktime(year1, month1, day1, hour1, 0, 0);
										
										download_number = (tatal_seconds1 - tatal_seconds2) / 3600;
										download_flag = 1;			/*��Ҫ���ݱ�־*/
										download_number += 1;		/*15��-17�㲹Ҫ�����Բ���3�����ݣ�15��16��17��������1 ֻ�ܲ�Ҫ15��16����������*/
										if(usart_debug)
										{
													printf("%02u%02u%02u   %02u%02u%02u\r\n", month2, day2, hour2, month1, day1,hour1);
													printf("��Ҫ�����ݸ���:%d\r\n", download_number);
										}
							  }
				}
				#endif
				
				#if (SENSOR_ID == SENSOR_W) 
				/*EN2-B*/
				else if((strstr(rx2_buffer,"<F001") != 0)	&&(rx2_count == 45) && (rx2_buffer[19] == '1') && (rx2_buffer[39] == '0'))	//����ʱ��<F001201804071345211001234501000001000000000>
				{
						sscanf(rx2_buffer+7,"%02u",&Usart_Date.Year);
						sscanf(rx2_buffer+9,"%02u",&Usart_Date.Month);
						sscanf(rx2_buffer+11,"%02u",&Usart_Date.Date);
						sscanf(rx2_buffer+13,"%02u",&Usart_Time.Hours);
						sscanf(rx2_buffer+15,"%02u",&Usart_Time.Minutes);
						sscanf(rx2_buffer+17,"%02u",&Usart_Time.Seconds);
						Usart_Date.Year = (rx2_buffer[7] - 48) * 10 + (rx2_buffer[8] - 48);
					
						if(usart_debug)
						{
								printf("%02u-%02u-%02u %02u:%02u:%02u\r\n", Usart_Date.Year, Usart_Date.Month, Usart_Date.Date, Usart_Time.Hours, Usart_Time.Minutes, Usart_Time.Seconds);
						}

						if(PCF8563_Set_Time(Usart_Date.Year, Usart_Date.Month, Usart_Date.Date, Usart_Time.Hours, Usart_Time.Minutes, Usart_Time.Seconds) != HAL_OK)
						{
								memset(data_buf, 0, sizeof(data_buf));				/*��������*/
								i = WindOutputMinDataFill(&usart_tm, (struct wind_minute *)data_buf); 
//								get_sys_time(&Usart_Date, &Usart_Time);
								data_buf[70] = '}';
								data_buf[71] = '\r';
								data_buf[72] = '\n';
								data_buf[73] = '\0';
								HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);     /*����PA1Ϊ�ߵ�ƽ 485�����ڷ���ģʽ*/
								printf("%s", data_buf);			/*˲ʱ��������*/
								//HAL_UART_Transmit(&huart2, (uint8_t *)data_buf, 73, 0xFFFF);
								HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);     /*����PA1Ϊ�͵�ƽ 485�����ڽ���ģʽ*/
						}
						else
						{
								usart_tm.tm_year 		= Usart_Date.Year;
								usart_tm.tm_mon 		= Usart_Date.Month;
								usart_tm.tm_mday 		= Usart_Date.Date;
								usart_tm.tm_hour 		= Usart_Time.Hours;
								usart_tm.tm_min 		= Usart_Time.Minutes;
								usart_tm.tm_sec 		= Usart_Time.Seconds;
							
								memset(data_buf, 0, sizeof(data_buf));				/*��������*/
								i = WindOutputMinDataFill(&usart_tm, (struct wind_minute *)data_buf); 
//								get_sys_time(&Usart_Date, &Usart_Time);
								data_buf[70] = '}';
								data_buf[71] = '\0';
								HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);     /*����PA1Ϊ�ߵ�ƽ 485�����ڷ���ģʽ*/
								printf("%s", data_buf);			/*˲ʱ��������*/
								//HAL_UART_Transmit(&huart2, (uint8_t *)data_buf, 71, 0xFFFF);
								HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);     /*����PA1Ϊ�͵�ƽ 485�����ڽ���ģʽ*/
								sync_time();			/*���óɹ�֮��ͬ��һ��ʱ��*/
						}	
				}
				else if((strstr(rx2_buffer,"<F001") != 0)	&&(rx2_count == 45) && (rx2_buffer[39] == '1') && (rx2_buffer[19] == '0'))	//���ñ���ֵ��̨վ��<F001000000000000000001234501000001000012300>
				{
						wind_sample.station = (rx2_buffer[22] - 48) * 10000 + (rx2_buffer[23] - 48) * 1000 + (rx2_buffer[24] - 48) * 100 + (rx2_buffer[25] - 48) * 10 + (rx2_buffer[26] - 48);
						wind_sample.ws_3s_warn = (rx2_buffer[40] - 48) * 10 + (rx2_buffer[41] - 48);
						eepromm_write_flag 				= 4;					/*�Ƿ�д���ı�־*/
					
						/*��Ҫ���õ�����д��EEPROM����ȥ*/
						if(data_eeprom_write(EEPROM_ADDR, 		(uint8_t *)&eepromm_write_flag, 				1) == HAL_OK)
							{
									if(usart_debug)
										{
												printf("write EEPROM_ADDR ok\r\n");
										}
							}
							else
							{
									if(usart_debug)
										{
												printf("write EEPROM_ADDR failed\r\n");
										}
							}
							if(data_eeprom_write(STATION_ADDR, 	(uint8_t *)&wind_sample.station,				4) == HAL_OK)
							{
									if(usart_debug)
										{
												printf("write STATION_ADDR ok\r\n");
										}
							}
							else
							{
									if(usart_debug)
										{
												printf("write STATION_ADDR failed\r\n");
										}
							}
							if(data_eeprom_write(WARN_WINDSPEED_ADDR, 	(uint8_t *)&wind_sample.ws_3s_warn, 	2) == HAL_OK)
							{
										if(usart_debug)
										{
												printf("write WARN_WINDSPEED_ADDR ok\r\n");
										}
							}
							else
							{
									if(usart_debug)
									{
											printf("write WARN_WINDSPEED_ADDR failed\r\n");
									}
							}
						
							/*���óɹ����ظ���λ����*/
							memset(data_buf, 0, sizeof(data_buf));				/*��������*/
							i = WindOutputMinDataFill(&usart_tm, (struct wind_minute *)data_buf); 
							data_buf[70] = '}';
							data_buf[71] = '\0';
							HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);     /*����PA1Ϊ�ߵ�ƽ 485�����ڷ���ģʽ*/
							printf("%s", data_buf);			/*˲ʱ��������*/
							//HAL_UART_Transmit(&huart2, (uint8_t *)data_buf, 71, 0xFFFF);
							HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);     /*����PA1Ϊ�͵�ƽ 485�����ڽ���ģʽ*/
				}
				else if((strstr(rx2_buffer,"<F001") != 0)	&&(rx2_count == 45) && (rx2_buffer[39] == '0') && (rx2_buffer[19] == '0'))	//��λ������Ҫ����<F001000000000000000001234501000001000000000>
				{
							memset(data_buf, 0, sizeof(data_buf));				/*��������*/
							i = WindOutputMinDataFill(&usart_tm, (struct wind_minute *)data_buf); 
							data_buf[70] = '}';
							data_buf[71] = '\0';
							HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);     /*����PA1Ϊ�ߵ�ƽ 485�����ڷ���ģʽ*/
							printf("%s", data_buf);			/*˲ʱ��������*/
							//HAL_UART_Transmit(&huart2, (uint8_t *)data_buf, 71, 0xFFFF);
							HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);     /*����PA1Ϊ�͵�ƽ 485�����ڽ���ģʽ*/	
				}
				else if((strstr(rx2_buffer,"<F002") != 0)	&&(rx2_count == 45) && (rx2_buffer[39] == '0') && (rx2_buffer[19] == '0'))	//<F002000000000000000001234501061401071300000>
				{
							if(0 ==download_number)
							{
										/*��ʼʱ������ʱ*/
										year2 = 0;
										sscanf(rx2_buffer+29,"%02u", &day2);
										sscanf(rx2_buffer+31,"%02u", &hour2);
										/*��ֹʱ������ʱ*/
										year1 = 0;
										sscanf(rx2_buffer+35,"%02u", &day1);
										sscanf(rx2_buffer+37,"%02u", &hour1);
			//							if(day1 <= usart_tm.tm_mday)		/*����Ҫ���������ڵ�ǰ����֮ǰ*/
			//							{
			//									month2 = usart_tm.tm_mon;
			//									month1 = usart_tm.tm_mon;
			//							}
			//							else
			//							{
			//									month2 = usart_tm.tm_mon - 1;
			//									month1 = usart_tm.tm_mon - 1;
			//									if((month2 > 12) | (month2 == 0))
			//									{
			//											month2 = 12;
			//											month1 = 12;
			//									}
			//							}

											if(day2 <= day1)		/*����Ҫ����ʼʱ�����ֹʱ������ͬһ���·���*/
											{
													month2 = usart_tm.tm_mon;
													month1 = usart_tm.tm_mon;
													year2  = usart_tm.tm_year;
													year1  = usart_tm.tm_year;
											}
											else
											{
													year2  = usart_tm.tm_year;
													year1  = usart_tm.tm_year;
													month1 = usart_tm.tm_mon;
													month2 = usart_tm.tm_mon - 1;
													if(month2 <= 0)
													{
															month2 = 12;
															year2 -= 1;
													}
											}

										
										tatal_seconds2 = l_mktime(year2, month2, day2, hour2, 0, 0);
										tatal_seconds1 = l_mktime(year1, month1, day1, hour1, 0, 0);
										
										
										download_number = (tatal_seconds1 - tatal_seconds2) / 3600;
										download_flag = 1;			/*��Ҫ���ݱ�־*/
										download_number += 1;		/*15��-17�㲹Ҫ�����Բ���3�����ݣ�15��16��17��������1 ֻ�ܲ�Ҫ15��16����������*/
										if(usart_debug)
										{
													printf("%02u%02u%02u   %02u%02u%02u\r\n", month2, day2, hour2, month1, day1,hour1);
													printf("��Ҫ�����ݸ���:%d\r\n", download_number);
										}
							}						
				}
				else if((strlen(rx2_buffer) == 29) && (strncasecmp("<frequency ", rx2_buffer, 11) == 0))		/*����Ƶ�ʻ���ɷ��ٵ�ϵ��ws_rate_k��ws_rate_c  <frequency 000.2032 100.0001>*/
				{
						wind_sample.ws_rate_k = atof(rx1_buffer + 11);
						wind_sample.ws_rate_c = atof(rx1_buffer + 21);
					
						printf("ws_rate_k=%f	ws_rate_c=%f\r\n", wind_sample.ws_rate_k, wind_sample.ws_rate_c);
						
						if(data_eeprom_write(WINDSPEED_K_ADDR, 	(uint8_t *)&wind_sample.ws_rate_k, 		8) == HAL_OK)
						{
								printf("write WINDSPEED_K_ADDR ok\r\n");
						}
						else
						{
								printf("write WINDSPEED_K_ADDR failed\r\n");
						}
						if(data_eeprom_write(WINDSPEED_C_ADDR, 	(uint8_t *)&wind_sample.ws_rate_c, 		8) == HAL_OK)
						{
								printf("write WINDSPEED_C_ADDR ok\r\n");
						}
						else
						{
								printf("write WINDSPEED_C_ADDR failed\r\n");
						}
				}
				else if((strlen(rx2_buffer) == 36) && (strncasecmp("<correct ", rx2_buffer, 9) == 0))		//<correct 000.0232 100.0001 111.1234>
				{
						wind_sample.ws_a = atof(rx2_buffer + 10);
						wind_sample.ws_b = atof(rx2_buffer + 19);
						wind_sample.ws_c = atof(rx2_buffer + 28);
						printf("ws_a=%f	ws_b=%f	ws_c=%f\r\n", wind_sample.ws_a, wind_sample.ws_b, wind_sample.ws_c);
						
						if(data_eeprom_write(WINDSPEED_CORRECT_A_ADDR, 	(uint8_t *)&wind_sample.ws_a, 		8) == HAL_OK)
						{
								printf("write WINDSPEED_CORRECT_A_ADDR ok\r\n");
						}
						else
						{
								printf("write WINDSPEED_CORRECT_A_ADDR failed\r\n");
						}
						if(data_eeprom_write(WINDSPEED_CORRECT_B_ADDR, 	(uint8_t *)&wind_sample.ws_b, 		8) == HAL_OK)
						{
								printf("write WINDSPEED_CORRECT_B_ADDR ok\r\n");
						}
						else
						{
								printf("write WINDSPEED_CORRECT_B_ADDR failed\r\n");
						}
						if(data_eeprom_write(WINDSPEED_CORRECT_C_ADDR, 	(uint8_t *)&wind_sample.ws_c, 		8) == HAL_OK)
						{
								printf("write WINDSPEED_CORRECT_C_ADDR ok\r\n");
						}
						else
						{
								printf("write WINDSPEED_CORRECT_C_ADDR failed\r\n");
						}
				}
				#endif
				else
				{
						HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);     /*����PA1Ϊ�ߵ�ƽ 485�����ڷ���ģʽ*/
						printf("F:%s\r\n", rx2_buffer);
						Processing_FileManagement_Command((uint8_t *)rx2_buffer, rx2_count);
						HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);     /*����PA1Ϊ�͵�ƽ 485�����ڽ���ģʽ*/
				}
				
				rx2_count = 0;
				rx2_cplt = false;                                              /* clear the flag of receive */
				memset(rx2_buffer, 0, sizeof(rx2_buffer));                      /* clear the register of receive */
		}
	}
}

/**����1�жϺ���*/
void USART1_IRQHandler(void)
{
  UART_HandleTypeDef *huart=&huart1;
  uint32_t tmp_flag = 0, tmp_it_source = 0;

  tmp_flag = __HAL_UART_GET_FLAG(huart, UART_FLAG_PE);
  tmp_it_source = __HAL_UART_GET_IT_SOURCE(huart, UART_IT_PE);  
  /* UART parity error interrupt occurred ------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  { 
    huart->ErrorCode |= HAL_UART_ERROR_PE;
  }
  
  tmp_flag = __HAL_UART_GET_FLAG(huart, UART_FLAG_FE);
  tmp_it_source = __HAL_UART_GET_IT_SOURCE(huart, UART_IT_ERR);
  /* UART frame error interrupt occurred -------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  {
    huart->ErrorCode |= HAL_UART_ERROR_FE;
  }
  
  tmp_flag = __HAL_UART_GET_FLAG(huart, UART_FLAG_NE);
  /* UART noise error interrupt occurred -------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  {
    huart->ErrorCode |= HAL_UART_ERROR_NE;
  }
  
  tmp_flag = __HAL_UART_GET_FLAG(huart, UART_FLAG_ORE);
  /* UART Over-Run interrupt occurred ----------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  {
    huart->ErrorCode |= HAL_UART_ERROR_ORE;
  }
  
  tmp_flag = __HAL_UART_GET_FLAG(huart, UART_FLAG_RXNE);
  tmp_it_source = __HAL_UART_GET_IT_SOURCE(huart, UART_IT_RXNE);
  /* UART in mode Receiver ---------------------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  {
    /*UART_Receive_IT(huart);*/
    uint8_t data=0;
  
    data=huart->Instance->DR;  /* the byte just received  */
    
		
		if(!rx1_cplt)
    {
      if(data == '<')
      {
        cr1_begin=true;
        rx1_count=0;
        rx1_buffer[rx1_count]=data;
        rx1_count++; 
      }
     
      else if(cr1_begin == true)
      {
        rx1_buffer[rx1_count]=data;
        rx1_count++; 
        if(rx1_count>UART_RX_BUF_SIZE-1)  /* rx buffer full */
        {
          /* Set transmission flag: trasfer complete*/
          rx1_cplt = true;
        }
        
        if(data == '>')
        {
          rx1_cplt = true;
          cr1_begin = false;
        }
      }
      else
      {
        rx1_count = 0;
      }
    }

		
		
//    if(!rx1_cplt)
//    {
//      if(cr1_begin==true)  /* received '\r' */
//      {
//        cr1_begin=false;
//        if(data=='\n')  /* received '\r' and '\n' */
//        {
//          /* Set transmission flag: trasfer complete*/
//          rx1_cplt=true;
//        }
//        else
//        {
//          rx1_count=0;
//        }
//      }
//      else
//      {
//        if(data=='\r')  /* get '\r' */
//        {
//          cr1_begin=true;
//        }
//        else  /* continue saving data */
//        {
//          rx1_buffer[rx1_count]=data;
//          rx1_count++;
//          if(rx1_count>UART_RX_BUF_SIZE-1)  /* rx buffer full */
//          {
//            /* Set transmission flag: trasfer complete*/
//            rx1_cplt=true;
//          } 
//        }
//       }
//      }
    
  	 /* received a data frame ���ݽ�����ɾ��ͷŻ�����*/
    if(rx1_cplt==true)
    {
      if(semaphore_usart1!=NULL)
      {
         /* Release mutex */
        osSemaphoreRelease(semaphore_usart1);
				//printf("1���յ����ݲ��ͷ����ź���\r\n");
      }
    }

   
    }
  
  
  tmp_flag = __HAL_UART_GET_FLAG(huart, UART_FLAG_TXE);
  tmp_it_source = __HAL_UART_GET_IT_SOURCE(huart, UART_IT_TXE);
  /* UART in mode Transmitter ------------------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  {
    /*UART_Transmit_IT(huart);*/
  }

  tmp_flag = __HAL_UART_GET_FLAG(huart, UART_FLAG_TC);
  tmp_it_source = __HAL_UART_GET_IT_SOURCE(huart, UART_IT_TC);
  /* UART in mode Transmitter end --------------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  {
    /*UART_EndTransmit_IT(huart);*/
  }  

  if(huart->ErrorCode != HAL_UART_ERROR_NONE)
  {
    /* Clear all the error flag at once */
    __HAL_UART_CLEAR_PEFLAG(huart);
    
    /* Set the UART state ready to be able to start again the process */
    huart->State = HAL_UART_STATE_READY;
    
    HAL_UART_ErrorCallback(huart);
  } 
}

/**����2�жϺ���*/
void USART2_IRQHandler(void)
{
  UART_HandleTypeDef *huart=&huart2;
  uint32_t tmp_flag = 0, tmp_it_source = 0;

  tmp_flag = __HAL_UART_GET_FLAG(huart, UART_FLAG_PE);
  tmp_it_source = __HAL_UART_GET_IT_SOURCE(huart, UART_IT_PE);  
  /* UART parity error interrupt occurred ------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  { 
    huart->ErrorCode |= HAL_UART_ERROR_PE;
  }
  
  tmp_flag = __HAL_UART_GET_FLAG(huart, UART_FLAG_FE);
  tmp_it_source = __HAL_UART_GET_IT_SOURCE(huart, UART_IT_ERR);
  /* UART frame error interrupt occurred -------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  {
    huart->ErrorCode |= HAL_UART_ERROR_FE;
  }
  
  tmp_flag = __HAL_UART_GET_FLAG(huart, UART_FLAG_NE);
  /* UART noise error interrupt occurred -------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  {
    huart->ErrorCode |= HAL_UART_ERROR_NE;
  }
  
  tmp_flag = __HAL_UART_GET_FLAG(huart, UART_FLAG_ORE);
  /* UART Over-Run interrupt occurred ----------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  {
    huart->ErrorCode |= HAL_UART_ERROR_ORE;
  }
  
  tmp_flag = __HAL_UART_GET_FLAG(huart, UART_FLAG_RXNE);
  tmp_it_source = __HAL_UART_GET_IT_SOURCE(huart, UART_IT_RXNE);
  /* UART in mode Receiver ---------------------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  {
    /*UART_Receive_IT(huart);*/
    uint8_t data=0;
  
    data=huart->Instance->DR;  /* the byte just received  */
    
		
		if(!rx2_cplt)
    {
      if(data == '<')
      {
        cr2_begin=true;
        rx2_count=0;
        rx2_buffer[rx2_count]=data;
        rx2_count++; 
      }
     
      else if(cr2_begin == true)
      {
        rx2_buffer[rx2_count]=data;
        rx2_count++; 
        if(rx2_count>UART_RX_BUF_SIZE-1)  /* rx buffer full */
        {
          /* Set transmission flag: trasfer complete*/
          rx2_cplt = true;
        }
        
        if(data == '>')
        {
          rx2_cplt = true;
          cr2_begin = false;
        }
      }
      else
      {
        rx2_count = 0;
      }
    }

		
		
//    if(!rx2_cplt)
//    {
//      if(cr2_begin==true)  /* received '\r' */
//      {
//        cr2_begin=false;
//        if(data=='\n')  /* received '\r' and '\n' */
//        {
//          /* Set transmission flag: trasfer complete*/
//          rx2_cplt=true;
//        }
//        else
//        {
//          rx2_count=0;
//        }
//      }
//      else
//      {
//        if(data=='\r')  /* get '\r' */
//        {
//          cr2_begin=true;
//        }
//        else  /* continue saving data */
//        {
//          rx2_buffer[rx2_count]=data;
//          rx2_count++;
//          if(rx2_count>UART_RX_BUF_SIZE-1)  /* rx buffer full */
//          {
//            /* Set transmission flag: trasfer complete*/
//            rx2_cplt=true;
//          } 
//        }
//       }
//      }
    
  	 /* received a data frame ���ݽ�����ɾ��ͷŻ�����*/
    if(rx2_cplt==true)
    {
      if(semaphore_usart2!=NULL)
      {
					 /* Release mutex */
					osSemaphoreRelease(semaphore_usart2);
					//printf("2���յ����ݲ��ͷ����ź���\r\n");
      }
    }

   
    }
  
  
  tmp_flag = __HAL_UART_GET_FLAG(huart, UART_FLAG_TXE);
  tmp_it_source = __HAL_UART_GET_IT_SOURCE(huart, UART_IT_TXE);
  /* UART in mode Transmitter ------------------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  {
    /*UART_Transmit_IT(huart);*/
  }

  tmp_flag = __HAL_UART_GET_FLAG(huart, UART_FLAG_TC);
  tmp_it_source = __HAL_UART_GET_IT_SOURCE(huart, UART_IT_TC);
  /* UART in mode Transmitter end --------------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  {
    /*UART_EndTransmit_IT(huart);*/
  }  

  if(huart->ErrorCode != HAL_UART_ERROR_NONE)
  {
    /* Clear all the error flag at once */
    __HAL_UART_CLEAR_PEFLAG(huart);
    
    /* Set the UART state ready to be able to start again the process */
    huart->State = HAL_UART_STATE_READY;
    
    HAL_UART_ErrorCallback(huart);
  } 
}


