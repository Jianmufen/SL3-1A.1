/**
  ******************************************************************************
  * File Name          : time_related.c
  * Description        : This file provides code for some time related functions
  *                      
  ******************************************************************************
  *
  * COPYRIGHT(c) 2015 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "time_related.h"

#include "usart_module.h"
#include "time.h"
#include "usart.h"

/** @addtogroup STM32L1xx_HAL_Examples
  * @{
  */

/** @addtogroup Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/


/**
  * @brief  synchronize internal RTC with pcf8563 since the mcu doesn't have VBAT 
  * @param  None:
  * @retval HAL status
  */
HAL_StatusTypeDef sync_time(void)
{
  HAL_StatusTypeDef status=HAL_OK;
  RTC_TimeTypeDef time;
  RTC_DateTypeDef date;
  
  /* read date&time from pcf8563 */
  status = PCF8563_Read_Time();
	
  if(status != HAL_OK)
  {
		printf("read pcf8563 time error\r\n");
    return status;
  }
	else
	{
//		printf("PCF8563_Time:\"20%d-%d-%d %d:%d:%d\"\r\n", 
//									PCF_DataStruct_Time.RTC_Year,PCF_DataStruct_Time.RTC_Month,PCF_DataStruct_Time.RTC_Day,
//									PCF_DataStruct_Time.RTC_Hour,PCF_DataStruct_Time.RTC_Minute,PCF_DataStruct_Time.RTC_Second );
	}
  date.Year     =  PCF_DataStruct_Time.RTC_Year;
	date.Month    =  PCF_DataStruct_Time.RTC_Month;
	date.Date     =  PCF_DataStruct_Time.RTC_Day;
	time.Hours    =  PCF_DataStruct_Time.RTC_Hour;
	time.Minutes  =  PCF_DataStruct_Time.RTC_Minute;
	time.Seconds  =  PCF_DataStruct_Time.RTC_Second;
  /* check param */
  if(IS_RTC_YEAR(date.Year) && IS_RTC_MONTH(date.Month) && IS_RTC_DATE(date.Date) &&
     IS_RTC_HOUR24(time.Hours) && IS_RTC_MINUTES(time.Minutes) && IS_RTC_SECONDS(time.Seconds))
  {
    /* fill unused param */
    date.WeekDay=0x01;
    time.DayLightSaving=RTC_DAYLIGHTSAVING_NONE;
    time.StoreOperation=RTC_STOREOPERATION_RESET;
    time.SubSeconds=0;
    time.TimeFormat=RTC_HOURFORMAT12_AM;
    
    /* set RTC data&time */
    status=HAL_RTC_SetDate(&hrtc,&date,FORMAT_BIN);
    if(status!=HAL_OK)
    {
      return status;
    }
    status=HAL_RTC_SetTime(&hrtc,&time,FORMAT_BIN);
    if(status!=HAL_OK)
    {
      return status;
    }
  }
  else
  {
    return HAL_ERROR;
  }
   /* ��ȡ��ǰʱ�� */
  HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
  /* ��ȡ��ǰ���� */
  HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
  
//  printf("RTC_Time: %02d-%02d-%02d %02d:%02d:%02d\r\n ", 2000+date.Year,date.Month, date.Date,time.Hours, time.Minutes, time.Seconds);
	
  return HAL_OK;
}



/**
  * @}
  */

/**
  * @}
  */



/* Converts Gregorian date to seconds since 1970-01-01 00:00:00.
* Assumes input in normal date format, i.e. 1980-12-31 23:59:59
* => year=1980, mon=12, day=31, hour=23, min=59, sec=59.
*
* [For the Julian calendar (which was used in Russia before 1917,
* Britain & colonies before 1752, anywhere else before 1582,
* and is still in use by some communities) leave out the
* -year/100+year/400 terms, and add 10.]
*
* This algorithm was first published by Gauss (I think).
*
* WARNING: this function will overflow on 2106-02-07 06:28:16 on
* machines where long is 32-bit! (However, as time_t is signed, we
* will already get problems at other places on 2038-01-19 03:14:08)
*/
//�����1970��1��1��0ʱ������ʱ�侭��������
unsigned long
l_mktime(const unsigned int year0, const unsigned int mon0,
const unsigned int day, const unsigned int hour,
const unsigned int min, const unsigned int sec)
{
			unsigned long mon = mon0, year = year0;

			/* 1..12 -> 11,12,1..10 */
			if (0 >= (long)(mon -= 2)) {
				mon += 12;	/* Puts Feb last since it has leap day */
				year -= 1;
			}

			return ((((unsigned long)
				(year / 4 - year / 100 + year / 400 + 367 * mon / 12 + day) +
				year * 365 - 719499
				) * 24 + hour /* now have hours */
				) * 60 + min /* now have minutes */
				) * 60 + sec; /* finally seconds */
}



/*	���������ж�����
 *	�㷨���£�
 *	��ķ����ɭ���㹫ʽ
 *	W= (d+2*m+3*(m+1)/5+y+y/4-y/100+y/400) mod 7
 *	�ڹ�ʽ��d��ʾ�����е�������m��ʾ�·�����y��ʾ������
 *	ע�⣺�ڹ�ʽ���и���������ʽ��ͬ�ĵط���
 *	��һ�ºͶ��¿�������һ���ʮ���º�ʮ���£����������2004-1-10����ɣ�2003-13-10�����빫ʽ���㡣
 *	
 *	����ֵ��0-6�ֱ��Ӧ����һ~������
 */
int CalculateWeekDay(int year, int month, int day)
{
	int weekday = 0;

	if ((month == 1) || (month == 2))
	{
		month += 12;
		year -= 1;
	}
	weekday = (day + 2 * month + 3 * (month + 1) / 5 + year + year / 4 - year / 100 + year / 400) % 7;

	return weekday;
}



//�ж�һ������Ƿ�Ϊ���꣬�Ǿͷ���1�����Ǿͷ���0
inline int isLeapYear(int year)
{
	return((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0));
}

const unsigned short DayOfMon[][13]=
{
	/* Normal years.  */
	{ 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
	/* Leap years.  */
	{ 0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

//��һ������ʱ������1����
void AddaMinute(int *year, int *month, int *day, int *hour, int *minute, int *second)
{
	char leapYear = isLeapYear(*year);  //�Ƿ�����
	*minute += 1;  //����1����
	if (*minute > 59)
	{
		*minute = 0;
		*hour += 1;
		if (*hour > 23)
		{
			*hour = 0;
			*day += 1;
			if (*day > DayOfMon[leapYear][*month])  //���ڵ�������������
			{
				*day = 1;
				*month += 1;
				if (*month > 12)  //����
				{
					*month = 1;
					*year += 1;
				}
			}
		}
	}
}

//��һ������ʱ������5����
void Add5Minute(int *year, int *month, int *day, int *hour, int *minute, int *second)
{
	for (int i = 0; i < 5; i++)
	{
		AddaMinute(year, month, day, hour, minute, second);
	}
}

//��һ������ʱ������1Сʱ
void AddaHour(int *year, int *month, int *day, int *hour, int *minute, int *second)
{
	char leapYear = isLeapYear(*year);  //�Ƿ�����
	*hour += 1;  //����1Сʱ
	if (*hour > 23)
	{
		*hour = 0;
		*day += 1;
		if (*day > DayOfMon[leapYear][*month])  //���ڵ�������������
		{
			*day = 1;
			*month += 1;
			if (*month > 12)  //����
			{
				*month = 1;
				*year += 1;
			}
		}
	}
}

//��һ������ʱ�����1Сʱ
void SubaHour(int *year, int *month, int *day, int *hour, int *minute, int *second)
{
	char leapYear;  //�Ƿ�����
	if (*hour > 0)  //����0��
		*hour -= 1;  //��С1Сʱ���������
	else if (*hour == 0)  //0��
	{
		*hour = 23;  //��С1Сʱ����
		*day -= 1;
		if (*day < 1)  //����
		{
			*month -= 1;
			if (*month < 1)  //����
			{
				*month = 12;
				*year -= 1;
			}
			leapYear = isLeapYear(*year);  //�Ƿ�����
			*day = DayOfMon[leapYear][*month];  //ȷ���·ݺ����ٵõ�����
		}
	}
}


//��һ������ʱ������1��
void AddaDay(int *year, int *month, int *day, int *hour, int *minute, int *second)
{
	char leapYear = isLeapYear(*year);  //�Ƿ�����  1������  0��ƽ��
	*day += 1;  //����1��
	if (*day > DayOfMon[leapYear][*month])  //���ڵ�������������
	{
			*day    = 1;
			*month += 1;
			if (*month > 12)  //����
			{
				*month = 1;
				*year += 1;
			}
	}
}

//��һ������ʱ�����1��
void SubaDay(int *year, int *month, int *day, int *hour, int *minute, int *second)
{
	char leapYear;  //�Ƿ�����
	if (*day > 1)  //������
		*day -= 1;  //��С1�죬�������
	else if (*day == 1)  //����
	{
			*month -= 1;  /*��һ���1����*/
			if (*month < 1)  //����
			{
				*month = 12;
				*year -= 1;
			}
			leapYear = isLeapYear(*year);  //�Ƿ�����
			*day = DayOfMon[leapYear][*month];  //ȷ���·ݺ����ٵõ�����
	}
}
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
