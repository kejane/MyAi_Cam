/**
  ******************************************************************************
  * @file    tim.c
  * @brief   This file provides code for the configuration
  *          of the TIM instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "tim.h"
#include "main.h"
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

TIM_HandleTypeDef htim4;

/* TIM4 init function */
void MX_TIM4_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 240;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 10000;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
      rt_kprintf("error!\r\n");
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    rt_kprintf("error!\r\n");
  }
  if (HAL_TIM_IC_Init(&htim4) != HAL_OK)
  {
      rt_kprintf("error!\r\n");
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
      rt_kprintf("error!\r\n");
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 8;
  if (HAL_TIM_IC_ConfigChannel(&htim4, &sConfigIC, TIM_CHANNEL_4) != HAL_OK)
  {
      rt_kprintf("error!\r\n");
  }
		HAL_TIM_IC_Start_IT(&htim4,TIM_CHANNEL_4);   //��ʼ����TIM4��ͨ��4
	__HAL_TIM_ENABLE_IT(&htim4,TIM_IT_UPDATE);   //ʹ�ܸ����ж�
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(tim_baseHandle->Instance==TIM4)
  {
  /* USER CODE BEGIN TIM4_MspInit 0 */

  /* USER CODE END TIM4_MspInit 0 */
    /* TIM4 clock enable */
    __HAL_RCC_TIM4_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**TIM4 GPIO Configuration
    PB9     ------> TIM4_CH4
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* TIM4 interrupt Init */
    HAL_NVIC_SetPriority(TIM4_IRQn, 1, 1);
    HAL_NVIC_EnableIRQ(TIM4_IRQn);
  /* USER CODE BEGIN TIM4_MspInit 1 */

  /* USER CODE END TIM4_MspInit 1 */
  }
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* tim_baseHandle)
{

  if(tim_baseHandle->Instance==TIM4)
  {
  /* USER CODE BEGIN TIM4_MspDeInit 0 */

  /* USER CODE END TIM4_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM4_CLK_DISABLE();

    /**TIM4 GPIO Configuration
    PB9     ------> TIM4_CH4
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_9);

    /* TIM4 interrupt Deinit */
    HAL_NVIC_DisableIRQ(TIM4_IRQn);
  /* USER CODE BEGIN TIM4_MspDeInit 1 */

  /* USER CODE END TIM4_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
//ң��������״̬
//[7]:�յ����������־
//[6]:�õ���һ��������������Ϣ
//[5]:����	
//[4]:����������Ƿ��Ѿ�������								   
//[3:0]:�����ʱ��
uint8_t 	RmtSta=0;	  	  
uint16_t Dval;		//�½���ʱ��������ֵ
uint32_t RmtRec=0;	//������յ�������	   		    
uint8_t  RmtCnt=0;	//�������µĴ���	 

//��ʱ��4�ж˷�����
void TIM4_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim4);//��ʱ�����ô�����
} 
//��ʱ�����£�������жϻص�����
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance==TIM4)
	{
 		if(RmtSta&0x80)//�ϴ������ݱ����յ���
		{	
			RmtSta&=~0X10;						//ȡ���������Ѿ���������
			if((RmtSta&0X0F)==0X00)RmtSta|=1<<6;//����Ѿ����һ�ΰ����ļ�ֵ��Ϣ�ɼ�
			if((RmtSta&0X0F)<14)RmtSta++;
			else
			{
				RmtSta&=~(1<<7);//���������ʶ
				RmtSta&=0XF0;	//��ռ�����	
			}						 	   	
		}	
	}
}

//��ʱ�����벶���жϻص�����
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)//�����жϷ���ʱִ��
{
	if(htim->Instance==TIM4)
	{
		if(RDATA)//�����ز���
		{
			TIM_RESET_CAPTUREPOLARITY(&htim4,TIM_CHANNEL_4);   					//һ��Ҫ�����ԭ�������ã���
			TIM_SET_CAPTUREPOLARITY(&htim4,TIM_CHANNEL_4,TIM_ICPOLARITY_FALLING);//CC4P=1 ����Ϊ�½��ز���
			__HAL_TIM_SET_COUNTER(&htim4,0);  	//��ն�ʱ��ֵ   	  
		  	RmtSta|=0X10;							//����������Ѿ�������
		}else //�½��ز���
		{
			Dval=HAL_TIM_ReadCapturedValue(&htim4,TIM_CHANNEL_4);//��ȡCCR4Ҳ������CC4IF��־λ
			TIM_RESET_CAPTUREPOLARITY(&htim4,TIM_CHANNEL_4);   //һ��Ҫ�����ԭ�������ã���
			TIM_SET_CAPTUREPOLARITY(&htim4,TIM_CHANNEL_4,TIM_ICPOLARITY_RISING);//����TIM4ͨ��4�����ز���
			if(RmtSta&0X10)							//���һ�θߵ�ƽ���� 
			{
 				if(RmtSta&0X80)//���յ���������
				{
					
					if(Dval>300&&Dval<800)			//560Ϊ��׼ֵ,560us
					{
						RmtRec<<=1;					//����һλ.
						RmtRec|=0;					//���յ�0	   
					}else if(Dval>1400&&Dval<1800)	//1680Ϊ��׼ֵ,1680us
					{
						RmtRec<<=1;					//����һλ.
						RmtRec|=1;					//���յ�1
					}else if(Dval>2200&&Dval<2600)	//�õ�������ֵ���ӵ���Ϣ 2500Ϊ��׼ֵ2.5ms
					{
						RmtCnt++; 					//������������1��
						RmtSta&=0XF0;				//��ռ�ʱ��		
					}
 				}else if(Dval>4200&&Dval<4700)		//4500Ϊ��׼ֵ4.5ms
				{
					RmtSta|=1<<7;					//��ǳɹ����յ���������
					RmtCnt=0;						//�����������������
				}						 
			}
			RmtSta&=~(1<<4);
		}
	}
}
//����������
//����ֵ:
//	 0,û���κΰ�������
//����,���µİ�����ֵ.
uint8_t Remote_Scan(void)
{        
	uint8_t sta=0;       
	uint8_t t1,t2;  
	if(RmtSta&(1<<6))//�õ�һ��������������Ϣ��
	{ 
	    t1=RmtRec>>24;			//�õ���ַ��
	    t2=(RmtRec>>16)&0xff;	//�õ���ַ���� 
	        t1=RmtRec>>8;
	        t2=RmtRec; 	
	        if(t1==(uint8_t)~t2)sta=t1;//��ֵ��ȷ	 
		//}   
		if((sta==0)||((RmtSta&0X80)==0))//�������ݴ���/ң���Ѿ�û�а�����
		{
		 	RmtSta&=~(1<<6);//������յ���Ч������ʶ
			RmtCnt=0;		//�����������������
		}
	}  
    return sta;
}
/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
