#ifndef __SENSOR_H
#define __SENSOR_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32l1xx_hal.h"
#include "main.h"	 
#include "time.h"		 
	 
/*������ѡ�񣺷硢��*/	
#define SENSOR_W   1					/*��*/
#define SENSOR_R   2					/*��*/

/*������ID ����*/
//#define SENSOR_ID SENSOR_W
#define SENSOR_ID SENSOR_R
/**/

#define QC_R 0  //������ȷ
#define QC_L 1  //����ȱ��	 
//#define TEMP_CORRECTION_VALUE    (0)  /*PT100���¶ȶ���ֵ*/
/* length of array */
#define LENGTH_OF(x)   (sizeof(x)/sizeof(*(x)))
	

/*���������*/
#if (SENSOR_ID == SENSOR_W) 
struct wind_data;
struct wind_minute;
struct WIND_QC;
extern struct WIND_SAMPLE wind_sample;
/*��������ݸ�ʽ*/	 
struct wind_data
{
	char WD_3S[4];                /*3S˲ʱƽ������ 0111=111��"*/
	char WS_3S[4];                /*3S˲ʱƽ������  0013=1.3m/s*/
	char WD_3S_M[4];              /*3S������� */
	char WS_3S_M[4];              /*3S�������*/
	char WS_3S_M_T[4];            /*3S������ٳ��ֵ�ʱ��  ����*/
	char WD_2M[4];                /*2Mƽ������*/
	char WS_2M[4];                /*2Mƽ������*/
	char WD_10M[4];               /*10Mƽ������*/
	char WS_10M[4];               /*10Mƽ������*/
	char WD_10M_M[4];             /*10M�������*/
	char WS_10M_M[4];             /*10M�������*/
	char WS_10M_M_T[4];           /*10M������ٳ��ֵ�ʱ��*/
};

struct wind_minute 			/*��ķ�������֡*/
{
	char start_id[2];                /*��ʼ�� "<F"*/
	char station_id[5];              /*վ�� 00001*/
	char time_id[14];                /*ʱ�� 20171228094600=2017��12��28��9��46����00��*/
	struct wind_data wind;            /*�������������*/
	char end_id[3];                  /*��������>\r\n��*/
};

/*��Ĳ���ֵ*/
struct WIND_SAMPLE
{
	/*���ٲ���  Ƶ��Ϊ1�����4��  ��0.25�����һ��*/
	unsigned int ws_rate;             /*0.25���ڷ��ٲ�����Ƶ��*/
	uint16_t ws_rate_3s[12];      /*3S��12�����ٲ���Ƶ��*/
	float    ws_0;                /*��������*/
	float    ws_0_3s[12];         /*12����������*/
	uint8_t  ws_s_n;              /*����֮����ٲ����Ĵ��� ���ڵ���12֮��ֵ��Զ����12*/
	float    ws_3s;               /*˲ʱ����*/
	float    ws_3s_f[60];         /*1������60��˲ʱ����*/
	uint16_t ws_1s;               /*˲ʱ���ٵ�10��*/  
	uint16_t ws_1s_max;           /*˲ʱ����10�������ֵ*/
	uint16_t ws_1s_max_t;         /*˲ʱ����10�������ֵ���ֵ�ʱ��  1234=12��34��*/
	uint16_t ws_1s_m[120];        /*���2������120��˲ʱ���ٵ�10��*/
	uint8_t  ws_1s_m_n;           /*����֮��˲ʱ���ٲ�������*/
	uint16_t ws_1m;               /*1���ӵ�ƽ������*/
	uint16_t ws_1m_h[60];         /*���60�����ڵ�60��1����ƽ������*/
	uint8_t  ws_1m_h_n;           /*����֮��1���ӷ��ٲ����Ĵ���  ���ڵ���60֮�� ��ԶΪ60*/
  uint16_t ws_2m;               /*2���ӵ�ƽ������*/
	uint16_t ws_2m_h[60];         /*���60��2���ӵ�ƽ������*/
	uint16_t ws_10m;              /*10���ӵ�ƽ������*/
	uint16_t ws_10m_h[60];        /*���60��10���ӵ�ƽ������*/
	uint16_t ws_10m_max;          /*1Сʱ��60��10����ƽ�����ٵ����ֵ*/
	uint16_t ws_10m_max_t;        /*10����ƽ���������ֵ���ֵ�ʱ�� 1234=12��34����*/
	
	/*���ݲ���Ƶ�ʼ�����ٵ�ϵ��k��������  �������� = ws_rate_k*����Ƶ��+ws_rate_c   */
	double ws_rate_k;              /*���ٵ�Ƶ��ϵ��*/
	double ws_rate_c;              /*���ٵ�Ƶ�ʳ�����*/
	
	/*���ٶ���ֵ  ������ķ��� = ws_a*�������ٵ�ƽ�� + ws_b*�������� + ws_c    */
	double ws_a;                   /*���ٶ��������Ķ�����*/
	double ws_b;                   /*���ٶ���������һ����*/
	double ws_c;                   /*���ٶ��������ĳ�����*/
	
	/*�������  Ƶ��Ϊ1�����һ�� */
	uint16_t wd_1s_g;             /*���������ֵ*/
	uint16_t wd_1s;               /*�������ֵ*/
//	uint16_t wd_3s[3];          /*���3��ķ���ֵ*/
	uint16_t wd_3s;               /*˲ʱ����*/
	uint16_t wd_1s_max;           /*����˲ʱ����*/
	uint16_t wd_1s_2m[120];       /*���2������120���������ֵ  ���Լ���1��2����ʸ��ƽ������*/
	uint8_t  wd_1s_2m_n;          /*����֮���������Ĵ���  ���ڵ���120����Զ��120��*/
	uint16_t wd_1m;               /*1���ӷ���*/
	uint16_t wd_1m_h[60];         /*60������60��1���ӷ���*/
	uint8_t  wd_1m_h_n;           /*����֮��1���ӷ�������Ĵ���  ���ڵ���60֮�� ��Զ��60��*/
	uint16_t wd_2m;               /*2���ӷ���*/
	uint16_t wd_2m_h[60];        /*���60��2���ӷ���*/
	uint16_t wd_10m;              /*10���ӷ���*/
	uint16_t wd_10m_h[60];        /*���60��10���ӷ���*/
	uint16_t wd_10m_max;          /*10���ӷ������ֵ��Ӧ��10���ӷ���*/
	
	/*�豸״̬*/
	uint32_t station;             /*�豸��̨վ��*/
	uint16_t  ws_3s_warn;          /*��籨��ֵ*/
//	uint8_t  wind_speed_warn;     /*��籨����ֵ*/
	uint8_t  wind_warn_flag;      /*�����ֵı�־��falseΪδ���ִ�磬trueΪ���ִ��*/
	uint32_t wind_warn_time;      /*�����ֵ�ʱ�䣺ʱ�� 1234=12��34��*/
	uint8_t wind_warn_minutes;		/*��籨�����ֺ󣬷��Ӽ���*/
	uint8_t  open_time;           /*CPU��ʼ�ϵ�֮�����Ӽ���*/
	uint8_t  debug_wd_1;          /*��������ĸ����뼰��������*/
	uint8_t  debug_wd_2;          /*�����ƽ��ֵ*/
	uint8_t  debug_ws_1;          /*���ٵĲ���Ƶ�ʼ���������*/
	uint8_t  debug_ws_2;          /*���ٵ�ƽ��ֵ*/
	uint8_t  debug_ws_3;          /*���ٵĶ���ֵ*/
	uint8_t  debug_ws_4;          /*���ٵĲ���Ƶ��ϵ��*/
};


/*�ʿ���ṹ��*/
struct WIND_QC
{
	uint8_t wd_s_qc;              /*˲ʱ�����ʿ���*/
	uint8_t ws_s_qc;              /*˲ʱ�����ʿ���*/
	uint8_t wd_m_qc;              /*2�ַ����ʿ���*/
	uint8_t ws_m_qc;              /*2�ַ����ʿ���*/
	uint8_t wd_10_qc;              /*ʮ�ַ����ʿ���*/
	uint8_t ws_10_qc;              /*ʮ�ַ����ʿ���*/
	uint8_t wd_10_max_qc;              /*ʮ���������ʿ���*/
	uint8_t ws_10_max_qc;              /*ʮ���������ʿ���*/
};
#endif
/*���������*/
#if (SENSOR_ID == SENSOR_R) 
/*�����ṹ������*/
struct rain_data;
struct rain_hour_data;
extern struct RAIN_SAMPLE rain_sample;

/*��Сʱ���ݸ�ʽ*/	 
struct rain_data
{
	char RAIN_M[120];          /*1Сʱ�ڵ�60����������  01��-00��*/
	char RAIN_10MIN[4];        /*10��������"*/
	char RAIN_H[4];           /*����60���������ۼ�"*/
	char RAIN_D[4];           /*������"*/
	char RAIN_H_M[4];         /*����60�����������"*/
	char RAIN_H_M_T[4];       /*����60��������������ֵ�ʱ��*/
};

struct rain_hour_data                /*���Сʱ������֡*/
{
	char start_flag[7];             /*��ʼ�� "<R00001"*/      /*վ�� 00001*/
	char time_flag[14];             /*ʱ�� 20171228094600=2017��12��28��9��46����00��*/
	struct rain_data data;   		   /*�������������*/
	char end_flag[10];               /*��������>   0000\r\n��*/
};

/*��Ĳ���ֵ*/
struct RAIN_SAMPLE
{
	/*��������*/
	uint8_t  rain_1m;            	  /*1���ӵ�����*/
	uint8_t  rain_60m_1m[60];    		 /*1Сʱ�ڵ�60����������*/
	uint16_t rain_10m;           		 /*10���������ۼ�*/
	uint16_t rain_60m;            /*����60���������ۼ�*/
	uint16_t rain_60m_max;        /*����60���������ۼ����*/
	uint16_t rain_60m_max_t;      /*����60���������ۼ������ֵ�ʱ��*/
	uint16_t rain_60m_warn_flag;  /*���ִ�������60��������������ֵ�ı�־ falseδ���֣�true����*/
	uint16_t rain_60m_warn;       /*����60��������������ֵ*/
	uint16_t rain_60m_warn_t;     /*����60��������������ֵ���ֵ�ʱ��*/
//	uint16_t rain_h;              /*Сʱ�����ۼ� ��01�ֵ�00�ֽ���*/
//	uint16_t rain_h_warn;         /*Сʱ����Сʱ����������ֵ*/
//	uint16_t rain_h_warn_t;       /*Сʱ����Сʱ����������ֵ���ֵ�ʱ��*/
//	uint16_t rain_h_warn_flag;    /*Сʱ����Сʱ����������ֵ���ֵı�־*/
////	uint16_t rain_h_d[24];        /*���24��Сʱ����*/
//	uint16_t rain_h_max;          /*һ����Сʱ�������ֵ*/
//	uint16_t rain_h_max_t;        /*Сʱ�������ֵ���ֵ�ʱ�� 1211=12��11��*/
	uint16_t rain_d_warn_flag;    /*����������������������ֵ�ı�־falseδ���֣�true����*/
	uint16_t rain_d_warn;         /*������������ֵ*/
	uint16_t rain_d_warn_t;       /*������������ֵ���ֵ�ʱ��*/
	uint16_t rain_d;              /*������*/
//	uint16_t rain_d_m[31];        /*���31��������*/
	
	/*�豸״̬*/
	uint32_t station;             /*�豸̨վ��*/
};





#endif
	 



/*��������*/
#if (SENSOR_ID == SENSOR_W) 
float     CalculateWindSpeed(uint32_t frequence);
float     CalculateCorrectWindSpeed(float windspeed);
uint32_t  GraytoDecimal(uint32_t x);
uint16_t  CalculateWindDirection(uint8_t gray);
uint16_t  AverageVector(const uint16_t *DataBuffer,uint8_t Count);
uint8_t WindOutputMinDataFill(const struct tm *Datetime, struct wind_minute *data);
uint16_t  AverageAll(const uint16_t *DataBuffer,uint32_t Count);
void      WindCalculate(RTC_DateTypeDef *datew ,RTC_TimeTypeDef *timew);
#else
uint8_t rain_hour_fill(struct tm *time, struct rain_hour_data *rain);
#endif

#ifdef __cplusplus
}
#endif
#endif
