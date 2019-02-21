#include "key.h"

//������ʼ������
void KEY_Init(void) //IO��ʼ��
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	__HAL_RCC_GPIOA_CLK_ENABLE();
	
	/*Configure GPIO pins : PA1 PA4 PA5 PA6 
                           PA7 PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6 
                          |GPIO_PIN_7|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
}

//����������
//���ذ���ֵ
//mode:0,��֧��������;1,֧��������;
uint8_t KEY_Scan(uint8_t mode)
{
	static uint8_t key_up=1;//�������ɿ���־
	if(mode)key_up=1;  //֧������		  
	if(key_up&&KEY_State&&(KEY_Escape==0||KEY_Enter==0||KEY_Right==0||KEY_Left==0||KEY_Data==0))
	{
		HAL_Delay(10);//ȥ���� 
		key_up=0;
		if((KEY_Escape==0)&&KEY_State)     return KEY_Escape_PRES;
		else if((KEY_Enter==0)&&KEY_State) return KEY_Enter_PRES;
		else if((KEY_Right==0)&&KEY_State) return KEY_Right_PRES;
		else if((KEY_Left==0)&&KEY_State)  return KEY_Left_PRES;
		else if((KEY_Data==0)&&KEY_State)  return KEY_Data_PRES;
	}
	else if(KEY_Escape==1&&KEY_Enter==1&&KEY_Right==1&&KEY_Left==1&&KEY_Data==1&&KEY_State==0)
		key_up=1;
	  return 0;//�ް�������
}







