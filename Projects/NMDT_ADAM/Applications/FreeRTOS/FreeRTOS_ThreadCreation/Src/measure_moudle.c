/* Includes ------------------------------------------------------------------*/
#include "measure_moudle.h"
#include "cmsis_os.h"

#include "usart_module.h"
#include "storage_module.h"
#include "sys_time_module.h"


#define measureSTACK_SIZE   384//configMINIMAL_STACK_SIZE
#define measurePRIORITY     osPriorityNormal

/*�������ݻ�������*/
uint8_t data_buf[252]={0};
// ADת�����ֵ
__IO uint16_t ADC_ConvertedValue;
/* os relative */
static osThreadId MeasureThreadHandle;
static osSemaphoreId semaphore;
static osMutexId mutex;

/* Private function prototypes -----------------------------------------------*/
static void Measure_Thread(void const *argument);
static void TemperaturesMeasure(TEMP *Temp,const RTC_DateTypeDef *date,const RTC_TimeTypeDef *time,const void *debug);
static void OnBoardMeasure(Device_State *device_m,const void *debug);
static void AD7705_CH2_Measure(Device_State *device_a,const void *debug);
/*��������*/
TEMP Temperatures=
{
	.TEMP_1H_MIN = +800,
	.TEMP_1H_MAX = -600,
};

static Device_State device_m;
static Debug  debug={0};
/**
  * @brief  Init Measurement Module. 
  * @retval 0:success;-1:failed
  */
int32_t init_measure_module(void)
{
	/*����PB6��ʼ��*/
	MX_GPIO_Init();
  
  /* Init AD7705 */
  if(AD7705_Init())
  {
    printf("AD7705 OK!\r\n");
  }
  else
  {
    printf("AD7705 Err!\r\n");	
  }
	
	/* Define used semaphore */
  osSemaphoreDef(SEM);
  /* Create the semaphore used by the two threads */
  semaphore=osSemaphoreCreate(osSemaphore(SEM), 1);
  if(semaphore == NULL)
  {
    printf("Create Semaphore failed!\r\n");
    return -1;
  }
	
	 /* Create the mutex */
  osMutexDef(Mutex);
  mutex=osMutexCreate(osMutex(Mutex));
  if(mutex == NULL)
  {
    printf("Create Mutex failed!\r\n");
    return -1;
  }
	
	/* Create a thread to update system date and time */
  osThreadDef(Measure, Measure_Thread, measurePRIORITY, 0, measureSTACK_SIZE);
  MeasureThreadHandle=osThreadCreate(osThread(Measure), NULL);
  if(MeasureThreadHandle == NULL)
  {
    printf("Create Measure Thread failed!\r\n");
    return -1;
  }
  
  
  return 0;
}

/**
  * @brief  Start a measurement. 
  * @retval 0:success;-1:failed
  */
int32_t start_measure(void)
{
  /* Release the semaphore */
  if(semaphore==NULL)
  {
    return -1;
  }
  
  if(osSemaphoreRelease(semaphore)!=osOK)
  {
    return -1;
  }
  
  return 0;
}

/*����������*/
static void Measure_Thread(void const *argument)
{
  /* RTC Time*/
  /*static*/ RTC_TimeTypeDef time={0};
  /*static*/ RTC_DateTypeDef date={0};
	/*static*/ struct tm datetime={0};
	static uint8_t number_1=0;
	
	/* get system time use struct tm */
  (void)get_sys_time_tm(&datetime);
	
	while(osSemaphoreWait(semaphore,1)==osOK);   /* measurement semaphore */
  
  while(1)
  {
    /* Try to obtain the semaphore */
    if(osSemaphoreWait(semaphore,osWaitForever)==osOK)
    {
      /* get current system time */
      (void)get_sys_time(&date,&time);
			
			/* struct tm */
      datetime.tm_year=date.Year+2000;
      datetime.tm_mon=date.Month;
      datetime.tm_mday=date.Date;
      datetime.tm_hour=time.Hours;
      datetime.tm_min=time.Minutes;
      datetime.tm_sec=time.Seconds;
			
			 /* Wait until a Mutex becomes available */
      if(osMutexWait(mutex,1000)==osOK)
      {
        
        /* Temperature Measurement �¶Ȳ���*/
        TemperaturesMeasure(&Temperatures,&date,&time,&debug);
				
				/*���ڷ������AD7705��ͨ��2��ʼ����*/
				if(debug.ad7705_ch2)
				{
					AD7705_CH2_Measure(&device_m,&debug);
				}
				
				if(time.Seconds==30)
        {
          /* OnBoard Measurement */
          OnBoardMeasure(&device_m,&debug);
          
        }
        
				if(time.Seconds == 0)
				{
					/*����������*/
					number_1 = OutputHourDataFill(data_buf,&datetime,&debug);
					/*�洢��SD��*/
					if(number_1 == 252)
					{
						if(start_storage())
						{
							printf("SD semaphore free error!\r\n");
						}
						/*������ѯ���Ƶ�data_timeֵȷ�����ͷ������ݵļ��*/
						if((device_m.data_time == 0)||(device_m.data_time == 1))
						{
							HAL_UART_Transmit(&huart1, data_buf, sizeof(data_buf), 0xFFFF);
						}
						else if((time.Minutes %device_m.data_time == 0)&&(device_m.data_time < 60))
						{
							HAL_UART_Transmit(&huart1, data_buf, sizeof(data_buf), 0xFFFF);
						}
						else if(device_m.data_time > 59)
						{
							printf("CPU dont autom send data!\r\n");
						}
					}
					else
					{
						printf("data number :%d\r\n",number_1);
					}
					
				}
        
        /* Release mutex */
        osMutexRelease(mutex);
				
			}
		}
	}
}

/**
  * @brief  Temperature Measurement 
  * @param  Temperature Measure Structure
  * @retval None
  */
static void TemperaturesMeasure(TEMP *Temp,const RTC_DateTypeDef *date,const RTC_TimeTypeDef *time,const void *debug_t)
{
	Debug *usart_debug = (Debug *)debug_t;
	/*�¶�AD����*/
	Temp->TEMP_AD = AD7705_SingleMeasurement(RT_AD7705_CH,RT_AD7705_GAIN,RT_AD7705_BUFFER);
	Temp->TEMP_AD_2S[time->Seconds /2] =  Temp->TEMP_AD;
	
	/*ת��Ϊ��ѹֵ*/
	Temp->TEMP_V  = (((float)Temp->TEMP_AD)/((float)(MAX_AD_VALUE)))*REF_EXT_VOLTAGE;      
	Temp->TEMP_V_2S[time->Seconds /2] = Temp->TEMP_V;
	
	/*ת��Ϊ����*/
	Temp->TEMP_R  = (Temp->TEMP_V / CURRENT_FLOW);
	Temp->TEMP_R_2S[time->Seconds /2] =  Temp->TEMP_R;
	
	/*�����¶�ֵ*/
	Temp->TEMP_T = CalculateTemperature(Temp->TEMP_R);
	/*�¶ȶ���ֵ*/
	Temp->TEMP_T += Temp->temp_correction_value;
	Temp->TEMP_T_2S[time->Seconds /2] = Temp->TEMP_T * 10;
	
	/*ȷ���¶�ֵ��Χ*/
	if(Temp->TEMP_T > 80.0)
	{
		Temp->TEMP_T = 80.0;
	}
	if(Temp->TEMP_T < -60.0)
	{
		Temp->TEMP_T = -60.0;
	}
	
	/*����*/
	if(usart_debug->temperature_debug == 1)
	{
		printf("TEMP_AD :%u  TEMP_V:%f  TEMP_R:%f \r\n",Temp->TEMP_AD,Temp->TEMP_V,Temp->TEMP_R);
		printf(" TEMP_T1:%f\r\n",Temp->TEMP_T-Temp->temp_correction_value);
		printf(" TEMP_T2:%f\r\n",Temp->TEMP_T);
	}
	
	/*����1���ӵ��¶�ƽ��ֵ*/
	if(time->Seconds==0)
	{
		/*ȥ�������Сֵ����ƽ��ֵ*/
		Temp->TEMP_1M = AverageWithoutMaxMin(Temp->TEMP_T_2S,LENGTH_OF(Temp->TEMP_T_2S));
		
		/*CPU�ϵ��һ����ʹ�ò���ֵ����1����ƽ���¶�ֵ*/
		if(usart_debug->first_power)
		{
			Temp->TEMP_1M = Temp->TEMP_T;
		}
		
		/*Сʱ�¶������Сֵ����*/
		if((Temp->TEMP_1M > Temp->TEMP_1H_MAX) || (time->Minutes==1))
		{
			Temp->TEMP_1H_MAX      = Temp->TEMP_1M;
			Temp->TEMP_1H_MAX_TIME = time->Hours *10 +time->Minutes ;
		}
		if((Temp->TEMP_1M > Temp->TEMP_1H_MIN) || (time->Minutes==1))
		{
			Temp->TEMP_1H_MIN      = Temp->TEMP_1M;
			Temp->TEMP_1H_MIN_TIME = time->Hours *10 + time->Minutes ;
		}
		 
		/*���� 1���Ӵ��ڴ�ӡ�¶������Сֵ��ʱ�� 1�����ڵ�AD����ֵ ����ֵ �¶�ֵ*/
		if(usart_debug->rain_debug)
		{
			printf("TEMP_1H_MAX:%04u  TEMP_1H_MAX_TIME:%04u  TEMP_1H_MIN:%04u  TEMP_1H_MIN_TIME:%04u",
			        Temp->TEMP_1H_MAX,Temp->TEMP_1H_MAX_TIME,Temp->TEMP_1H_MIN,Temp->TEMP_1H_MIN_TIME);
			
			printf("AD;%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u",
			       Temp->TEMP_AD_2S[0],Temp->TEMP_AD_2S[1],Temp->TEMP_AD_2S[2],Temp->TEMP_AD_2S[3],Temp->TEMP_AD_2S[4],
			       Temp->TEMP_AD_2S[5],Temp->TEMP_AD_2S[6],Temp->TEMP_AD_2S[7],Temp->TEMP_AD_2S[8],Temp->TEMP_AD_2S[9],
			       Temp->TEMP_AD_2S[10],Temp->TEMP_AD_2S[11],Temp->TEMP_AD_2S[12],Temp->TEMP_AD_2S[13],Temp->TEMP_AD_2S[14],
			       Temp->TEMP_AD_2S[15],Temp->TEMP_AD_2S[16],Temp->TEMP_AD_2S[17],Temp->TEMP_AD_2S[18],Temp->TEMP_AD_2S[19],
			       Temp->TEMP_AD_2S[20],Temp->TEMP_AD_2S[21],Temp->TEMP_AD_2S[22],Temp->TEMP_AD_2S[23],Temp->TEMP_AD_2S[24],
			       Temp->TEMP_AD_2S[25],Temp->TEMP_AD_2S[26],Temp->TEMP_AD_2S[27],Temp->TEMP_AD_2S[28],Temp->TEMP_AD_2S[29]);
			
			printf("TEMP_T;%04u-%04u-%04u-%04u-%04u-%04u-%04u-%04u-%04u-%04u-%04u-%04u-%04u-%04u-%04u-%04u-%04u-%04u-%04u-%04u-%04u-%04u-%04u-%04u-%04u-%04u-%04u-%04u-%04u-%04u",
			       Temp->TEMP_T_2S[0],Temp->TEMP_T_2S[1],Temp->TEMP_T_2S[2],Temp->TEMP_T_2S[3],Temp->TEMP_T_2S[4],
			       Temp->TEMP_T_2S[5],Temp->TEMP_T_2S[6],Temp->TEMP_T_2S[7],Temp->TEMP_T_2S[8],Temp->TEMP_T_2S[9],
			       Temp->TEMP_T_2S[10],Temp->TEMP_T_2S[11],Temp->TEMP_T_2S[12],Temp->TEMP_T_2S[13],Temp->TEMP_T_2S[14],
			       Temp->TEMP_T_2S[15],Temp->TEMP_T_2S[16],Temp->TEMP_T_2S[17],Temp->TEMP_T_2S[18],Temp->TEMP_T_2S[19],
			       Temp->TEMP_T_2S[20],Temp->TEMP_T_2S[21],Temp->TEMP_T_2S[22],Temp->TEMP_T_2S[23],Temp->TEMP_T_2S[24],
			       Temp->TEMP_T_2S[25],Temp->TEMP_T_2S[26],Temp->TEMP_T_2S[27],Temp->TEMP_T_2S[28],Temp->TEMP_T_2S[29]);
		}
	}
}
	
/*��·���ѹ����*/
static void OnBoardMeasure(Device_State *device_b,const void *debug_b)
{
	Debug *usart_debug = (Debug *)debug_b;
	
	/*��·��AD����*/
	device_b->board_AD = ADC_ConvertedValue;
	
	/*��·���ѹ*/
	device_b->board_voltage    = ((float)device_b->board_AD / MAX_ADC1_VALUE) * REF_ADC1_VOLTAGE;
	device_b->board_voltage    = (device_b->board_voltage) *(VI_R1 + VI_R2) / VI_R1;
	device_b->board_voltage_10 = (uint32_t) (device_b->board_voltage*10);
	/*����*/
	if(usart_debug->rain_debug)
	{
		printf("board_AD;%u  board_voltage:%f\r\n",ADC_ConvertedValue,device_b->board_voltage);
	}
	
	/*���Ʋ�����ѹ���߻��߹���*/
	if(device_b->board_voltage_10 > VI_UPPER_LIMIT)
	{
		device_b->board_voltage_10 = 145;
	}
	if(device_b->board_voltage_10 < VI_LOWER_LIMIT)
	{
		device_b->board_voltage_10 = 55;
	}
}

/**
  * ��������: ADת�������ص�����
  * �������: hadc��AD�豸���;��
  * �� �� ֵ: ��
  * ˵    ��: ��
  */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc1)
{
  ADC_ConvertedValue=HAL_ADC_GetValue(hadc1);
}  




/*AD7705��ͨ��2���������ھ����ʲô���������ֻ�⵽ͨ��2�ĵ�ѹ*/
static void AD7705_CH2_Measure(Device_State *device_a,const void *debug_a)
{
	Debug *usart_debug = (Debug *)debug_a;
	
	device_a->AD7705_CH2_AD = AD7705_SingleMeasurement(AD7705_CH2,AD7705_CH2_GAIN,AD7705_CH2_BUFFER);
	device_a->AD7705_CH2_V  = ((float)device_a->AD7705_CH2_AD) / ((float)(MAX_AD_VALUE)) *REF_EXT_VOLTAGE;
	
	if(usart_debug->ad7705_ch2)
	{
		printf("AD7705_CH2_AD:%u   AD7705_CH2_V:%f\r\n",device_a->AD7705_CH2_AD,device_a->AD7705_CH2_V);
	}
}




