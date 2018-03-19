#include "exti.h"
#include "delay.h" 
#include "ov7670.h"
#include "led.h"
#include "Pic_Process.h"
#include "usart.h"	
u8 ov_sta;
extern volatile u8 ThereIsACircle; //Pic_Process.c�ж���
extern volatile u8 timeIsUp;
extern u16 AddWaterTime;
extern u8 AddWaterNum;
u8 Processing = 0;
 //�ⲿ�ж�5~9������򣬶�ȡ����һ֡��ͼ��
void EXTI9_5_IRQHandler(void)
{		 		
	if(EXTI->PR&(1<<9))			//��9�ߵ��ж�
	{     
		EXTI->PR=1<<9;     //���LINE9�ϵ��жϱ�־λ	

				OV7670_WRST=0;	 	//��λдָ��	
				delay_us(1);
				OV7670_WRST=1;	
				OV7670_WREN=1;		//����д��FIFO
				ov_sta++;
				delay_us(200);
	}
						  
} 
 //3���жϷ�����
void EXTI3_IRQHandler(void)
{		
	if(EXTI->PR&(1<<3))			//��3�ߵ��ж�
	{ 
		EXTI->PR=1<<3;     //���LINE3�ϵ��жϱ�־λ	
		delay_ms(500);
		if(GlassArea == 1)
		{
			Glass_Light_On = 0;
			Light_On=0; //�ƹ�ر�
			On_Off = 0; 	//��
			ThereIsACircle = 0;
//			AddWaterNum = 0;
			Processing = 0;
			Light_On_Glass_Water = 0; //ˮ�ƹر�
		}
		else if(Processing == 0 && GlassArea == 0)
		{		
				
//				TIM4->CR1&=0x00;							//�رն�ʱ��
//				TIM4->CNT = 0;               //����������			
				TIM2->CNT = 0;
			  timeIsUp = 0;
			  AddWaterTime = 0;
			if(GlassAreaAssitSignal == 0) //��͸����
			{
								Light_On=1; 			//�ƹ��
							  printf("0"); 
			}else if(GlassAreaAssitSignal == 1) //͸����
			{
								Glass_Light_On = 1;
								printf("1"); 
			}

				TIM3->CR1|=0x01;	//ʹ�ܶ�ʱ��

				//delay_ms(100);		//�����һ���ж�
				//TIM3->SR&=~(1<<0);	//����жϱ�־λ	

			
//				TIM3->CNT = 0;
//				TIM3->EGR = 1;
//				while((TIM3->SR&0x01) == 0)
//				;
//		  	TIM3->SR&=~(1<<0);	//����жϱ�־λ	
//				TIM3->DIER|=1<<0;   //��������ж�
		}
	} 
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
	MY_NVIC_Init(1,0,EXTI3_IRQn,2);		//��ռ1,�����ȼ�0����2	
}

