#ifndef __SENSOR_H
#define __SENSOR_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32l1xx_hal.h"
#include "main.h"	 
#include "time.h"		 
	 
/*传感器选择：风、雨*/	
#define SENSOR_W   1					/*风*/
#define SENSOR_R   2					/*雨*/

/*传感器ID 配置*/
//#define SENSOR_ID SENSOR_W
#define SENSOR_ID SENSOR_R
/**/

#define QC_R 0  //数据正确
#define QC_L 1  //数据缺测	 
//#define TEMP_CORRECTION_VALUE    (0)  /*PT100的温度订正值*/
/* length of array */
#define LENGTH_OF(x)   (sizeof(x)/sizeof(*(x)))
	

/*风分钟数据*/
#if (SENSOR_ID == SENSOR_W) 
struct wind_data;
struct wind_minute;
struct WIND_QC;
extern struct WIND_SAMPLE wind_sample;
/*风分钟数据格式*/	 
struct wind_data
{
	char WD_3S[4];                /*3S瞬时平均风向 0111=111°"*/
	char WS_3S[4];                /*3S瞬时平均风速  0013=1.3m/s*/
	char WD_3S_M[4];              /*3S极大风向 */
	char WS_3S_M[4];              /*3S极大风速*/
	char WS_3S_M_T[4];            /*3S极大风速出现的时间  分秒*/
	char WD_2M[4];                /*2M平均风向*/
	char WS_2M[4];                /*2M平均风速*/
	char WD_10M[4];               /*10M平均风向*/
	char WS_10M[4];               /*10M平均风速*/
	char WD_10M_M[4];             /*10M极大风向*/
	char WS_10M_M[4];             /*10M极大风速*/
	char WS_10M_M_T[4];           /*10M极大风速出现的时间*/
};

struct wind_minute 			/*风的分钟数据帧*/
{
	char start_id[2];                /*开始符 "<F"*/
	char station_id[5];              /*站号 00001*/
	char time_id[14];                /*时间 20171228094600=2017年12月28日9点46分钟00秒*/
	struct wind_data wind;            /*风分钟数据主体*/
	char end_id[3];                  /*结束符“>\r\n”*/
};

/*风的采样值*/
struct WIND_SAMPLE
{
	/*风速采样  频率为1秒采样4次  即0.25秒采样一次*/
	unsigned int ws_rate;             /*0.25秒内风速采样的频率*/
	uint16_t ws_rate_3s[12];      /*3S内12个风速采样频率*/
	float    ws_0;                /*采样风速*/
	float    ws_0_3s[12];         /*12个采样风速*/
	uint8_t  ws_s_n;              /*开机之后风速采样的次数 大于等于12之后值永远等于12*/
	float    ws_3s;               /*瞬时风速*/
	float    ws_3s_f[60];         /*1分钟内60个瞬时风速*/
	uint16_t ws_1s;               /*瞬时风速的10倍*/  
	uint16_t ws_1s_max;           /*瞬时风速10倍的最大值*/
	uint16_t ws_1s_max_t;         /*瞬时风速10倍的最大值出现的时间  1234=12分34秒*/
	uint16_t ws_1s_m[120];        /*最近2分钟内120个瞬时风速的10倍*/
	uint8_t  ws_1s_m_n;           /*开机之后瞬时风速采样次数*/
	uint16_t ws_1m;               /*1分钟的平均风速*/
	uint16_t ws_1m_h[60];         /*最近60分钟内的60个1分钟平均风速*/
	uint8_t  ws_1m_h_n;           /*开机之后1分钟风速采样的次数  大于等于60之后 永远为60*/
  uint16_t ws_2m;               /*2分钟的平均风速*/
	uint16_t ws_2m_h[60];         /*最近60个2分钟的平均风速*/
	uint16_t ws_10m;              /*10分钟的平均风速*/
	uint16_t ws_10m_h[60];        /*最近60个10分钟的平均风速*/
	uint16_t ws_10m_max;          /*1小时内60个10分钟平均风速的最大值*/
	uint16_t ws_10m_max_t;        /*10分钟平均风速最大值出现的时间 1234=12点34分钟*/
	
	/*根据采样频率计算风速的系数k及常数项  采样风速 = ws_rate_k*采样频率+ws_rate_c   */
	double ws_rate_k;              /*风速的频率系数*/
	double ws_rate_c;              /*风速的频率常数项*/
	
	/*风速订正值  订正后的风速 = ws_a*采样风速的平方 + ws_b*采样风速 + ws_c    */
	double ws_a;                   /*风速订正函数的二次项*/
	double ws_b;                   /*风速订正函数的一次项*/
	double ws_c;                   /*风速订正函数的常数项*/
	
	/*风向采样  频率为1秒采样一次 */
	uint16_t wd_1s_g;             /*格雷码采样值*/
	uint16_t wd_1s;               /*风向采样值*/
//	uint16_t wd_3s[3];          /*最近3秒的风向值*/
	uint16_t wd_3s;               /*瞬时风向*/
	uint16_t wd_1s_max;           /*最大的瞬时风向*/
	uint16_t wd_1s_2m[120];       /*最近2分钟内120个风向采样值  可以计算1、2分钟矢量平均风向*/
	uint8_t  wd_1s_2m_n;          /*开机之后风向采样的次数  大于等于120后永远等120了*/
	uint16_t wd_1m;               /*1分钟风向*/
	uint16_t wd_1m_h[60];         /*60分钟内60个1分钟风向*/
	uint8_t  wd_1m_h_n;           /*开机之后1分钟风向采样的次数  大于等于60之后 永远等60了*/
	uint16_t wd_2m;               /*2分钟风向*/
	uint16_t wd_2m_h[60];        /*最近60个2分钟风向*/
	uint16_t wd_10m;              /*10分钟风向*/
	uint16_t wd_10m_h[60];        /*最近60个10分钟风向*/
	uint16_t wd_10m_max;          /*10分钟风速最大值对应的10分钟风向*/
	
	/*设备状态*/
	uint32_t station;             /*设备的台站号*/
	uint16_t  ws_3s_warn;          /*大风报警值*/
//	uint8_t  wind_speed_warn;     /*大风报警阈值*/
	uint8_t  wind_warn_flag;      /*大风出现的标志：false为未出现大风，true为出现大风*/
	uint32_t wind_warn_time;      /*大风出现的时间：时分 1234=12点34分*/
	uint8_t wind_warn_minutes;		/*大风报警出现后，分钟计数*/
	uint8_t  open_time;           /*CPU开始上电之后秒钟计数*/
	uint8_t  debug_wd_1;          /*风向采样的格雷码及采样风向*/
	uint8_t  debug_wd_2;          /*风向的平均值*/
	uint8_t  debug_ws_1;          /*风速的采样频率及采样风速*/
	uint8_t  debug_ws_2;          /*风速的平均值*/
	uint8_t  debug_ws_3;          /*风速的订正值*/
	uint8_t  debug_ws_4;          /*风速的采样频率系数*/
};


/*质控码结构体*/
struct WIND_QC
{
	uint8_t wd_s_qc;              /*瞬时风向质控码*/
	uint8_t ws_s_qc;              /*瞬时风速质控码*/
	uint8_t wd_m_qc;              /*2分风向质控码*/
	uint8_t ws_m_qc;              /*2分风速质控码*/
	uint8_t wd_10_qc;              /*十分风向质控码*/
	uint8_t ws_10_qc;              /*十分风速质控码*/
	uint8_t wd_10_max_qc;              /*十分最大风向质控码*/
	uint8_t ws_10_max_qc;              /*十分最大风速质控码*/
};
#endif
/*雨分钟数据*/
#if (SENSOR_ID == SENSOR_R) 
/*雨量结构体声明*/
struct rain_data;
struct rain_hour_data;
extern struct RAIN_SAMPLE rain_sample;

/*雨小时数据格式*/	 
struct rain_data
{
	char RAIN_M[120];          /*1小时内的60个分钟雨量  01分-00分*/
	char RAIN_10MIN[4];        /*10分钟雨量"*/
	char RAIN_H[4];           /*连续60分钟雨量累计"*/
	char RAIN_D[4];           /*日雨量"*/
	char RAIN_H_M[4];         /*连续60分钟最大雨量"*/
	char RAIN_H_M_T[4];       /*连续60分钟最大雨量出现的时间*/
};

struct rain_hour_data                /*雨的小时钟数据帧*/
{
	char start_flag[7];             /*开始符 "<R00001"*/      /*站号 00001*/
	char time_flag[14];             /*时间 20171228094600=2017年12月28日9点46分钟00秒*/
	struct rain_data data;   		   /*雨分钟数据主体*/
	char end_flag[10];               /*结束符“>   0000\r\n”*/
};

/*雨的采样值*/
struct RAIN_SAMPLE
{
	/*雨量采样*/
	uint8_t  rain_1m;            	  /*1分钟的雨量*/
	uint8_t  rain_60m_1m[60];    		 /*1小时内的60个分钟雨量*/
	uint16_t rain_10m;           		 /*10分钟雨量累计*/
	uint16_t rain_60m;            /*连续60分钟雨量累计*/
	uint16_t rain_60m_max;        /*连续60分钟雨量累计最大*/
	uint16_t rain_60m_max_t;      /*连续60分钟雨量累计最大出现的时间*/
	uint16_t rain_60m_warn_flag;  /*出现大于连续60分钟雨量报警阈值的标志 false未出现，true出现*/
	uint16_t rain_60m_warn;       /*连续60分钟雨量报警阈值*/
	uint16_t rain_60m_warn_t;     /*连续60分钟雨量报警阈值出现的时间*/
//	uint16_t rain_h;              /*小时雨量累计 从01分到00分结束*/
//	uint16_t rain_h_warn;         /*小时雨量小时雨量报警阈值*/
//	uint16_t rain_h_warn_t;       /*小时雨量小时雨量报警阈值出现的时间*/
//	uint16_t rain_h_warn_flag;    /*小时雨量小时雨量报警阈值出现的标志*/
////	uint16_t rain_h_d[24];        /*最近24个小时雨量*/
//	uint16_t rain_h_max;          /*一天内小时雨量最大值*/
//	uint16_t rain_h_max_t;        /*小时雨量最大值出现的时间 1211=12号11点*/
	uint16_t rain_d_warn_flag;    /*日雨量大于日雨量报警阈值的标志false未出现，true出现*/
	uint16_t rain_d_warn;         /*日雨量报警阈值*/
	uint16_t rain_d_warn_t;       /*日雨量报警阈值出现的时间*/
	uint16_t rain_d;              /*日雨量*/
//	uint16_t rain_d_m[31];        /*最近31个日雨量*/
	
	/*设备状态*/
	uint32_t station;             /*设备台站号*/
};





#endif
	 



/*函数声明*/
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
