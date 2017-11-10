#include "exti.h"
#include "delay.h" 
#include "led.h" 
#include "key.h"
#include "beep.h"
#include "ov7670.h"
//#include "ov7670cfg.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//�ⲿ�ж� ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/4
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 

//�ⲿ�ж�0�������
void EXTI0_IRQHandler(void)
{
	delay_ms(10);	//����
	if(WK_UP==1)	 
	{
		BEEP=!BEEP; 
	}		 
	EXTI->PR=1<<0;  //���LINE0�ϵ��жϱ�־λ  
}	
//�ⲿ�ж�2�������
void EXTI2_IRQHandler(void)
{
	delay_ms(10);	//����
	if(KEY2==0)	  
	{	 
		LED0=!LED0; 			 
	}		 
	EXTI->PR=1<<2;  //���LINE2�ϵ��жϱ�־λ  
}
//�ⲿ�ж�3�������
void EXTI3_IRQHandler(void)
{
	delay_ms(10);	//����
	if(KEY1==0)	 
	{
		LED1=!LED1;
	}		 
	EXTI->PR=1<<3;  //���LINE3�ϵ��жϱ�־λ  
}
//�ⲿ�ж�4�������
void EXTI4_IRQHandler(void)
{
	delay_ms(10);	//����
	if(KEY0==0)	 
	{		
		LED0=!LED0;	
		LED1=!LED1;		
	}		 
	EXTI->PR=1<<4;  //���LINE4�ϵ��жϱ�־λ  
}
	   
//�ⲿ�жϳ�ʼ������
//��ʼ��PE2~4,PA0Ϊ�ж�����.
//void EXTIX_Init(void)
//{
//	KEY_Init(); 
//	Ex_NVIC_Config(GPIO_E,2,FTIR); 		//�½��ش���
//	Ex_NVIC_Config(GPIO_E,3,FTIR); 		//�½��ش���
//	Ex_NVIC_Config(GPIO_E,4,FTIR); 		//�½��ش���
// 	Ex_NVIC_Config(GPIO_A,0,RTIR); 	 	//�����ش��� 
//	MY_NVIC_Init(3,2,EXTI2_IRQn,2);		//��ռ3�������ȼ�2����2
//	MY_NVIC_Init(2,2,EXTI3_IRQn,2);		//��ռ2�������ȼ�2����2	   
//	MY_NVIC_Init(1,2,EXTI4_IRQn,2);		//��ռ1�������ȼ�2����2	   
//	MY_NVIC_Init(0,2,EXTI0_IRQn,2);		//��ռ0�������ȼ�2����2	   
//}

u8 ov_sta;
 //�ⲿ�ж�5~9�������
void EXTI9_5_IRQHandler(void)
{		 		
	if(EXTI->PR&(1<<9))//��9�ߵ��ж�
	{     
		if(ov_sta<2)
		{
			if(ov_sta==0)
			{
				OV7670_WRST=0;	 	//��λдָ��	
				delay_us(1);
				OV7670_WRST=1;	
				OV7670_WREN=1;		//����д��FIFO
			}else OV7670_WREN=0;	//��ֹд��FIFO 	 
			ov_sta++;
		}
	}
	EXTI->PR=1<<9;     //���LINE9�ϵ��жϱ�־λ						  
} 
//�ⲿ�ж�9��ʼ��
void EXTI9_Init(void)
{												  
	Ex_NVIC_Config(GPIO_F,9,RTIR); 			//������ش���			  
	MY_NVIC_Init(0,0,EXTI9_5_IRQn,2);		//��ռ0,�����ȼ�0����2	   
}












