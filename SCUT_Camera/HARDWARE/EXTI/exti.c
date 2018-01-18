#include "exti.h"
#include "delay.h" 
#include "ov7670.h"
#include "led.h"
#include "Pic_Process.h"
u8 ov_sta;
extern volatile u8 ThereIsACircle; //Pic_Process.c�ж���

 //�ⲿ�ж�5~9������򣬶�ȡ����һ֡��ͼ��
void EXTI9_5_IRQHandler(void)
{		 		
	if(EXTI->PR&(1<<9))			//��9�ߵ��ж�
	{     

				OV7670_WRST=0;	 	//��λдָ��	
				delay_us(1);
				OV7670_WRST=1;	
				OV7670_WREN=1;		//����д��FIFO
				ov_sta++;
				delay_us(200);
	}
	EXTI->PR=1<<9;     //���LINE9�ϵ��жϱ�־λ						  
} 
 //3���жϷ�����
void EXTI3_IRQHandler(void)
{		
		delay_ms(50);
		if(GlassArea == 1)
		{
			Light_On=0; //�ƹ�ر�
			On_Off = 0; 	//��
			ThereIsACircle = 0;
		}
		else
		{		Light_On=1; //�ƹ��
				TIM3->CR1|=0x01;	//ʹ�ܶ�ʱ��	
				delay_ms(100);		//�����һ���ж�
				TIM3->SR&=~(1<<0);	//����жϱ�־λ				
		}

	EXTI->PR=1<<3;     //���LINE3�ϵ��жϱ�־λ						  
} 




//�ⲿ�ж�9��ʼ��
void EXTI9_Init(void)
{												  
	Ex_NVIC_Config(GPIO_F,9,RTIR); 			//�����ش���			  
	MY_NVIC_Init(0,0,EXTI9_5_IRQn,2);		//��ռ0,�����ȼ�0����2	   
}

void EXTIX_Init(void)	//�ⲿ�жϳ�ʼ��	
{	
	Ex_NVIC_Config(GPIO_D,3,3); 			//2018.1.17��
	MY_NVIC_Init(3,0,EXTI3_IRQn,2);		//��ռ1,�����ȼ�0����2	
}

