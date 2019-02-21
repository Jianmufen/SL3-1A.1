/* Includes ------------------------------------------------------------------*/
#include "sys_time_module.h"
#include "cmsis_os.h"
#include "sensor.h"

#include "usart_module.h"
#include "storage_module.h"
#include "display_module.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define systimeSTACK_SIZE   (512)
#define systimePRIORITY     osPriorityHigh

/* RTC Time*/
static RTC_TimeTypeDef sys_time;
static RTC_DateTypeDef sys_date;

#if (SENSOR_ID == SENSOR_W) 
static uint32_t ws_frequence;	/*����Ƶ��*/
#endif


static FIL file;  
#if (SENSOR_ID == SENSOR_R) 
static uint8_t minute_h = 0;			/*����60���������������ֺ󣬷��Ӽ���*/
static uint8_t minute_d = 0;			/*�������������ֺ󣬷��Ӽ���*/
#else

#endif
/* os relative */
static osThreadId SysTimeThreadHandle;
static osSemaphoreId semaphore;
static osMutexId mutex;
/* Private function prototypes -----------------------------------------------*/
static void SysTime_Thread(void const *argument);

/**
  * @brief  Init System Time Module. 
  * @retval 0:success;-1:failed
  */
int32_t init_sys_time_module(void)
{
			#if (SENSOR_ID == SENSOR_W) 
			/*��ʼ����Ĳ�������*/
			wind_sample.wd_1s_2m_n = 0;			/*����Ĳ�������������֮���0-120������120�͵���120����2������120��������*/
			wind_sample.ws_s_n     = 0;
			wind_sample.wd_1m_h_n  = 0;
			wind_sample.ws_1s_m_n  = 0;
	
			wind_sample.wind_warn_flag = false;		/*��籨����־*/
			#else
			rain_sample.rain_60m_warn_flag  = false;		/*������������־*/
			rain_sample.rain_d_warn_flag  	= false;
			#endif
			
			
			/*��ʼ��RTC*/
			MX_RTC_Init();
			 /* Init extern RTC PCF8563 */
			if(IIC_Init() == HAL_ERROR)
			{
//				printf("init pcf8563 failed!\r\n");
			}
			else
			{
				/* synchronize internal RTC with pcf8563 */
				sync_time();
			}
			/* Init RTC Internal */
			
			
			/*��ʼ����ʱ��6   ���ж�ģʽ��������ʱ��*/
//			TIM6_Init();
//			HAL_TIM_Base_Start_IT(&htim6);
			
			/*��ʼ����ʱ��2*/
			TIM3_Init();
//			HAL_TIM_Base_Start(&htim3); 		/*������ʱ��3�����ⲿ�źż���*/
			HAL_TIM_Base_Start_IT(&htim3); /* TIM3 Time Base */
			HAL_TIM_IC_Start_IT(&htim3,TIM_CHANNEL_1);   /* TIM3 Input Catpture in Channel1 */
			
			/* Define used semaphore */
			osSemaphoreDef(SEM);
			/* Create the semaphore used by the two threads */
			semaphore=osSemaphoreCreate(osSemaphore(SEM), 1);
			if(semaphore == NULL)
			{
//				printf("Create Semaphore failed!\r\n");
				return -1;
			}
			
			/* Create the mutex */
			osMutexDef(Mutex);
			mutex=osMutexCreate(osMutex(Mutex));
			if(mutex == NULL)
			{
//				printf("Create Mutex failed!\r\n");
				return -1;
			}
			
			/* Create a thread to update system date and time */
			osThreadDef(SysTime, SysTime_Thread, systimePRIORITY, 0, systimeSTACK_SIZE);
			SysTimeThreadHandle=osThreadCreate(osThread(SysTime), NULL);
			if(SysTimeThreadHandle == NULL)
			{
//				printf("Create System Time Thread failed!\r\n");
				return -1;
			}
			return 0;
}

/**
  * @brief  get System Date and Time. 
  * @retval 0:success;-1:failed
  */
int32_t get_sys_time(RTC_DateTypeDef *sDate,RTC_TimeTypeDef *sTime)
{
  /* Wait until a Mutex becomes available */
  if(osMutexWait(mutex,500)==osOK)
  {
    if(sDate)
    {
      *sDate=sys_date;
    }
    if(sTime)
    {
      *sTime=sys_time;
    }
    
    /* Release mutex */
    osMutexRelease(mutex);
    
    return 0;
  }
  else
  {
    /* Time */
    if(sTime)
    {
      sTime->Seconds=0;
      sTime->Minutes=0;
      sTime->Hours=0;
    }
    /* Date */
    if(sDate)
    {
      sDate->Date=1;
      sDate->WeekDay=RTC_WEEKDAY_SUNDAY;
      sDate->Month=(uint8_t)RTC_Bcd2ToByte(RTC_MONTH_JANUARY);
      sDate->Year=0;
    }
    
    return -1;
  }
}

int32_t get_sys_time_tm(struct tm *DateTime)
{
  /* Wait until a Mutex becomes available */
  if(osMutexWait(mutex,500)==osOK)
  {
    if(DateTime)
    {
      DateTime->tm_year 	= sys_date.Year+2000;
      DateTime->tm_mon 		= sys_date.Month;
      DateTime->tm_mday		= sys_date.Date;
      DateTime->tm_hour 	= sys_time.Hours;
      DateTime->tm_min 		= sys_time.Minutes;
      DateTime->tm_sec  	= sys_time.Seconds;
    }
    
    /* Release mutex */
    osMutexRelease(mutex);
    
    return 0;
  }
  else
  {
    if(DateTime)
    {
      DateTime->tm_year=2000;
      DateTime->tm_mon=0;
      DateTime->tm_mday=0;
      DateTime->tm_hour=0;
      DateTime->tm_min=0;
      DateTime->tm_sec=0;
    }
    
    return -1;
  }
}

int32_t set_sys_time(RTC_DateTypeDef *sDate,RTC_TimeTypeDef *sTime)
{
  int32_t res=0;
  
  /* Wait until a Mutex becomes available */
  if(osMutexWait(mutex,500)==osOK)
  {
    if(sDate)
    {
      sys_date=*sDate;
    }
    if(sTime)
    {
      sys_time=*sTime;
    }
    
    /* check param */
    if(IS_RTC_YEAR(sys_date.Year) && IS_RTC_MONTH(sys_date.Month) && IS_RTC_DATE(sys_date.Date) &&
       IS_RTC_HOUR24(sys_time.Hours) && IS_RTC_MINUTES(sys_time.Minutes) && IS_RTC_SECONDS(sys_time.Seconds))
    {
    
      if((HAL_RTC_SetDate(&hrtc,&sys_date,FORMAT_BIN)==HAL_OK)&&  /* internal RTC */
         (HAL_RTC_SetTime(&hrtc,&sys_time,FORMAT_BIN)==HAL_OK)&&
         (PCF8563_Set_Time(sys_date.Year,sys_date.Month,sys_date.Date,sys_time.Hours,sys_time.Minutes,sys_time.Seconds)==HAL_OK) )     /* PCF8563 */
      {
        res=0;
      }
      else
      {
        res=-1;
      }
    }
    else
    {
      res=-1;
    }
    
    /* Release mutex */
    osMutexRelease(mutex);
    
    return res;
  }
  else
  {
    return -1;
  }
}
/*�紫����*/
#if (SENSOR_ID == SENSOR_W) 

/**
  * ��������: ������ģʽ�¶�ʱ���Ķ�ʱ�ص�����
  * �������: htim����ʱ�����
  * �� �� ֵ: ��
  * ˵    ��: ��
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
			uint8_t i = 0;
	
			if(htim->Instance==TIM3)
			{
						/* use TIM3's 1-second time base to determine wind speed frequence */
						wind_sample.ws_rate = ws_frequence;
						ws_frequence = 0;
								
						/*������Ϣ*/
						if(usart_debug)
						{
								printf("ws_rate=%d		", wind_sample.ws_rate);
						}
				
						/*���ٲ���Ƶ��*/
						for(i=11 ; i>0 ; i--)
						{
								wind_sample.ws_rate_3s[i] = wind_sample.ws_rate_3s[i-1];
						}
						wind_sample.ws_rate_3s[0] = wind_sample.ws_rate;
						wind_sample.ws_0 = CalculateWindSpeed(wind_sample.ws_rate);		/*���ݲ���Ƶ�ʵõ���������*/
						
						/*������Ϣ*/
						if(usart_debug)
						{
								printf("		ws_0=%f\r\n", wind_sample.ws_0);
						}
						
						wind_sample.ws_rate = 0;                 /*���������Ƶ��*/
						/*��������*/
						for(i=11 ; i>0 ; i--)
						{
								wind_sample.ws_0_3s[i] = wind_sample.ws_0_3s[i-1];
						}
						wind_sample.ws_0_3s[0] = wind_sample.ws_0;
						if(wind_sample.ws_s_n < 12)
						{
								wind_sample.ws_s_n++;
						}
			}
}


/**
  * @brief  Input Capture callback in non blocking mode
  * @param  htim : TIM IC handle
  * @retval None��ʱ�������жϻص�����
  */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  if(htim->Instance==TIM3)
  {
				/*use TIM3's CH1 Input Capture interrupt to count wind speed frequence */
				ws_frequence++;
//				printf("ws_frequence++\r\n");
  }
}
#endif


/**
  * @brief  System sys_time update
  * @param  thread not used
  * @retval None
  */
static void SysTime_Thread(void const *argument)
{
		uint8_t i = 0, j = 0;
		FRESULT  res=FR_OK;
		uint32_t byteswritten1 = 0;
		char path_file[24]={0};
		uint32_t offset_save = 0;
		static struct tm data_time = {0};
		uint16_t rain_history = 0;
		unsigned int year_h = 0, month_h = 0, day_h = 0, hour_h = 0, minute_h = 0, time_h1 = 0, time_h2 = 0, time_c = 0;;
		/* Init IWDG  */
		IWDG_Init();
	
		while(1)
		{
			/* Try to obtain the semaphore */
			if(osSemaphoreWait(semaphore,osWaitForever)==osOK)
			{
				/* Wait until a Mutex becomes available */
				if(osMutexWait(mutex,500)== osOK)
				{
					HAL_RTC_GetTime(&hrtc,&sys_time,FORMAT_BIN);
					HAL_RTC_GetDate(&hrtc,&sys_date,FORMAT_BIN);
					
//						HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);     /*��ʼ��PA1Ϊ�ߵ�ƽ 485�����ڷ���ģʽ*/
//						HAL_UART_Transmit(&huart2, (uint8_t *)"Hello World!\r\n", 15, 0xFF);
//						HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);     /*��ʼ��PA1Ϊ�͵�ƽ 485�����ڽ���ģʽ*/
					
					/*��Ҫ����*/
					if(download_flag)
					{
								if(start_storage() == 0)
									{
											if(usart_debug)
											{
													printf("��ʼ��Ҫ����\r\n");
											}
											download_flag = 0;
									}
					}
					
					/*����ʱ��*/
					if(usart_debug)
					{
							printf("RTC:20%02u-%02u-%02u %02u:%02u:%02u\r\n",\
								 sys_date.Year, sys_date.Month, sys_date.Date, sys_time.Hours, sys_time.Minutes, sys_time.Seconds);
					}
					
					
					/* synchronize internal RTC with pcf8563 */
					if((sys_time.Minutes==25) && (sys_time.Seconds==15))
					{
//							printf("sync time success\r\n");
							sync_time();
					}
	
					/*EN2-B*/
					#if (SENSOR_ID == SENSOR_W)                          /*�紫����*/
					WindCalculate(&sys_date,&sys_time);                  /*���������*/
					
					if(sys_time.Seconds == 0)                              /*ÿ����*/
					{
							if(wind_sample.wind_warn_flag)  		/*������һ�γ���֮���20����*/
							{
									wind_sample.wind_warn_minutes++;				/*һ���Ӽ���һ��*/
									if(wind_sample.wind_warn_minutes > 20)
									{
											wind_sample.wind_warn_flag = false;
											HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2,GPIO_PIN_RESET);          /*��PD2 = 0��������ֹͣ��*/
									}
							}
							
							/*���ṹ��tm*/
							data_time.tm_year 	= sys_date.Year;
							data_time.tm_mon 		= sys_date.Month;
							data_time.tm_mday 	= sys_date.Date;
							data_time.tm_hour 	= sys_time.Hours;
							data_time.tm_min 		= sys_time.Minutes;
							data_time.tm_sec 		= sys_time.Seconds;
							memset(data_buf, 0, sizeof(data_buf));				/*��������*/
							i = WindOutputMinDataFill(&data_time, (struct wind_minute *)data_buf); /*fill wind datd of minutes*/
							/*������Ϣ*/
							if(usart_debug)
							{
									printf("�������ݵķ���ʱ��=%d\r\n", sys_time.Minutes);
									printf("�������ݵ��ֽ��� = %d\r\n", i);
									printf("����=[%s]\r\n", data_buf);
							}						
							/*ÿ����00�뷢�ͷ�������*/
							//printf("%s", data_buf);
							
							/*�洢���ݵ�SD��*/	
							offset_save = (sys_time.Hours * 60 + sys_time.Minutes) * 72;
							memset(path_file, 0, sizeof(path_file));
							snprintf(path_file, sizeof(path_file), "/DATA/%02u/%02u.txt", sys_date.Month, sys_date.Date);
							res = f_open(&file, (const char *)path_file, FA_OPEN_ALWAYS | FA_WRITE);
							if(usart_debug)
							{
									printf("offset_save=%d Hours=%d Minutes = %d\r\n", offset_save, sys_time.Hours, sys_time.Minutes);
									printf("path_file = %s\r\n", path_file);
							}
							if(res == FR_OK)
							{
									if(usart_debug)
									 {
												printf("���ļ��ɹ�\r\n");
									 }
									 res = f_lseek(&file, offset_save);
									 if(res != FR_OK)
										{
												f_close(&file);	
												if(usart_debug)
												{
															printf("�ļ�ѡַ����:%d\r\n",res);
												}
												if(BSP_SD_Init() != MSD_OK)
												 {
															if(usart_debug)
																{
																		printf("sd init failed!\r\n");
																}													      
												 }
												else
												 {
															if(usart_debug)
																{
																		printf("sd1 init ok!\r\n");
																}																		      
												 }
										}
									else
									 {
													if(usart_debug)
													{
																printf("�ļ�ѡַ�ɹ�:%d\r\n",res);
													}		
													res = f_write(&file, (uint8_t *)data_buf, 70, &byteswritten1);
													if(res != FR_OK)
													{
															f_close(&file);	
															if(usart_debug)
																{
																		printf("д����ʧ�ܣ�%d\r\n",res);
																}																
															if(BSP_SD_Init()!=MSD_OK)
																{
																		if(usart_debug)
																			{
																					printf("sd2 init failed!\r\n");
																			}																	
																}
															else
																{
																		if(usart_debug)
																			{
																					printf("sd3 init ok!\r\n");
																			}																		
																}
														}	
													 else
														{
																	if(usart_debug)
																		{
																				printf("д���ݳɹ�:%s\r\n",data_buf);
																		}				
																	
														}	
													 res = f_close(&file);	
													 if(res == FR_OK)
														{
																	if(usart_debug)
																		{
																				printf("�ر��ļ��ɹ�:%d\r\n",res);
																		}																
														}
									 }							
						}
							else 
							{
//											printf("���ļ�ʧ�ܣ�%d\r\n", res);
											if(BSP_SD_Init()!=MSD_OK)
												{
															if(usart_debug)
																{
																		printf("sd4 init failed!\r\n");
																}					
															
												}
											else
												{
													if(usart_debug)
														{
															printf("sd5 init ok!\r\n");
														}				
												}
									}
							
							if(sys_time.Minutes  == 0)
							{
									/*���㼫���*/			
									wind_sample.wd_1s_max   = 0;
									wind_sample.ws_1s_max   = 0;
									wind_sample.ws_1s_max_t = 1;
								
									/*��������*/
									wind_sample.wd_10m_max   = 0;
									wind_sample.ws_10m_max   = 0;
									wind_sample.ws_10m_max_t = 0;
							}
					}
					
					#endif
					
					/*SL3-1A*/
					#if (SENSOR_ID == SENSOR_R)                     /*����������*/
					if(power_flag)
					{
								rain_history = 0;
								res = f_open(&file, (const char *)"1.txt", FA_READ);
								if(res == FR_OK)
								{
											memset(data_buf, 0, sizeof(data_buf));	
											res = f_read(&file, data_buf, 24, &byteswritten1);	
											if(res == FR_OK)
											{
														//printf("data_buf=%s\r\n", data_buf);
														//printf("data_buf[17]=%d\r\n", data_buf[17]);
														//printf("data_buf[18]=%d\r\n", data_buf[18]);
														//printf("data_buf[19]=%d\r\n", data_buf[19]);
														//printf("data_buf[20]=%d\r\n", data_buf[20]);
														year_h 	 = (data_buf[0] - 48)  * 1000 + (data_buf[1]  - 48) * 100 + (data_buf[2]  - 48) * 10 + (data_buf[3]  - 48);
														month_h  = (data_buf[5] - 48)  * 10 + (data_buf[6]  - 48);
														day_h 	 = (data_buf[8] - 48)  * 10 + (data_buf[9]  - 48);
														hour_h	 = (data_buf[11] - 48)  * 10 + (data_buf[12] - 48);
														minute_h = (data_buf[14] - 48) * 10 + (data_buf[15] - 48);
														if(hour_h < 20)
														{
																	//printf("%04d-%02d-%02d %02d:%02d\r\n", year_h, month_h, day_h, hour_h, minute_h);
																	time_h1 = l_mktime(year_h, month_h, day_h, 20, 0, 0);							/*һ��Ľ���ʱ��*/
																	SubaDay((int *)&year_h, (int *)&month_h, (int *)&day_h, 0, 0, 0);	/*��һ��*/
																	//printf("%04d-%02d-%02d %02d:%02d\r\n", year_h, month_h, day_h, hour_h, minute_h);
																	time_h2 = l_mktime(year_h, month_h, day_h, 20, 01, 0);						/*һ��Ŀ�ʼʱ��*/
																	time_c 	= l_mktime(sys_date.Year + 2000, sys_date.Month, sys_date.Date, \
																								sys_time.Hours, sys_time.Minutes, sys_time.Seconds);/*��ǰʱ��*/
															
																	//printf("%d %d %d\r\n", time_h2, time_c, time_h1);
															
																	if((time_c < time_h1) && (time_c > time_h2))
																	{
																				//printf("1\r\n");
																				rain_history = (data_buf[17] - 48) * 1000 + (data_buf[18] - 48) * 100 + \
																												(data_buf[19] - 48) * 10 + (data_buf[20] - 48);
																				rain_sample.rain_d += rain_history;
																	}
																	else
																	{
																				//printf("2\r\n");
																				rain_history = 0;
																				rain_sample.rain_d += rain_history;
																	}
														}
														else
														{
																	time_h2 = l_mktime(year_h, month_h, day_h, 20, 01, 0);						/*һ��Ŀ�ʼʱ��*/
																	AddaDay((int *)&year_h, (int *)&month_h, (int *)&day_h, 0, 0, 0);	/*��һ��*/
																	time_h1 = l_mktime(year_h, month_h, day_h, 20, 0, 0);							/*һ��Ľ���ʱ��*/
																	time_c 	= l_mktime(sys_date.Year + 2000, sys_date.Month, sys_date.Date, \
																								sys_time.Hours, sys_time.Minutes, sys_time.Seconds);/*��ǰʱ��*/
															
																	if((time_c < time_h1) &&(time_c > time_h2))
																	{
																				//printf("3\r\n");
																				rain_history = (data_buf[17] - 48) * 1000 + (data_buf[18] - 48) * 100 + \
																												(data_buf[19] - 48) * 10 + (data_buf[20] - 48);
																				rain_sample.rain_d += rain_history;
																	}
																	else
																	{
																				//printf("4\r\n");
																				rain_history = 0;
																				rain_sample.rain_d += rain_history;
																	}
														}
														//printf("rain_sample.rain_d=%d\r\n", rain_sample.rain_d);
														f_close(&file);	
														power_flag = 0;
											}
											else
											{
														f_close(&file);	
														rain_history = 0;
														power_flag ++;
											}
								}
								else
								{
											rain_history = 0;
											power_flag++;
								}
								
								if(power_flag > 10)
								{
											power_flag = 0;
								}
					}
					/*ÿ����  ��������һ�쿪ʼ�� 20:00:01  �����ڵڶ���� 20:00:00*/
					if(sys_time.Seconds == 1)
					{
								/*ÿһ���ӽ�����������SD������*/
								memset(data_buf, 0, sizeof(data_buf));	
								snprintf(data_buf, sizeof(data_buf), "20%02d-%02d-%02d %02d:%02d,%04d\r\n", sys_date.Year,	sys_date.Month, \
														sys_date.Date, sys_time.Hours, sys_time.Minutes, rain_sample.rain_d);
								res = f_open(&file, (const char *)"/1.txt", FA_OPEN_ALWAYS | FA_WRITE);
								if(res == FR_OK)
								{
											//printf("data_buf=%s", data_buf);
											res = f_write(&file, (uint8_t *)data_buf, 24, &byteswritten1);
											if(res == FR_OK)
											{
														f_close(&file);	
											}
								}
								/***********************************ÿ���Ӽ���һ������****************************************************/
								rain_sample.rain_60m_1m[sys_time.Minutes] = rain_sample.rain_1m;		/*���Ӷ�Ӧ���浽������*/
								
								/*������Ϣ*/
								if(usart_debug)
								{
										printf("rain_1m = %d	rain_d = %d\r\n", rain_sample.rain_1m, rain_sample.rain_d);	//���ӡ�Сʱ��������
								}
								
								rain_sample.rain_1m = 0;                                  /*1������������*/
								
								j = sys_time.Minutes;
								rain_sample.rain_10m = 0;                                 /*����10������������*/
								for(i = 0; i < 10; i++)
								{
										rain_sample.rain_10m += rain_sample.rain_60m_1m[j];     /*����10���������ۼ�*/
										j--;
										if(j > 59)
										{
												j = 59;
										}
								}
								if(usart_debug)
								{
										printf("rain_10m = %d\r\n", rain_sample.rain_10m);			/*������Ϣ*/		
								}
								
								rain_sample.rain_60m = 0;																	 /*����60������������*/
								for(i = 0; i < 60; i++)
								{
										rain_sample.rain_60m += rain_sample.rain_60m_1m[i];     /*���60���������ۼ�*/
								}
								
								/*����60�����������ֵ*/
								if(rain_sample.rain_60m > rain_sample.rain_60m_max )
								{
										rain_sample.rain_60m_max   =	rain_sample.rain_60m;
										rain_sample.rain_60m_max_t = 	sys_time.Hours * 100 + sys_time.Minutes; /*����60�����������ֵ���ֵ�ʱ��*/
								}
								
								/*����60��������������������60���������ۼ�ֵ���ڱ���ֵ*/
								if(rain_sample.rain_60m >= rain_sample.rain_60m_warn )
								{
										rain_sample.rain_60m_warn_t =	sys_time.Hours * 100 + sys_time.Minutes ;		/*����60������������ֵ���ֵ�ʱ��*/
										HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);                    /*����60������������������ֵ ����LED2*/
										rain_sample.rain_60m_warn_flag = true;
										minute_h = 0;																															/*��������60���ӱ����󾭹���ʱ����ӵ��ۼ�ֵ*/
								}
								
								/*����60���ӱ��������ڼ��ڣ�δ���ֳ�������ֵ�����������LED2 ʹ����������*/
								if(rain_sample.rain_60m_warn_flag)
								{
										minute_h++;
										if(minute_h > 20)
										{
												rain_sample.rain_60m_warn_flag = false;
												HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);                    /*20����֮��û�г��ֳ���������ֵ������ ����LED2*/
										}
								}
								
								/*��������������1�����������ۼ�ֵ���ڱ���ֵ*/
								if(rain_sample.rain_d >= rain_sample.rain_d_warn )
								{
										rain_sample.rain_d_warn_t =	sys_time.Hours * 100 + sys_time.Minutes ;		/*Сʱ��������ֵ���ֵ�����*/
										HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);                    /*Сʱ��������������ֵ ����LED3*/       
										rain_sample.rain_d_warn_flag = true;
										minute_d = 0;																															/*����Сʱ�����󾭹���ʱ����ӵ��ۼ�ֵ*/
								}
								
								/*���������ֵı�־*/
								if(rain_sample.rain_d_warn_flag)
								{
										minute_d++;
										if(minute_d > 20)
										{
												rain_sample.rain_d_warn_flag = false;
												HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);                    /*20����֮��û�г��ֳ���������ֵ������ ����LED3*/
										}
								}
								
								/*��������������*/
								if((rain_sample.rain_60m_warn_flag) || (rain_sample.rain_d_warn_flag))							/*Сʱ�������ձ�������һ����2��������*/
								{
										HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2,  GPIO_PIN_SET);												/*�͵�ƽʹ��������*/
										/*������Ϣ*/
										if(usart_debug)
										{
												printf("buzzer loud\r\n");
										}
								}
								else																																							/*Сʱ�������ձ�����������*/
								{
										HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2,  GPIO_PIN_RESET);												/*ʹ����������*/
								}
								
								/*������Ϣ*/
								if(usart_debug)
								{
										printf("rain_60m = %d		rain_60m_max = %d		rain_60m_max_t = %d\r\n", rain_sample.rain_60m, rain_sample.rain_60m_max, rain_sample.rain_60m_max_t);
										printf("rain_10m = %d\r\n", rain_sample.rain_10m);
										printf("rain_60m_warn = %d		rain_60m_warn_t = %d\r\n", rain_sample.rain_60m_warn, rain_sample.rain_60m_warn_t);
										printf("rain_d_warn = %d		rain_d_warn_t = %d\r\n", rain_sample.rain_d_warn, rain_sample.rain_d_warn_t);
								}
										
								/*ÿСʱ*/
								if(sys_time.Minutes == 0)
								{
										/*���ṹ��tm*/
										data_time.tm_year 	= sys_date.Year;
										data_time.tm_mon 		= sys_date.Month;
										data_time.tm_mday 	= sys_date.Date;
										data_time.tm_hour 	= sys_time.Hours;
										data_time.tm_min 		= sys_time.Minutes;
										data_time.tm_sec 		= sys_time.Seconds;
//								/*��������60�����������ֵ�ͳ��ֵ�ʱ��*/
//								rain_sample.rain_60m_max = 0;
//								rain_sample.rain_60m_max_t = 1;
							
										/*���Сʱ����*/
										memset(data_buf, 0, sizeof(data_buf));									/*���㻺��*/
										i = rain_hour_fill(&data_time, (struct rain_hour_data *)data_buf);
									
										/*������Ϣ*/
										if(usart_debug)
										{
												printf("�������ݵķ���ʱ��=%d\r\n", sys_time.Minutes);
												printf("sizeof(rain_hour_data) = %d\r\n", i);
												printf("rain_hour_data:[%s]\r\n", data_buf);
//												printf("2RTC:20%02u-%02u-%02u %02u:%02u:%02u\r\n", sys_date.Year, sys_date.Month,\
//																sys_date.Date, sys_time.Hours, sys_time.Minutes, sys_time.Seconds);
										}						
										
										/*�洢���ݵ�SD��*/	
										offset_save = ((sys_date.Date - 1) * 24 + sys_time.Hours) * 171;
										memset(path_file, 0, sizeof(path_file));
										snprintf(path_file, sizeof(path_file), "/DATA/%02u.txt", sys_date.Month);
										res = f_open(&file, (const char *)path_file, FA_OPEN_ALWAYS | FA_WRITE);
										if(usart_debug)
										{
												printf("offset_save=%d day=%d Hours = %d	minute=%d\r\n", offset_save, sys_date.Date, sys_time.Hours, sys_time.Minutes);
												printf("offset_save = %d	sizeof(path_file)=%d\r\n", offset_save, sizeof(path_file));
												printf("path_file = %s\r\n", path_file);
										}
										if(res == FR_OK)
											{
													if(usart_debug)
													 {
																printf("���ļ��ɹ�\r\n");
													 }
													 res = f_lseek(&file, offset_save);
													 res = FR_OK;
													 if(res != FR_OK)
														{
																f_close(&file);	
																if(usart_debug)
																{
																			printf("�ļ�ѡַ����:%d\r\n",res);
																}
																if(BSP_SD_Init() != MSD_OK)
																 {
																			if(usart_debug)
																				{
																						printf("sd init failed!\r\n");
																				}													      
																 }
																else
																 {
																			if(usart_debug)
																				{
																						printf("sd1 init ok!\r\n");
																				}																		      
																 }
														}
													else
													 {
																	if(usart_debug)
																	{
																				printf("�ļ�ѡַ�ɹ�:%d\r\n",res);
																	}		
																	res = f_write(&file, (uint8_t *)data_buf, 171, &byteswritten1);
																	if(res != FR_OK)
																	{
																			f_close(&file);	
																			if(usart_debug)
																				{
																						printf("д����ʧ�ܣ�%d\r\n",res);
																				}																
																			if(BSP_SD_Init()!=MSD_OK)
																				{
																						if(usart_debug)
																							{
																									printf("sd2 init failed!\r\n");
																							}																	
																				}
																			else
																				{
																						if(usart_debug)
																							{
																									printf("sd3 init ok!\r\n");
																							}																		
																				}
																		}	
																	 else
																		{
																					if(usart_debug)
																						{
																								printf("д���ݳɹ�:%s\r\n",data_buf);
																						}				
																					
																		}	
																	 res = f_close(&file);	
																	 if(res == FR_OK)
																		{
																					if(usart_debug)
																						{
																								printf("�ر��ļ��ɹ�:%d\r\n",res);
																						}																
																		}
													 }							
										}
										else 
										{
//												printf("���ļ�ʧ�ܣ�%d\r\n", res);
												if(BSP_SD_Init()!=MSD_OK)
													{
																if(usart_debug)
																	{
																			printf("sd4 init failed!\r\n");
																	}					
																
													}
												else
													{
														if(usart_debug)
															{
																printf("sd5 init ok!\r\n");
															}				
													}
										}
										
										
										/*ÿ��*/
										if(sys_time.Hours == 20)
											{				
														rain_sample.rain_d = 0;                                 /*����������*/
											}
								}
						
					}
					#endif
					/* Release mutex */
					osMutexRelease(mutex);
					
				}
				else
				{
					//printf("û�еȵ������ź���\r\n");
				}
			}
    
    
    if(hiwdg.Instance)
    {
					HAL_IWDG_Refresh(&hiwdg);  /* refresh the IWDG */
					//printf("ι����\r\n");
    }
    
  }
}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
  
  /* Release the semaphore every 1 second */
   if(semaphore!=NULL)
  {
			//printf("����A�жϲ�����\r\n");
			osSemaphoreRelease(semaphore);
  }
}


