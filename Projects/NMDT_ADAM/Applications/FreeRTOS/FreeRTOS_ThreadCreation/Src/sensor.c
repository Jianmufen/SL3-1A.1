/* Includes ------------------------------------------------------------------*/
#include "sensor.h"

#include "usart_module.h"
#include "sys_time_module.h"

/*���岢�ҳ�ʼ���ṹ�� ��Ϊ�ṹ����ȫ�ֱ���  ����Ĭ�ϳ�ֵΪ0*/
#if (SENSOR_ID == SENSOR_W) 
struct WIND_SAMPLE  wind_sample;
static struct WIND_QC wind_qc;
#else
struct RAIN_SAMPLE rain_sample;
#endif

#if (SENSOR_ID == SENSOR_W) 

/******************************************************************************************
 *���ٵļ���
 ******************************************************************************************/
//����Ƶ�ʼ������ֵ
float CalculateWindSpeed(uint32_t frequence)
{
  if(frequence==0)
  {
    return 0.0;
  }
  
  return (wind_sample.ws_rate_k *(float)frequence * 4+ wind_sample.ws_rate_c + 0.3); 
}

/*���ٵĶ���*/
float CalculateCorrectWindSpeed(float windspeed)
{
	if(windspeed == 0.0)
  {
    return 0.0;
  }
	else
	{
		return (wind_sample.ws_a * windspeed * windspeed + wind_sample.ws_b * windspeed + wind_sample.ws_c);
	}
}

/******************************************************************************************
 *����ļ���
 ******************************************************************************************/
//������תΪʮ����
uint32_t GraytoDecimal(uint32_t x)
{
	unsigned long y=x;
	while(x>>=1)
	{
		y^=x;
	}
	return y;
}


//����������Ӧ��
static const uint16_t WD_TABLE[128]=
{
	0,3,6,8,11,14,17,20,22,25,28,31,34,37,39,42,45,48,51,53,56,59,62,65,68,70,73,76,79,82,84,87,
	90,93,96,98,101,104,107,110,112,115,118,121,124,127,129,132,135,138,141,143,146,149,152,155,158,160,163,166,169,172,174,177,
	180,183,186,188,191,194,197,200,202,205,208,211,214,217,219,222,225,228,231,233,236,239,242,245,248,250,253,256,259,262,264,267,
	270,273,276,278,281,284,287,290,292,295,298,301,304,307,309,312,315,318,321,323,326,329,332,335,338,340,343,346,349,352,354,357
};

//����7λ������������
uint16_t CalculateWindDirection(uint8_t gray)
{
		uint8_t temp=0;
					
		gray &= 0x7F;
		temp = GraytoDecimal(gray);  //������תΪʮ����        
		if(temp > 127)
		{
			temp = 127;
		}
		return WD_TABLE[temp];
}

/*����ʸ��ƽ��ֵ*/
#define PI 3.14159265f
uint16_t AverageVector(const uint16_t *DataBuffer,uint8_t Count)
{
		uint8_t i = 0;
		float aver_x = 0, aver_y = 0;  //x�����y����ķ���
		float aver = 0;
		float temp;
		
		
		if(Count == 0) 
			return DataBuffer[0];
		
		for(i = 0; i < Count; i++)
		{
				temp = (((float)DataBuffer[i]) * PI) / 180;  //�Ƕ�תΪ����
				aver_x += sin(temp);
				aver_y += cos(temp);
		}
		aver_x /= Count;
		aver_y /= Count;  							//x�����y�����ƽ������
		aver = atan2(aver_x, aver_y);  	//�õ�ƽ������ֵ
		aver = (aver*180) / PI;


		aver += 0.5;  //��������
		if(aver_x < 0)  //����������atan2()�������Ϊ[-�У���]����[-�У�0)֮���ֵ��2��ת����[�У�2��)�����Եõ�[0��2��)������Ҫ�������ֵ
		{
				aver += 360;
		}
		if(aver >= 360) 
		{
				aver=0;
		}

		return (int16_t)(aver);
}

/*********************************************************************
 * ����ƽ��ֵ����������ֵ
 */
/**
  * @brief  calculating average
  * @param  
  * @retval None
  */
uint16_t AverageAll(const uint16_t *DataBuffer,uint32_t Count)
{
  float sum_aver=0;
  int16_t average=0;
  uint32_t i=0;
  
  if(Count<2)
  {
    return DataBuffer[0];
  }
  
  for(i=0;i<Count;i++)
  {
    sum_aver+=DataBuffer[i];
  }
  
  
  sum_aver/=Count;
  
  /* round-off */
  if(sum_aver>=0)
  {
    average=(int16_t)(sum_aver+0.5);
  }
  else
  {
    average=(int16_t)(sum_aver-0.5);
  }

  
  return average;
}

/********************************************************************************************************************************
 *����纯��  ��������Ҫ�ķ�ı���  ÿ���Ӽ���һ��
 **********************************************************************************************************************************/
void WindCalculate(RTC_DateTypeDef *date ,RTC_TimeTypeDef *time)
{
	uint8_t i = 0;
	float ws_total = 0;
	/***************************************����˲ʱ���ٷ���*************************************************************/
	                        /*******************˲ʱ����*********************/
	wind_sample.wd_1s_g = 0;                                          /*��������һ�ζ�ȡ�ĸ�����*/
	wind_sample.wd_1s_g = GPIOC->IDR&0x7F;                            /*��ȡ������*/
	wind_sample.wd_1s   = CalculateWindDirection(wind_sample.wd_1s_g);/*�õ���������ֵ*/
	if(usart_debug)
	{
			printf("��������%d\r\n",wind_sample.wd_1s);
	}
	/*�������2���ӵ�120���������ֵ*/
	for(i = 119;i > 0; i--)
	{
			wind_sample.wd_1s_2m[i] = wind_sample.wd_1s_2m[i-1]; 
	}
	wind_sample.wd_1s_2m[0] =  wind_sample.wd_1s;
	if(wind_sample.wd_1s_2m_n < 120)
	{
			wind_sample.wd_1s_2m_n++;                                       /*����֮���������Ĵ���*/ 
	}
	                                    
	/*���ж�CPU�����Ƿ񳬹���3����*/
	if(wind_sample.wd_1s_2m_n >2)                                      /*����ʱ��ﵽ��3����*/
	{
			wind_sample.wd_3s = AverageVector(wind_sample.wd_1s_2m, 3);      /*�õ�˲ʱ����*/
	}
	else
	{
		wind_sample.wd_3s = wind_sample.wd_1s;                         /*��������3���ӣ�˲ʱ������Ƿ������ֵ*/
	}
	if(usart_debug)
	{
			printf("˲ʱ����%d\r\n",wind_sample.wd_3s);
	}
	                        /*******************˲ʱ����*********************/
	if(wind_sample.ws_s_n > 8)                                          /*����ʱ��ﵽ��3����*/
	{
			for(i = 0; i < wind_sample.ws_s_n; i++)
			{
					ws_total += wind_sample.ws_0_3s[i];                         /*����֮��0.25S��������֮��*/
			}
			wind_sample.ws_3s = ws_total / wind_sample.ws_s_n;              /*˲ʱ����*/
	}
	else                                                              /*CPU����ʱ�䲻��3����*/
	{
			wind_sample.ws_3s = wind_sample.ws_0;
	}
	/*��������*/
	wind_sample.ws_3s = wind_sample.ws_a * wind_sample.ws_3s * wind_sample.ws_3s + wind_sample.ws_b * wind_sample.ws_3s + wind_sample.ws_c;
	if(usart_debug)
	{
			printf("˲ʱ���٣�%f\r\n",wind_sample.ws_3s);
	}
	wind_sample.ws_1s = (uint16_t)((wind_sample.ws_3s + 0.05) * 10);   /*˲ʱ���ٵ�10��ȡ��*/
	for(i = 119; i > 0; i--)
	{
			wind_sample.ws_1s_m[i] = wind_sample.ws_1s_m[i-1];
	}
	wind_sample.ws_1s_m[0] = wind_sample.ws_1s;
	if(wind_sample.ws_1s >= (wind_sample.ws_3s_warn * 10))             /*��籨������*/
	{
			wind_sample.wind_warn_time = (time->Hours * 100) + (time->Minutes );/*�����ֵ�ʱ�䣺1234=12��34��*/
			wind_sample.wind_warn_flag = true;
			wind_sample.wind_warn_minutes = 0;											/*ֻҪ���ִ�籨���������¿�ʼ��ʱ*/
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET);     /*��PD2 = 1����������*/
	}
	if(usart_debug)
	{
			printf("��籨��ʱ��:wind_warn_time=%d\r\n", wind_sample.wind_warn_time);
	}
	if(wind_sample.ws_1s_m_n < 120)
	{
			wind_sample.ws_1s_m_n++;                                       /*����֮��˲ʱ���ٲ�������*/
	}
	/*�����*/
	if(wind_sample.ws_1s > wind_sample.ws_1s_max)
		{
				wind_sample.ws_1s_max   = wind_sample.ws_1s;             		 /*���˲ʱ����*/
				wind_sample.wd_1s_max   = wind_sample.wd_3s;                  /*���˲ʱ���ٶ�Ӧ��˲ʱ����*/
				wind_sample.ws_1s_max_t = time->Minutes *100 + time->Seconds ; /*���˲ʱ���ٳ��ֵ�ʱ��*/
		}
	if(usart_debug)
		{
				printf("���˲ʱ���٣�%d  ����%d  ʱ�䣺%d\r\n",wind_sample.ws_1s_max,wind_sample.wd_1s_max,wind_sample.ws_1s_max_t);
		}
		 /*ÿ���Ӽ���1�ַ�����١�2�ַ�����١�ʮ�ַ������*/  
		if(time->Seconds  == 0)                                        
		{
			                /*******************1�ַ���*********************/
			if((wind_sample.wd_1s_2m_n > 40) && (wind_sample.wd_1s_2m_n < 61))     /*����֮��ĵ�һ������1�ַ���Ĳ���ֵ��ȱ��*/
			{
					wind_sample.wd_1m = AverageVector(wind_sample.wd_1s_2m, wind_sample.wd_1s_2m_n);  /*1�ַ���*/
					wind_sample.wd_1m_h[0] = wind_sample.wd_1m;
					wind_sample.wd_1m_h_n++;                                             /*�����1�α�Ϊ1*/
			}
			else if(wind_sample.wd_1s_2m_n < 41)
			{
					if(usart_debug)
					{
							printf("1 minute wind direction error\r\n");
					}
			}
			else if(wind_sample.wd_1s_2m_n > 60)
			{
					wind_sample.wd_1m = AverageVector(wind_sample.wd_1s_2m, 60);     /*1�ַ���*/
					for(i = 59; i > 0; i--)
					{
							wind_sample.wd_1m_h[i] = wind_sample.wd_1m_h[i-1];
					}
					wind_sample.wd_1m_h[0] = wind_sample.wd_1m;
					if(wind_sample.wd_1m_h_n < 60)
					{
							wind_sample.wd_1m_h_n++;  
					}
			}
			if(usart_debug)
				{
						printf("1�ַ���%d\r\n",wind_sample.wd_1m);
				}
			            /*******************1�ַ���*********************/
			if((wind_sample.ws_1s_m_n > 40) && (wind_sample.ws_1s_m_n < 61))     /*����֮��ĵ�һ������1�ַ��ٵĲ���ֵ��ȱ��*/
			{
					wind_sample.ws_1m = AverageAll(wind_sample.ws_1s_m, wind_sample.ws_1s_m_n);  /*1�ַ���*/
					wind_sample.ws_1m_h[0] = wind_sample.ws_1m;
					wind_sample.ws_1m_h_n++;                                             /*�����1�α�Ϊ1*/
			}
			else if(wind_sample.ws_1s_m_n<41)
			{
					if(usart_debug)
					{
							printf("1 minute wind speed error\r\n");
					}
			}
			else if(wind_sample.ws_1s_m_n > 60)
			{
					wind_sample.ws_1m = AverageAll(wind_sample.ws_1s_m, 60);     /*1�ַ���*/
					for(i = 59; i > 0; i--)
					{
							wind_sample.ws_1m_h[i] = wind_sample.ws_1m_h[i-1];
					}
					wind_sample.ws_1m_h[0] = wind_sample.ws_1m;
					if(wind_sample.wd_1m_h_n < 60)
					{
							wind_sample.ws_1m_h_n++;  
					}
			}
			if(usart_debug)
				{
						printf("1�ַ��٣�%d\r\n",wind_sample.ws_1m);
				}
						                /*******************2�ַ���*********************/
			if(wind_sample.wd_1s_2m_n > 80)                                 /*����2����֮��2�ַ���Ĳ���ֵ��ȱ��*/
			{
					wind_sample.wd_2m = AverageVector(wind_sample.wd_1s_2m, wind_sample.wd_1s_2m_n);  /*2�ַ���*/
					for(i = 59; i > 0; i--)
					{
						wind_sample.wd_2m_h[i] = wind_sample.wd_2m_h[i-1];
					}
					wind_sample.wd_2m_h[0] = wind_sample.wd_2m;
					wind_qc.ws_m_qc = QC_R;
			}
			else if(wind_sample.wd_1s_2m_n < 81)
			{
					if(usart_debug)
					{
							printf("2 minute wind direction error\r\n");
					}
					wind_qc.wd_m_qc = QC_L;
					wind_sample.wd_2m = 0;
			}
			if(usart_debug)
				{
						printf("2�ַ���%d\r\n",wind_sample.wd_2m);
				}
			                         /*******************2�ַ���*********************/
			if(wind_sample.ws_1s_m_n > 80)                                 /*����2����֮��2�ַ��ٵĲ���ֵ��ȱ��*/
			{
					wind_sample.ws_2m = AverageAll(wind_sample.ws_1s_m, wind_sample.ws_1s_m_n);  /*2�ַ���*/
					for(i = 59; i > 0; i--)
					{
							wind_sample.wd_2m_h[i] = wind_sample.wd_2m_h[i-1];
					}
					wind_sample.ws_2m_h[0] = wind_sample.ws_2m;
			}
			else if(wind_sample.ws_1s_m_n < 81)
			{
					if(usart_debug)
					{
							printf("2 minute wind speed error\r\n");
					}
					wind_qc.ws_m_qc = QC_L;
					wind_sample.ws_2m = 0;
			}
			if(usart_debug)
				{
						printf("2�ַ��٣�%d\r\n",wind_sample.ws_2m);
				}
									                /*******************10�ַ���*********************/
			if(wind_sample.ws_1m_h_n == 9)                                   /*������9����Ч��1����ƽ������*/
			{
					wind_sample.wd_10m = AverageVector(wind_sample.wd_1m_h, 9);  /*10�ַ���*/
					wind_sample.wd_10m_h[0] = wind_sample.wd_10m;
					wind_qc.wd_10_qc = QC_R;
			}
			else if(wind_sample.ws_1m_h_n >9)
			{
					wind_sample.wd_10m = AverageVector(wind_sample.wd_1m_h, 10);  /*10�ַ���*/
					for(i = 59; i > 0; i--)
					{
						wind_sample.wd_10m_h[i] = wind_sample.wd_10m_h[i-1];
					}
					wind_sample.wd_10m_h[0] = wind_sample.wd_10m;
					wind_qc.wd_10_qc = QC_R;
			}
			else if(wind_sample.wd_1s_2m_n < 9)
			{
					if(usart_debug)
					{
							printf("10 minute wind direction error\r\n");
					}
					wind_qc.wd_10_qc = QC_L;
					wind_sample.wd_10m = 0;
			}
			if(usart_debug)
				{
						printf("10�ַ���%d\r\n",wind_sample.wd_10m);
				}
			                        /*******************10�ַ���*********************/
			if(wind_sample.ws_1m_h_n == 9)                                   /*������9����Ч��1����ƽ������*/
			{
					wind_sample.ws_10m = AverageAll(wind_sample.ws_1m_h, 9);  /*10�ַ���*/
					wind_sample.ws_10m_h[0] = wind_sample.ws_10m;
					wind_qc.ws_10_qc = QC_R;
			}
			else if(wind_sample.ws_1m_h_n > 9)
			{
				wind_sample.ws_10m = AverageVector(wind_sample.ws_1m_h, 10);  /*10�ַ���*/
				for(i = 59; i > 0; i--)
				{
					wind_sample.ws_10m_h[i] = wind_sample.ws_10m_h[i-1];
				}
				wind_sample.ws_10m_h[0] = wind_sample.ws_10m;
				wind_qc.ws_10_qc = QC_R;
			}
			else if(wind_sample.ws_1m_h_n < 9)
			{
					if(usart_debug)
					{
							printf("10 minute wind speed error\r\n");
					}
					wind_qc.ws_10_qc = QC_L;
					wind_sample.ws_10m = 0;
			}
			
			if(wind_sample.ws_10m > wind_sample.ws_10m_max )
			{
					wind_sample.ws_10m_max   =	wind_sample.ws_10m;                   /*ʮ����������*/
					wind_sample.wd_10m_max   =	wind_sample.wd_10m ;             			/*ʮ����������*/
					wind_sample.ws_10m_max_t =	time->Hours * 100 + time->Minutes ; 	/*ʮ���������ٳ��ֵ�ʱ��*/
			}
			if(usart_debug)
				{
						printf("10�ַ��٣�%d\r\n",wind_sample.ws_10m);
						printf("10�����磺ws_10m_max=%d	ws_10m_max=%d	ws_10m_max_t=%d\r\n", wind_sample.ws_10m_max, wind_sample.wd_10m_max, wind_sample.ws_10m_max_t);
				}
		}
}

/***************************************************************************************************************
 *��������ݵ����
 ****************************************************************************************************************/
uint8_t WindOutputMinDataFill(const struct tm *Datetime, struct wind_minute *data)
{
		uint8_t   count        = 0;
		
		count  =  snprintf(data->start_id, sizeof(data->start_id) + 1, "<F");
		count +=   snprintf(data->station_id, sizeof(data->station_id) + 1, "%05u",wind_sample.station);
		if(Datetime)
		{
			count  +=  snprintf(data->time_id, sizeof(data->time_id) + 1,"20%02u%02u%02u%02u%02u%02u",
													Datetime->tm_year ,Datetime->tm_mon ,Datetime->tm_mday ,Datetime->tm_hour ,Datetime->tm_min ,Datetime->tm_sec );
		}
		else
		{
			count  +=  snprintf(data->time_id, sizeof(data->time_id) + 1, "%04u%02u%02u%02u%02u%02u",2000,0,0,0,0,0);
		}
		count  +=  snprintf(data->wind.WD_3S   				,sizeof(data->wind.WD_3S) + 1			,"%04u",wind_sample.wd_3s  );
		count  +=  snprintf(data->wind.WS_3S      		,sizeof(data->wind.WS_3S) + 1     ,"%04u",wind_sample.ws_1s  );
		count  +=  snprintf(data->wind.WD_3S_M    		,sizeof(data->wind.WD_3S_M) + 1   ,"%04u",wind_sample.wd_1s_max  );
		count  +=  snprintf(data->wind.WS_3S_M    		,sizeof(data->wind.WS_3S_M) + 1   ,"%04u",wind_sample.ws_1s_max  );
		count  +=  snprintf(data->wind.WS_3S_M_T  		,sizeof(data->wind.WS_3S_M_T)+ 1  ,"%04u",wind_sample.ws_1s_max_t  );
		if(wind_qc.wd_m_qc == QC_R)
		{
				count  +=  snprintf(data->wind.WD_2M      ,sizeof(data->wind.WD_2M) + 1     ,"%04u",wind_sample.wd_2m  );
		}
		else if(wind_qc.wd_m_qc == QC_L)
		{
				count  +=  snprintf(data->wind.WD_2M      ,sizeof(data->wind.WD_2M) + 1     ,"0000");
		}
		if(wind_qc.ws_m_qc == QC_R)
		{
				count  +=  snprintf(data->wind.WS_2M      ,sizeof(data->wind.WS_2M) + 1     ,"%04u",wind_sample.ws_2m  );
		}
		else if(wind_qc.ws_m_qc == QC_L)
		{
				count  +=  snprintf(data->wind.WS_2M      ,sizeof(data->wind.WS_2M) + 1     ,"0000");
		}
		if(wind_qc.wd_10_qc == QC_R)
		{
				count  +=  snprintf(data->wind.WD_10M     ,sizeof(data->wind.WD_10M) + 1    ,"%04u",wind_sample.wd_10m  );
		}
		else if(wind_qc.wd_10_qc == QC_L)
		{
				count  +=  snprintf(data->wind.WD_10M      ,sizeof(data->wind.WD_10M) + 1     ,"0000");
		}
		if(wind_qc.ws_10_qc == QC_R)
		{
				count  +=  snprintf(data->wind.WS_10M     ,sizeof(data->wind.WS_10M) + 1	    ,"%04u",wind_sample.ws_10m  );
		}
		else if(wind_qc.ws_10_qc == QC_L)
		{
				count  +=  snprintf(data->wind.WS_10M      ,sizeof(data->wind.WS_10M) + 1 	  ,"0000");
		}
		
		count  +=  snprintf(data->wind.WD_10M_M   		,sizeof(data->wind.WD_10M_M) + 1 	,"%04u",wind_sample.wd_10m_max  );
		count  +=  snprintf(data->wind.WS_10M_M   		,sizeof(data->wind.WS_10M_M) + 1  ,"%04u",wind_sample.ws_10m_max  );
		count  +=  snprintf(data->wind.WS_10M_M_T			,sizeof(data->wind.WS_10M_M_T) + 1,"%04u",wind_sample.ws_10m_max_t  );
	
		count  +=  snprintf(data->end_id          		,sizeof(data->end_id) + 1,">\r\n");
		
		return count;
}



#endif
/*ʹ������������*/
#if (SENSOR_ID == SENSOR_R) 
/***************************************************************************************************************
 *����Сʱ���ݵ����
 ****************************************************************************************************************/
uint8_t rain_hour_fill(struct tm *time, struct rain_hour_data *rain)
{
		uint8_t   count    = 0;
		uint8_t   i				 = 0;
		
		count  =  snprintf(rain->start_flag ,sizeof(rain->start_flag) + 1, "<R%05u", rain_sample.station);
		count  +=  snprintf(rain->time_flag ,sizeof(rain->time_flag) + 1, "20%02u%02u%02u%02u%02u%02u",\
				time->tm_year, time->tm_mon, time->tm_mday, time->tm_hour, time->tm_min, time->tm_sec);
		
		for(i = 0; i < 59; i++)
		{
				count += snprintf(rain->data.RAIN_M + 2*i, 3, "%02u", rain_sample.rain_60m_1m[i + 1]);
		}
		count += snprintf(rain->data.RAIN_M + 2*59, 3, "%02u", rain_sample.rain_60m_1m[0]);
		
//		count += snprintf(rain->data.RAIN_M, sizeof(rain->data.RAIN_M) + 1, "%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u",\
//			rain_sample.rain_60m_1m[0], rain_sample.rain_60m_1m[1], rain_sample.rain_60m_1m[2], rain_sample.rain_60m_1m[3],\
//			rain_sample.rain_60m_1m[4], rain_sample.rain_60m_1m[5], rain_sample.rain_60m_1m[6], rain_sample.rain_60m_1m[7],\
//			rain_sample.rain_60m_1m[8], rain_sample.rain_60m_1m[9], rain_sample.rain_60m_1m[10], rain_sample.rain_60m_1m[11],\
//			rain_sample.rain_60m_1m[12], rain_sample.rain_60m_1m[13], rain_sample.rain_60m_1m[14], rain_sample.rain_60m_1m[15],\
//			rain_sample.rain_60m_1m[16], rain_sample.rain_60m_1m[17], rain_sample.rain_60m_1m[18], rain_sample.rain_60m_1m[19],\
//			rain_sample.rain_60m_1m[20], rain_sample.rain_60m_1m[21], rain_sample.rain_60m_1m[22], rain_sample.rain_60m_1m[23],\
//			rain_sample.rain_60m_1m[24], rain_sample.rain_60m_1m[25], rain_sample.rain_60m_1m[26], rain_sample.rain_60m_1m[27],\
//			rain_sample.rain_60m_1m[28], rain_sample.rain_60m_1m[29], rain_sample.rain_60m_1m[30], rain_sample.rain_60m_1m[31],\
//			rain_sample.rain_60m_1m[32], rain_sample.rain_60m_1m[33], rain_sample.rain_60m_1m[34], rain_sample.rain_60m_1m[35],\
//			rain_sample.rain_60m_1m[36], rain_sample.rain_60m_1m[37], rain_sample.rain_60m_1m[38], rain_sample.rain_60m_1m[39],\
//			rain_sample.rain_60m_1m[40], rain_sample.rain_60m_1m[41], rain_sample.rain_60m_1m[42], rain_sample.rain_60m_1m[43],\
//			rain_sample.rain_60m_1m[44], rain_sample.rain_60m_1m[45], rain_sample.rain_60m_1m[46], rain_sample.rain_60m_1m[47],\
//			rain_sample.rain_60m_1m[48], rain_sample.rain_60m_1m[49], rain_sample.rain_60m_1m[50], rain_sample.rain_60m_1m[51],\
//			rain_sample.rain_60m_1m[52], rain_sample.rain_60m_1m[53], rain_sample.rain_60m_1m[54], rain_sample.rain_60m_1m[55],\
//			rain_sample.rain_60m_1m[56], rain_sample.rain_60m_1m[57], rain_sample.rain_60m_1m[58], rain_sample.rain_60m_1m[59]);
		
		count  +=  snprintf(rain->data.RAIN_10MIN, 	sizeof(rain->data.RAIN_10MIN) + 1,	"%04u", rain_sample.rain_10m);
		count  +=  snprintf(rain->data.RAIN_H,			sizeof(rain->data.RAIN_H) + 1, 			"%04u", rain_sample.rain_60m);
		count  +=  snprintf(rain->data.RAIN_D,			sizeof(rain->data.RAIN_D) + 1, 			"%04u", rain_sample.rain_d);
		count  +=  snprintf(rain->data.RAIN_H_M,		sizeof(rain->data.RAIN_H_M) + 1, 		"%04u", rain_sample.rain_60m_max);
		count  +=  snprintf(rain->data.RAIN_H_M_T,	sizeof(rain->data.RAIN_H_M_T) + 1, 	"%04u", rain_sample.rain_60m_max_t);
			
		count  +=  snprintf(rain->end_flag ,sizeof(rain->end_flag) + 1, ">   %04u\r\n", rain_sample.rain_60m);
		
		return count;
}

#endif

