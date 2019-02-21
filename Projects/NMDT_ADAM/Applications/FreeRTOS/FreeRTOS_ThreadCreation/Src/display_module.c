/* Includes ------------------------------------------------------------------*/
#include "display_module.h"
#include "cmsis_os.h"

#include "sys_time_module.h"
#include "storage_module.h"
#include "usart_module.h"
#include "eeprom.h"
#include "lcdfont.h"

#define MAX_DISPLAY_ON_TIME            (200)   /* display on time , unit:second */

static char screen_time = 0;			/*�ڵ�ǰ��Ļ��ʾ��ʱ�䣬��λ����*/
static unsigned int button_flag = 0;						/*���԰�����ʾ��*/
static char key_flag = 0;			/*��ѯ���ݰ����Ƿ��£�1���£�0δ����*/

#if (SENSOR_ID == SENSOR_R) 
//struct parameter
//{
//		int16_t A1;				/*���ݲ�ѯ��������*/
//		int16_t B1;
//		int16_t C1;
//	
//		int16_t A2;				/*���ݲ�ѯ��ʱ����*/
//		int16_t B2;
//		int16_t C2;
//};
//struct parameter p1;

/*��̬ȫ�ֱ���*/
static int screen_year 		= 0, screen_month = 0, screen_day = 0;				/*��ѯ���ݵ������ݵ�������*/
static int screen_month1 	= 0, screen_day1 = 0, screen_hour1 = 0;				/*��ѯ���ݵ�ʱ���ݵ�����ʱ*/
static int screen_rain 		= 0;				/*��ѯ���ݵ���ʾ����*/
#endif



static uint8_t disp_on 				= true;
static int16_t disp_on_count 	= MAX_DISPLAY_ON_TIME;  /* display on time counter */

/* Private typedef -----------------------------------------------------------*/
/** 
  * @brief  Button Callback Function Definition
  */
typedef void (*ButtonCallbackFunc)(void *,int16_t);
/** 
  * @brief  LCD Display Function Definition
  */
typedef void (*DisplayFunc)(void *);

/** 
  * @brief  LCD Display Screen Structure
  */
typedef struct DisplayScreen
{
  int16_t selection;         /* current selection */
  int16_t screen_number;     /* current screen number */
	int16_t screen_leftright;
  
  ButtonCallbackFunc button_func;      /* button callback function */
  DisplayFunc        display_func;     /* display callback function */
} DisplayScreenTypeDef;
/** 
  * @brief  LCD Display Menu Structure
  */
typedef struct DisplayMenu
{
  DisplayScreenTypeDef Screen;
  
  struct DisplayMenu *prev;    /* previous menu */
  struct DisplayMenu *next;    /* next menu */
} DisplayMenuTypeDef;



/* Private define ------------------------------------------------------------*/
#define displaySTACK_SIZE   (512)
#define displayPRIORITY     osPriorityNormal
#define QUEUE_SIZE ((uint32_t)1)
/* Private variables ---------------------------------------------------------*/
static uint8_t nian,yue,ri,shi,fen,miao;//����ʱ�����
/* RTC Time*/
static RTC_TimeTypeDef Time;
static RTC_DateTypeDef Date;
/* Set Time */
static RTC_TimeTypeDef setTime;
static RTC_DateTypeDef setDate;


/* Display Menus and Screens -------------------------------------------------*/
/* Menus */
static DisplayMenuTypeDef *CurrentDisplay;  /* Current Display Menu */

/* os relative��ʾ���� */
static osThreadId DisplayThreadHandle;
/*static osSemaphoreId semaphore;*/
static osMessageQId ButtonQueue;  /* ��������button queue */
/* Private function prototypes -----------------------------------------------*/
static void Display_Thread(void const *argument);
static void init_display_menus(void);

#if (SENSOR_ID == SENSOR_W) 
static DisplayMenuTypeDef MainMenu;  /* Main Menu */
static DisplayMenuTypeDef TimeMenu;  /* Time Set Menu */
static DisplayMenuTypeDef DataMenu;  /* Data  Menu */
static void TimeScreen_ButtonHandler(void *Menu,int16_t button);
static void TimeScreen_Display(void *Menu);//ʱ��������
static void MainScreen_ButtonHandler(void *Menu,int16_t button);
static void MainScreen_Display(void *Menu);//���˵���
static void DataScreen_ButtonHandler(void *Menu,int16_t button);
static void DataScreen_Display(void *Menu);//������
#else
static DisplayMenuTypeDef MainMenu;  /* Mainr Menu */
static void MainScreen_ButtonHandler(void *Menu,int16_t button);
static void MainScreen_Display(void *Menu);//���˵���
static void OLED_Refresh_Gram_Main(void);

static DisplayMenuTypeDef DatarMenu;  /* Datar  Menu */
static void DatarScreen_ButtonHandler(void *Menu,int16_t button);
static void DatarScreen_Display(void *Menu);//������

static DisplayMenuTypeDef DemandMenu;  /* Demand Menu */
static void DemandScreen_ButtonHandler(void *Menu,int16_t button);
static void DemandScreen_Display(void *Menu);//��ѯ�˵���

static DisplayMenuTypeDef DayMenu;  /* Day Menu */
static void DayScreen_ButtonHandler(void *Menu,int16_t button);
static void DayScreen_Display(void *Menu);//�ղ˵���

static DisplayMenuTypeDef HourMenu;  /* Hour Menu */
static void HourScreen_ButtonHandler(void *Menu,int16_t button);
static void HourScreen_Display(void *Menu);//Сʱ�˵���

static DisplayMenuTypeDef StationMenu;  /* Station Menu */
static void StationScreen_ButtonHandler(void *Menu,int16_t button);
static void StationScreen_Display(void *Menu);//վ�Ų˵���

static DisplayMenuTypeDef WarnMenu;  /* Warn Menu */
static void WarnScreen_ButtonHandler(void *Menu,int16_t button);
static void WarnScreen_Display(void *Menu);//�����˵���

static DisplayMenuTypeDef TimerMenu;  /* Timer Menu */
static void TiemrScreen_ButtonHandler(void *Menu,int16_t button);
static void TimerScreen_Display(void *Menu);//ʱ��˵���

static DisplayMenuTypeDef SetMenu;  /* Set Menu */
static void SetScreen_ButtonHandler(void *Menu,int16_t button);
static void SetScreen_Display(void *Menu);//���ò˵���


typedef struct
{
	int warn_1;
	int warn_2;
	int warn_3;
	int warn_4;
}Warn;
static Warn warn_set = {0};  /*������������̨վ����Ҫ�ı���*/

typedef struct
{
	int station_1;
	int station_2;
	int station_3;
	int station_4;
	int station_5;
}Station;
static Station station_set = {0};  /*������������̨վ����Ҫ�ı���*/
#endif

__STATIC_INLINE void short_delay(void);
__STATIC_INLINE void turn_on_display(void);
__STATIC_INLINE void turn_off_display(void);

/**��ʼ����ʾ����
  * @brief  Init Display Module. 
  * @retval 0:success;-1:failed
  */
int32_t init_display_module(void)
{
	/*��ʼ��LCD��*/
			OLED_Init();
  /* init display menus */
  init_display_menus();
	
	#if (SENSOR_ID == SENSOR_R) 
			OLED_OpenMenu_SL3();
			HAL_Delay(10000);
	#endif
	
	#if (SENSOR_ID == SENSOR_W) 
			OLED_OpenMenu_EN2();
			HAL_Delay(10000);
	#endif
		/*������ʾ��*/
//		test_oled1();
//		HAL_Delay(1000);
//		OLED_Clear();
//		HAL_Delay(1000);
//		test_oled2();
//		HAL_Delay(10000);
//		OLED_Clear();
//		HAL_Delay(1000);
//		test_oled();
//		HAL_Delay(1000);
//		OLED_Clear();
//		HAL_Delay(1000);
//		test_oled3();
//		HAL_Delay(1000);
//		test_oled4();
//		HAL_Delay(1000);
//		OLED_Clear();
//		HAL_Delay(1000);
//		shou_point();
//		HAL_Delay(10000);
		
		
 
  /* Create the queue used by the button interrupt to pass the button value.����һ�����䰴��ֵ�ö��� */
  osMessageQDef(button_queue,QUEUE_SIZE,uint16_t);
  ButtonQueue=osMessageCreate(osMessageQ(button_queue),NULL);
  if(ButtonQueue == NULL)
  {
//    printf("Create Button Queue failed!\r\n");
    return -1;
  }
  
  /* Create a thread to update system date and time������ʾ���� */
  osThreadDef(Display, Display_Thread, displayPRIORITY, 0, displaySTACK_SIZE);
  DisplayThreadHandle=osThreadCreate(osThread(Display), NULL);
  if(DisplayThreadHandle == NULL)
  {
//    printf("Create Display Thread failed!\r\n");
    return -1;
  }
  
  
  return 0;
}

/**
  * @brief  EXTI line detection callbacks.
  * @param  GPIO_Pin: Specifies the port pin connected to corresponding EXTI line.
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
//	uint8_t i_g = 0;
  /* Disable Interrupt */
  HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
	HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
  
  /* eliminate key jitter */
	short_delay();
  if(HAL_GPIO_ReadPin(GPIOC, GPIO_Pin)==GPIO_PIN_RESET)
  {
			button_flag = 0;
			/* Put the Button Value to the Message Queue */
			if(GPIO_Pin == GPIO_PIN_8)  /* ESCAPE button */
			{
				if(ButtonQueue)
				{
					osMessagePut(ButtonQueue, ESCAPE, 100);
				}
			}
			
			if(GPIO_Pin==GPIO_PIN_9)  /* ENTER button */
			{
				if(ButtonQueue)
				{
					osMessagePut(ButtonQueue, ENTER, 100);
				}
			}
			
			if(GPIO_Pin==GPIO_PIN_10)  /* LEFT button */
			{
				if(ButtonQueue)
				{
					osMessagePut(ButtonQueue, DOWN, 100);
				}
			}
			
			if(GPIO_Pin == GPIO_PIN_11)  /* RIGHT button */
			{
				if(ButtonQueue)
				{
					osMessagePut(ButtonQueue, UP, 100);
				}
			}
			
			 if(GPIO_Pin == GPIO_PIN_12)  /* UP button */
			{
				if(ButtonQueue)
				{
					osMessagePut(ButtonQueue, LEFT, 100);
				}
			}
			
			 if(GPIO_Pin == GPIO_PIN_13)  /* DOWN button */
			{
      if(ButtonQueue)
      {
        osMessagePut(ButtonQueue, RIGHT, 100);
      }
    }
  }
	
	if(HAL_GPIO_ReadPin(GPIOA,GPIO_Pin)	==	GPIO_PIN_RESET)
	{
			#if (SENSOR_ID == SENSOR_R) 
			if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_7) == 0)
			{
					rain_sample.rain_1m++ ;   /*��������*/
					rain_sample.rain_d++;     /*������*/
			}
			else if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_7) == 1)
			{
					printf("no  rain\n");
			}
//			#else
//			if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_6) == 1)
//			{
//					printf("no  wind\n");
//			}
//			else if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_6) == 0)
//			{
//					wind_sample.ws_rate++;			/*һ�η����������һ��*/
//					
//			}
			#endif
	}
	
	
  
  /* Enable Interrupt */
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

/*��ʾ����*/
static void Display_Thread(void const *argument)
{
  osEvent event;
  int16_t button_value=0;
  struct tm datetime={0};
  
  (void)get_sys_time_tm(&datetime);
  
  OLED_Clear();  /* clear screen */

  while(1)
  {
			/* Get the message from the queue */
			event = osMessageGet(ButtonQueue, 1000);

			button_flag++;				/*ˢ��һ�Σ��Լ�һ��*/
			if (event.status == osEventMessage)
			{
					/* get button value */
					button_value = event.value.v;
					
					/* button handler */
					if(disp_on && CurrentDisplay->Screen.button_func)
					{
								(*CurrentDisplay->Screen.button_func)(CurrentDisplay,button_value);
					}

					/* turn on display */
					turn_on_display();
				
			}
			
			/* get data&time */
			(void)get_sys_time(&Date,&Time);
			
			/* display on */
			if(disp_on == true)
			{
				
					/* display function */
					if(CurrentDisplay->Screen.display_func)
					{
							(*CurrentDisplay->Screen.display_func)(CurrentDisplay);
					}
					
					disp_on_count--;
					if(disp_on_count < MAX_DISPLAY_ON_TIME*2/5)
					{
							/* turn off backlight */
							HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_RESET);     /*��ʼ��PB14Ϊ�͵�ƽ ����Ʋ���*/
					}
					if(disp_on_count < 0)
					{
							disp_on_count = 0;
					}
//					if(disp_on_count < 1)
//					{
//							/* turn off display */
//							turn_off_display();
//							screen_time = 0;
//							button_flag = 0;
//					}
			}
		}
}

#if (SENSOR_ID == SENSOR_W) 
/*��ʾ�˵�*/
static void init_display_menus(void)
{
	/* Normal_Mode Menu */
	DataMenu.prev=&MainMenu;
  DataMenu.next=NULL;
  DataMenu.Screen.screen_number=3;
  DataMenu.Screen.selection=0;
  DataMenu.Screen.button_func=DataScreen_ButtonHandler;
  DataMenu.Screen.display_func=DataScreen_Display;

	
  /* Time Menu */
  TimeMenu.prev=&MainMenu;
  TimeMenu.next=NULL;
  TimeMenu.Screen.screen_number=0;
  TimeMenu.Screen.selection=0;
  TimeMenu.Screen.button_func=TimeScreen_ButtonHandler;
  TimeMenu.Screen.display_func=TimeScreen_Display;
  
  
  /* Main Menu */
  MainMenu.prev=NULL;
  MainMenu.next=&DataMenu;
  MainMenu.Screen.screen_number=0;
  MainMenu.Screen.selection=0;
  MainMenu.Screen.button_func=MainScreen_ButtonHandler;
  MainMenu.Screen.display_func=MainScreen_Display;
	
  /* Current Menu */
  /*CurrentDisplay=&DataMenu;   //just use Data Menu for now*/
  CurrentDisplay=&MainMenu;  /* display main menu */
}

/*���ݲ˵�����*/
static void DataScreen_ButtonHandler(void *Menu,int16_t button)
{
  DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
  
  switch(button)
  {
			case ESCAPE:
				if(menu->prev)
			  {
						OLED_Clear();  /* clear screen */
						CurrentDisplay 											 = menu->prev;
						CurrentDisplay->Screen.screen_number = 0;
						CurrentDisplay->Screen.selection		 = 0;
			  }
			  break;
			case ENTER:
				break;
			case LEFT:
				break;
			case RIGHT:
				break;
			case UP:
				menu->Screen.screen_number++;
				break;
			case DOWN:
				menu->Screen.screen_number--;
				break;
			default:
				break;
  }
  
  if(menu->Screen.screen_number < 0)
  {
    menu->Screen.screen_number	=	4;
  }
  else if(menu->Screen.screen_number > 4)
  {
    menu->Screen.screen_number	=	0;
  }
  
}

/*��������ʾ*/
static void DataScreen_Display(void *Menu)
{
	DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
//  uint8_t highlight=false;
	
  if(menu->Screen.screen_number == 0)
	{
			char a[] = {"    ʱ  ��  ��  "};
			char b[] = {"                "};
			char c[] = {"˲ʱ����:     ��"};
			char d[] = {"˲ʱ����:    m/s"};
			char *p = NULL;
			uint8_t i = 0, j = 0;
			
			p = a;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x80 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = b;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x90 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = c;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x88 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = d;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x98 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
		
			/*�ڶ���*/
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf,sizeof(disp_buf),"%02u",Time.Hours);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x81 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf,sizeof(disp_buf),"%02u",Time.Minutes);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x83 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf,sizeof(disp_buf),"%02u",Time.Seconds);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x85 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			/*������*/
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf, sizeof(disp_buf),	"%4d",	wind_sample.wd_3s);
			p = disp_buf;
			for(i=0;i<2;i++)
			{
						OLED_W_Command(0x8d + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			/*������*/
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf, sizeof(disp_buf),	":%2d.%dm",	wind_sample.ws_1s/10, wind_sample.ws_1s%10);
			p = disp_buf;
			for(i=0;i<3;i++)
			{
						OLED_W_Command(0x9c + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
				
//				/* Time */
//				memset(disp_buf, 0, sizeof(disp_buf));
//				snprintf((char *)disp_buf, sizeof(disp_buf), "  %02u  %02u  %02u    ",
//								Time.Hours,Time.Minutes,Time.Seconds);
//				OLED_ShowString(0,0,disp_buf,false);
//				OLED_China(0, 32, 3,false);//ʱ
//				OLED_China(0, 64, 4,false);//��
//				OLED_China(0, 96, 5,false);//��
//				
//				snprintf(disp_buf, sizeof(disp_buf), "                ");
//				OLED_ShowString(16, 0, disp_buf, false);
//				
//				/*˲ʱ����*/
//				snprintf(disp_buf, sizeof(disp_buf),	"        : %4d  ",	wind_sample.wd_3s);
//				OLED_ShowString(32,	0, disp_buf, false);//˲ʱ����
//				OLED_China(32, 0,	 9, false);//˲
//				OLED_China(32, 16, 3, false);//ʱ
//				OLED_China(32, 32, 10, false);//��
//				OLED_China(32, 48, 11, false);//��
//				OLED_China(32, 112, 36, false);//�ȣ���
//				
//				/*˲ʱ����*/
//				snprintf(disp_buf, sizeof(disp_buf), "        :%2d.%dm/s", wind_sample.ws_1s/10,wind_sample.ws_1s%10);
//				OLED_ShowString(48, 0, disp_buf, false);//˲ʱ����
//				OLED_China(48,  0, 9, false);//˲
//				OLED_China(48, 16, 3, false);//ʱ
//				OLED_China(48, 32, 10, false);//��
//				OLED_China(48, 48, 12, false);//��
	}
	else if(menu->Screen.screen_number == 1)
	{
			char a[] = {"    ʱ  ��  ��  "};
			char b[] = {"                "};
			char c[] = {"���ַ���:     ��"};
			char d[] = {"���ַ���:    m/s"};
			char *p = NULL;
			uint8_t i = 0, j = 0;
			
			p = a;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x80 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = b;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x90 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = c;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x88 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = d;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x98 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
		
			/*�ڶ���*/
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf,sizeof(disp_buf),"%02u",Time.Hours);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x81 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf,sizeof(disp_buf),"%02u",Time.Minutes);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x83 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf,sizeof(disp_buf),"%02u",Time.Seconds);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x85 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			/*������*/
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf, sizeof(disp_buf),	"%4d",	wind_sample.wd_2m);
			p = disp_buf;
			for(i=0;i<2;i++)
			{
						OLED_W_Command(0x8d + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			/*������*/
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf, sizeof(disp_buf), ":%2d.%dm", wind_sample.ws_2m/10,wind_sample.ws_2m%10);
			p = disp_buf;
			for(i=0;i<3;i++)
			{
						OLED_W_Command(0x9c + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
		
//				/* Time */
//				memset(disp_buf, 0, sizeof(disp_buf));
//				snprintf((char *)disp_buf,sizeof(disp_buf),"  %02u  %02u  %02u    ",
//								Time.Hours,Time.Minutes,Time.Seconds);
//				OLED_ShowString(0,0,disp_buf,false);
//				OLED_China(0, 32, 3,false);//ʱ
//				OLED_China(0, 64, 4,false);//��
//				OLED_China(0, 96, 5,false);//��
//				
//				
//				snprintf(disp_buf, sizeof(disp_buf), "                ");
//				OLED_ShowString(16, 0, disp_buf, false);
//				
//				/*2�ַ���*/
//				snprintf(disp_buf, sizeof(disp_buf), "        : %4d  ", wind_sample.wd_2m);
//				OLED_ShowString(32, 0, disp_buf, false);//���ַ���
//				OLED_China(32,  0, 37, false);//��
//				OLED_China(32, 16, 4, false);//��
//				OLED_China(32, 32, 10, false);//��
//				OLED_China(32, 48, 11, false);//��
//				OLED_China(32, 112, 36, false);//�ȣ���
//			
//				/*���ַ���*/
//				snprintf(disp_buf, sizeof(disp_buf), "        :%2d.%dm/s", wind_sample.ws_2m/10,wind_sample.ws_2m%10);
//				OLED_ShowString(48, 0, disp_buf, false);//���ַ���
//				OLED_China(48,  0, 37, false);//��
//				OLED_China(48, 16, 4, false);//��
//				OLED_China(48, 32, 10, false);//��
//				OLED_China(48, 48, 12, false);//��
	}
				
	else if(menu->Screen.screen_number == 2)
	{
			char a[] = {"    ʱ  ��  ��  "};
			char b[] = {"                "};
			char c[] = {"ʮ�ַ���:     ��"};
			char d[] = {"ʮ�ַ���:    m/s"};
			char *p = NULL;
			uint8_t i = 0, j = 0;
			
			p = a;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x80 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = b;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x90 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = c;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x88 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = d;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x98 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
		
			/*�ڶ���*/
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf,sizeof(disp_buf),"%02u",Time.Hours);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x81 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf,sizeof(disp_buf),"%02u",Time.Minutes);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x83 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf,sizeof(disp_buf),"%02u",Time.Seconds);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x85 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			/*������*/
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf, sizeof(disp_buf),	"%4d",	wind_sample.wd_10m);
			p = disp_buf;
			for(i=0;i<2;i++)
			{
						OLED_W_Command(0x8d + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			/*������*/
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf, sizeof(disp_buf), ":%2d.%dm", wind_sample.ws_10m/10,wind_sample.ws_10m%10);
			p = disp_buf;
			for(i=0;i<3;i++)
			{
						OLED_W_Command(0x9c + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
//				/* Time */
//				snprintf((char *)disp_buf,sizeof(disp_buf),"  %02u  %02u  %02u    ",
//								Time.Hours,Time.Minutes,Time.Seconds);
//				OLED_ShowString(0,0,disp_buf,false);
//				OLED_China(0, 32, 3,false);//ʱ
//				OLED_China(0, 64, 4,false);//��
//				OLED_China(0, 96, 5,false);//��
//				
//				snprintf(disp_buf, sizeof(disp_buf), "                ");
//				OLED_ShowString(16, 0, disp_buf, false);
//				
//				/*ʮ�ַ���*/
//				snprintf(disp_buf, sizeof(disp_buf), "        : %4d  ", wind_sample.wd_10m);
//				OLED_ShowString(32, 0, disp_buf, false);//ʮ�ַ���
//				OLED_China(32,   0, 38, false);//ʮ
//				OLED_China(32,  16, 4, false);//��
//				OLED_China(32,  32, 10, false);//��
//				OLED_China(32,  48, 11, false);//��
//				OLED_China(32, 112, 36, false);//�ȣ���
//		
//				/*ʮ�ַ���*/
//				snprintf(disp_buf, sizeof(disp_buf), "        :%2d.%dm/s", wind_sample.ws_10m/10,wind_sample.ws_10m%10);
//				OLED_ShowString(48, 0, disp_buf,false);//ʮ�ַ���
//				OLED_China(48,  0, 38, false);//ʮ
//				OLED_China(48, 16, 4, false);//��
//				OLED_China(48, 32, 10, false);//��
//				OLED_China(48, 48, 12, false);//��
	}
	else if(menu->Screen.screen_number == 3)
	{
			char a[] = {"    ʱ  ��  ��  "};
			char b[] = {"����ʱ��:       "};
			char c[] = {"�������:     ��"};
			char d[] = {"�������:    m/s"};
			char *p = NULL;
			uint8_t i = 0, j = 0;
			
			p = a;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x80 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = b;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x90 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = c;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x88 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = d;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x98 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
		
			/*��һ��*/
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf,sizeof(disp_buf),"%02u",Time.Hours);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x81 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf,sizeof(disp_buf),"%02u",Time.Minutes);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x83 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf,sizeof(disp_buf),"%02u",Time.Seconds);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x85 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			/*�ڶ���*/
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf, sizeof(disp_buf), " %02d-%02d", wind_sample.ws_1s_max_t/100, wind_sample.ws_1s_max_t % 100);
			p = disp_buf;
			for(i=0;i<3;i++)
			{
						OLED_W_Command(0x95 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			/*������*/
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf, sizeof(disp_buf),	"%4d",	wind_sample.wd_10m);
			p = disp_buf;
			for(i=0;i<2;i++)
			{
						OLED_W_Command(0x8d + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			/*������*/
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf, sizeof(disp_buf), ":%2d.%dm", wind_sample.ws_10m/10,wind_sample.ws_10m%10);
			p = disp_buf;
			for(i=0;i<3;i++)
			{
						OLED_W_Command(0x9c + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
//				/* Time */
//				snprintf((char *)disp_buf,sizeof(disp_buf),"  %02u  %02u  %02u    ",
//								Time.Hours,Time.Minutes,Time.Seconds);
//				OLED_ShowString(0,0,disp_buf,false);
//				OLED_China(0, 32, 3,false);//ʱ
//				OLED_China(0, 64, 4,false);//��
//				OLED_China(0, 96, 5,false);//��
//				
//				/*����ʱ��*/
//				snprintf(disp_buf, sizeof(disp_buf), "        :  %02d-%02d", wind_sample.ws_1s_max_t  / 100, wind_sample.ws_1s_max_t % 100);
//				OLED_ShowString(16, 0, disp_buf, false);//����ʱ��
//				OLED_China(16,  0, 15, false);//��
//				OLED_China(16, 16, 14, false);//��
//				OLED_China(16, 32, 3, false);//ʱ
//				OLED_China(16, 48, 18, false);//��
//				
//				/*�������*/
//				snprintf(disp_buf, sizeof(disp_buf), "        : %4d  ", wind_sample.wd_1s_max );
//				OLED_ShowString(32, 0, disp_buf, false);//�������
//				OLED_China(32,  0, 15, false);//��
//				OLED_China(32, 16, 14, false);//��
//				OLED_China(32, 32, 10, false);//��
//				OLED_China(32, 48, 11, false);//��
//				OLED_China(32, 112, 36, false);//�ȣ���
//			
//				/*�������*/
//				snprintf(disp_buf, sizeof(disp_buf), "        :%2d.%dm/s", wind_sample.ws_1s_max/10,wind_sample.ws_1s_max%10);
//				OLED_ShowString(48, 0, disp_buf,false);//�������
//				OLED_China(48, 0, 15, false);//��
//				OLED_China(48, 16, 14, false);//��
//				OLED_China(48, 32, 10, false);//��
//				OLED_China(48, 48, 12, false);//��
	}
	else if(menu->Screen.screen_number == 4)
	{
			char a[] = {"    ʱ  ��  ��  "};
			char b[] = {"���ʱ��:       "};
			char c[] = {"������:     ��"};
			char d[] = {"������:    m/s"};
			char *p = NULL;
			uint8_t i = 0, j = 0;
			
			p = a;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x80 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = b;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x90 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = c;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x88 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = d;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x98 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
		
			/*��һ��*/
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf,sizeof(disp_buf),"%02u",Time.Hours);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x81 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf,sizeof(disp_buf),"%02u",Time.Minutes);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x83 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf,sizeof(disp_buf),"%02u",Time.Seconds);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x85 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			/*�ڶ���*/
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf, sizeof(disp_buf), " %02d-%02d", wind_sample.ws_10m_max_t / 100, wind_sample.ws_10m_max_t % 100);
			p = disp_buf;
			for(i=0;i<3;i++)
			{
						OLED_W_Command(0x95 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			/*������*/
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf, sizeof(disp_buf),	"%4d",	wind_sample.wd_10m);
			p = disp_buf;
			for(i=0;i<2;i++)
			{
						OLED_W_Command(0x8d + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			/*������*/
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf, sizeof(disp_buf), ":%2d.%dm", wind_sample.ws_10m/10,wind_sample.ws_10m%10);
			p = disp_buf;
			for(i=0;i<3;i++)
			{
						OLED_W_Command(0x9c + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
//				/* Time */
//				snprintf((char *)disp_buf,sizeof(disp_buf),"  %02u  %02u  %02u    ",
//								Time.Hours,Time.Minutes,Time.Seconds);
//				OLED_ShowString(0,0,disp_buf,false);
//				OLED_China(0, 32, 3,false);//ʱ
//				OLED_China(0, 64, 4,false);//��
//				OLED_China(0, 96, 5,false);//��
//				
//				/*���ʱ��*/
//				snprintf(disp_buf, sizeof(disp_buf), "        :  %02d-%02d", wind_sample.ws_10m_max_t / 100, wind_sample.ws_10m_max_t % 100);
//				OLED_ShowString(16, 0, disp_buf, false);//���ʱ��
//				OLED_China(16,  0, 13, false);//��
//				OLED_China(16, 16, 14, false);//��
//				OLED_China(16, 32, 3, false);//ʱ
//				OLED_China(16, 48, 18, false);//��
//				
//				
//				/*������*/
//				snprintf(disp_buf, sizeof(disp_buf), "        : %4d  ", wind_sample.wd_10m_max );
//				OLED_ShowString(32, 0, disp_buf, false);//������
//				OLED_China(32,  0, 13, false);//��
//				OLED_China(32, 16, 14, false);//��
//				OLED_China(32, 32, 10, false);//��
//				OLED_China(32, 48, 11, false);//��
//				OLED_China(32, 112, 36, false);//�ȣ���
//				
//				/*������*/
//				snprintf(disp_buf, sizeof(disp_buf), "        :%2d.%dm/s", wind_sample.ws_10m_max/10,wind_sample.ws_10m_max%10);
//				OLED_ShowString(48, 0, disp_buf, false);//������
//				OLED_China(48,  0, 13, false);//��
//				OLED_China(48, 16, 14, false);//��
//				OLED_China(48, 32, 10, false);//��
//				OLED_China(48, 48, 12, false);//��
	}
	
//	OLED_Refresh_Gram();
}



/*ʱ��������*/
static void TimeScreen_ButtonHandler(void *Menu,int16_t button)
{
  DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
  int16_t selected_timevalue=0,min_value=0,max_value=0;
  uint8_t *selected=NULL;
	
  switch(menu->Screen.selection)
  {
  case 0:  /* year */
    selected=&setDate.Year;
    selected_timevalue=setDate.Year;
    min_value=0;
    max_value=99;
    break;
  case 1:  /* month */
    selected=&setDate.Month;
    selected_timevalue=setDate.Month;
    min_value=1;
    max_value=12;
    break;
  case 2:  /* day */
    selected=&setDate.Date;
    selected_timevalue=setDate.Date;
    min_value=1;
    max_value=31;
    break;
  case 3:  /* hour */
    selected=&setTime.Hours;
    selected_timevalue=setTime.Hours;
    min_value=0;
    max_value=23/*59*/;   /* 16.3.23 hour:0-23 */
    break;
  case 4:  /* minute */
    selected=&setTime.Minutes;
    selected_timevalue=setTime.Minutes;
    min_value=0;
    max_value=59;
    break;
  case 5:  /* second */
    selected=&setTime.Seconds;
    selected_timevalue=setTime.Seconds;
    min_value=0;
    max_value=59;
    break;
  default:
    menu->Screen.selection=5;
    selected=&setTime.Seconds;
    selected_timevalue=setTime.Seconds;
    min_value=0;
    max_value=59;
    break;
  }
  
  switch(button)
  {
  case ESCAPE:
		if((setDate.Year != nian)||(setDate.Month != yue)||(setDate.Date != ri)||(setTime.Hours != shi)||(setTime.Minutes != fen)||(setTime.Seconds != miao))
		{
			/* set data&time */
			/* fill unused value */
			setTime.DayLightSaving=RTC_DAYLIGHTSAVING_NONE;
			setTime.StoreOperation=RTC_STOREOPERATION_RESET;
			setTime.SubSeconds=0;
			setTime.TimeFormat=RTC_HOURFORMAT12_AM;
			if(set_sys_time(&setDate,&setTime)<0)
				{
					OLED_Clear();
					OLED_ShowString(0,24,"set time failed!",1);
					OLED_Refresh_Gram();
				}
				else
					{
						OLED_Clear();
						OLED_ShowString(16,24,"set time ok!",1);
						OLED_Refresh_Gram();
					}
					osDelay(500);
		}
    if(menu->prev)
			{
				CurrentDisplay=menu->prev;
				CurrentDisplay->Screen.screen_number=0;
				CurrentDisplay->Screen.selection=1;
				OLED_Clear();  /* clear screen */
			}
    break;
  case ENTER:
    menu->Screen.selection++;
    break;
  case UP:
    selected_timevalue--;
    break;
  case DOWN:
    selected_timevalue++;
	  break;
  default:
    break;
  }
  
  if(menu->Screen.selection<0)
  {
    menu->Screen.selection = 5;
  }
  else if(menu->Screen.selection > 5)
  {
    menu->Screen.selection = 0;
  }
  
  if(selected_timevalue < min_value)
  {
    selected_timevalue = max_value;
  }
  else if(selected_timevalue > max_value)
  {
    selected_timevalue = min_value;
  }
  /* set selected value */
  *selected=selected_timevalue;
}

/*ʱ������ʾ*/
/*ʱ������ʾ*/
static void TimeScreen_Display(void *Menu)
{
			DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
//			uint8_t highlight=false;
		 
			char a[] = {"                "};
			char b[] = {"    20  ��      "};
			char c[] = {"      ��  ��    "};
			char d[] = {"    ʱ  ��  ��  "};
			char *p = NULL;
			uint8_t i = 0, j = 0;
			
			p = a;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x80 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = b;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x90 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = c;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x88 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = d;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x98 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
		
			/*�ڶ���*/
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf,sizeof(disp_buf),"%02u",setDate.Year);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x93 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			/*������*/
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf,sizeof(disp_buf),"%02u",setDate.Month);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x8a + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf,sizeof(disp_buf),"%02u",setDate.Date);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x8c + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			/*������*/
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf,sizeof(disp_buf),"%02u",setTime.Hours);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x99 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf,sizeof(disp_buf),"%02u",setTime.Minutes);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x9b + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf,sizeof(disp_buf),"%02u",setTime.Seconds);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x9d + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			/*����*/
			if((menu->Screen.selection == 0) && (button_flag < 2))
			{
						pic_clear_u();				
						pic_u(0x01, 0x10, 0x93, 0x90, TAB6);
			}
			else if((menu->Screen.selection == 1) && (button_flag < 2))
			{
						pic_clear_u();				
						pic_u(0x01, 0x10, 0x8a, 0x80, TAB6);
			}
			else if((menu->Screen.selection == 2) && (button_flag < 2))
			{
						pic_clear_u();				
						pic_u(0x01, 0x10, 0x8c, 0x80, TAB6);
			}
			else if((menu->Screen.selection == 3) && (button_flag < 2))
			{
						pic_clear_u();				
						pic_u(0x01, 0x10, 0x99, 0x90, TAB6);
			}
			else if((menu->Screen.selection == 4) && (button_flag < 2))
			{
						pic_clear_u();				
						pic_u(0x01, 0x10, 0x9b, 0x90, TAB6);
			}
			else if((menu->Screen.selection == 5) && (button_flag < 2))
			{
						pic_clear_u();				
						pic_u(0x01, 0x10, 0x9d, 0x90, TAB6);
			}
			
//			/* year */
//			OLED_ShowString(0, 33, "20", false);//��20��
//			snprintf(disp_buf,sizeof(disp_buf),"%02u",setDate.Year);
//			highlight=(menu->Screen.selection==0);
//			OLED_ShowString(0, 49, disp_buf,highlight);//��17�ꡱ
//			OLED_China(0, 73, 0, false);//��
//			
//			/* month */
//			snprintf(disp_buf,sizeof(disp_buf),"%02u",setDate.Month);
//			highlight=(menu->Screen.selection==1);
//			OLED_ShowString(24, 25, disp_buf, highlight);//��06�¡�
//			OLED_China(24, 49, 1, false);//��
//			
//			/* day */
//			snprintf(disp_buf,sizeof(disp_buf),"%02u",setDate.Date);
//			highlight=(menu->Screen.selection==2);
//			OLED_ShowString(24, 65, disp_buf, highlight);//��22�ա�
//			OLED_China(24, 89, 2, false);//��
//			
//			/* hour */
//			snprintf(disp_buf,sizeof(disp_buf),"%02u",setTime.Hours);
//			highlight=(menu->Screen.selection==3);
//			OLED_ShowString(48, 9, disp_buf, highlight);//��10�㡱
//			OLED_China(48, 33, 3, false);//ʱ
//			
//			/* minute */
//			snprintf(disp_buf,sizeof(disp_buf),"%02u",setTime.Minutes);
//			highlight=(menu->Screen.selection==4);
//			OLED_ShowString(48,49,disp_buf,highlight);//��10�֡�
//			OLED_China(48, 73, 4,false);//��
//			
//			/* second */
//			snprintf(disp_buf,sizeof(disp_buf),"%02u",setTime.Seconds);
//			highlight=(menu->Screen.selection==5);
//			OLED_ShowString(48, 89, disp_buf, highlight);//��10�롱
//			OLED_China(48, 108, 5, false);//��
//			
//			OLED_Refresh_Gram();
}	




/*���˵�����*/
static void MainScreen_ButtonHandler(void *Menu,int16_t button)
{
	DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
  
  switch(button)
  {
  case ESCAPE:
    break;
  case ENTER:   /* enter next menu */
    if(menu->next)
    {
      CurrentDisplay=menu->next;
      CurrentDisplay->Screen.screen_number=0;
      CurrentDisplay->Screen.selection=0;
      OLED_Clear();  /* clear screen */
  
      /* get system time */
      get_sys_time(&setDate,&setTime);
			if(menu->Screen.selection == 1)
			{
					nian   =  setDate.Year;
					yue    =  setDate.Month;
					ri     =  setDate.Date;
					shi    =  setTime.Hours;
					fen    =  setTime.Minutes;
					miao   =  setTime.Seconds;
			}
    }
    break;
  case LEFT:
    menu->Screen.selection--;
    break;
  case RIGHT:
    menu->Screen.selection++;
    break;
  default:
    break;
  }
  
  
  if(menu->Screen.selection < 0)
  {
    menu->Screen.selection = 1;
  }
  else if(menu->Screen.selection > 1)
  {
    menu->Screen.selection = 0;
  }
}

/*���˵�����ʾ*/
static void MainScreen_Display(void *Menu)
{
		DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
//		uint8_t highlight=false;
  
			char a[] = {"    ��  ��  ��  "};
			char b[] = {"    ʱ  ��  ��  "};
			char c[] = {"                "};
			char d[] = {"  ����    ��ʱ  "};
			char *p = NULL;
			uint8_t i = 0, j = 0;
			
			p = a;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x80 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = b;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x90 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = c;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x88 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = d;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x98 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
		
			/*��һ��*/
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf,sizeof(disp_buf),"%02u",Date.Year);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x81 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf,sizeof(disp_buf),"%02u",Date.Month);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x83 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf,sizeof(disp_buf),"%02u",Date.Date);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x85 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			/*�ڶ���*/
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf,sizeof(disp_buf),"%02u",Time.Hours);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x91 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf,sizeof(disp_buf),"%02u",Time.Minutes);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x93 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf,sizeof(disp_buf),"%02u",Time.Seconds);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x95 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			/*����*/
			if((menu->Screen.selection == 0) && (button_flag < 2))
			{
						pic_clear_u();				
						pic_u(0x02, 0x10, 0x99, 0x90, TAB6);
			}
			else if((menu->Screen.selection == 1) && (button_flag < 2))
			{
						pic_clear_u();				
						pic_u(0x02, 0x10, 0x9d, 0x90, TAB6);
			}
	
//		screen_time++;			/*��ʾ��ǰ�������Լ�һ��*/
//		if(usart_debug)
//		{
//					printf("screen_time=%d\r\n", screen_time);
//		}
//		/* Date */
//		snprintf((char *)disp_buf,sizeof(disp_buf)," 20%02u  %02u  %02u   ",
//							Date.Year,Date.Month,Date.Date);
//		OLED_ShowString(8,0,disp_buf,false);
//		OLED_China(8,40,0,false);//��
//		OLED_China(8,72,1,false);//��
//		OLED_China(8,104,2,false);//��
//		/* Time */
//		snprintf((char *)disp_buf,sizeof(disp_buf),"  %02u  %02u  %02u    ",
//						Time.Hours,Time.Minutes,Time.Seconds);
//		OLED_ShowString(24,0,disp_buf,false);
//		OLED_China(24,32,3,false);//ʱ
//		OLED_China(24,64,4,false);//��
//		OLED_China(24,96,5,false);//��
//	
//  
//		/* ���� */
//		highlight = (menu->Screen.selection == 0);
//		OLED_China(48,16,6,highlight);//��
//		OLED_China(48,32,7,highlight);//��
//  
//		/* ��ʱ */
//		highlight=(menu->Screen.selection==1);
//		OLED_China(48,80,8,highlight);//��
//		OLED_China(48,96,3,highlight);//ʱ
  
		/* Determine the next Menu */
		switch(menu->Screen.selection)
		{
				case 0:   /* Data Menu */
					menu->next=&DataMenu;
					break;
				case 1:   /* Communication Menu */
					menu->next=&TimeMenu;
					break;
				default:
					menu->next=&DataMenu;
					break;
		}
		
//		OLED_Refresh_Gram();
}

#else
/*��ʾ�˵�*/
static void init_display_menus(void)
{
		/* Main Menu */
		MainMenu.prev									=	NULL;
		MainMenu.next									=	&DatarMenu;
		MainMenu.Screen.screen_number	=	0;
		MainMenu.Screen.selection			=	0;
		MainMenu.Screen.button_func		=	MainScreen_ButtonHandler;
		MainMenu.Screen.display_func	=	MainScreen_Display;
	
		/* Data Menu */
		DatarMenu.prev=&MainMenu;
		DatarMenu.next=NULL;
		DatarMenu.Screen.screen_number=3;
		DatarMenu.Screen.selection=0;
		DatarMenu.Screen.button_func=DatarScreen_ButtonHandler;
		DatarMenu.Screen.display_func=DatarScreen_Display;

		/* Demand Menu */
		DemandMenu.prev =	&MainMenu;
		DemandMenu.next	=	&DayMenu;
		DemandMenu.Screen.screen_number =		0;
		DemandMenu.Screen.selection     = 	0;
		DemandMenu.Screen.button_func   = 	DemandScreen_ButtonHandler;
		DemandMenu.Screen.display_func  = 	DemandScreen_Display;
  
		/* Day Menu */
		DayMenu.prev =	&DemandMenu;
		DayMenu.next	=	NULL;
		DayMenu.Screen.screen_number =	0;
		DayMenu.Screen.selection     = 	0;
		DayMenu.Screen.button_func   = 	DayScreen_ButtonHandler;
		DayMenu.Screen.display_func  = 	DayScreen_Display;
  
		/* Hour Menu */
		HourMenu.prev =	&DemandMenu;
		HourMenu.next	=	NULL;
		HourMenu.Screen.screen_number =  	0;
		HourMenu.Screen.selection     = 	0;
		HourMenu.Screen.button_func   = 	HourScreen_ButtonHandler;
		HourMenu.Screen.display_func  = 	HourScreen_Display;
		
		/* Set Menu */
		SetMenu.prev =	&MainMenu;
		SetMenu.next	=	&StationMenu;
		SetMenu.Screen.screen_number =	0;
		SetMenu.Screen.selection     = 	0;
		SetMenu.Screen.button_func   = 	SetScreen_ButtonHandler;
		SetMenu.Screen.display_func  = 	SetScreen_Display;
		
		/* Station Menu */
		StationMenu.prev =	&SetMenu;
		StationMenu.next	=	NULL;
		StationMenu.Screen.screen_number =	0;
		StationMenu.Screen.selection     = 	0;
		StationMenu.Screen.button_func   = 	StationScreen_ButtonHandler;
		StationMenu.Screen.display_func  = 	StationScreen_Display;
		
		/* Warn Menu */
		WarnMenu.prev =	&SetMenu;
		WarnMenu.next	=	NULL;
		WarnMenu.Screen.screen_number =	0;
		WarnMenu.Screen.selection     = 	0;
		WarnMenu.Screen.button_func   = 	WarnScreen_ButtonHandler;
		WarnMenu.Screen.display_func  = 	WarnScreen_Display;
		
		/* Timer Menu */
		TimerMenu.prev =	&SetMenu;
		TimerMenu.next	=NULL;
		TimerMenu.Screen.screen_number =	0;
		TimerMenu.Screen.selection     = 	0;
		TimerMenu.Screen.button_func   = 	TiemrScreen_ButtonHandler;
		TimerMenu.Screen.display_func  = 	TimerScreen_Display;
		/* Current Menu */
		CurrentDisplay=&MainMenu;  /* display main menu */
}

/*���˵�����*/
static void MainScreen_ButtonHandler(void *Menu,int16_t button)
{
		DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
		
		switch(button)
		{
				case ESCAPE:
					break;
				case ENTER:   /* enter next menu */
					if(menu->next)
					{
							button_flag  = 0;
							screen_time = 0;
							CurrentDisplay 													= menu->next;
							CurrentDisplay->Screen.screen_number  	= 0;
							CurrentDisplay->Screen.selection 				= 0;
							CurrentDisplay->Screen.screen_leftright = 0;
							OLED_Clear();  /* clear screen */
  
							if(menu->Screen.selection == 1)			/*�����ѯ�˵�*/
							{
										/* get system time */
										get_sys_time(&setDate,&setTime);
									
										/*��������ʾ��ʱ��*/
										screen_year 	= setDate.Year;
										screen_month  = setDate.Month;
										screen_day 		= setDate.Date;
								
										/*ʱ������ʾ��ʱ��*/
										screen_month1 	= setDate.Month;
										screen_day1		  = setDate.Date;
										screen_hour1 		= setTime.Hours;
							}
					}
					break;
			case LEFT:
					menu->Screen.selection--;
					break;
			case RIGHT:
					menu->Screen.selection++;
					break;
			default:
					break;
		}
		
		if(menu->Screen.selection<0)
		{
				menu->Screen.selection=2;
		}
		else if(menu->Screen.selection>2)
		{
				menu->Screen.selection=0;
		}
}

/*���˵�����ʾ*/
static void MainScreen_Display(void *Menu)
{
		DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
		uint8_t highlight=false;
		uint8_t i = 0, j = 0;
		char *p = NULL;
  
//		/* Date */
//		snprintf((char *)disp_buf,sizeof(disp_buf)," 20%02u  %02u  %02u   ",
//							Date.Year,Date.Month,Date.Date);
//		OLED_ShowString(0,0,disp_buf,false);
//		OLED_China(0,  40,  0,false);//��
//		OLED_China(0,  72,  1,false);//��
//		OLED_China(0, 104, 2,false);//��
//		/* Time */
//		snprintf((char *)disp_buf,sizeof(disp_buf),"  %02u  %02u  %02u    ",
//						Time.Hours,Time.Minutes,Time.Seconds);
//		OLED_ShowString(16,0,disp_buf,false);
//		OLED_China(16, 32, 3,false);//ʱ
//		OLED_China(16, 64, 4,false);//��
//		OLED_China(16, 96, 5,false);//��
//	
//		snprintf((char *)disp_buf,sizeof(disp_buf),"       :%4u.%1umm",rain_sample.rain_d / 10, rain_sample.rain_d % 10);
//		OLED_ShowString(32,0,disp_buf,false);
//		OLED_China(32,  8, 2,false);//��
//		OLED_China(32, 24, 19,false);//��
//		OLED_China(32, 40, 20,false);//��
//		
//		/* ˲ʱ */
//		highlight = (menu->Screen.selection == 0);
//		OLED_China(48,  8, 9,highlight);//˲
//		OLED_China(48, 24, 3,highlight);//ʱ
//	
//		/* ��ѯ */
//		highlight=(menu->Screen.selection == 1);
//		OLED_China(48, 48, 26,highlight);//��
//		OLED_China(48, 64, 27,highlight);//ѯ
//		
//		/* ���� */
//		highlight=(menu->Screen.selection == 2);
//		OLED_China(48,  88, 28,highlight);//��
//		OLED_China(48, 104, 29,highlight);//��
//					
//		OLED_Refresh_Gram();

//		char a[] = {"    ��  ��  ��  "};
//		char b[] = {"    ʱ  ��  ��  "};
		char c[] = {"������:       mm"};
		char d[] = {"˲ʱ  ��ѯ  ����"};
		
		/*��һ��*/
		snprintf((char *)disp_buf,sizeof(disp_buf),"  %02u  %02u  %02u    ",Date.Year,Date.Month,Date.Date);
		p = disp_buf;
		for(i=0;i<8;i++)
		{
					OLED_W_Command(0x80 + i);
					OLED_W_Data(*p);
					p++;
					OLED_W_Data(*p);
					p++;
		}
			
		OLED_W_Command(0x82);
		char e[] = {"��"};
		p = e;
		for(j = 0; j < 2; j++)
		{
					OLED_W_Data(*p);
					p++;
		}
		
		OLED_W_Command(0x84);
		char f[] = {"��"};
		p = f;
		for(j = 0; j < 2; j++)
		{
					OLED_W_Data(*p);
					p++;
		}
		
		OLED_W_Command(0x86);
		char g[] = {"��"};
		p = g;
		for(j = 0; j < 2; j++)
		{
					OLED_W_Data(*p);
					p++;
		}
		
		/*�ڶ���*/
		snprintf((char *)disp_buf,sizeof(disp_buf),"  %02u  %02u  %02u    ", Time.Hours,Time.Minutes,Time.Seconds);
		p = disp_buf;
		for(i=0;i<8;i++)
		{
					OLED_W_Command(0x90 + i);
					OLED_W_Data(*p);
					p++;
					OLED_W_Data(*p);
					p++;
		}
			
		OLED_W_Command(0x92);
		char h[] = {"ʱ"};
		p = h;
		for(j = 0; j < 2; j++)
		{
					OLED_W_Data(*p);
					p++;
		}
		
		OLED_W_Command(0x94);
		char k[] = {"��"};
		p = k;
		for(j = 0; j < 2; j++)
		{
					OLED_W_Data(*p);
					p++;
		}
		
		OLED_W_Command(0x96);
		char l[] = {"��"};
		p = l;
		for(j = 0; j < 2; j++)
		{
					OLED_W_Data(*p);
					p++;
		}
		
		/*������*/
		p = c;
		for(i=0;i<8;i++)
		{
					OLED_W_Command(0x88 + i);
					OLED_W_Data(*p);
					p++;
					OLED_W_Data(*p);
					p++;
		}
		memset(disp_buf, 0, sizeof(disp_buf));
		snprintf((char *)disp_buf,sizeof(disp_buf),"%4u.%1u", rain_sample.rain_d / 10, rain_sample.rain_d % 10);
		p = disp_buf;
		OLED_W_Command(0x88 + 4);
		OLED_W_Data(*p);
		p++;
		OLED_W_Data(*p);
		p++;
		OLED_W_Command(0x88 + 5);
		OLED_W_Data(*p);
		p++;
		OLED_W_Data(*p);
		p++;
		OLED_W_Command(0x88 + 6);
		OLED_W_Data(*p);
		p++;
		OLED_W_Data(*p);
		p++;
		
		/*�����кͷ���*/
		if((menu->Screen.selection == 0) && (button_flag  < 2))
		{
					pic_clear_u();				
					pic_u(0x02, 0x10, 0x98, 0x90, TAB6);
		}
		else if((menu->Screen.selection == 1) && (button_flag  < 2))
		{
					pic_clear_u();				
					pic_u(0x02, 0x10, 0x9b, 0x90, TAB6);
		}
		else if((menu->Screen.selection == 2) && (button_flag  < 2))
		{
					pic_clear_u();			
					pic_u(0x02, 0x10, 0x9e, 0x90, TAB6);
		}
		p = d;
		for(i = 0; i < 8; i++)
		{
					OLED_W_Command(0x98 + i);
					for(j = 0; j < 2; j++)
					{
								OLED_W_Data(*p);
								p++;
					}
		}

//		if(menu->Screen.selection == 0)
//		{
//					pic_clear_d();				/*�ϰ�������*/
//					pic_d(0x02, 0x20, 0x98, 0x98, TAB6);
//			
//					OLED_W_Command(0x98);		/*˲*/
//					OLED_W_Data(0xb0);
//					OLED_W_Data(0xa2);
//			
//					OLED_W_Command(0x99);		/*ʱ*/
//					OLED_W_Data(0xb0);
//					OLED_W_Data(0xa3);
//			
//					OLED_W_Command(0x9b);		/*˲*/
//					OLED_W_Data(0xb0);
//					OLED_W_Data(0xa2);
//			
//					OLED_W_Command(0x9c);		/*ʱ*/
//					OLED_W_Data(0xb0);
//					OLED_W_Data(0xa3);
//			
//					OLED_W_Command(0x9e);		/*˲*/
//					OLED_W_Data(0xb0);
//					OLED_W_Data(0xa2);
//			
//					OLED_W_Command(0x9f);		/*ʱ*/
//					OLED_W_Data(0xb0);
//					OLED_W_Data(0xa3);
//		}
//		else if(menu->Screen.selection == 1)
//		{
//					OLED_W_Command(0x98);		/*˲*/
//					OLED_W_Data(0xb0);
//					OLED_W_Data(0xa2);
//			
//					OLED_W_Command(0x99);		/*ʱ*/
//					OLED_W_Data(0xb0);
//					OLED_W_Data(0xa3);
//			
//					pic_clear_d();				
//					pic_d(0x02, 0x20, 0x9b, 0x98, TAB6);
//			
//					OLED_W_Command(0x9b);		/*˲*/
//					OLED_W_Data(0xb0);
//					OLED_W_Data(0xa2);
//			
//					OLED_W_Command(0x9c);		/*ʱ*/
//					OLED_W_Data(0xb0);
//					OLED_W_Data(0xa3);
//			
//					OLED_W_Command(0x9e);		/*˲*/
//					OLED_W_Data(0xb0);
//					OLED_W_Data(0xa2);
//			
//					OLED_W_Command(0x9f);		/*ʱ*/
//					OLED_W_Data(0xb0);
//					OLED_W_Data(0xa3);
//		}
//		
//		else if(menu->Screen.selection == 2)
//		{
//					OLED_W_Command(0x98);		/*˲*/
//					OLED_W_Data(0xb0);
//					OLED_W_Data(0xa2);
//			
//					OLED_W_Command(0x99);		/*ʱ*/
//					OLED_W_Data(0xb0);
//					OLED_W_Data(0xa3);
//			
//					OLED_W_Command(0x9b);		/*˲*/
//					OLED_W_Data(0xb0);
//					OLED_W_Data(0xa2);
//			
//					OLED_W_Command(0x9c);		/*ʱ*/
//					OLED_W_Data(0xb0);
//					OLED_W_Data(0xa3);
//			
//					pic_clear_d();				
//					pic_d(0x02, 0x20, 0x9e, 0x98, TAB6);
//			
//					OLED_W_Command(0x9e);		/*˲*/
//					OLED_W_Data(0xb0);
//					OLED_W_Data(0xa2);
//			
//					OLED_W_Command(0x9f);		/*ʱ*/
//					OLED_W_Data(0xb0);
//					OLED_W_Data(0xa3);
//		}
		/* Determine the next Menu */
		switch(menu->Screen.selection)
		{
				case 0:   /* Data Menu */
					menu->next=&DatarMenu;
					break;
				case 1:   /* Communication Menu */
					menu->next=&DemandMenu;
					break;
				case 2:
					menu->next=&SetMenu;
					break;
				default:
					menu->next=&DatarMenu;
					break;
		}
		
		
//		if(screen_time)
//		{
//					OLED_Refresh_Gram_Main();
//		}
//		else
//		{
//					OLED_Refresh_Gram();
//					screen_time = 1;
//		}
}

//�����Դ浽���˵�		 
static void OLED_Refresh_Gram_Main(void)
{
			uint8_t i = 0, j = 0;
			/*ˢ��*/
			for(i = 0; i < 16; i++)
			{
						OLED_W_Command(0x34);							/*ʹ����չָ�����ͼ��ʾ��*/
						OLED_W_Command(0x90+i);						/*���û�ͼ��Y��ַ���ϰ���һ����32�м�0-31��(0x80+x)�������ڵ�x�л�ͼ������0x90=0x80+x����x=0x10�������ڵ�16�п�ʼ��ͼ���°���ͬ��*/
						OLED_W_Command(0x80 + 5);					/*���û�ͼ��X��ַ���е�ַֻ��2����0x80�������ϰ�����ͼ��0x88�������°�����ͼ���ϰ�����0-7�����һ�е��ڰ��У���(0x80+0)-(0x80+7)*/
						OLED_W_Command(0x30);							/*ʹ�û���ָ�*/
				
						OLED_W_Data(OLED_GRAM[i + 16][10]);
						OLED_W_Data(OLED_GRAM[i + 16][11]);
			}
			
			/*ˢ����*/
			for(i = 0; i < 16; i++)
			{
						OLED_W_Command(0x34);							/*ʹ����չָ�����ͼ��ʾ��*/
						OLED_W_Command(0x80+i);						/*���û�ͼ��Y��ַ���ϰ���һ����32�м�0-31��(0x80+x)�������ڵ�x�л�ͼ������0x90=0x80+x����x=0x10�������ڵ�16�п�ʼ��ͼ���°���ͬ��*/
						OLED_W_Command(0x88 + 4);					/*���û�ͼ��X��ַ���е�ַֻ��2����0x80�������ϰ�����ͼ��0x88�������°�����ͼ���ϰ�����0-7�����һ�е��ڰ��У���(0x80+0)-(0x80+7)*/
						OLED_W_Command(0x30);							/*ʹ�û���ָ�*/
				
						OLED_W_Data(OLED_GRAM[i + 32][8]);
						OLED_W_Data(OLED_GRAM[i + 32][9]);
						OLED_W_Data(OLED_GRAM[i + 32][10]);
						OLED_W_Data(OLED_GRAM[i + 32][11]);
						OLED_W_Data(OLED_GRAM[i + 32][12]);
						OLED_W_Data(OLED_GRAM[i + 32][13]);
			}
			
//			/*ˢ����*/
//			for(i = 0; i < 16; i++)
//			{
//						OLED_W_Command(0x34);							/*ʹ����չָ�����ͼ��ʾ��*/
//						OLED_W_Command(0x80+i + 16);			/*���û�ͼ��Y��ַ���ϰ���һ����32�м�0-31��(0x80+x)�������ڵ�x�л�ͼ������0x90=0x80+x����x=0x10�������ڵ�16�п�ʼ��ͼ���°���ͬ��*/
//						OLED_W_Command(0x88);							/*���û�ͼ��X��ַ���е�ַֻ��2����0x80�������ϰ�����ͼ��0x88�������°�����ͼ���ϰ�����0-7�����һ�е��ڰ��У���(0x80+0)-(0x80+7)*/
//						OLED_W_Command(0x30);							/*ʹ�û���ָ�*/
//				
//						for(j = 0; j < 16; j++)
//						{
//									OLED_W_Data(OLED_GRAM[i + 48][j]);
//						}
//			}
			
			OLED_W_Command(0x36);				/*��ͼ��ʾ��*/
			OLED_W_Command(0x30);				/*ʹ�û���ָ�*/
}

/*˲ʱ���ݲ˵�����*/
static void DatarScreen_ButtonHandler(void *Menu,int16_t button)
{
		DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
		
		switch(button)
		{
				case ESCAPE:
					if(menu->prev)
					{
							button_flag = 0;
							CurrentDisplay=menu->prev;
							CurrentDisplay->Screen.screen_number=0;
							CurrentDisplay->Screen.selection=0;
							OLED_Clear();  /* clear screen */
					}
					break;
				case ENTER:   /* enter next menu */
					break;
			case LEFT:
					break;
			case RIGHT:
					break;
			default:
					break;
		}
}

/*˲ʱ���ݲ˵�����ʾ*/
static void DatarScreen_Display(void *Menu)
{
	
			char a[] = {"10  ����:     mm"};
			char b[] = {"60  ����:     mm"};
			char c[] = {"60�����:     mm"};
			char d[] = {"����ʱ��:    :  "};
			char *p = NULL;
			uint8_t i = 0, j = 0;
			
			p = a;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x80 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = b;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x90 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = c;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x88 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = d;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x98 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			/*��һ��*/
			memset(disp_buf, 0, sizeof(disp_buf));		
			snprintf((char *)disp_buf,sizeof(disp_buf),"%2u.%01u", rain_sample.rain_10m / 10, rain_sample.rain_10m % 10);/*ʮ������*/
			p = disp_buf;
			for(i=0;i<2;i++)
			{
						OLED_W_Command(0x85 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			/*�ڶ���*/
			memset(disp_buf, 0, sizeof(disp_buf));		
			snprintf((char *)disp_buf,sizeof(disp_buf),"%2u.%01u", rain_sample.rain_60m / 10, rain_sample.rain_60m % 10);/*60������������*/
			p = disp_buf;
			for(i=0;i<2;i++)
			{
						OLED_W_Command(0x95 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			/*������*/
			memset(disp_buf, 0, sizeof(disp_buf));		
			snprintf((char *)disp_buf,sizeof(disp_buf),"%2u.%01u", rain_sample.rain_60m_max / 10, rain_sample.rain_60m_max % 10);/*60���������������*/
			p = disp_buf;
			for(i=0;i<2;i++)
			{
						OLED_W_Command(0x8d + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			/*������*/
			memset(disp_buf, 0, sizeof(disp_buf));		
			snprintf((char *)disp_buf,sizeof(disp_buf)," %02u:%02u", rain_sample.rain_60m_max_t / 100, rain_sample.rain_60m_max_t % 100);/*Сʱ�����������ʱ��*/
			p = disp_buf;
			for(i=0;i<3;i++)
			{
						OLED_W_Command(0x9d + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
//		DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
//		memset(disp_buf, 0, sizeof(disp_buf));									/*���㻺��*/
//		/* Date */
//		snprintf((char *)disp_buf,sizeof(disp_buf),"10      : %2u.%01umm", rain_sample.rain_10m / 10, rain_sample.rain_10m % 10);/*ʮ������*/
//		OLED_ShowString(0,0,disp_buf,false);
//		OLED_China(0, 32, 4,  false);//��
//		OLED_China(0, 48, 30, false);//��

//		snprintf((char *)disp_buf,sizeof(disp_buf),"60      : %2u.%01umm", rain_sample.rain_60m / 10, rain_sample.rain_60m % 10);/*60������������*/
//		OLED_ShowString(16,0,disp_buf,false);
//		OLED_China(16,    32, 4,false);//��
//		OLED_China(16,    48, 30,false);//��
//	
//		snprintf((char *)disp_buf,sizeof(disp_buf),"60      : %2u.%01umm", rain_sample.rain_60m_max / 10, rain_sample.rain_60m_max % 10);/*60���������������*/
//		OLED_ShowString(32,0,disp_buf,false);
//		OLED_China(32,    16, 4,false);//��
//		OLED_China(32,    32, 13,false);//��
//		OLED_China(32,    48, 14,false);//��
//	
//		snprintf((char *)disp_buf,sizeof(disp_buf),"        :  %02u:%02u", rain_sample.rain_60m_max_t / 100, rain_sample.rain_60m_max_t % 100);/*Сʱ�����������ʱ��*/
//		OLED_ShowString(48,0,disp_buf,false);
//		OLED_China(48,     0, 39,false);//��
//		OLED_China(48,    16, 40,false);//��
//		OLED_China(48,    32, 3,false);//ʱ
//		OLED_China(48,    48, 18,false);//��
//		
//		OLED_Refresh_Gram();
}

/*���ݲ�ѯ�˵�����*/
static void DemandScreen_ButtonHandler(void *Menu,int16_t button)
{
			DisplayMenuTypeDef *menu = (DisplayMenuTypeDef *)Menu;
			int selected_timevalue = 0, min_value = 0, max_value = 0;
			int *selected = NULL;
			
			switch(menu->Screen.screen_leftright)
			{
					case 0:
						break;
					case 1:
						if(menu->Screen.selection == 0)
						{
								selected 						= &screen_year;
								selected_timevalue	=	screen_year;
								min_value						=	0;
								max_value						=	99;
						}
						else if(menu->Screen.selection == 1)
						{
								selected 						= &screen_month1;
								selected_timevalue	=	screen_month1;
								min_value						=	1;
								max_value						=	12;
						}
						break;
					case 2:
						if(menu->Screen.selection == 0)
						{
								selected 						= &screen_month;
								selected_timevalue	=	screen_month;
								min_value						=	1;
								max_value						=	12;
						}
						else if(menu->Screen.selection == 1)
						{
								selected 						= &screen_day1;
								selected_timevalue	=	screen_day1;
								min_value						=	1;
								max_value						=	31;
						}
						break;
					case 3:
						if(menu->Screen.selection == 0)
						{
								selected 						= &screen_day;
								selected_timevalue	=	screen_day;
								min_value						=	1;
								max_value						=	31;
						}
						else if(menu->Screen.selection == 1)
						{
								selected 						= &screen_hour1;
								selected_timevalue	=	screen_hour1;
								min_value						=	0;
								max_value						=	23;
						}
						break;
					default:
							break;
			}
		
			switch(button)
			{
						case ESCAPE:
							if(menu->prev)
							{
									button_flag = 0;
									CurrentDisplay														= menu->prev;
									CurrentDisplay->Screen.screen_number  		= 0;
									CurrentDisplay->Screen.screen_leftright  	= 0;
									CurrentDisplay->Screen.selection					= 1;
									OLED_Clear();  /* clear screen */
							}
							break;
							
						case ENTER:   /* enter next menu */
							menu->Screen.screen_leftright++;
							button_flag = 0;
							if(menu->Screen.screen_leftright > 3)
							{
									CurrentDisplay 														= menu->next;
									CurrentDisplay->Screen.screen_number  		= 0;
									CurrentDisplay->Screen.screen_leftright 	= 0;
									CurrentDisplay->Screen.selection					= 0;
									OLED_Clear();  /* clear screen */
							 }
							break;
						case DOWN:
							if(menu->Screen.screen_leftright == 0)
							{
									menu->Screen.selection--;
							}
							else
							{
									selected_timevalue--;
//									printf("DOWN\r\n");
							}
							break;
						case UP:
							if(menu->Screen.screen_leftright == 0)
							{
									menu->Screen.selection++;
							}
							else
							{
									selected_timevalue++;
//									printf("UP\r\n");
							}
							break;
						default:
							break;
			}
		
			if(menu->Screen.selection < 0)
			{
					menu->Screen.selection = 1;
			}
			else if(menu->Screen.selection > 1)
			{
					menu->Screen.selection = 0;
			}
			
			if(selected_timevalue < min_value)
			{
					selected_timevalue = max_value;
			}
			else if(selected_timevalue > max_value)
			{
					selected_timevalue = min_value;
			}
			
			if(menu->Screen.screen_leftright < 0)
			{
						menu->Screen.screen_leftright = 3;
			}
			else if(menu->Screen.screen_leftright > 3)
			{
						menu->Screen.screen_leftright = 0;
			}
			/* set selected value */
			*selected = selected_timevalue;
}

/*���ݲ�ѯ�˵�����ʾ*/
static void DemandScreen_Display(void *Menu)
{
		DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
//		uint8_t highlight = false;
	
			char a[] = {"    ���ݲ�ѯ    "};
			char b[] = {"                "};
			char c[] = {"������:   -  -  "};
			char d[] = {"ʱ����:   -  -  "};
			char *p = NULL;
			uint8_t i = 0, j = 0;
			
			p = a;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x80 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = b;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x90 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = c;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x88 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = d;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x98 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			/*������*/
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf((char *)disp_buf,sizeof(disp_buf),"%02u-%02u-%02u", screen_year, screen_month, screen_day);
			p = disp_buf;
			for(i=0;i<4;i++)
			{
						OLED_W_Command(0x8c + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			/*������*/
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf((char *)disp_buf,sizeof(disp_buf),"%02u-%02u-%02u", screen_month1, screen_day1, screen_hour1);
			p = disp_buf;
			for(i=0;i<4;i++)
			{
						OLED_W_Command(0x9c + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			/*����*/
			if((menu->Screen.selection == 0) && (menu->Screen.screen_leftright == 0) && (button_flag < 2))
			{
						pic_clear_u();				
						pic_u(0x03, 0x10, 0x88, 0x80, TAB6);
			}
			else if((menu->Screen.selection == 0) && (menu->Screen.screen_leftright == 1)  && (button_flag < 2))
			{
						pic_clear_u();				
						pic_u(0x01, 0x10, 0x8c, 0x80, TAB6);
			}
			else if((menu->Screen.selection == 0) && (menu->Screen.screen_leftright == 2)  && (button_flag < 2))
			{
						pic_clear_u();				
						pic_u(0x01, 0x10, 0x8d, 0x80, TAB8);
						pic_u(0x01, 0x10, 0x8e, 0x80, TAB7);
			}
			else if((menu->Screen.selection == 0) && (menu->Screen.screen_leftright == 3)  && (button_flag < 2))
			{
						pic_clear_u();				
						pic_u(0x01, 0x10, 0x8f, 0x80, TAB6);
			}
			else if((menu->Screen.selection == 1) && (menu->Screen.screen_leftright == 0)  && (button_flag < 2))
			{
						pic_clear_u();				
						pic_u(0x03, 0x10, 0x98, 0x90, TAB6);
			}
			else if((menu->Screen.selection == 1) && (menu->Screen.screen_leftright == 1)  && (button_flag < 2))
			{
						pic_clear_u();				
						pic_u(0x01, 0x10, 0x9c, 0x90, TAB6);
			}
			else if((menu->Screen.selection == 1) && (menu->Screen.screen_leftright == 2)  && (button_flag < 2))
			{
						pic_clear_u();				
						pic_u(0x01, 0x10, 0x9d, 0x90, TAB8);
						pic_u(0x01, 0x10, 0x9e, 0x90, TAB7);
			}
			else if((menu->Screen.selection == 1) && (menu->Screen.screen_leftright == 3)  && (button_flag < 2))
			{
						pic_clear_u();				
						pic_u(0x01, 0x10, 0x9f, 0x90, TAB6);
			}
			
//		/*ÿһ��ȫ��д��*/
//		memset(disp_buf, 0, sizeof(disp_buf));
//		snprintf((char *)disp_buf,sizeof(disp_buf),"                ");
//		OLED_ShowString(0, 0,disp_buf,false);		
//		OLED_ShowString(16, 0,disp_buf,false);
//	
//		OLED_China(0, 32, 6,false);//��
//		OLED_China(0, 48, 7,false);//��
//		OLED_China(0, 64, 26,false);//��
//		OLED_China(0, 80, 27,false);//ѯ

//		snprintf((char *)disp_buf,sizeof(disp_buf),"      : %02u-%02u-%02u", screen_year, screen_month, screen_day);
//		OLED_ShowString(32, 0,disp_buf,false);
//	
//		highlight = ((menu->Screen.selection == 0) & (menu->Screen.screen_leftright == 0));
//		OLED_China(32,      0, 2, highlight);//��
//		OLED_China(32,     16, 6, highlight);//��
//	  OLED_China(32,     32, 7, highlight);//��
//	
//		highlight = ((menu->Screen.selection == 0) & (menu->Screen.screen_leftright == 1));
//		snprintf(disp_buf, sizeof(disp_buf), "%02u", screen_year);		/*��*/
//		OLED_ShowString(32, 64,disp_buf,highlight);
//	
//		highlight = ((menu->Screen.selection == 0) & (menu->Screen.screen_leftright == 2));
//		snprintf(disp_buf, sizeof(disp_buf), "%02u", screen_month);		/*��*/
//		OLED_ShowString(32, 88,disp_buf,highlight);
//		
//		highlight = ((menu->Screen.selection == 0) & (menu->Screen.screen_leftright == 3));
//		snprintf(disp_buf, sizeof(disp_buf), "%02u", screen_day);		/*��*/
//		OLED_ShowString(32, 112,disp_buf,highlight);
//	
//		snprintf((char *)disp_buf,sizeof(disp_buf),"      : %02u-%02u-%02u", screen_month1, screen_day1, screen_hour1);
//		OLED_ShowString(48, 0,disp_buf,false);
//	
//		highlight = ((menu->Screen.selection == 1) & (menu->Screen.screen_leftright == 0));
//		OLED_China(48,      0, 3, highlight);//ʱ
//		OLED_China(48,     16, 6, highlight);//��
//		OLED_China(48,     32, 7, highlight);//��
//	
//		highlight = ((menu->Screen.selection == 1) & (menu->Screen.screen_leftright == 1));
//		snprintf(disp_buf, sizeof(disp_buf), "%02u", screen_month1);		/*��*/
//		OLED_ShowString(48, 64,disp_buf,highlight);
//	
//		highlight = ((menu->Screen.selection == 1) & (menu->Screen.screen_leftright == 2));
//		snprintf(disp_buf, sizeof(disp_buf), "%02u", screen_day1);		/*��*/
//		OLED_ShowString(48, 88,disp_buf,highlight);
//		
//		highlight = ((menu->Screen.selection == 1) & (menu->Screen.screen_leftright == 3));
//		snprintf(disp_buf, sizeof(disp_buf), "%02u", screen_hour1);		/*��*/
//		OLED_ShowString(48, 112,disp_buf,highlight);
		
		/* Determine the next Menu */
		switch(menu->Screen.selection)
		{
				case 0:   /* Data Menu */
					menu->next = &DayMenu;
					break;
				case 1:   /* Communication Menu */
					menu->next = &HourMenu;
					break;
				default:
					menu->next = &DayMenu;
					break;
		}
		
//		OLED_Refresh_Gram();
}

/*�����ݲ�ѯ�˵�����*/
static void DayScreen_ButtonHandler(void *Menu,int16_t button)
{
		DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
		
		switch(button)
		{
				case ESCAPE:
					key_flag = 0;
					if(menu->prev)
					{
							button_flag = 0;
							screen_rain = 0;
							CurrentDisplay=menu->prev;
							CurrentDisplay->Screen.screen_number=0;
							CurrentDisplay->Screen.selection=0;
							OLED_Clear();  /* clear screen */
					}
					break;
				case ENTER:   /* enter next menu */
					button_flag = 0;
					key_flag = 1;
					if(menu->Screen.selection == 0)
					{
							SubaDay(&screen_year, &screen_month, &screen_day, 0, 0, 0);
							if(usart_debug)
							{
									printf("time:%d-%d-%d\r\n",screen_year, screen_month, screen_day);
							}
					}
					else if(menu->Screen.selection == 1)
					{
							AddaDay(&screen_year, &screen_month, &screen_day, 0, 0, 0);
							if(usart_debug)
							{
									printf("time:%d-%d-%d\r\n",screen_year, screen_month, screen_day);
							}
					}
					break;
			case LEFT:
					key_flag = 0;
					menu->Screen.selection--;
					break;
			case RIGHT:
					key_flag = 0;
					menu->Screen.selection++;
					break;
			default:
					key_flag = 0;
					break;
		}
		
		if(key_flag == 1)
		{
				key_flag = 0;
				/*��������֮�󣬿���ȷ��������ʾ��һ���������*/
				if((screen_month > Date.Month) || ((screen_month == Date.Month) && ((screen_day - Date.Date) > 2)) || ((screen_month == Date.Month) && ((screen_day - Date.Date) == 1) && (Time.Hours < 20)))		/*Ҫ��ѯ��ʱ����ڵ�ǰ��ʱ��*/
				{
						screen_rain = 0;
				}
				else if(((screen_month == Date.Month) && (screen_day == Date.Date) && (Time.Hours < 20)) || ((screen_month == Date.Month) && ((screen_day - Date.Date) == 1) && (Time.Hours >= 20)))			/*Ҫ��ѯ��ʱ����ǵ�ǰ��ʱ��*/
				{
						screen_rain = rain_sample.rain_d;							/*��ʾ���������͵��ڵ�ǰ��������*/
				}
				else																							/*��ѯ��ǰ��������*/
				{
						screen_rain = 0;
						memset(data_buf, 0 , sizeof(data_buf));
						if(((Time.Minutes == 0) && (Time.Seconds < 5)) || ((Time.Minutes == 59) && (Time.Seconds > 55)))
						{
								osDelay(10000);					/*�����ȴ�10����*/
						}
						else 
						{
								if(read_file_hour(screen_month, screen_day, 20, data_buf))
								{
											screen_rain = (data_buf[149] - 48) * 1000 + (data_buf[150] - 48) * 100 + (data_buf[151] - 48) * 10 + (data_buf[152] - 48);
											if(usart_debug)
											{
														printf("screen_rain=%d\r\n", screen_rain);
											}
								}
								else
								{
										if(usart_debug)
											{
														printf("��ѯ����ʧ��\r\n");
											}
								}
						}
				}
		}
		else
		{
				if(usart_debug)
				{
							printf("key_flag=%d\r\n", key_flag);
				}
		}
		
		
		if(menu->Screen.selection < 0)
		{
				menu->Screen.selection = 1;
		}
		else if(menu->Screen.selection > 1)
		{
				menu->Screen.selection = 0;
		}
}

/*�����ݲ�ѯ�˵�����ʾ*/
static void DayScreen_Display(void *Menu)
{
		DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
//		uint8_t highlight=false;
  
			char a[] = {"    ��  ��  ��  "};
			char b[] = {"                "};
			char c[] = {"����:         mm"};
			char d[] = {"��һ��    ��һ��"};
			char *p = NULL;
			uint8_t i = 0, j = 0;
			
			p = a;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x80 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = b;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x90 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = c;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x88 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = d;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x98 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			/*��һ��*/
			memset(disp_buf, 0, sizeof(disp_buf));	/*��*/
			snprintf(disp_buf,sizeof(disp_buf),"%02u", screen_year);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x81 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			memset(disp_buf, 0, sizeof(disp_buf));  /*��*/
			snprintf(disp_buf,sizeof(disp_buf),"%02u", screen_month);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x83 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			memset(disp_buf, 0, sizeof(disp_buf));  /*��*/
			snprintf(disp_buf,sizeof(disp_buf),"%02u", screen_day);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x85 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			/*������*/
			memset(disp_buf, 0, sizeof(disp_buf));	/*��*/
			snprintf((char *)disp_buf,sizeof(disp_buf)," %3u.%01u", screen_rain / 10, screen_rain % 10);
			p = disp_buf;
			for(i=0;i<3;i++)
			{
						OLED_W_Command(0x8c + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			/*����*/
			if((menu->Screen.selection == 0) && (button_flag < 2)) 
			{
						pic_clear_u();				
						pic_u(0x03, 0x10, 0x98, 0x90, TAB6);
			}
			else if((menu->Screen.selection == 1) && (button_flag < 2)) 
			{
						pic_clear_u();				
						pic_u(0x03, 0x10, 0x9d, 0x90, TAB6);
			}
	
//		memset(disp_buf, 0 , sizeof(disp_buf));
//		snprintf(disp_buf,sizeof(disp_buf),"                ");
//		OLED_ShowString(0, 0,disp_buf,false);
//		OLED_China(0, 40, 2, false);//��
//		OLED_China(0, 56, 6,false);//��
//		OLED_China(0, 72, 7,false);//��
//		
//		snprintf(disp_buf,sizeof(disp_buf),"  %02u  %02u  %02u    ", screen_year, screen_month, screen_day);
//		OLED_ShowString(0, 0,disp_buf,false);
//		OLED_China(0,     32,  0, false);//��
//		OLED_China(0,     64,  1, false);//��
//		OLED_China(0,     96, 2, false);//��
//	
//		snprintf((char *)disp_buf,sizeof(disp_buf),"    :    %3u.%01umm", screen_rain / 10, screen_rain % 10);
//		OLED_ShowString(32, 0,disp_buf,false);
//		OLED_China(32,      0, 19, false);//��
//		OLED_China(32,     16, 20, false);//��
//	
//		snprintf((char *)disp_buf,sizeof(disp_buf),"                ");
//		OLED_ShowString(48, 0,disp_buf,false);
//		highlight = (menu->Screen.selection == 0);
//		OLED_China(48,      8, 31, highlight);//��
//		OLED_China(48,     24, 33, highlight);//һ
//	  OLED_China(48,     40, 2, highlight);//��
//	
//		highlight = (menu->Screen.selection == 1);
//		OLED_China(48,     72, 32, highlight);//��
//		OLED_China(48,     88, 33, highlight);//һ
//		OLED_China(48,    104, 2, highlight);//��
//		
//		OLED_Refresh_Gram();
}

/*Сʱ���ݲ�ѯ�˵�����*/
static void HourScreen_ButtonHandler(void *Menu,int16_t button)
{
		DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
	  
		switch(button)
		{
				case ESCAPE:
					key_flag = 0;
					if(menu->prev)
					{
							screen_rain 													= 0;
							CurrentDisplay 												= menu->prev;
							CurrentDisplay->Screen.screen_number	=	0;
							CurrentDisplay->Screen.selection			=	1;
							OLED_Clear();  /* clear screen */
					}
					break;
				case ENTER:   /* enter next menu */
					key_flag = 1;
					if(menu->Screen.selection == 0)
					{
							SubaHour(0, &screen_month1, &screen_day1, &screen_hour1, 0, 0);
							if(usart_debug)
							{
									printf("time:%d-%d-%d\r\n",screen_month1, screen_day1, screen_hour1);
							}
					}
					else if(menu->Screen.selection == 1)
					{
							AddaHour(0, &screen_month1, &screen_day1, &screen_hour1, 0, 0);
							if(usart_debug)
							{
									printf("time:%d-%d-%d\r\n",screen_month1, screen_day1, screen_hour1);
							}
					}
					break;
				case LEFT:
						key_flag = 0;
						menu->Screen.selection--;
						break;
				case RIGHT:
						key_flag = 0;
						menu->Screen.selection++;
						break;
				default:
						key_flag = 0;
						break;
		}
		
		if(key_flag)
		{
				/*��������֮�󣬿���ȷ��������ʾ��һ���������*/
				if((screen_month1 > Date.Month) || ((screen_month1 == Date.Month) && (screen_day1 == Date.Date) && (screen_hour1 > Time.Hours)))		/*Ҫ��ѯ��ʱ����ڵ�ǰ��ʱ��*/
				{
						screen_rain = 0;
				}
		//		else if((screen_month1 == Date.Month) && (screen_day1 == Date.Date) && (screen_hour1 == Time.Hours))			/*Ҫ��ѯ��ʱ����ǵ�ǰ��ʱ��*/
		//		{
		//				screen_rain = rain_sample.rain_60m;							/*��ʾ��ʱ�����͵��ڵ�ǰ��ʱ����*/
		//		}
				else																							/*��ѯ��ǰ��ʱ����*/
				{
						screen_rain = 0;
						memset(data_buf, 0 , sizeof(data_buf));
						if(((Time.Minutes == 0) && (Time.Seconds < 5)) || ((Time.Minutes == 59) && (Time.Seconds > 55)))
							{
									osDelay(10000);					/*�����ȴ�10����*/
							}
							else
							{
									if(read_file_hour(screen_month1, screen_day1, screen_hour1, data_buf))
									{
												screen_rain = (data_buf[145] - 48) * 1000 + (data_buf[146] - 48) * 100 + (data_buf[147] - 48) * 10 + (data_buf[148] - 48);
												if(usart_debug)
												{
															printf("screen_rain=%d\r\n", screen_rain);
												}
									}
									else
									{
											if(usart_debug)
											{
														printf("��ѯ����ʧ��\r\n");
											}
									}
							}
				}
		}
		else
		{
				if(usart_debug)
				{
							printf("key_flag=%d\r\n", key_flag);
				}
		}
		
		
		
		if(menu->Screen.selection < 0)
		{
				menu->Screen.selection = 1;
		}
		else if(menu->Screen.selection > 1)
		{
				menu->Screen.selection = 0;
		}
}

/*Сʱ���ݲ�ѯ�˵�����ʾ*/
static void HourScreen_Display(void *Menu)
{
		DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
//		uint8_t highlight=false;
		
			char a[] = {"    ��  ��  ʱ  "};
			char b[] = {"                "};
			char c[] = {"����:         mm"};
			char d[] = {"��Сʱ    ��Сʱ"};
			char *p = NULL;
			uint8_t i = 0, j = 0;
			
			p = a;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x80 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = b;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x90 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = c;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x88 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = d;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x98 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			/*��һ��*/
			memset(disp_buf, 0, sizeof(disp_buf));	/*��*/
			snprintf(disp_buf,sizeof(disp_buf),"%02u", screen_month1);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x81 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			memset(disp_buf, 0, sizeof(disp_buf));  /*��*/
			snprintf(disp_buf,sizeof(disp_buf),"%02u", screen_day1);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x83 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			memset(disp_buf, 0, sizeof(disp_buf));  /*ʱ*/
			snprintf(disp_buf,sizeof(disp_buf),"%02u", screen_hour1);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x85 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			/*������*/
			memset(disp_buf, 0, sizeof(disp_buf));	
			snprintf((char *)disp_buf,sizeof(disp_buf)," %3u.%01u", screen_rain / 10, screen_rain % 10);
			p = disp_buf;
			for(i=0;i<3;i++)
			{
						OLED_W_Command(0x8c + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			/*����*/
			if((menu->Screen.selection == 0) && (button_flag < 2))
			{
						pic_clear_u();				
						pic_u(0x03, 0x10, 0x98, 0x90, TAB6);
			}
			else if((menu->Screen.selection == 1)  && (button_flag < 2))
			{
						pic_clear_u();				
						pic_u(0x03, 0x10, 0x9d, 0x90, TAB6);
			}
			
//		memset(disp_buf, 0, sizeof(disp_buf));
//		snprintf(disp_buf,sizeof(disp_buf),"                ");
//		OLED_ShowString(0, 0,disp_buf,false);
//		OLED_ShowString(16, 0,disp_buf,false);
//		OLED_ShowString(32, 0,disp_buf,false);
//		OLED_ShowString(48, 0,disp_buf,false);
//		OLED_China(0, 32, 41, false);//С
//		OLED_China(0, 48, 3, false);//ʱ
//		OLED_China(0, 64, 6, false);//��
//		OLED_China(0, 80, 7, false);//��
//		
//		memset(disp_buf, 0, sizeof(disp_buf));
//		snprintf(disp_buf,sizeof(disp_buf),"  %02u  %02u  %02u    ", screen_month1, screen_day1, screen_hour1);
//		OLED_ShowString(0, 0,disp_buf,false);
//		OLED_China(0,     32,  1, false); //��
//		OLED_China(0,     64, 2, false);	//��
//		OLED_China(0, 		 96, 3, false); //ʱ
//	
//		snprintf(disp_buf, sizeof(disp_buf),"    :    %3u.%01umm", screen_rain / 10, screen_rain % 10);
//		OLED_ShowString(32, 0,disp_buf,false);
//		OLED_China(32,      0, 19, false);//��
//		OLED_China(32,     16, 20, false);//��
//	
//		highlight = (menu->Screen.selection == 0);
//		OLED_China(48,      8, 31, highlight);//��
//		OLED_China(48,     24, 41, highlight);//С
//	  OLED_China(48,     40, 3, highlight); //ʱ
//	
//		highlight = (menu->Screen.selection == 1);
//		OLED_China(48,     72, 32, highlight);//��
//		OLED_China(48,     88, 41, highlight);//С
//		OLED_China(48,    104, 3, highlight); //ʱ
//		
//		OLED_Refresh_Gram();
}


/*���ò˵�����*/
static void SetScreen_ButtonHandler(void *Menu,int16_t button)
{
		DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
		
		switch(button)
		{
				case ESCAPE:
					if(menu->prev)
					{
							CurrentDisplay 												=	menu->prev;
							CurrentDisplay->Screen.screen_number	=	0;
							CurrentDisplay->Screen.selection			=	2;
							OLED_Clear();  /* clear screen */
					}
					break;
				case ENTER:   /* enter next menu */
					if(menu->next)
					{
							CurrentDisplay												=	menu->next;
							CurrentDisplay->Screen.screen_number	=	0;
							CurrentDisplay->Screen.selection			=	0;
							OLED_Clear();  /* clear screen */
  
							if(menu->Screen.selection == 0)
							{
										station_set.station_1 = rain_sample.station / 10000;
										station_set.station_2 = (rain_sample.station % 10000) / 1000;
										station_set.station_3 = (rain_sample.station % 1000) / 100;
										station_set.station_4 = (rain_sample.station % 100) / 10;
										station_set.station_5 = rain_sample.station % 10;
							}
							else if(menu->Screen.selection == 1)
							{
										warn_set.warn_1  = rain_sample.rain_60m_warn  / 100;
										warn_set.warn_2 = (rain_sample.rain_60m_warn % 100) / 10;
										warn_set.warn_3 =  rain_sample.rain_d_warn / 100;
										warn_set.warn_4 = (rain_sample.rain_d_warn % 100) / 10;
							}
							else if(menu->Screen.selection == 2)
							{
									/* get system time */
									get_sys_time(&setDate, &setTime);
								
									nian   =  setDate.Year;
									yue    =  setDate.Month;
									ri     =  setDate.Date;
									shi    =  setTime.Hours;
									fen    =  setTime.Minutes;
									miao   =  setTime.Seconds;
							}
					}
					break;
			case DOWN:
					menu->Screen.selection--;
					break;
			case UP:
					menu->Screen.selection++;
					break;
			default:
					break;
		}
		
		if(menu->Screen.selection < 0)
		{
				menu->Screen.selection = 2;
		}
		else if(menu->Screen.selection > 2)
		{
				menu->Screen.selection = 0;
		}
}

/*���ò˵�����ʾ*/
static void SetScreen_Display(void *Menu)
{
		DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
//		uint8_t highlight=false;
	
			char a[] = {"      ����      "};
			char b[] = {"    վ������    "};
			char c[] = {"    ��������    "};
			char d[] = {"    ʱ������    "};
			char *p = NULL;
			uint8_t i = 0, j = 0;
			
			p = a;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x80 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = b;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x90 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = c;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x88 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = d;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x98 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
		
			/*����*/
			if((menu->Screen.selection == 0) && (button_flag < 2))
			{
						pic_clear_u();				
						pic_u(0x04, 0x10, 0x92, 0x90, TAB6);
			}
			else if((menu->Screen.selection == 1)  && (button_flag < 2))
			{
						pic_clear_u();				
						pic_u(0x04, 0x10, 0x8a, 0x80, TAB6);
			}
			else if((menu->Screen.selection == 2) && (button_flag < 2)) 
			{
						pic_clear_u();				
						pic_u(0x04, 0x10, 0x9a, 0x90, TAB6);
			}
			
//		OLED_China(0, 48, 28, false);//��
//		OLED_China(0, 64, 29, false);//��
//	
//		highlight = (menu->Screen.selection == 0);
//		OLED_China(16, 32, 34, highlight);//վ
//		OLED_China(16, 48, 35, highlight);//��
//		OLED_China(16, 64, 28, highlight);//��
//		OLED_China(16, 80, 29, highlight);//��
//	
//		highlight = (menu->Screen.selection == 1);
//		OLED_China(32, 32, 23, highlight);//��
//		OLED_China(32, 48, 24, highlight);//��
//		OLED_China(32, 64, 28, highlight);//��
//		OLED_China(32, 80, 29, highlight);//��
//	
//		highlight = (menu->Screen.selection == 2);
//		OLED_China(48, 32, 3, highlight);//ʱ
//		OLED_China(48, 48, 18, highlight);//��
//		OLED_China(48, 64, 28, highlight);//��
//		OLED_China(48, 80, 29, highlight);//��
  
		/* Determine the next Menu */
		switch(menu->Screen.selection)
		{
				case 0:   /* station set menu*/
					menu->next = &StationMenu;
					break;
				case 1:   /*  warn set  Menu */
					menu->next = &WarnMenu;
					break;
				case 2:   /* timer set Menu */
					menu->next = &TimerMenu;
					break;
				default:
					menu->next = &StationMenu;
					break;
		}
		
//		OLED_Refresh_Gram();
}

/*վ��������*/
static void StationScreen_ButtonHandler(void *Menu,int16_t button)
{
  DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
  int16_t selected_timevalue=0,min_value=0,max_value=0;
  int *selected=NULL;
	
  switch(menu->Screen.selection)
  {
			case 0:  
				break;
			case 1:  
				selected           = &station_set.station_1 ;
				selected_timevalue = station_set.station_1;
				min_value          = 0;
				max_value          = 9;
				break;
			case 2:  /* day */
				selected           = &station_set.station_2 ;
				selected_timevalue = station_set.station_2;
				min_value          = 0;
				max_value          = 9;
				break;
			case 3:  /* hour */
				selected           = &station_set.station_3 ;
				selected_timevalue = station_set.station_3;
				min_value          = 0;
				max_value          = 9;
				break;
			case 4:  /* minute */
				selected           = &station_set.station_4 ;
				selected_timevalue = station_set.station_4;
				min_value          = 0;
				max_value          = 9;
				break;
			case 5:  /* second */
				selected           = &station_set.station_5 ;
				selected_timevalue = station_set.station_5;
				min_value          = 0;
				max_value          = 9;
				break;
			default:
				selected           = &station_set.station_5 ;
				selected_timevalue = station_set.station_5;
				min_value          = 0;
				max_value          = 9;
				break;
  }
	
	rain_sample.station = station_set.station_1 * 10000 + station_set.station_2 * 1000 + station_set.station_3 * 100 + station_set.station_4 * 10 + station_set.station_5;
  
  switch(button)
  {
			case ESCAPE:
				if(menu->prev)
					{
						rain_sample.station = station_set.station_1 * 10000 + station_set.station_2 * 1000 + station_set.station_3 * 100 + station_set.station_4 * 10 + station_set.station_5  ;
						if(data_eeprom_write(STATION_ADDR, 	(uint8_t *)&rain_sample.station,				4) == HAL_OK)
						{
								if(usart_debug)
								{
										printf("write STATION_ADDR ok=%d\r\n", rain_sample.station);
								}
						}
						else
						{
								if(usart_debug)
								{
										printf("write STATION_ADDR failed\r\n");
								}
						}
						CurrentDisplay 												= menu->prev;
						CurrentDisplay->Screen.screen_number	=	0;
						CurrentDisplay->Screen.selection			=	0;
						OLED_Clear();  /* clear screen */
					}
				break;
			case ENTER:
				menu->Screen.selection++;
				break;
			case UP:
				selected_timevalue--;
				break;
			case DOWN:
				selected_timevalue++;
				break;
			default:
				break;
  }
  
	
  if(menu->Screen.selection<0)
  {
    menu->Screen.selection = 5;
  }
  else if(menu->Screen.selection > 5)
  {
    menu->Screen.selection = 0;
  }
  
  if(selected_timevalue < min_value)
  {
    selected_timevalue = max_value;
  }
  else if(selected_timevalue > max_value)
  {
    selected_timevalue = min_value;
  }
  /* set selected value */
  *selected=selected_timevalue;
}

/*վ������ʾ*/
static void StationScreen_Display(void *Menu)
{
		DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
//		uint8_t highlight=false;
		
			char a[] = {"    վ������    "};
			char b[] = {"                "};
			char c[] = {"̨վ��:         "};
			char d[] = {"                "};
			char *p = NULL;
			uint8_t i = 0, j = 0;
			
			p = a;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x80 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = b;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x90 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = c;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x88 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = d;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x98 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
		
			/*������*/
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf((char *)disp_buf,sizeof(disp_buf)," %01u%01u%01u%01u%01u", station_set.station_1, \
			station_set.station_2, station_set.station_3, station_set.station_4, station_set.station_5);
			p = disp_buf;
			for(i=0; i<3; i++)
			{
						OLED_W_Command(0x8d + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}

/*			
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf((char *)disp_buf,sizeof(disp_buf)," %01u", station_set.station_2);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x8c + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf((char *)disp_buf,sizeof(disp_buf)," %01u", station_set.station_3);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x8d + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf((char *)disp_buf,sizeof(disp_buf)," %01u", station_set.station_4);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x8e + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf((char *)disp_buf,sizeof(disp_buf)," %01u", station_set.station_5);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x8f + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
*/
			
			/*����*/
			if((menu->Screen.selection == 0) && (button_flag < 2))
			{
						pic_clear_u();				
						pic_u(0x03, 0x10, 0x88, 0x80, TAB6);
			}
			else if((menu->Screen.selection == 1)  && (button_flag < 2))
			{
						pic_clear_u();				
						pic_u(0x01, 0x10, 0x8d, 0x80, TAB8);
			}
			else if((menu->Screen.selection == 2) && (button_flag < 2)) 
			{
						pic_clear_u();				
						pic_u(0x01, 0x10, 0x8e, 0x80, TAB7);
			}
			else if((menu->Screen.selection == 3)  && (button_flag < 2))
			{
						pic_clear_u();				
						pic_u(0x01, 0x10, 0x8e, 0x80, TAB8);
			}
			else if((menu->Screen.selection == 4) && (button_flag < 2)) 
			{
						pic_clear_u();				
						pic_u(0x01, 0x10, 0x8f, 0x80, TAB7);
			}
			else if((menu->Screen.selection == 5) && (button_flag < 2)) 
			{
						pic_clear_u();				
						pic_u(0x01, 0x10, 0x8f, 0x80, TAB8);
			}
			
//		OLED_China(0, 32, 34, false);//վ
//		OLED_China(0, 48, 35, false);//��
//		OLED_China(0, 64, 28, false);//��
//		OLED_China(0, 80, 29, false);//��
//		
//		OLED_Char(40,  48, ':',false);
//		highlight = (menu->Screen.selection == 0);
//		OLED_China(40,  0, 42, highlight);//̨
//		OLED_China(40, 16, 34, highlight);//վ
//		OLED_China(40, 32, 35, highlight);//��

//		highlight = (menu->Screen.selection == 1);
//		snprintf((char *)disp_buf,sizeof(disp_buf),"%01u",station_set.station_1 );
//		OLED_ShowString(40,88,disp_buf,highlight);
//		
//		highlight = (menu->Screen.selection == 2);
//		snprintf((char *)disp_buf,sizeof(disp_buf),"%01u",station_set.station_2 );
//		OLED_ShowString(40,96,disp_buf,highlight);
//		
//		highlight = (menu->Screen.selection == 3);
//		snprintf((char *)disp_buf,sizeof(disp_buf),"%01u",station_set.station_3 );
//		OLED_ShowString(40,104,disp_buf,highlight);
//		
//		highlight = (menu->Screen.selection == 4);
//		snprintf((char *)disp_buf,sizeof(disp_buf),"%01u",station_set.station_4 );
//		OLED_ShowString(40,112,disp_buf,highlight);
//		
//		highlight = (menu->Screen.selection == 5);
//		snprintf((char *)disp_buf,sizeof(disp_buf),"%01u",station_set.station_5 );
//		OLED_ShowString(40,120,disp_buf,highlight);
//		
//		OLED_Refresh_Gram();
}	


/*����������*/
static void WarnScreen_ButtonHandler(void *Menu,int16_t button)
{
		DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
		int16_t selected_timevalue=0,min_value=0,max_value=0;
		int *selected=NULL;
		
		switch(menu->Screen.screen_number)
		{
				case 0:  
					break;
				case 1:  
					if(menu->Screen.selection == 0)
					{
							selected           = &warn_set.warn_3;
							selected_timevalue = warn_set.warn_3;
							min_value          = 0;
							max_value          = 9;
					}
					else if(menu->Screen.selection == 1)
					{
							selected           = &warn_set.warn_1;
							selected_timevalue = warn_set.warn_1;
							min_value          = 0;
							max_value          = 9;
					}
					break;
				case 2:  
					if(menu->Screen.selection == 0)
					{
							selected           = &warn_set.warn_4;
							selected_timevalue = warn_set.warn_4;
							min_value          = 0;
							max_value          = 9;
					}
					else if(menu->Screen.selection == 1)
					{
							selected           = &warn_set.warn_2;
							selected_timevalue = warn_set.warn_2;
							min_value          = 0;
							max_value          = 9;
					}
					break;
				default:
					break;
		}
	
		rain_sample.rain_60m_warn = warn_set.warn_1 * 100 + warn_set.warn_2 * 10;
		rain_sample.rain_d_warn 	= warn_set.warn_3 * 100 + warn_set.warn_4 * 10;
  
		switch(button)
		{
				case ESCAPE:
					if(menu->prev)
						{
								if(data_eeprom_write(WARN_60M_ADDR, 	(uint8_t *)&rain_sample.rain_60m_warn, 	2) == HAL_OK)
								{
										if(usart_debug)
										{
											printf("write WARN_60M_ADDR ok=%d\r\n", rain_sample.rain_60m_warn);
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
													printf("write WARN_1D_ADDR ok=%d\r\n", rain_sample.rain_d_warn);
											}	
								}
								else
								{
										if(usart_debug)
											{
													printf("write WARN_1D_ADDR failed\r\n");
											}	
								}
							
								CurrentDisplay = menu->prev;
								CurrentDisplay->Screen.screen_number=0;
								CurrentDisplay->Screen.selection=1;
								OLED_Clear();  /* clear screen */
						}
					break;
				case ENTER:
					menu->Screen.screen_number++;
					break;
				case UP:
					if(menu->Screen.screen_number == 0)
					{
							menu->Screen.selection++;
					}
					else
					{
							selected_timevalue--;
					}
					break;
				case DOWN:
					if(menu->Screen.screen_number == 0)
					{
							menu->Screen.selection--;
					}
					else
					{
							selected_timevalue++;
					}
					break;
				default:
					break;
		}
  
		if(menu->Screen.selection < 0)
		{
				menu->Screen.selection = 1;
		}
		else if(menu->Screen.selection > 1)
		{
				menu->Screen.selection = 0;
		}
		
		if(menu->Screen.screen_number < 0)
		{
				menu->Screen.screen_number = 2;
		}
		else if(menu->Screen.screen_number > 2)
		{
				menu->Screen.screen_number = 0;
		}
		
		if(selected_timevalue < min_value)
		{
				selected_timevalue = max_value;
		}
		else if(selected_timevalue > max_value)
		{
				selected_timevalue = min_value;
		}
		/* set selected value */
		*selected = selected_timevalue;
}

/*��������ʾ*/
static void WarnScreen_Display(void *Menu)
{
			DisplayMenuTypeDef *menu = (DisplayMenuTypeDef *)Menu;
	
			char a[] = {"    ��������    "};
			char b[] = {"                "};
			char c[] = {"������:       mm"};
			char d[] = {"ʱ����:       mm"};
			char *p = NULL;
			uint8_t i = 0, j = 0;
			
			p = a;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x80 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = b;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x90 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = c;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x88 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = d;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x98 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
		
			/*������*/
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf((char *)disp_buf, sizeof(disp_buf), "%01u%01u",warn_set.warn_3, warn_set.warn_4);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x8e + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
/*			
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf((char *)disp_buf, sizeof(disp_buf), "%01u",warn_set.warn_4);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x8e + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
/*			
			/*������*/
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf((char *)disp_buf, sizeof(disp_buf), "%01u%01u",warn_set.warn_1, warn_set.warn_2);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x9e + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
/*			
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf((char *)disp_buf, sizeof(disp_buf), "%01u",warn_set.warn_2);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x9e + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
/*			
			/*����*/
			if((menu->Screen.selection == 0) && (menu->Screen.screen_number == 0) && (button_flag < 2))
			{
						pic_clear_u();				
						pic_u(0x03, 0x10, 0x88, 0x80, TAB6);
			}
			else if((menu->Screen.selection == 0) && (menu->Screen.screen_number == 1) && (button_flag < 2))
			{
						pic_clear_u();				
						pic_u(0x01, 0x10, 0x8e, 0x80, TAB7);
			}
			else if((menu->Screen.selection == 0) && (menu->Screen.screen_number == 2) && (button_flag < 2))
			{
						pic_clear_u();				
						pic_u(0x01, 0x10, 0x8e, 0x80, TAB8);
			}
			else if((menu->Screen.selection == 1) && (menu->Screen.screen_number == 0) && (button_flag < 2))
			{
						pic_clear_u();				
						pic_u(0x03, 0x10, 0x98, 0x90, TAB6);
			}
			else if((menu->Screen.selection == 1) && (menu->Screen.screen_number == 1) && (button_flag < 2))
			{
						pic_clear_u();				
						pic_u(0x01, 0x10, 0x9e, 0x90, TAB7);
			}
			else if((menu->Screen.selection == 1) && (menu->Screen.screen_number == 2))
			{
						pic_clear_u();				
						pic_u(0x01, 0x10, 0x9e, 0x90, TAB8);
			}
			
//		memset(disp_buf, 0, sizeof(disp_buf));
//		snprintf((char *)disp_buf, sizeof(disp_buf), "                ");
//		OLED_ShowString(0, 0, disp_buf, false);
//		OLED_ShowString(16, 0, disp_buf, false);
//	
//		DisplayMenuTypeDef *menu = (DisplayMenuTypeDef *)Menu;
//		uint8_t highlight = false;
// 
//		OLED_China(0, 32, 23, false);//��
//		OLED_China(0, 48, 24, false);//��
//		OLED_China(0, 64, 28, false);//��
//		OLED_China(0, 80, 29, false);//��
//		
//		OLED_Char(32,  48, ':',false);
//		OLED_Char(32, 112, 'm',false);
//		OLED_Char(32, 120, 'm',false);
//		highlight = ((menu->Screen.selection == 0) && (menu->Screen.screen_number == 0));
//		OLED_China(32,  0, 2, highlight);//��
//		OLED_China(32, 16, 19, highlight);//��
//		OLED_China(32, 32, 20, highlight);//��
//	
//		OLED_Char(48,  48, ':',false);
//		OLED_Char(48, 112, 'm',false);
//		OLED_Char(48, 120, 'm',false);
//		highlight = ((menu->Screen.selection == 1) && (menu->Screen.screen_number == 0));
//		OLED_China(48,  0, 3, highlight);//ʱ
//		OLED_China(48, 16, 19, highlight);//��
//		OLED_China(48, 32, 20, highlight);//��

//		/*������*/
//		highlight = ((menu->Screen.selection == 0) && (menu->Screen.screen_number == 1));
//		snprintf((char *)disp_buf, sizeof(disp_buf), "%01u",warn_set.warn_3);
//		OLED_ShowString(32, 96, disp_buf, highlight);
//		
//		highlight = ((menu->Screen.selection == 0) && (menu->Screen.screen_number == 2));
//		snprintf((char *)disp_buf, sizeof(disp_buf), "%01u", warn_set.warn_4 );
//		OLED_ShowString(32, 104, disp_buf, highlight);
//		
//		highlight = ((menu->Screen.selection == 1) && (menu->Screen.screen_number == 1));
//		snprintf((char *)disp_buf,sizeof(disp_buf),"%01u",warn_set.warn_1 );
//		OLED_ShowString(48, 96, disp_buf, highlight);
//		
//		highlight = ((menu->Screen.selection == 1) && (menu->Screen.screen_number == 2));
//		snprintf((char *)disp_buf, sizeof(disp_buf), "%01u",warn_set.warn_2);
//		OLED_ShowString(48, 104, disp_buf, highlight);
//		
//		
//		OLED_Refresh_Gram();
}	

/*ʱ��������*/
static void TiemrScreen_ButtonHandler(void *Menu,int16_t button)
{
			DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
			int16_t selected_timevalue=0,min_value=0,max_value=0;
			uint8_t *selected=NULL;
			
			switch(menu->Screen.selection)
			{
			case 0:  /* year */
				selected=&setDate.Year;
				selected_timevalue=setDate.Year;
				min_value=0;
				max_value=99;
				break;
			case 1:  /* month */
				selected=&setDate.Month;
				selected_timevalue=setDate.Month;
				min_value=1;
				max_value=12;
				break;
			case 2:  /* day */
				selected=&setDate.Date;
				selected_timevalue=setDate.Date;
				min_value=1;
				max_value=31;
				break;
			case 3:  /* hour */
				selected=&setTime.Hours;
				selected_timevalue=setTime.Hours;
				min_value=0;
				max_value=23/*59*/;   /* 16.3.23 hour:0-23 */
				break;
			case 4:  /* minute */
				selected=&setTime.Minutes;
				selected_timevalue=setTime.Minutes;
				min_value=0;
				max_value=59;
				break;
			case 5:  /* second */
				selected=&setTime.Seconds;
				selected_timevalue=setTime.Seconds;
				min_value=0;
				max_value=59;
				break;
			default:
				menu->Screen.selection=5;
				selected=&setTime.Seconds;
				selected_timevalue=setTime.Seconds;
				min_value=0;
				max_value=59;
				break;
			}
			
			switch(button)
			{
			case ESCAPE:
				if((setDate.Year!=nian)||(setDate.Month!=yue)||(setDate.Date!=ri)||(setTime.Hours!=shi)||(setTime.Minutes!=fen)||(setTime.Seconds!=miao))
				{
					/* set data&time */
					/* fill unused value */
					setTime.DayLightSaving=RTC_DAYLIGHTSAVING_NONE;
					setTime.StoreOperation=RTC_STOREOPERATION_RESET;
					setTime.SubSeconds=0;
					setTime.TimeFormat=RTC_HOURFORMAT12_AM;
					if(set_sys_time(&setDate,&setTime)<0)
						{
							OLED_Clear();
							OLED_ShowString(0,24,"set time failed!",1);
							OLED_Refresh_Gram();
						}
						else
							{
								OLED_Clear();
								OLED_ShowString(16,24,"set time ok!",1);
								OLED_Refresh_Gram();
							}
							osDelay(500);
				}
				if(menu->prev)
					{
						CurrentDisplay=menu->prev;
						CurrentDisplay->Screen.screen_number=0;
						CurrentDisplay->Screen.selection=2;
						OLED_Clear();  /* clear screen */
					}
				break;
			case ENTER:
				menu->Screen.selection++;
				break;
			case UP:
				selected_timevalue--;
				break;
			case DOWN:
				selected_timevalue++;
				break;
			default:
				break;
			}
			
			if(menu->Screen.selection<0)
			{
				menu->Screen.selection=5;
			}
			else if(menu->Screen.selection>5)
			{
				menu->Screen.selection=0;
			}
			
			if(selected_timevalue<min_value)
			{
				selected_timevalue=max_value;
			}
			else if(selected_timevalue>max_value)
			{
				selected_timevalue=min_value;
			}
			/* set selected value */
			*selected=selected_timevalue;
}

/*ʱ������ʾ*/
static void TimerScreen_Display(void *Menu)
{
			DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
//			uint8_t highlight=false;
		 
			char a[] = {"                "};
			char b[] = {"    20  ��      "};
			char c[] = {"      ��  ��    "};
			char d[] = {"    ʱ  ��  ��  "};
			char *p = NULL;
			uint8_t i = 0, j = 0;
			
			p = a;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x80 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = b;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x90 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = c;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x88 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
			
			p = d;
			for(i = 0; i < 8; i++)
			{
						OLED_W_Command(0x98 + i);
						for(j = 0; j < 2; j++)
						{
									OLED_W_Data(*p);
									p++;
						}
			}
		
			/*�ڶ���*/
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf,sizeof(disp_buf),"%02u",setDate.Year);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x93 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			/*������*/
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf,sizeof(disp_buf),"%02u",setDate.Month);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x8a + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf,sizeof(disp_buf),"%02u",setDate.Date);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x8c + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			/*������*/
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf,sizeof(disp_buf),"%02u",setTime.Hours);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x99 + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf,sizeof(disp_buf),"%02u",setTime.Minutes);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x9b + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			memset(disp_buf, 0, sizeof(disp_buf));
			snprintf(disp_buf,sizeof(disp_buf),"%02u",setTime.Seconds);
			p = disp_buf;
			for(i=0;i<1;i++)
			{
						OLED_W_Command(0x9d + i);
						OLED_W_Data(*p);
						p++;
						OLED_W_Data(*p);
						p++;
			}
			
			/*����*/
			if((menu->Screen.selection == 0) && (button_flag < 2))
			{
						pic_clear_u();				
						pic_u(0x01, 0x10, 0x93, 0x90, TAB6);
			}
			else if((menu->Screen.selection == 1) && (button_flag < 2))
			{
						pic_clear_u();				
						pic_u(0x01, 0x10, 0x8a, 0x80, TAB6);
			}
			else if((menu->Screen.selection == 2) && (button_flag < 2))
			{
						pic_clear_u();				
						pic_u(0x01, 0x10, 0x8c, 0x80, TAB6);
			}
			else if((menu->Screen.selection == 3) && (button_flag < 2))
			{
						pic_clear_u();				
						pic_u(0x01, 0x10, 0x99, 0x90, TAB6);
			}
			else if((menu->Screen.selection == 4) && (button_flag < 2))
			{
						pic_clear_u();				
						pic_u(0x01, 0x10, 0x9b, 0x90, TAB6);
			}
			else if((menu->Screen.selection == 5) && (button_flag < 2))
			{
						pic_clear_u();				
						pic_u(0x01, 0x10, 0x9d, 0x90, TAB6);
			}
			
//			/* year */
//			OLED_ShowString(0, 33, "20", false);//��20��
//			snprintf(disp_buf,sizeof(disp_buf),"%02u",setDate.Year);
//			highlight=(menu->Screen.selection==0);
//			OLED_ShowString(0, 49, disp_buf,highlight);//��17�ꡱ
//			OLED_China(0, 73, 0, false);//��
//			
//			/* month */
//			snprintf(disp_buf,sizeof(disp_buf),"%02u",setDate.Month);
//			highlight=(menu->Screen.selection==1);
//			OLED_ShowString(24, 25, disp_buf, highlight);//��06�¡�
//			OLED_China(24, 49, 1, false);//��
//			
//			/* day */
//			snprintf(disp_buf,sizeof(disp_buf),"%02u",setDate.Date);
//			highlight=(menu->Screen.selection==2);
//			OLED_ShowString(24, 65, disp_buf, highlight);//��22�ա�
//			OLED_China(24, 89, 2, false);//��
//			
//			/* hour */
//			snprintf(disp_buf,sizeof(disp_buf),"%02u",setTime.Hours);
//			highlight=(menu->Screen.selection==3);
//			OLED_ShowString(48, 9, disp_buf, highlight);//��10�㡱
//			OLED_China(48, 33, 3, false);//ʱ
//			
//			/* minute */
//			snprintf(disp_buf,sizeof(disp_buf),"%02u",setTime.Minutes);
//			highlight=(menu->Screen.selection==4);
//			OLED_ShowString(48,49,disp_buf,highlight);//��10�֡�
//			OLED_China(48, 73, 4,false);//��
//			
//			/* second */
//			snprintf(disp_buf,sizeof(disp_buf),"%02u",setTime.Seconds);
//			highlight=(menu->Screen.selection==5);
//			OLED_ShowString(48, 89, disp_buf, highlight);//��10�롱
//			OLED_China(48, 108, 5, false);//��
//			
//			OLED_Refresh_Gram();
}	

#endif

/*�̵���ʱ*/
__STATIC_INLINE void short_delay(void)
{
  volatile uint32_t i=0;
  
	for(i=0;i<4000;i++);
//  while(i--)
//  {
//    asm("nop");
//  }
}

__STATIC_INLINE void turn_off_display(void)
{
			/* turn off display */
			disp_on = false;
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_RESET);     /*��ʼ��PB14Ϊ�͵�ƽ ����Ʋ���*/ /* backlight off */
//			OLED_Clear();  /* clear screen */
//			
//			CurrentDisplay = &MainMenu;  /* display main menu */
//			CurrentDisplay->Screen.screen_number=0;
//			CurrentDisplay->Screen.selection=0;
}

/**
  * @brief  turn on display
  * @param  None
  * @retval None
  */
__STATIC_INLINE void turn_on_display(void)
{
		/* turn on display */
		disp_on	=	true;  /* display on */
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_SET);     /*��ʼ��PB14Ϊ�ߵ�ƽ �������*/ /* backlight on */
		disp_on_count = MAX_DISPLAY_ON_TIME;
}




/* Includes ------------------------------------------------------------------*/
//#include "display_module.h"
//#include "cmsis_os.h"

//#include "sys_time_module.h"
//#include "storage_module.h"
//#include "usart_module.h"
//#include "eeprom.h"

//#define MAX_DISPLAY_ON_TIME            (120)   /* display on time , unit:second */

//#if (SENSOR_ID == SENSOR_R) 
////struct parameter
////{
////		int16_t A1;				/*���ݲ�ѯ��������*/
////		int16_t B1;
////		int16_t C1;
////	
////		int16_t A2;				/*���ݲ�ѯ��ʱ����*/
////		int16_t B2;
////		int16_t C2;
////};
////struct parameter p1;

///*��̬ȫ�ֱ���*/
//static int screen_year 		= 0, screen_month = 0, screen_day = 0;				/*��ѯ���ݵ������ݵ�������*/
//static int screen_month1 	= 0, screen_day1 = 0, screen_hour1 = 0;		/*��ѯ���ݵ�ʱ���ݵ�����ʱ*/
//static int screen_rain 		= 0;				/*��ѯ���ݵ���ʾ����*/
//#endif



//static uint8_t disp_on 				= true;
//static int16_t disp_on_count 	= MAX_DISPLAY_ON_TIME;  /* display on time counter */

///* Private typedef -----------------------------------------------------------*/
///** 
//  * @brief  Button Callback Function Definition
//  */
//typedef void (*ButtonCallbackFunc)(void *,int16_t);
///** 
//  * @brief  LCD Display Function Definition
//  */
//typedef void (*DisplayFunc)(void *);

///** 
//  * @brief  LCD Display Screen Structure
//  */
//typedef struct DisplayScreen
//{
//  int16_t selection;         /* current selection */
//  int16_t screen_number;     /* current screen number */
//	int16_t screen_leftright;
//  
//  ButtonCallbackFunc button_func;      /* button callback function */
//  DisplayFunc        display_func;     /* display callback function */
//} DisplayScreenTypeDef;
///** 
//  * @brief  LCD Display Menu Structure
//  */
//typedef struct DisplayMenu
//{
//  DisplayScreenTypeDef Screen;
//  
//  struct DisplayMenu *prev;    /* previous menu */
//  struct DisplayMenu *next;    /* next menu */
//} DisplayMenuTypeDef;



///* Private define ------------------------------------------------------------*/
//#define displaySTACK_SIZE   (512)
//#define displayPRIORITY     osPriorityNormal
//#define QUEUE_SIZE ((uint32_t)1)
///* Private variables ---------------------------------------------------------*/
//static uint8_t nian,yue,ri,shi,fen,miao;//����ʱ�����
///* RTC Time*/
//static RTC_TimeTypeDef Time;
//static RTC_DateTypeDef Date;
///* Set Time */
//static RTC_TimeTypeDef setTime;
//static RTC_DateTypeDef setDate;


///* Display Menus and Screens -------------------------------------------------*/
///* Menus */
//static DisplayMenuTypeDef *CurrentDisplay;  /* Current Display Menu */

///* os relative��ʾ���� */
//static osThreadId DisplayThreadHandle;
///*static osSemaphoreId semaphore;*/
//static osMessageQId ButtonQueue;  /* ��������button queue */
///* Private function prototypes -----------------------------------------------*/
//static void Display_Thread(void const *argument);
//static void init_display_menus(void);

//#if (SENSOR_ID == SENSOR_W) 
//static DisplayMenuTypeDef MainMenu;  /* Main Menu */
//static DisplayMenuTypeDef TimeMenu;  /* Time Set Menu */
//static DisplayMenuTypeDef DataMenu;  /* Data  Menu */
//static void TimeScreen_ButtonHandler(void *Menu,int16_t button);
//static void TimeScreen_Display(void *Menu);//ʱ��������
//static void MainScreen_ButtonHandler(void *Menu,int16_t button);
//static void MainScreen_Display(void *Menu);//���˵���
//static void DataScreen_ButtonHandler(void *Menu,int16_t button);
//static void DataScreen_Display(void *Menu);//������
//#else
//static DisplayMenuTypeDef MainMenu;  /* Mainr Menu */
//static void MainScreen_ButtonHandler(void *Menu,int16_t button);
//static void MainScreen_Display(void *Menu);//���˵���

//static DisplayMenuTypeDef DatarMenu;  /* Datar  Menu */
//static void DatarScreen_ButtonHandler(void *Menu,int16_t button);
//static void DatarScreen_Display(void *Menu);//������

//static DisplayMenuTypeDef DemandMenu;  /* Demand Menu */
//static void DemandScreen_ButtonHandler(void *Menu,int16_t button);
//static void DemandScreen_Display(void *Menu);//��ѯ�˵���

//static DisplayMenuTypeDef DayMenu;  /* Day Menu */
//static void DayScreen_ButtonHandler(void *Menu,int16_t button);
//static void DayScreen_Display(void *Menu);//�ղ˵���

//static DisplayMenuTypeDef HourMenu;  /* Hour Menu */
//static void HourScreen_ButtonHandler(void *Menu,int16_t button);
//static void HourScreen_Display(void *Menu);//Сʱ�˵���

//static DisplayMenuTypeDef StationMenu;  /* Station Menu */
//static void StationScreen_ButtonHandler(void *Menu,int16_t button);
//static void StationScreen_Display(void *Menu);//վ�Ų˵���

//static DisplayMenuTypeDef WarnMenu;  /* Warn Menu */
//static void WarnScreen_ButtonHandler(void *Menu,int16_t button);
//static void WarnScreen_Display(void *Menu);//�����˵���

//static DisplayMenuTypeDef TimerMenu;  /* Timer Menu */
//static void TiemrScreen_ButtonHandler(void *Menu,int16_t button);
//static void TimerScreen_Display(void *Menu);//ʱ��˵���

//static DisplayMenuTypeDef SetMenu;  /* Set Menu */
//static void SetScreen_ButtonHandler(void *Menu,int16_t button);
//static void SetScreen_Display(void *Menu);//���ò˵���


//typedef struct
//{
//	int warn_1;
//	int warn_2;
//	int warn_3;
//	int warn_4;
//}Warn;
//static Warn warn_set = {0};  /*������������̨վ����Ҫ�ı���*/

//typedef struct
//{
//	int station_1;
//	int station_2;
//	int station_3;
//	int station_4;
//	int station_5;
//}Station;
//static Station station_set = {0};  /*������������̨վ����Ҫ�ı���*/
//#endif

//__STATIC_INLINE void short_delay(void);
//__STATIC_INLINE void turn_on_display(void);
//__STATIC_INLINE void turn_off_display(void);

///**��ʼ����ʾ����
//  * @brief  Init Display Module. 
//  * @retval 0:success;-1:failed
//  */
//int32_t init_display_module(void)
//{
//	/*��ʼ��LCD��*/
//			OLED_Init();
//  /* init display menus */
//  init_display_menus();
//	
//	#if (SENSOR_ID == SENSOR_R) 
//			OLED_OpenMenu_SL3();
//			HAL_Delay(10000);
//	#endif
//	
//		/*����RT12864M-1��ʾ��*/
////		test_oled1();
////		HAL_Delay(1000);
////		OLED_Clear();
////		HAL_Delay(1000);
////		test_oled2();
////		HAL_Delay(1000);
////		OLED_Clear();
////		HAL_Delay(1000);
////		test_oled();
////		HAL_Delay(1000);
//		OLED_Clear();
//		HAL_Delay(1000);
//		test_oled3();
//		HAL_Delay(10000);
////		test_oled4();
////		HAL_Delay(10000);
//		
//		
// 
//  /* Create the queue used by the button interrupt to pass the button value.����һ�����䰴��ֵ�ö��� */
//  osMessageQDef(button_queue,QUEUE_SIZE,uint16_t);
//  ButtonQueue=osMessageCreate(osMessageQ(button_queue),NULL);
//  if(ButtonQueue == NULL)
//  {
//    printf("Create Button Queue failed!\r\n");
//    return -1;
//  }
//  
//  /* Create a thread to update system date and time������ʾ���� */
//  osThreadDef(Display, Display_Thread, displayPRIORITY, 0, displaySTACK_SIZE);
//  DisplayThreadHandle=osThreadCreate(osThread(Display), NULL);
//  if(DisplayThreadHandle == NULL)
//  {
//    printf("Create Display Thread failed!\r\n");
//    return -1;
//  }
//  
//  
//  return 0;
//}

///**
//  * @brief  EXTI line detection callbacks.
//  * @param  GPIO_Pin: Specifies the port pin connected to corresponding EXTI line.
//  * @retval None
//  */
//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
//{
////	uint8_t i_g = 0;
//  /* Disable Interrupt */
//  HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
//	HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
//  
//  /* eliminate key jitter */
//	short_delay();
//  if(HAL_GPIO_ReadPin(GPIOC, GPIO_Pin)==GPIO_PIN_RESET)
//  {
//			/* Put the Button Value to the Message Queue */
//			if(GPIO_Pin == GPIO_PIN_8)  /* ESCAPE button */
//			{
//				if(ButtonQueue)
//				{
//					osMessagePut(ButtonQueue, ESCAPE, 100);
//				}
//			}
//			
//			if(GPIO_Pin==GPIO_PIN_9)  /* ENTER button */
//			{
//				if(ButtonQueue)
//				{
//					osMessagePut(ButtonQueue, ENTER, 100);
//				}
//			}
//			
//			if(GPIO_Pin==GPIO_PIN_10)  /* LEFT button */
//			{
//				if(ButtonQueue)
//				{
//					osMessagePut(ButtonQueue, DOWN, 100);
//				}
//			}
//			
//			if(GPIO_Pin == GPIO_PIN_11)  /* RIGHT button */
//			{
//				if(ButtonQueue)
//				{
//					osMessagePut(ButtonQueue, UP, 100);
//				}
//			}
//			
//			 if(GPIO_Pin == GPIO_PIN_12)  /* UP button */
//			{
//				if(ButtonQueue)
//				{
//					osMessagePut(ButtonQueue, LEFT, 100);
//				}
//			}
//			
//			 if(GPIO_Pin == GPIO_PIN_13)  /* DOWN button */
//			{
//      if(ButtonQueue)
//      {
//        osMessagePut(ButtonQueue, RIGHT, 100);
//      }
//    }
//  }
//	
//	if(HAL_GPIO_ReadPin(GPIOA,GPIO_Pin)	==	GPIO_PIN_RESET)
//	{
//			#if (SENSOR_ID == SENSOR_R) 
//			if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_7) == 0)
//			{
//					rain_sample.rain_1m++ ;   /*��������*/
//					rain_sample.rain_d++;     /*������*/
//			}
//			else if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_7) == 1)
//			{
//					printf("no  rain\n");
//			}
////			#else
////			if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_6) == 1)
////			{
////					printf("no  wind\n");
////			}
////			else if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_6) == 0)
////			{
////					wind_sample.ws_rate++;			/*һ�η����������һ��*/
////					
////			}
//			#endif
//	}
//	
//	
//  
//  /* Enable Interrupt */
//  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
//	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
//}

///*��ʾ����*/
//static void Display_Thread(void const *argument)
//{
//  osEvent event;
//  int16_t button_value=0;
//  struct tm datetime={0};
//  
//  (void)get_sys_time_tm(&datetime);
//  
//  OLED_Clear();  /* clear screen */

//  while(1)
//  {
//			/* Get the message from the queue */
//			event = osMessageGet(ButtonQueue, 1000);

//			if (event.status == osEventMessage)
//			{
//					/* get button value */
//					button_value = event.value.v;
//					
//					/* button handler */
//					if(disp_on && CurrentDisplay->Screen.button_func)
//					{
//						(*CurrentDisplay->Screen.button_func)(CurrentDisplay,button_value);
//					}

//					/* turn on display */
//					turn_on_display();
//				
//			}
//			
//			/* get data&time */
//			(void)get_sys_time(&Date,&Time);
//			
//			/* display on */
//			if(disp_on == true)
//			{
//				
//					/* display function */
//					if(CurrentDisplay->Screen.display_func)
//					{
//							(*CurrentDisplay->Screen.display_func)(CurrentDisplay);
//					}
//					
//					disp_on_count--;
//					if(disp_on_count<MAX_DISPLAY_ON_TIME*2/3)
//					{
//							/* turn off backlight */
//							HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_SET);     /*��ʼ��PB14Ϊ�ߵ�ƽ �������*/
//					}
//					if(disp_on_count < 1)
//					{
//							/* turn off display */
//							turn_off_display();
//					}
//			}
//		}
//}

//#if (SENSOR_ID == SENSOR_W) 
///*��ʾ�˵�*/
//static void init_display_menus(void)
//{
//	/* Normal_Mode Menu */
//	DataMenu.prev=&MainMenu;
//  DataMenu.next=NULL;
//  DataMenu.Screen.screen_number=3;
//  DataMenu.Screen.selection=0;
//  DataMenu.Screen.button_func=DataScreen_ButtonHandler;
//  DataMenu.Screen.display_func=DataScreen_Display;

//	
//  /* Time Menu */
//  TimeMenu.prev=&MainMenu;
//  TimeMenu.next=NULL;
//  TimeMenu.Screen.screen_number=0;
//  TimeMenu.Screen.selection=0;
//  TimeMenu.Screen.button_func=TimeScreen_ButtonHandler;
//  TimeMenu.Screen.display_func=TimeScreen_Display;
//  
//  
//  /* Main Menu */
//  MainMenu.prev=NULL;
//  MainMenu.next=&DataMenu;
//  MainMenu.Screen.screen_number=0;
//  MainMenu.Screen.selection=0;
//  MainMenu.Screen.button_func=MainScreen_ButtonHandler;
//  MainMenu.Screen.display_func=MainScreen_Display;
//	
//  /* Current Menu */
//  /*CurrentDisplay=&DataMenu;   //just use Data Menu for now*/
//  CurrentDisplay=&MainMenu;  /* display main menu */
//}

///*���ݲ˵�����*/
//static void DataScreen_ButtonHandler(void *Menu,int16_t button)
//{
//  DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
//  
//  switch(button)
//  {
//			case ESCAPE:
//				if(menu->prev)
//			  {
//						CurrentDisplay 											 = menu->prev;
//						CurrentDisplay->Screen.screen_number = 0;
//						CurrentDisplay->Screen.selection		 = 0;
//						OLED_Clear();  /* clear screen */
//			  }
//			  break;
//			case ENTER:
//				break;
//			case LEFT:
//				break;
//			case RIGHT:
//				break;
//			case UP:
//				menu->Screen.screen_number++;
//				break;
//			case DOWN:
//				menu->Screen.screen_number--;
//				break;
//			default:
//				break;
//  }
//  
//  if(menu->Screen.screen_number < 0)
//  {
//    menu->Screen.screen_number	=	4;
//  }
//  else if(menu->Screen.screen_number > 4)
//  {
//    menu->Screen.screen_number	=	0;
//  }
//  
//}

///*��������ʾ*/
//static void DataScreen_Display(void *Menu)
//{
//	DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
////  uint8_t highlight=false;
//	
//  if(menu->Screen.screen_number == 0)
//	{
//				/* Time */
//				memset(disp_buf, 0, sizeof(disp_buf));
//				snprintf((char *)disp_buf, sizeof(disp_buf), "  %02uʱ%02u��%02u��  ",
//								Time.Hours,Time.Minutes,Time.Seconds);
//				rt12864m_show(1, disp_buf);		/*ʱ����*/
//				
//				rt12864m_show(2, "                ");
//				
//				/*˲ʱ����*/
//				snprintf(disp_buf, sizeof(disp_buf),	"˲ʱ����: %4d��",	wind_sample.wd_3s);
//				rt12864m_show(3, disp_buf);
//		
//				/*˲ʱ����*/
//				snprintf(disp_buf, sizeof(disp_buf), "˲ʱ����:%4dm/s", wind_sample.ws_1s);
//				rt12864m_show(4, disp_buf);
//	}
//	else if(menu->Screen.screen_number == 1)
//	{
//				/* Time */
//				memset(disp_buf, 0, sizeof(disp_buf));
//				snprintf((char *)disp_buf, sizeof(disp_buf), "  %02uʱ%02u��%02u��  ",
//								Time.Hours,Time.Minutes,Time.Seconds);
//				rt12864m_show(1, disp_buf);		/*ʱ����*/
//				
//				rt12864m_show(2, "                ");
//				
//				/*2�ַ���*/
//				snprintf(disp_buf, sizeof(disp_buf),	"���ַ���: %4d��",	wind_sample.wd_2m);
//				rt12864m_show(3, disp_buf);
//			
//				/*���ַ���*/
//				snprintf(disp_buf, sizeof(disp_buf), "���ַ���:%4dm/s", wind_sample.ws_2m);
//				rt12864m_show(4, disp_buf);
//	}
//				
//	else if(menu->Screen.screen_number == 2)
//	{
//				/* Time */
//				memset(disp_buf, 0, sizeof(disp_buf));
//				snprintf((char *)disp_buf, sizeof(disp_buf), "  %02uʱ%02u��%02u��  ",
//								Time.Hours,Time.Minutes,Time.Seconds);
//				rt12864m_show(1, disp_buf);		/*ʱ����*/
//				
//				rt12864m_show(2, "                ");
//				
//				/*ʮ�ַ���*/
//				snprintf(disp_buf, sizeof(disp_buf), "ʮ�ַ���: %4d��", wind_sample.wd_10m);
//				rt12864m_show(3, disp_buf);
//		
//				/*ʮ�ַ���*/
//				snprintf(disp_buf, sizeof(disp_buf), "ʮ�ַ���:%4dm/s", wind_sample.ws_10m);
//				rt12864m_show(4, disp_buf);
//	}
//	else if(menu->Screen.screen_number == 3)
//	{
//				/* Time */
//				memset(disp_buf, 0, sizeof(disp_buf));
//				snprintf((char *)disp_buf, sizeof(disp_buf), "  %02uʱ%02u��%02u��  ",
//								Time.Hours,Time.Minutes,Time.Seconds);
//				rt12864m_show(1, disp_buf);		/*ʱ����*/
//				
//				/*����ʱ��*/
//				snprintf(disp_buf, sizeof(disp_buf), "����ʱ��:  %02d-%02d", wind_sample.ws_1s_max_t  / 100, wind_sample.ws_1s_max_t % 100);
//				rt12864m_show(2, disp_buf);
//				
//				/*�������*/
//				snprintf(disp_buf, sizeof(disp_buf), "�������: %4d��", wind_sample.wd_1s_max );
//				rt12864m_show(3, disp_buf);
//			
//				/*�������*/
//				snprintf(disp_buf, sizeof(disp_buf), "�������:%4dm/s", wind_sample.ws_1s_max);
//				rt12864m_show(4, disp_buf);
//	}
//	else if(menu->Screen.screen_number == 4)
//	{
//				/* Time */
//				memset(disp_buf, 0, sizeof(disp_buf));
//				snprintf((char *)disp_buf, sizeof(disp_buf), "  %02uʱ%02u��%02u��  ",
//								Time.Hours,Time.Minutes,Time.Seconds);
//				rt12864m_show(1, disp_buf);		/*ʱ����*/
//				
//				/*���ʱ��*/
//				snprintf(disp_buf, sizeof(disp_buf), "���ʱ��:  %02d-%02d", wind_sample.ws_10m_max_t / 100, wind_sample.ws_10m_max_t % 100);
//				rt12864m_show(2, disp_buf);
//				
//				
//				/*������*/
//				snprintf(disp_buf, sizeof(disp_buf), "������: %4d��", wind_sample.wd_10m_max );
//				rt12864m_show(3, disp_buf);
//				
//				/*������*/
//				snprintf(disp_buf, sizeof(disp_buf), "������:%4dm/s" ,wind_sample.ws_10m_max  );
//				rt12864m_show(4, disp_buf);
//	}
//}



///*ʱ��������*/
//static void TimeScreen_ButtonHandler(void *Menu,int16_t button)
//{
//  DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
//  int16_t selected_timevalue=0,min_value=0,max_value=0;
//  uint8_t *selected=NULL;
//	
//  switch(menu->Screen.selection)
//  {
//  case 0:  /* year */
//    selected=&setDate.Year;
//    selected_timevalue=setDate.Year;
//    min_value=0;
//    max_value=99;
//    break;
//  case 1:  /* month */
//    selected=&setDate.Month;
//    selected_timevalue=setDate.Month;
//    min_value=1;
//    max_value=12;
//    break;
//  case 2:  /* day */
//    selected=&setDate.Date;
//    selected_timevalue=setDate.Date;
//    min_value=1;
//    max_value=31;
//    break;
//  case 3:  /* hour */
//    selected=&setTime.Hours;
//    selected_timevalue=setTime.Hours;
//    min_value=0;
//    max_value=23/*59*/;   /* 16.3.23 hour:0-23 */
//    break;
//  case 4:  /* minute */
//    selected=&setTime.Minutes;
//    selected_timevalue=setTime.Minutes;
//    min_value=0;
//    max_value=59;
//    break;
//  case 5:  /* second */
//    selected=&setTime.Seconds;
//    selected_timevalue=setTime.Seconds;
//    min_value=0;
//    max_value=59;
//    break;
//  default:
//    menu->Screen.selection=5;
//    selected=&setTime.Seconds;
//    selected_timevalue=setTime.Seconds;
//    min_value=0;
//    max_value=59;
//    break;
//  }
//  
//  switch(button)
//  {
//  case ESCAPE:
//		if((setDate.Year != nian)||(setDate.Month != yue)||(setDate.Date != ri)||(setTime.Hours != shi)||(setTime.Minutes != fen)||(setTime.Seconds != miao))
//		{
//			/* set data&time */
//			/* fill unused value */
//			setTime.DayLightSaving=RTC_DAYLIGHTSAVING_NONE;
//			setTime.StoreOperation=RTC_STOREOPERATION_RESET;
//			setTime.SubSeconds=0;
//			setTime.TimeFormat=RTC_HOURFORMAT12_AM;
//			if(set_sys_time(&setDate,&setTime)<0)
//				{
//					OLED_Clear();
//					rt12864m_show(2, "set time failed!");
//				}
//				else
//					{
//						OLED_Clear();
//						rt12864m_show(2, "set   time   ok!");
//					}
//					osDelay(500);
//		}
//    if(menu->prev)
//			{
//				CurrentDisplay=menu->prev;
//				CurrentDisplay->Screen.screen_number=0;
//				CurrentDisplay->Screen.selection=1;
//				OLED_Clear();  /* clear screen */
//			}
//    break;
//  case ENTER:
//    menu->Screen.selection++;
//    break;
//  case UP:
//    selected_timevalue++;
//    break;
//  case DOWN:
//    selected_timevalue--;
//	  break;
//  default:
//    break;
//  }
//  
//  if(menu->Screen.selection<0)
//  {
//    menu->Screen.selection = 5;
//  }
//  else if(menu->Screen.selection > 5)
//  {
//    menu->Screen.selection = 0;
//  }
//  
//  if(selected_timevalue < min_value)
//  {
//    selected_timevalue = max_value;
//  }
//  else if(selected_timevalue > max_value)
//  {
//    selected_timevalue = min_value;
//  }
//  /* set selected value */
//  *selected=selected_timevalue;
//}

///*ʱ������ʾ*/
//static void TimeScreen_Display(void *Menu)
//{
//  DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
//	
//	if(menu->Screen.selection == 0)
//	{
//				snprintf(disp_buf,sizeof(disp_buf),"    20%02u��      ",setDate.Year);
//				/*����*/
//				pic_clear_u();				/*�ϰ�������*/
//				pic_u(0x01, 0x10, 0x83, 0x80, TAB6);		/*0x80�����һ�У�0x01������һ���֣�0x83�����һ�еĵ��ĸ��ַ���*/
//				rt12864m_show(1, disp_buf);
//		
//				rt12864m_show(2, "                ");		/*�ڶ��в���ʾ����*/
//		
//				snprintf(disp_buf,sizeof(disp_buf),"    %02u��%02u��    ", setDate.Month, setDate.Date);
//				rt12864m_show(3, disp_buf);
//		
//				snprintf(disp_buf,sizeof(disp_buf),"  %02uʱ%02u��%02u��  ", setTime.Hours, setTime.Minutes, setTime.Seconds);
//				rt12864m_show(4, disp_buf);
//	}
//	else if(menu->Screen.selection == 1)
//	{
//				snprintf(disp_buf,sizeof(disp_buf),"    20%02u��      ",setDate.Year);
//				rt12864m_show(1, disp_buf);
//		
//				rt12864m_show(2, "                ");		/*�ڶ��в���ʾ����*/
//		
//				snprintf(disp_buf,sizeof(disp_buf),"    %02u��%02u��    ", setDate.Month, setDate.Date);
//				/*����*/
//				pic_clear_u();
//				pic_u(0x01, 0x10, 0x90, 0x88, TAB6);		/*0x88��������У�0x01������һ���֣�0x90��������еĵ������ַ���*/
//				rt12864m_show(3, disp_buf);
//		
//				snprintf(disp_buf,sizeof(disp_buf),"  %02uʱ%02u��%02u��  ", setTime.Hours, setTime.Minutes, setTime.Seconds);
//				rt12864m_show(4, disp_buf);
//	}
//	else if(menu->Screen.selection == 2)
//	{
//				snprintf(disp_buf,sizeof(disp_buf),"    20%02u��      ",setDate.Year);
//				rt12864m_show(1, disp_buf);
//		
//				rt12864m_show(2, "                ");		/*�ڶ��в���ʾ����*/
//		
//				snprintf(disp_buf,sizeof(disp_buf),"    %02u��%02u��    ", setDate.Month, setDate.Date);
//				/*����*/
//				pic_clear_u();				/*�ϰ�������*/
//				pic_u(0x01, 0x10, 0x92, 0x88, TAB6);		/*0x88��������У�0x01������һ���֣�0x92��������еĵ�����ַ���,0x20�����Եĸ߶�Ϊ16������*/
//				rt12864m_show(3, disp_buf);
//		
//				snprintf(disp_buf,sizeof(disp_buf),"  %02uʱ%02u��%02u��  ", setTime.Hours, setTime.Minutes, setTime.Seconds);
//				rt12864m_show(4, disp_buf);
//	}
//	else if(menu->Screen.selection == 3)
//	{
//				snprintf(disp_buf,sizeof(disp_buf),"    20%02u��      ",setDate.Year);
//				rt12864m_show(1, disp_buf);
//		
//				rt12864m_show(2, "                ");		/*�ڶ��в���ʾ����*/
//		
//				snprintf(disp_buf,sizeof(disp_buf),"    %02u��%02u��    ", setDate.Month, setDate.Date);
//				rt12864m_show(3, disp_buf);
//		
//				snprintf(disp_buf,sizeof(disp_buf),"  %02uʱ%02u��%02u��  ", setTime.Hours, setTime.Minutes, setTime.Seconds);
//				/*����*/
//				pic_clear_u();				/*�ϰ�������*/
//				pic_u(0x01, 0x10, 0x99, 0x98, TAB6);		/*0x98��������У�0x01������һ���֣�0x99��������еĵ�һ���ַ���*/
//				rt12864m_show(4, disp_buf);
//	}
//	else if(menu->Screen.selection == 4)
//	{
//				snprintf(disp_buf,sizeof(disp_buf),"    20%02u��      ",setDate.Year);
//				rt12864m_show(1, disp_buf);
//		
//				rt12864m_show(2, "                ");		/*�ڶ��в���ʾ����*/
//		
//				snprintf(disp_buf,sizeof(disp_buf),"    %02u��%02u��    ", setDate.Month, setDate.Date);
//				rt12864m_show(3, disp_buf);
//		
//				snprintf(disp_buf,sizeof(disp_buf),"  %02uʱ%02u��%02u��  ", setTime.Hours, setTime.Minutes, setTime.Seconds);
//				/*����*/
//				pic_clear_u();				/*�ϰ�������*/
//				pic_u(0x01, 0x10, 0x9b, 0x98, TAB6);		/*0x80�����һ�У�0x01������һ���֣�0x9b��������еĵ��ĸ��ַ���*/
//				rt12864m_show(4, disp_buf);
//	}
//	else if(menu->Screen.selection == 5)
//	{
//				snprintf(disp_buf,sizeof(disp_buf),"    20%02u��      ",setDate.Year);
//				rt12864m_show(1, disp_buf);
//		
//				rt12864m_show(2, "                ");		/*�ڶ��в���ʾ����*/
//		
//				snprintf(disp_buf,sizeof(disp_buf),"    %02u��%02u��    ", setDate.Month, setDate.Date);
//				rt12864m_show(3, disp_buf);
//		
//				snprintf(disp_buf,sizeof(disp_buf),"  %02uʱ%02u��%02u��  ", setTime.Hours, setTime.Minutes, setTime.Seconds);
//				/*����*/
//				pic_clear_u();				/*�ϰ�������*/
//				pic_u(0x01, 0x10, 0x9d, 0x98, TAB6);		/*0x98��������У�0x01������һ���֣�0x9d��������еĵ������ַ���*/
//				rt12864m_show(4, disp_buf);
//	}
//}	



///*���˵�����*/
//static void MainScreen_ButtonHandler(void *Menu,int16_t button)
//{
//	DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
//  
//  switch(button)
//  {
//  case ESCAPE:
//    break;
//  case ENTER:   /* enter next menu */
//    if(menu->next)
//    {
//      CurrentDisplay=menu->next;
//      CurrentDisplay->Screen.screen_number=0;
//      CurrentDisplay->Screen.selection=0;
//      OLED_Clear();  /* clear screen */
//  
//      /* get system time */
//      get_sys_time(&setDate,&setTime);
//			if(menu->Screen.selection == 1)
//			{
//					nian   =  setDate.Year;
//					yue    =  setDate.Month;
//					ri     =  setDate.Date;
//					shi    =  setTime.Hours;
//					fen    =  setTime.Minutes;
//					miao   =  setTime.Seconds;
//			}
//    }
//    break;
//  case LEFT:
//    menu->Screen.selection--;
//    break;
//  case RIGHT:
//    menu->Screen.selection++;
//    break;
//  default:
//    break;
//  }
//  
//  
//  if(menu->Screen.selection < 0)
//  {
//    menu->Screen.selection = 1;
//  }
//  else if(menu->Screen.selection > 1)
//  {
//    menu->Screen.selection = 0;
//  }
//}

///*���˵�����ʾ*/
//static void MainScreen_Display(void *Menu)
//{
//		DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
//  
//		/* Date */
//		snprintf(disp_buf, sizeof(disp_buf), "  20%02u��%02u��%02u��",
//							Date.Year,Date.Month,Date.Date);
//		rt12864m_show(1, disp_buf);
//		/* Time */
//		snprintf((char *)disp_buf,sizeof(disp_buf),"  %02uʱ%02u��%02u��  ",
//						Time.Hours,Time.Minutes,Time.Seconds);
//		rt12864m_show(2, disp_buf);
//	
//		if(menu->Screen.selection == 0)
//		{
//					OLED_W_Command(0x98);		/*  */
//					OLED_W_Data(0xa1);
//					OLED_W_Data(0xa0);
//			
////					pic_clear_u();				/*�ϰ�������*/
//					pic_u(0x02, 0x10, 0x99, 0x98, TAB6);
//			
//					OLED_W_Command(0x99);		/*��*/
//					OLED_W_Data(0xc8);
//					OLED_W_Data(0xf0);
//			
//					OLED_W_Command(0x9a);		/*��*/
//					OLED_W_Data(0xcc);
//					OLED_W_Data(0xd8);
//			
//					OLED_W_Command(0x9b);		/*  */
//					OLED_W_Data(0xa1);
//					OLED_W_Data(0xa0);
//				
//					OLED_W_Command(0x9c);		/*  */
//					OLED_W_Data(0xa1);
//					OLED_W_Data(0xa0);
//					
//					OLED_W_Command(0x9d);		/*��*/
//					OLED_W_Data(0xc8);
//					OLED_W_Data(0xf0);
//			
//					OLED_W_Command(0x9e);		/*��*/
//					OLED_W_Data(0xcc);
//					OLED_W_Data(0xd8);
//					
//					OLED_W_Command(0x9f);		/*  */
//					OLED_W_Data(0xa1);
//					OLED_W_Data(0xa0);
//		}
//		else if(menu->Screen.selection == 1)
//		{
//					OLED_W_Command(0x98);		/*  */
//					OLED_W_Data(0xa1);
//					OLED_W_Data(0xa0);
//			
//					OLED_W_Command(0x99);		/*��*/
//					OLED_W_Data(0xc8);
//					OLED_W_Data(0xf0);
//			
//					OLED_W_Command(0x9a);		/*��*/
//					OLED_W_Data(0xcc);
//					OLED_W_Data(0xd8);
//			
//					OLED_W_Command(0x9b);		/*  */
//					OLED_W_Data(0xa1);
//					OLED_W_Data(0xa0);
//				
//					OLED_W_Command(0x9c);		/*  */
//					OLED_W_Data(0xa1);
//					OLED_W_Data(0xa0);
//			
////					pic_clear_u();				/*�ϰ�������*/
//					pic_u(0x02, 0x10, 0x9d, 0x98, TAB6);
//					
//					OLED_W_Command(0x9d);		/*��*/
//					OLED_W_Data(0xc8);
//					OLED_W_Data(0xf0);
//			
//					OLED_W_Command(0x9e);		/*��*/
//					OLED_W_Data(0xcc);
//					OLED_W_Data(0xd8);
//					
//					OLED_W_Command(0x9f);		/*  */
//					OLED_W_Data(0xa1);
//					OLED_W_Data(0xa0);
//		}
//		
//		/* Determine the next Menu */
//		switch(menu->Screen.selection)
//		{
//				case 0:   /* Data Menu */
//					menu->next=&DataMenu;
//					break;
//				case 1:   /* Communication Menu */
//					menu->next=&TimeMenu;
//					break;
//				default:
//					menu->next=&DataMenu;
//					break;
//		}
//}

//#else
///*��ʾ�˵�*/
//static void init_display_menus(void)
//{
//		/* Main Menu */
//		MainMenu.prev									=	NULL;
//		MainMenu.next									=	&DatarMenu;
//		MainMenu.Screen.screen_number	=	0;
//		MainMenu.Screen.selection			=	0;
//		MainMenu.Screen.button_func		=	MainScreen_ButtonHandler;
//		MainMenu.Screen.display_func	=	MainScreen_Display;
//	
//		/* Data Menu */
//		DatarMenu.prev=&MainMenu;
//		DatarMenu.next=NULL;
//		DatarMenu.Screen.screen_number=3;
//		DatarMenu.Screen.selection=0;
//		DatarMenu.Screen.button_func=DatarScreen_ButtonHandler;
//		DatarMenu.Screen.display_func=DatarScreen_Display;

//		/* Demand Menu */
//		DemandMenu.prev =	&MainMenu;
//		DemandMenu.next	=	&DayMenu;
//		DemandMenu.Screen.screen_number =		0;
//		DemandMenu.Screen.selection     = 	0;
//		DemandMenu.Screen.button_func   = 	DemandScreen_ButtonHandler;
//		DemandMenu.Screen.display_func  = 	DemandScreen_Display;
//  
//		/* Day Menu */
//		DayMenu.prev =	&DemandMenu;
//		DayMenu.next	=	NULL;
//		DayMenu.Screen.screen_number =	0;
//		DayMenu.Screen.selection     = 	0;
//		DayMenu.Screen.button_func   = 	DayScreen_ButtonHandler;
//		DayMenu.Screen.display_func  = 	DayScreen_Display;
//  
//		/* Hour Menu */
//		HourMenu.prev =	&DemandMenu;
//		HourMenu.next	=	NULL;
//		HourMenu.Screen.screen_number =  	0;
//		HourMenu.Screen.selection     = 	0;
//		HourMenu.Screen.button_func   = 	HourScreen_ButtonHandler;
//		HourMenu.Screen.display_func  = 	HourScreen_Display;
//		
//		/* Set Menu */
//		SetMenu.prev =	&MainMenu;
//		SetMenu.next	=	&StationMenu;
//		SetMenu.Screen.screen_number =	0;
//		SetMenu.Screen.selection     = 	0;
//		SetMenu.Screen.button_func   = 	SetScreen_ButtonHandler;
//		SetMenu.Screen.display_func  = 	SetScreen_Display;
//		
//		/* Station Menu */
//		StationMenu.prev =	&SetMenu;
//		StationMenu.next	=	NULL;
//		StationMenu.Screen.screen_number =	0;
//		StationMenu.Screen.selection     = 	0;
//		StationMenu.Screen.button_func   = 	StationScreen_ButtonHandler;
//		StationMenu.Screen.display_func  = 	StationScreen_Display;
//		
//		/* Warn Menu */
//		WarnMenu.prev =	&SetMenu;
//		WarnMenu.next	=	NULL;
//		WarnMenu.Screen.screen_number =	0;
//		WarnMenu.Screen.selection     = 	0;
//		WarnMenu.Screen.button_func   = 	WarnScreen_ButtonHandler;
//		WarnMenu.Screen.display_func  = 	WarnScreen_Display;
//		
//		/* Timer Menu */
//		TimerMenu.prev =	&SetMenu;
//		TimerMenu.next	=NULL;
//		TimerMenu.Screen.screen_number =	0;
//		TimerMenu.Screen.selection     = 	0;
//		TimerMenu.Screen.button_func   = 	TiemrScreen_ButtonHandler;
//		TimerMenu.Screen.display_func  = 	TimerScreen_Display;
//		/* Current Menu */
//		CurrentDisplay=&MainMenu;  /* display main menu */
//}

///*���˵�����*/
//static void MainScreen_ButtonHandler(void *Menu,int16_t button)
//{
//		DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
//		
//		switch(button)
//		{
//				case ESCAPE:
//					break;
//				case ENTER:   /* enter next menu */
//					if(menu->next)
//					{
//							CurrentDisplay 													= menu->next;
//							CurrentDisplay->Screen.screen_number  	= 0;
//							CurrentDisplay->Screen.selection 				= 0;
//							CurrentDisplay->Screen.screen_leftright = 0;
//							OLED_Clear();  /* clear screen */
//  
//							if(menu->Screen.selection == 1)			/*�����ѯ�˵�*/
//							{
//										/* get system time */
//										get_sys_time(&setDate,&setTime);
//									
//										/*��������ʾ��ʱ��*/
//										screen_year 	= setDate.Year;
//										screen_month  = setDate.Month;
//										screen_day 		= setDate.Date;
//								
//										/*ʱ������ʾ��ʱ��*/
//										screen_month1 	= setDate.Month;
//										screen_day1		  = setDate.Date;
//										screen_hour1 		= setTime.Hours;
//							}
//					}
//					break;
//			case LEFT:
//					menu->Screen.selection--;
//					break;
//			case RIGHT:
//					menu->Screen.selection++;
//					break;
//			default:
//					break;
//		}
//		
//		if(menu->Screen.selection<0)
//		{
//				menu->Screen.selection=2;
//		}
//		else if(menu->Screen.selection>2)
//		{
//				menu->Screen.selection=0;
//		}
//}

///*���˵�����ʾ*/
//static void MainScreen_Display(void *Menu)
//{
//		DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
//		uint8_t highlight=false;
//  
//		/* Date */
//		snprintf((char *)disp_buf,sizeof(disp_buf)," 20%02u  %02u  %02u   ",
//							Date.Year,Date.Month,Date.Date);
//		OLED_ShowString(0,0,disp_buf,false);
//		OLED_China(40,  0,  8,false);//��
//		OLED_China(72,  0,  9,false);//��
//		OLED_China(104, 0, 10,false);//��
//		/* Time */
//		snprintf((char *)disp_buf,sizeof(disp_buf),"  %02u  %02u  %02u    ",
//						Time.Hours,Time.Minutes,Time.Seconds);
//		OLED_ShowString(0,16,disp_buf,false);
//		OLED_China(32, 16, 14,false);//ʱ
//		OLED_China(64, 16, 27,false);//��
//		OLED_China(96, 16, 41,false);//��
//	
//		snprintf((char *)disp_buf,sizeof(disp_buf),"      : %4u.%1umm",rain_sample.rain_d / 10, rain_sample.rain_d % 10);
//		OLED_ShowString(0,32,disp_buf,false);
//		OLED_China(8,  32, 10,false);//��
//		OLED_China(24, 32, 28,false);//��
//		OLED_China(40, 32, 29,false);//��
//		
//		/* ˲ʱ */
//		highlight = (menu->Screen.selection == 0);
//		OLED_China(8,  48, 13,highlight);//˲
//		OLED_China(24, 48, 14,highlight);//ʱ
//  
//		/* ��ѯ */
//		highlight=(menu->Screen.selection == 1);
//		OLED_China(48, 48, 71,highlight);//��
//		OLED_China(64, 48, 72,highlight);//ѯ
//		
//		/* ���� */
//		highlight=(menu->Screen.selection == 2);
//		OLED_China(88,  48, 15,highlight);//��
//		OLED_China(104, 48, 16,highlight);//��
//  
//		/* Determine the next Menu */
//		switch(menu->Screen.selection)
//		{
//				case 0:   /* Data Menu */
//					menu->next=&DatarMenu;
//					break;
//				case 1:   /* Communication Menu */
//					menu->next=&DemandMenu;
//					break;
//				case 2:
//					menu->next=&SetMenu;
//					break;
//				default:
//					menu->next=&DatarMenu;
//					break;
//		}
//		
//		OLED_Refresh_Gram();
//}

///*˲ʱ���ݲ˵�����*/
//static void DatarScreen_ButtonHandler(void *Menu,int16_t button)
//{
//		DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
//		
//		switch(button)
//		{
//				case ESCAPE:
//					if(menu->prev)
//					{
//							CurrentDisplay=menu->prev;
//							CurrentDisplay->Screen.screen_number=0;
//							CurrentDisplay->Screen.selection=0;
//							OLED_Clear();  /* clear screen */
//					}
//					break;
//				case ENTER:   /* enter next menu */
//					break;
//			case LEFT:
//					break;
//			case RIGHT:
//					break;
//			default:
//					break;
//		}
//}

///*˲ʱ���ݲ˵�����ʾ*/
//static void DatarScreen_Display(void *Menu)
//{
////		DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
//		memset(disp_buf, 0, sizeof(disp_buf));									/*���㻺��*/
//		/* Date */
//		snprintf((char *)disp_buf,sizeof(disp_buf),"10      : %02u.%01umm", rain_sample.rain_10m / 10, rain_sample.rain_10m % 10);/*ʮ������*/
//		OLED_ShowString(0,0,disp_buf,false);
//		OLED_China(32,    0, 27,false);//��
//		OLED_China(48,    0, 40,false);//��

//		snprintf((char *)disp_buf,sizeof(disp_buf),"60      : %02u.%01umm", rain_sample.rain_60m / 10, rain_sample.rain_60m % 10);/*60������������*/
//		OLED_ShowString(0,16,disp_buf,false);
//		OLED_China(32,    16, 27,false);//��
//		OLED_China(48,    16, 40,false);//��
//	
//		snprintf((char *)disp_buf,sizeof(disp_buf),"60      : %02u.%01umm", rain_sample.rain_60m_max / 10, rain_sample.rain_60m_max % 10);/*60���������������*/
//		OLED_ShowString(0,32,disp_buf,false);
//		OLED_China(16,    32, 27,false);//��
//		OLED_China(32,    32, 69,false);//��
//		OLED_China(48,    32, 67,false);//��
//	
//		snprintf((char *)disp_buf,sizeof(disp_buf),"        :  %02u:%02u", rain_sample.rain_60m_max_t / 100, rain_sample.rain_60m_max_t % 100);/*Сʱ�����������ʱ��*/
//		OLED_ShowString(0,48,disp_buf,false);
//		OLED_China(0,     48, 87,false);//��
//		OLED_China(16,    48, 88,false);//��
//		OLED_China(32,    48, 14,false);//ʱ
//		OLED_China(48,    48, 68,false);//��
//		
//		OLED_Refresh_Gram();
//}

///*���ݲ�ѯ�˵�����*/
//static void DemandScreen_ButtonHandler(void *Menu,int16_t button)
//{
//			DisplayMenuTypeDef *menu = (DisplayMenuTypeDef *)Menu;
//			int selected_timevalue = 0, min_value = 0, max_value = 0;
//			int *selected = NULL;
//			
//			switch(menu->Screen.screen_leftright)
//			{
//					case 0:
//						break;
//					case 1:
//						if(menu->Screen.selection == 0)
//						{
//								selected 						= &screen_year;
//								selected_timevalue	=	screen_year;
//								min_value						=	0;
//								max_value						=	99;
//						}
//						else if(menu->Screen.selection == 1)
//						{
//								selected 						= &screen_month1;
//								selected_timevalue	=	screen_month1;
//								min_value						=	1;
//								max_value						=	12;
//						}
//						break;
//					case 2:
//						if(menu->Screen.selection == 0)
//						{
//								selected 						= &screen_month;
//								selected_timevalue	=	screen_month;
//								min_value						=	1;
//								max_value						=	12;
//						}
//						else if(menu->Screen.selection == 1)
//						{
//								selected 						= &screen_day1;
//								selected_timevalue	=	screen_day1;
//								min_value						=	1;
//								max_value						=	31;
//						}
//						break;
//					case 3:
//						if(menu->Screen.selection == 0)
//						{
//								selected 						= &screen_day;
//								selected_timevalue	=	screen_day;
//								min_value						=	1;
//								max_value						=	31;
//						}
//						else if(menu->Screen.selection == 1)
//						{
//								selected 						= &screen_hour1;
//								selected_timevalue	=	screen_hour1;
//								min_value						=	0;
//								max_value						=	23;
//						}
//						break;
//					default:
//							break;
//			}
//		
//			switch(button)
//			{
//						case ESCAPE:
//							if(menu->prev)
//							{
//									CurrentDisplay														= menu->prev;
//									CurrentDisplay->Screen.screen_number  		= 0;
//									CurrentDisplay->Screen.screen_leftright  	= 0;
//									CurrentDisplay->Screen.selection					= 1;
//									OLED_Clear();  /* clear screen */
//							}
//							break;
//							
//						case ENTER:   /* enter next menu */
//							menu->Screen.screen_leftright++;
//							if(menu->Screen.screen_leftright > 3)
//							{
//									CurrentDisplay 														= menu->next;
//									CurrentDisplay->Screen.screen_number  		= 0;
//									CurrentDisplay->Screen.screen_leftright 	= 0;
//									CurrentDisplay->Screen.selection					= 0;
//									OLED_Clear();  /* clear screen */
//							 }
//							break;
//						case DOWN:
//							if(menu->Screen.screen_leftright == 0)
//							{
//									menu->Screen.selection--;
//							}
//							else
//							{
//									selected_timevalue--;
////									printf("DOWN\r\n");
//							}
//							break;
//						case UP:
//							if(menu->Screen.screen_leftright == 0)
//							{
//									menu->Screen.selection++;
//							}
//							else
//							{
//									selected_timevalue++;
////									printf("UP\r\n");
//							}
//							break;
//						default:
//							break;
//			}
//		
//			if(menu->Screen.selection < 0)
//			{
//					menu->Screen.selection = 1;
//			}
//			else if(menu->Screen.selection > 1)
//			{
//					menu->Screen.selection = 0;
//			}
//			
//			if(selected_timevalue < min_value)
//			{
//					selected_timevalue = max_value;
//			}
//			else if(selected_timevalue > max_value)
//			{
//					selected_timevalue = min_value;
//			}
//			
//			if(menu->Screen.screen_leftright < 0)
//			{
//						menu->Screen.screen_leftright = 3;
//			}
//			else if(menu->Screen.screen_leftright > 3)
//			{
//						menu->Screen.screen_leftright = 0;
//			}
//			/* set selected value */
//			*selected = selected_timevalue;
//}

///*���ݲ�ѯ�˵�����ʾ*/
//static void DemandScreen_Display(void *Menu)
//{
//		DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
//		uint8_t highlight = false;
//  
//		/*ÿһ��ȫ��д��*/
//		memset(disp_buf, 0, sizeof(disp_buf));
//		snprintf((char *)disp_buf,sizeof(disp_buf),"                ");
//		OLED_ShowString(0, 0,disp_buf,false);		
//		OLED_ShowString(0, 16,disp_buf,false);
//	
//		OLED_China(32, 0, 58,false);//��
//		OLED_China(48, 0, 76,false);//��
//		OLED_China(64, 0, 71,false);//��
//		OLED_China(80, 0, 72,false);//ѯ

//		snprintf((char *)disp_buf,sizeof(disp_buf),"      : %02u-%02u-%02u", screen_year, screen_month, screen_day);
//		OLED_ShowString(0, 32,disp_buf,false);
//	
//		highlight = ((menu->Screen.selection == 0) & (menu->Screen.screen_leftright == 0));
//		OLED_China(0,      32, 10, highlight);//��
//		OLED_China(16,     32, 58, highlight);//��
//	  OLED_China(32,     32, 76, highlight);//��
//	
//		highlight = ((menu->Screen.selection == 0) & (menu->Screen.screen_leftright == 1));
//		snprintf(disp_buf, sizeof(disp_buf), "%02u", screen_year);		/*��*/
//		OLED_ShowString(64, 32,disp_buf,highlight);
//	
//		highlight = ((menu->Screen.selection == 0) & (menu->Screen.screen_leftright == 2));
//		snprintf(disp_buf, sizeof(disp_buf), "%02u", screen_month);		/*��*/
//		OLED_ShowString(88, 32,disp_buf,highlight);
//		
//		highlight = ((menu->Screen.selection == 0) & (menu->Screen.screen_leftright == 3));
//		snprintf(disp_buf, sizeof(disp_buf), "%02u", screen_day);		/*��*/
//		OLED_ShowString(112, 32,disp_buf,highlight);
//	
//		snprintf((char *)disp_buf,sizeof(disp_buf),"      : %02u-%02u-%02u", screen_month1, screen_day1, screen_hour1);
//		OLED_ShowString(0, 48,disp_buf,false);
//	
//		highlight = ((menu->Screen.selection == 1) & (menu->Screen.screen_leftright == 0));
//		OLED_China(0,      48, 14, highlight);//ʱ
//		OLED_China(16,     48, 58, highlight);//��
//		OLED_China(32,     48, 76, highlight);//��
//	
//		highlight = ((menu->Screen.selection == 1) & (menu->Screen.screen_leftright == 1));
//		snprintf(disp_buf, sizeof(disp_buf), "%02u", screen_month1);		/*��*/
//		OLED_ShowString(64, 48,disp_buf,highlight);
//	
//		highlight = ((menu->Screen.selection == 1) & (menu->Screen.screen_leftright == 2));
//		snprintf(disp_buf, sizeof(disp_buf), "%02u", screen_day1);		/*��*/
//		OLED_ShowString(88, 48,disp_buf,highlight);
//		
//		highlight = ((menu->Screen.selection == 1) & (menu->Screen.screen_leftright == 3));
//		snprintf(disp_buf, sizeof(disp_buf), "%02u", screen_hour1);		/*��*/
//		OLED_ShowString(112, 48,disp_buf,highlight);
//		
//		/* Determine the next Menu */
//		switch(menu->Screen.selection)
//		{
//				case 0:   /* Data Menu */
//					menu->next = &DayMenu;
//					break;
//				case 1:   /* Communication Menu */
//					menu->next = &HourMenu;
//					break;
//				default:
//					menu->next = &DayMenu;
//					break;
//		}
//		
//		OLED_Refresh_Gram();
//}

///*�����ݲ�ѯ�˵�����*/
//static void DayScreen_ButtonHandler(void *Menu,int16_t button)
//{
//		DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
//		
//		switch(button)
//		{
//				case ESCAPE:
//					if(menu->prev)
//					{
//							CurrentDisplay=menu->prev;
//							CurrentDisplay->Screen.screen_number=0;
//							CurrentDisplay->Screen.selection=0;
//							OLED_Clear();  /* clear screen */
//					}
//					break;
//				case ENTER:   /* enter next menu */
//					if(menu->Screen.selection == 0)
//					{
//							SubaDay(&screen_year, &screen_month, &screen_day, 0, 0, 0);
//							if(usart_debug)
//							{
//									printf("time:%d-%d-%d\r\n",screen_year, screen_month, screen_day);
//							}
//					}
//					else if(menu->Screen.selection == 1)
//					{
//							AddaDay(&screen_year, &screen_month, &screen_day, 0, 0, 0);
//							if(usart_debug)
//							{
//									printf("time:%d-%d-%d\r\n",screen_year, screen_month, screen_day);
//							}
//					}
//					break;
//			case LEFT:
//					menu->Screen.selection--;
//					break;
//			case RIGHT:
//					menu->Screen.selection++;
//					break;
//			default:
//					break;
//		}
//		
//		
//		/*��������֮�󣬿���ȷ��������ʾ��һ���������*/
//		if((screen_month > Date.Month) || ((screen_month == Date.Month) && (screen_day > Date.Date)))		/*Ҫ��ѯ��ʱ����ڵ�ǰ��ʱ��*/
//		{
//				screen_rain = 0;
//		}
//		else if((screen_month == Date.Month) && (screen_day == Date.Date))			/*Ҫ��ѯ��ʱ����ǵ�ǰ��ʱ��*/
//		{
//				screen_rain = rain_sample.rain_d;							/*��ʾ���������͵��ڵ�ǰ��������*/
//		}
//		else																							/*��ѯ��ǰ��������*/
//		{
//				screen_rain = 0;
//				memset(data_buf, 0 , sizeof(data_buf));
//				if(read_file_hour(screen_month, screen_day, 20, data_buf))
//				{
//							screen_rain = (data_buf[149] - 48) * 1000 + (data_buf[150] - 48) * 100 + (data_buf[151] - 48) * 10 + (data_buf[152] - 48);
//							if(usart_debug)
//							{
//										printf("screen_rain=%d\r\n", screen_rain);
//							}
//				}
//		}
//		
//		
//		
//		if(menu->Screen.selection < 0)
//		{
//				menu->Screen.selection = 1;
//		}
//		else if(menu->Screen.selection > 1)
//		{
//				menu->Screen.selection = 0;
//		}
//}

///*�����ݲ�ѯ�˵�����ʾ*/
//static void DayScreen_Display(void *Menu)
//{
//		DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
//		uint8_t highlight=false;
//  
//		memset(disp_buf, 0 , sizeof(disp_buf));
//		snprintf(disp_buf,sizeof(disp_buf),"                ");
//		OLED_ShowString(0, 0,disp_buf,false);
//		OLED_China(40, 0, 10, false);//��
//		OLED_China(56, 0, 58,false);//��
//		OLED_China(72, 0, 76,false);//��
//		
//		snprintf(disp_buf,sizeof(disp_buf),"  %02u  %02u  %02u    ", screen_year, screen_month, screen_day);
//		OLED_ShowString(0, 16,disp_buf,false);
//		OLED_China(32,     16,  8, false);//��
//		OLED_China(64,     16,  9, false);//��
//		OLED_China(96,     16, 10, false);//��
//	
//		snprintf((char *)disp_buf,sizeof(disp_buf),"    :    %3u.%01umm", screen_rain / 10, screen_rain % 10);
//		OLED_ShowString(0, 32,disp_buf,false);
//		OLED_China(0,      32, 28, false);//��
//		OLED_China(16,     32, 29, false);//��
//	
//		snprintf((char *)disp_buf,sizeof(disp_buf),"                ");
//		OLED_ShowString(0, 48,disp_buf,false);
//		highlight = (menu->Screen.selection == 0);
//		OLED_China(8,      48, 77, highlight);//��
//		OLED_China(24,     48, 78, highlight);//һ
//	  OLED_China(40,     48, 10, highlight);//��
//	
//		highlight = (menu->Screen.selection == 1);
//		OLED_China(72,     48, 79, highlight);//��
//		OLED_China(88,     48, 78, highlight);//һ
//		OLED_China(104,    48, 10, highlight);//��
//		
//		OLED_Refresh_Gram();
//}

///*Сʱ���ݲ�ѯ�˵�����*/
//static void HourScreen_ButtonHandler(void *Menu,int16_t button)
//{
//		DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
//	  
//		switch(button)
//		{
//				case ESCAPE:
//					if(menu->prev)
//					{
//							CurrentDisplay 												= menu->prev;
//							CurrentDisplay->Screen.screen_number	=	0;
//							CurrentDisplay->Screen.selection			=	0;
//							OLED_Clear();  /* clear screen */
//					}
//					break;
//				case ENTER:   /* enter next menu */
//					if(menu->Screen.selection == 0)
//					{
//							SubaHour(0, &screen_month1, &screen_day1, &screen_hour1, 0, 0);
//							if(usart_debug)
//							{
//									printf("time:%d-%d-%d\r\n",screen_month1, screen_day1, screen_hour1);
//							}
//					}
//					else if(menu->Screen.selection == 1)
//					{
//							AddaHour(0, &screen_month1, &screen_day1, &screen_hour1, 0, 0);
//							if(usart_debug)
//							{
//									printf("time:%d-%d-%d\r\n",screen_month1, screen_day1, screen_hour1);
//							}
//					}
//					break;
//				case LEFT:
//						menu->Screen.selection--;
//						break;
//				case RIGHT:
//						menu->Screen.selection++;
//						break;
//				default:
//						break;
//		}
//		
//		/*��������֮�󣬿���ȷ��������ʾ��һ���������*/
//		if((screen_month1 > Date.Month) || (screen_day1 > Date.Date) || ((screen_month1 == Date.Month) && (screen_day1 == Date.Date) && (screen_hour1 > Time.Hours)))		/*Ҫ��ѯ��ʱ����ڵ�ǰ��ʱ��*/
//		{
//				screen_rain = 0;
//		}
//		else if((screen_month1 == Date.Month) && (screen_day1 == Date.Date) && (screen_hour1 == Time.Hours))			/*Ҫ��ѯ��ʱ����ǵ�ǰ��ʱ��*/
//		{
//				screen_rain = rain_sample.rain_60m;							/*��ʾ���������͵��ڵ�ǰ��ʱ����*/
//		}
//		else																							/*��ѯ��ǰ��ʱ����*/
//		{
//				screen_rain = 0;
//				memset(data_buf, 0 , sizeof(data_buf));
//				if(read_file_hour(screen_month, screen_day, screen_hour1, data_buf))
//				{
//							screen_rain = (data_buf[145] - 48) * 1000 + (data_buf[146] - 48) * 100 + (data_buf[147] - 48) * 10 + (data_buf[148] - 48);
//							if(usart_debug)
//							{
//										printf("screen_rain=%d\r\n", screen_rain);
//							}
//				}
//		}
//		
//		
//		if(menu->Screen.selection < 0)
//		{
//				menu->Screen.selection = 1;
//		}
//		else if(menu->Screen.selection > 1)
//		{
//				menu->Screen.selection = 0;
//		}
//}

///*Сʱ���ݲ�ѯ�˵�����ʾ*/
//static void HourScreen_Display(void *Menu)
//{
//		DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
//		uint8_t highlight=false;
//  
//		memset(disp_buf, 0, sizeof(disp_buf));
//		snprintf(disp_buf,sizeof(disp_buf),"                ");
//		OLED_ShowString(0, 0,disp_buf,false);
//		OLED_China(32, 0, 73, false);//С
//		OLED_China(48, 0, 14, false);//ʱ
//		OLED_China(64, 0, 58, false);//��
//		OLED_China(80, 0, 76, false);//��
//		
//		memset(disp_buf, 0, sizeof(disp_buf));
//		snprintf(disp_buf,sizeof(disp_buf),"  %02u  %02u  %02u    ", screen_month1, screen_day1, screen_hour1);
//		OLED_ShowString(0, 16,disp_buf,false);
//		OLED_China(32,     16,  9, false);//��
//		OLED_China(64,     16, 10, false);//��
//		OLED_China(96, 		 16, 14, false);//ʱ
//	
//		snprintf(disp_buf, sizeof(disp_buf),"    :    %03u.%01umm", screen_rain / 10, screen_rain % 10);
//		OLED_ShowString(0, 32,disp_buf,false);
//		OLED_China(0,      32, 28, false);//��
//		OLED_China(16,     32, 29, false);//��
//	
//		highlight = (menu->Screen.selection == 0);
//		OLED_China(8,      48, 77, highlight);//��
//		OLED_China(24,     48, 73, highlight);//С
//	  OLED_China(40,     48, 14, highlight);//ʱ
//	
//		highlight = (menu->Screen.selection == 1);
//		OLED_China(72,     48, 79, highlight);//��
//		OLED_China(88,     48, 73, highlight);//С
//		OLED_China(104,    48, 14, highlight);//ʱ
//		
//		OLED_Refresh_Gram();
//}


///*���ò˵�����*/
//static void SetScreen_ButtonHandler(void *Menu,int16_t button)
//{
//		DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
//		
//		switch(button)
//		{
//				case ESCAPE:
//					if(menu->prev)
//					{
//							CurrentDisplay 												=	menu->prev;
//							CurrentDisplay->Screen.screen_number	=	0;
//							CurrentDisplay->Screen.selection			=	2;
//							OLED_Clear();  /* clear screen */
//					}
//					break;
//				case ENTER:   /* enter next menu */
//					if(menu->next)
//					{
//							CurrentDisplay												=	menu->next;
//							CurrentDisplay->Screen.screen_number	=	0;
//							CurrentDisplay->Screen.selection			=	0;
//							OLED_Clear();  /* clear screen */
//  
//							if(menu->Screen.selection == 0)
//							{
//										station_set.station_1 = rain_sample.station / 10000;
//										station_set.station_2 = (rain_sample.station % 10000) / 1000;
//										station_set.station_3 = (rain_sample.station % 1000) / 100;
//										station_set.station_4 = (rain_sample.station % 100) / 10;
//										station_set.station_5 = rain_sample.station % 10;
//							}
//							else if(menu->Screen.selection == 1)
//							{
//										warn_set.warn_1  = rain_sample.rain_60m_warn  / 100;
//										warn_set.warn_2 = (rain_sample.rain_60m_warn % 100) / 10;
//										warn_set.warn_3 =  rain_sample.rain_d_warn / 100;
//										warn_set.warn_4 = (rain_sample.rain_d_warn % 100) / 10;
//							}
//							else if(menu->Screen.selection == 2)
//							{
//									/* get system time */
//									get_sys_time(&setDate, &setTime);
//								
//									nian   =  setDate.Year;
//									yue    =  setDate.Month;
//									ri     =  setDate.Date;
//									shi    =  setTime.Hours;
//									fen    =  setTime.Minutes;
//									miao   =  setTime.Seconds;
//							}
//					}
//					break;
//			case DOWN:
//					menu->Screen.selection++;
//					break;
//			case UP:
//					menu->Screen.selection--;
//					break;
//			default:
//					break;
//		}
//		
//		if(menu->Screen.selection < 0)
//		{
//				menu->Screen.selection = 2;
//		}
//		else if(menu->Screen.selection > 2)
//		{
//				menu->Screen.selection = 0;
//		}
//}

///*���ò˵�����ʾ*/
//static void SetScreen_Display(void *Menu)
//{
//		DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
//		uint8_t highlight=false;
//  
//		OLED_China(48, 0, 15, false);//��
//		OLED_China(64, 0, 16, false);//��
//	
//		highlight = (menu->Screen.selection == 0);
//		OLED_China(32, 16,  7, highlight);//վ
//		OLED_China(48, 16, 80, highlight);//��
//		OLED_China(64, 16, 15, highlight);//��
//		OLED_China(80, 16, 16, highlight);//��
//	
//		highlight = (menu->Screen.selection == 1);
//		OLED_China(32, 32, 82, highlight);//��
//		OLED_China(48, 32, 83, highlight);//��
//		OLED_China(64, 32, 15, highlight);//��
//		OLED_China(80, 32, 16, highlight);//��
//	
//		highlight = (menu->Screen.selection == 2);
//		OLED_China(32, 48, 14, highlight);//ʱ
//		OLED_China(48, 48, 68, highlight);//��
//		OLED_China(64, 48, 15, highlight);//��
//		OLED_China(80, 48, 16, highlight);//��
//  
//		/* Determine the next Menu */
//		switch(menu->Screen.selection)
//		{
//				case 0:   /* station set menu*/
//					menu->next = &StationMenu;
//					break;
//				case 1:   /*  warn set  Menu */
//					menu->next = &WarnMenu;
//					break;
//				case 2:   /* timer set Menu */
//					menu->next = &TimerMenu;
//					break;
//				default:
//					menu->next = &StationMenu;
//					break;
//		}
//		
//		OLED_Refresh_Gram();
//}

///*վ��������*/
//static void StationScreen_ButtonHandler(void *Menu,int16_t button)
//{
//  DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
//  int16_t selected_timevalue=0,min_value=0,max_value=0;
//  int *selected=NULL;
//	
//  switch(menu->Screen.selection)
//  {
//			case 0:  
//				break;
//			case 1:  
//				selected           = &station_set.station_1 ;
//				selected_timevalue = station_set.station_1;
//				min_value          = 0;
//				max_value          = 9;
//				break;
//			case 2:  /* day */
//				selected           = &station_set.station_2 ;
//				selected_timevalue = station_set.station_2;
//				min_value          = 0;
//				max_value          = 9;
//				break;
//			case 3:  /* hour */
//				selected           = &station_set.station_3 ;
//				selected_timevalue = station_set.station_3;
//				min_value          = 0;
//				max_value          = 9;
//				break;
//			case 4:  /* minute */
//				selected           = &station_set.station_4 ;
//				selected_timevalue = station_set.station_4;
//				min_value          = 0;
//				max_value          = 9;
//				break;
//			case 5:  /* second */
//				selected           = &station_set.station_5 ;
//				selected_timevalue = station_set.station_5;
//				min_value          = 0;
//				max_value          = 9;
//				break;
//			default:
//				selected           = &station_set.station_5 ;
//				selected_timevalue = station_set.station_5;
//				min_value          = 0;
//				max_value          = 9;
//				break;
//  }
//	
//	rain_sample.station = station_set.station_1 * 10000 + station_set.station_2 * 1000 + station_set.station_3 * 100 + station_set.station_4 * 10 + station_set.station_5;
//  
//  switch(button)
//  {
//			case ESCAPE:
//				if(menu->prev)
//					{
//						rain_sample.station = station_set.station_1 * 10000 + station_set.station_2 * 1000 + station_set.station_3 * 100 + station_set.station_4 * 10 + station_set.station_5  ;
//						if(data_eeprom_write(STATION_ADDR, 	(uint8_t *)&rain_sample.station,				4) == HAL_OK)
//						{
//								if(usart_debug)
//								{
//										printf("write STATION_ADDR ok=%d\r\n", rain_sample.station);
//								}
//						}
//						else
//						{
//								if(usart_debug)
//								{
//										printf("write STATION_ADDR failed\r\n");
//								}
//						}
//						CurrentDisplay 												= menu->prev;
//						CurrentDisplay->Screen.screen_number	=	0;
//						CurrentDisplay->Screen.selection			=	0;
//						OLED_Clear();  /* clear screen */
//					}
//				break;
//			case ENTER:
//				menu->Screen.selection++;
//				break;
//			case UP:
//				selected_timevalue++;
//				break;
//			case DOWN:
//				selected_timevalue--;
//				break;
//			default:
//				break;
//  }
//  
//	
//  if(menu->Screen.selection<0)
//  {
//    menu->Screen.selection = 5;
//  }
//  else if(menu->Screen.selection > 5)
//  {
//    menu->Screen.selection = 0;
//  }
//  
//  if(selected_timevalue < min_value)
//  {
//    selected_timevalue = max_value;
//  }
//  else if(selected_timevalue > max_value)
//  {
//    selected_timevalue = min_value;
//  }
//  /* set selected value */
//  *selected=selected_timevalue;
//}

///*վ������ʾ*/
//static void StationScreen_Display(void *Menu)
//{
//		DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
//		uint8_t highlight=false;
// 
//		OLED_China(32, 0,  7, false);//վ
//		OLED_China(48, 0, 80, false);//��
//		OLED_China(64, 0, 15, false);//��
//		OLED_China(80, 0, 16, false);//��
//		
//		OLED_Char(48,  40, ':',false);
//		highlight = (menu->Screen.selection == 0);
//		OLED_China(0,  40, 81, highlight);//̨
//		OLED_China(16, 40,  7, highlight);//վ
//		OLED_China(32, 40, 80, highlight);//��

//		highlight = (menu->Screen.selection == 1);
//		snprintf((char *)disp_buf,sizeof(disp_buf),"%01u",station_set.station_1 );
//		OLED_ShowString(88,40,disp_buf,highlight);
//		
//		highlight = (menu->Screen.selection == 2);
//		snprintf((char *)disp_buf,sizeof(disp_buf),"%01u",station_set.station_2 );
//		OLED_ShowString(96,40,disp_buf,highlight);
//		
//		highlight = (menu->Screen.selection == 3);
//		snprintf((char *)disp_buf,sizeof(disp_buf),"%01u",station_set.station_3 );
//		OLED_ShowString(104,40,disp_buf,highlight);
//		
//		highlight = (menu->Screen.selection == 4);
//		snprintf((char *)disp_buf,sizeof(disp_buf),"%01u",station_set.station_4 );
//		OLED_ShowString(112,40,disp_buf,highlight);
//		
//		highlight = (menu->Screen.selection == 5);
//		snprintf((char *)disp_buf,sizeof(disp_buf),"%01u",station_set.station_5 );
//		OLED_ShowString(120,40,disp_buf,highlight);
//		
//		OLED_Refresh_Gram();
//}	


///*����������*/
//static void WarnScreen_ButtonHandler(void *Menu,int16_t button)
//{
//		DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
//		int16_t selected_timevalue=0,min_value=0,max_value=0;
//		int *selected=NULL;
//		
//		switch(menu->Screen.screen_number)
//		{
//				case 0:  
//					break;
//				case 1:  
//					if(menu->Screen.selection == 0)
//					{
//							selected           = &warn_set.warn_3;
//							selected_timevalue = warn_set.warn_3;
//							min_value          = 0;
//							max_value          = 9;
//					}
//					else if(menu->Screen.selection == 1)
//					{
//							selected           = &warn_set.warn_1;
//							selected_timevalue = warn_set.warn_1;
//							min_value          = 0;
//							max_value          = 9;
//					}
//					break;
//				case 2:  
//					if(menu->Screen.selection == 0)
//					{
//							selected           = &warn_set.warn_4;
//							selected_timevalue = warn_set.warn_4;
//							min_value          = 0;
//							max_value          = 9;
//					}
//					else if(menu->Screen.selection == 1)
//					{
//							selected           = &warn_set.warn_2;
//							selected_timevalue = warn_set.warn_2;
//							min_value          = 0;
//							max_value          = 9;
//					}
//					break;
//				default:
//					break;
//		}
//	
//		rain_sample.rain_60m_warn = warn_set.warn_1 * 100 + warn_set.warn_2 * 10;
//		rain_sample.rain_d_warn 	= warn_set.warn_3 * 100 + warn_set.warn_4 * 10;
//  
//		switch(button)
//		{
//				case ESCAPE:
//					if(menu->prev)
//						{
//								if(data_eeprom_write(WARN_60M_ADDR, 	(uint8_t *)&rain_sample.rain_60m_warn, 	2) == HAL_OK)
//								{
//										if(usart_debug)
//										{
//											printf("write WARN_60M_ADDR ok=%d\r\n", rain_sample.rain_60m_warn);
//										}	
//								}
//								else
//								{
//										if(usart_debug)
//											{
//													printf("write WARN_60M_ADDR failed\r\n");
//											}	
//								}
//								if(data_eeprom_write(WARN_1D_ADDR, 	(uint8_t *)&rain_sample.rain_d_warn, 		2) == HAL_OK)
//								{
//										if(usart_debug)
//											{
//													printf("write WARN_1D_ADDR ok=%d\r\n", rain_sample.rain_d_warn);
//											}	
//								}
//								else
//								{
//										if(usart_debug)
//											{
//													printf("write WARN_1D_ADDR failed\r\n");
//											}	
//								}
//							
//								CurrentDisplay = menu->prev;
//								CurrentDisplay->Screen.screen_number=0;
//								CurrentDisplay->Screen.selection=1;
//								OLED_Clear();  /* clear screen */
//						}
//					break;
//				case ENTER:
//					menu->Screen.screen_number++;
//					break;
//				case UP:
//					if(menu->Screen.screen_number == 0)
//					{
//							menu->Screen.selection++;
//					}
//					else
//					{
//							selected_timevalue++;
//					}
//					break;
//				case DOWN:
//					if(menu->Screen.screen_number == 0)
//					{
//							menu->Screen.selection--;
//					}
//					else
//					{
//							selected_timevalue--;
//					}
//					break;
//				default:
//					break;
//		}
//  
//		if(menu->Screen.selection < 0)
//		{
//				menu->Screen.selection = 1;
//		}
//		else if(menu->Screen.selection > 1)
//		{
//				menu->Screen.selection = 0;
//		}
//		
//		if(menu->Screen.screen_number < 0)
//		{
//				menu->Screen.screen_number = 2;
//		}
//		else if(menu->Screen.screen_number > 2)
//		{
//				menu->Screen.screen_number = 0;
//		}
//		
//		if(selected_timevalue < min_value)
//		{
//				selected_timevalue = max_value;
//		}
//		else if(selected_timevalue > max_value)
//		{
//				selected_timevalue = min_value;
//		}
//		/* set selected value */
//		*selected = selected_timevalue;
//}

///*��������ʾ*/
//static void WarnScreen_Display(void *Menu)
//{
//		memset(disp_buf, 0, sizeof(disp_buf));
//		snprintf((char *)disp_buf, sizeof(disp_buf), "                ");
//		OLED_ShowString(0, 0, disp_buf, false);
//		OLED_ShowString(0, 16, disp_buf, false);
//	
//		DisplayMenuTypeDef *menu = (DisplayMenuTypeDef *)Menu;
//		uint8_t highlight = false;
// 
//		OLED_China(32, 0, 82, false);//��
//		OLED_China(48, 0, 83, false);//��
//		OLED_China(64, 0, 15, false);//��
//		OLED_China(80, 0, 16, false);//��
//		
//		OLED_Char(48,  32, ':',false);
//		OLED_Char(112, 32, 'm',false);
//		OLED_Char(120, 32, 'm',false);
//		highlight = ((menu->Screen.selection == 0) && (menu->Screen.screen_number == 0));
//		OLED_China(0,  32, 10, highlight);//��
//		OLED_China(16, 32, 28, highlight);//��
//		OLED_China(32, 32, 29, highlight);//��
//	
//		OLED_Char(48,  48, ':',false);
//		OLED_Char(112, 48, 'm',false);
//		OLED_Char(120, 48, 'm',false);
//		highlight = ((menu->Screen.selection == 1) && (menu->Screen.screen_number == 0));
//		OLED_China(0,  48, 14, highlight);//ʱ
//		OLED_China(16, 48, 28, highlight);//��
//		OLED_China(32, 48, 29, highlight);//��

//		/*������*/
//		highlight = ((menu->Screen.selection == 0) && (menu->Screen.screen_number == 1));
//		snprintf((char *)disp_buf, sizeof(disp_buf), "%01u",warn_set.warn_3);
//		OLED_ShowString(96, 32, disp_buf, highlight);
//		
//		highlight = ((menu->Screen.selection == 0) && (menu->Screen.screen_number == 2));
//		snprintf((char *)disp_buf, sizeof(disp_buf), "%01u", warn_set.warn_4 );
//		OLED_ShowString(104, 32, disp_buf, highlight);
//		
//		highlight = ((menu->Screen.selection == 1) && (menu->Screen.screen_number == 1));
//		snprintf((char *)disp_buf,sizeof(disp_buf),"%01u",warn_set.warn_1 );
//		OLED_ShowString(96, 48, disp_buf, highlight);
//		
//		highlight = ((menu->Screen.selection == 1) && (menu->Screen.screen_number == 2));
//		snprintf((char *)disp_buf, sizeof(disp_buf), "%01u",warn_set.warn_2);
//		OLED_ShowString(104, 48, disp_buf, highlight);
//		
//		
//		OLED_Refresh_Gram();
//}	

///*ʱ��������*/
//static void TiemrScreen_ButtonHandler(void *Menu,int16_t button)
//{
//			DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
//			int16_t selected_timevalue=0,min_value=0,max_value=0;
//			uint8_t *selected=NULL;
//			
//			switch(menu->Screen.selection)
//			{
//			case 0:  /* year */
//				selected=&setDate.Year;
//				selected_timevalue=setDate.Year;
//				min_value=0;
//				max_value=99;
//				break;
//			case 1:  /* month */
//				selected=&setDate.Month;
//				selected_timevalue=setDate.Month;
//				min_value=1;
//				max_value=12;
//				break;
//			case 2:  /* day */
//				selected=&setDate.Date;
//				selected_timevalue=setDate.Date;
//				min_value=1;
//				max_value=31;
//				break;
//			case 3:  /* hour */
//				selected=&setTime.Hours;
//				selected_timevalue=setTime.Hours;
//				min_value=0;
//				max_value=23/*59*/;   /* 16.3.23 hour:0-23 */
//				break;
//			case 4:  /* minute */
//				selected=&setTime.Minutes;
//				selected_timevalue=setTime.Minutes;
//				min_value=0;
//				max_value=59;
//				break;
//			case 5:  /* second */
//				selected=&setTime.Seconds;
//				selected_timevalue=setTime.Seconds;
//				min_value=0;
//				max_value=59;
//				break;
//			default:
//				menu->Screen.selection=5;
//				selected=&setTime.Seconds;
//				selected_timevalue=setTime.Seconds;
//				min_value=0;
//				max_value=59;
//				break;
//			}
//			
//			switch(button)
//			{
//			case ESCAPE:
//				if((setDate.Year!=nian)||(setDate.Month!=yue)||(setDate.Date!=ri)||(setTime.Hours!=shi)||(setTime.Minutes!=fen)||(setTime.Seconds!=miao))
//				{
//					/* set data&time */
//					/* fill unused value */
//					setTime.DayLightSaving=RTC_DAYLIGHTSAVING_NONE;
//					setTime.StoreOperation=RTC_STOREOPERATION_RESET;
//					setTime.SubSeconds=0;
//					setTime.TimeFormat=RTC_HOURFORMAT12_AM;
//					if(set_sys_time(&setDate,&setTime)<0)
//						{
//							OLED_Clear();
//							OLED_ShowString(0,24,"set time failed!",1);
//							OLED_Refresh_Gram();
//						}
//						else
//							{
//								OLED_Clear();
//								OLED_ShowString(16,24,"set time ok!",1);
//								OLED_Refresh_Gram();
//							}
//							osDelay(500);
//				}
//				if(menu->prev)
//					{
//						CurrentDisplay=menu->prev;
//						CurrentDisplay->Screen.screen_number=0;
//						CurrentDisplay->Screen.selection=2;
//						OLED_Clear();  /* clear screen */
//					}
//				break;
//			case ENTER:
//				menu->Screen.selection++;
//				break;
//			case UP:
//				selected_timevalue++;
//				break;
//			case DOWN:
//				selected_timevalue--;
//				break;
//			default:
//				break;
//			}
//			
//			if(menu->Screen.selection<0)
//			{
//				menu->Screen.selection=5;
//			}
//			else if(menu->Screen.selection>5)
//			{
//				menu->Screen.selection=0;
//			}
//			
//			if(selected_timevalue<min_value)
//			{
//				selected_timevalue=max_value;
//			}
//			else if(selected_timevalue>max_value)
//			{
//				selected_timevalue=min_value;
//			}
//			/* set selected value */
//			*selected=selected_timevalue;
//}

///*ʱ������ʾ*/
//static void TimerScreen_Display(void *Menu)
//{
//			DisplayMenuTypeDef *menu=(DisplayMenuTypeDef *)Menu;
//			uint8_t highlight=false;
//		 
//			/* year */
//			OLED_ShowString(33,0,"20",false);//��20��
//			snprintf(disp_buf,sizeof(disp_buf),"%02u",setDate.Year);
//			highlight=(menu->Screen.selection==0);
//			OLED_ShowString(49,0,disp_buf,highlight);//��17�ꡱ
//			OLED_China(73,0,8,false);//��
//			
//			/* month */
//			snprintf(disp_buf,sizeof(disp_buf),"%02u",setDate.Month);
//			highlight=(menu->Screen.selection==1);
//			OLED_ShowString(25,24,disp_buf,highlight);//��06�¡�
//			OLED_China(49,24,9,false);//��
//			
//			/* day */
//			snprintf(disp_buf,sizeof(disp_buf),"%02u",setDate.Date);
//			highlight=(menu->Screen.selection==2);
//			OLED_ShowString(65,24,disp_buf,highlight);//��22�ա�
//			OLED_China(89,24,10,false);//��
//			
//			/* hour */
//			snprintf(disp_buf,sizeof(disp_buf),"%02u",setTime.Hours);
//			highlight=(menu->Screen.selection==3);
//			OLED_ShowString(9,48,disp_buf,highlight);//��10�㡱
//			OLED_China(33,48,14,false);//ʱ
//			
//			/* minute */
//			snprintf(disp_buf,sizeof(disp_buf),"%02u",setTime.Minutes);
//			highlight=(menu->Screen.selection==4);
//			OLED_ShowString(49,48,disp_buf,highlight);//��10�֡�
//			OLED_China(73,48,27,false);//��
//			
//			/* second */
//			snprintf(disp_buf,sizeof(disp_buf),"%02u",setTime.Seconds);
//			highlight=(menu->Screen.selection==5);
//			OLED_ShowString(89,48,disp_buf,highlight);//��10�롱
//			OLED_China(108,48,41,false);//��
//			
//			OLED_Refresh_Gram();
//}	

//#endif

///*�̵���ʱ*/
//__STATIC_INLINE void short_delay(void)
//{
//  volatile uint32_t i=0;
//  
//	for(i=0;i<3000;i++);
////  while(i--)
////  {
////    asm("nop");
////  }
//}

//__STATIC_INLINE void turn_off_display(void)
//{
//  /* turn off display */
//  disp_on = false;
//  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_RESET);     /*��ʼ��PB14Ϊ�ߵ�ƽ �������*/ /* backlight off */
//  OLED_Clear();  /* clear screen */
//  
//  CurrentDisplay = &MainMenu;  /* display main menu */
//  CurrentDisplay->Screen.screen_number=0;
//  CurrentDisplay->Screen.selection=0;
//}

///**
//  * @brief  turn on display
//  * @param  None
//  * @retval None
//  */
//__STATIC_INLINE void turn_on_display(void)
//{
//		/* turn on display */
//		disp_on	=	true;  /* display on */
//		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_SET);     /*��ʼ��PB14Ϊ�ߵ�ƽ �������*/ /* backlight on */
//		disp_on_count = MAX_DISPLAY_ON_TIME;
//}

