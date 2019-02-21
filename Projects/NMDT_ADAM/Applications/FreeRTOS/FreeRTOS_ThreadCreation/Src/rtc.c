/* Includes ------------------------------------------------------------------*/
#include "rtc.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
 RTC_HandleTypeDef hrtc;

/* RTC init function */
void MX_RTC_Init(void)
{
  RTC_AlarmTypeDef sAlarm;

  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = RTC_ASYNCH_PREDIV;
  hrtc.Init.SynchPrediv = RTC_SYNCH_PREDIV;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  HAL_RTC_Init(&hrtc);

/* Configure the RTC WakeUp Timer to generate a 1s interrupt
    */
 //HAL_RTCEx_SetWakeUpTimer_IT(&hrtc,0,RTC_WAKEUPCLOCK_CK_SPRE_16BITS);   //�ᵼ�³�������������  �о���

    /**Enable the Alarm A 
    */
	/* Set Alarm to 00:00:00 
     RTC Alarm Generation: Alarm on Hours, Minutes and Seconds */
  sAlarm.Alarm = RTC_ALARM_A;
	sAlarm.AlarmDateWeekDay = RTC_WEEKDAY_MONDAY;
	sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_WEEKDAY;
	//sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY;   //��1���ʱ��ֻ����һ���жϣ�
	sAlarm.AlarmMask =  RTC_ALARMMASK_DATEWEEKDAY | RTC_ALARMMASK_HOURS | RTC_ALARMMASK_MINUTES | RTC_ALARMMASK_SECONDS;   //�Ժ��ÿһ���Ӳ���һ���ж�
	sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_NONE;
	sAlarm.AlarmTime.TimeFormat = RTC_HOURFORMAT12_AM;
  sAlarm.AlarmTime.Hours = 0;
  sAlarm.AlarmTime.Minutes = 0;
  sAlarm.AlarmTime.Seconds = 0;
	sAlarm.AlarmTime.SubSeconds = 0;
  HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN);
	
    /**Enable the Alarm B 
    */
//  sAlarm.Alarm = RTC_ALARM_B;
//  sAlarm.AlarmDateWeekDay = RTC_WEEKDAY_MONDAY;
//  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_WEEKDAY;
//  sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY | RTC_ALARMMASK_HOURS | RTC_ALARMMASK_MINUTES;    //�Ժ��ÿ��һ���Ӳ��ܲ���һ���ж�
//  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_NONE;
//  sAlarm.AlarmTime.TimeFormat = RTC_HOURFORMAT12_AM;
//  sAlarm.AlarmTime.Hours = 10;
//  sAlarm.AlarmTime.Minutes = 03;//��Ч �о�
//  sAlarm.AlarmTime.Seconds = 30;//������   RTC_ALARM_B�ڴ����Ӳ����ж�
//  sAlarm.AlarmTime.SubSeconds = 0;
//  HAL_RTC_SetAlarm_IT(&hrtc,&sAlarm,RTC_FORMAT_BIN);

 
}

void HAL_RTC_MspInit(RTC_HandleTypeDef* rtcHandle)
{

	RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;
  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspInit 0 */
   __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();
    
    
    /*##-2- Configue LSE or LSI as RTC clock source ###############################*/
    /* Enable LSE and LSE Clock */
    RCC_OscInitStruct.OscillatorType=RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.PLL.PLLState=RCC_PLL_NONE;
#if (defined RTC_CLOCK_SOURCE_LSE)
    RCC_OscInitStruct.LSEState=RCC_LSE_ON;
#endif
    RCC_OscInitStruct.LSIState=RCC_LSI_ON;
    if(HAL_RCC_OscConfig(&RCC_OscInitStruct)!=HAL_OK)
    {
      Error_Handler("OscConfig failed\r\n");
    }
    
    /* select RTC clock source  */
    PeriphClkInitStruct.PeriphClockSelection=RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.RTCClockSelection=RTC_CLOCK_SELECTION;
    if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct)!=HAL_OK)
    {
      Error_Handler("RTC clock source config failed\r\n");
    }
  /* USER CODE END RTC_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_RTC_ENABLE();

    /* Peripheral interrupt init */
   // HAL_NVIC_SetPriority(RTC_Alarm_IRQn, 0, 0);
   // HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
  /* USER CODE BEGIN RTC_MspInit 1 */
/* RTC Alarm NVIC Init */
    /* Alarm */
    HAL_NVIC_SetPriority(RTC_Alarm_IRQn, 0x0A, 0);
    HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
		/* RTC WakeUp NVIC Init */
//    HAL_NVIC_SetPriority(RTC_WKUP_IRQn, 0x0A, 0);
//    HAL_NVIC_EnableIRQ(RTC_WKUP_IRQn);
  /* USER CODE END RTC_MspInit 1 */
  }
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspDeInit 0 */

  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();

    /* Peripheral interrupt Deinit*/
    //HAL_NVIC_DisableIRQ(RTC_Alarm_IRQn);
		//HAL_NVIC_DisableIRQ(RTC_WKUP_IRQn);

  }
  /* USER CODE BEGIN RTC_MspDeInit 1 */

  /* USER CODE END RTC_MspDeInit 1 */
} 

/* USER CODE BEGIN 1 */
//void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc)
//{
//  printf("Wake Up\r\n");
//}

/*
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
  //printf("Alarm A\r\n");
}
*/

//void HAL_RTCEx_AlarmBEventCallback(RTC_HandleTypeDef *hrtc)
//{
//  printf("Alarm B\r\n");
//}
/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
