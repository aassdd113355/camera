#include "timer.h"
#include "Pic_Process.h"
#include "delay.h" 
#include "led.h"
#include "usart.h"
extern int flag;
extern volatile u8 ThereIsACircle; //Pic_Process.c�ж���
u8 TIM3_count = 0;
u8 TIM2_count = 0;
extern u8 timeIsUp;//��ʱ��ʱ���þ���־ main����
//ͨ�ö�ʱ��3�жϳ�ʼ��
//����ʱ��ѡ��ΪAPB1��2������APB1Ϊ42M
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//��ʱ�����ʱ����㷽��:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=��ʱ������Ƶ��,��λ:Mhz
//����ʹ�õ��Ƕ�ʱ��3
void TIM3_Int_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<1;	//TIM3ʱ��ʹ��    
 	TIM3->ARR=arr;  	  //�趨�������Զ���װֵ 
	TIM3->PSC=psc;  	  //Ԥ��Ƶ��
		
	TIM3->DIER|=1<<0;   //��������ж�
	TIM3->SR&=~(1<<0);	//����жϱ�־λ
  MY_NVIC_Init(1,0,TIM3_IRQn,2);	//��ռ0�������ȼ�0����2	
	
	TIM3->CR1|=0x01;	//����ʱ��   By Bachman 2018 - 01 -19

}

void TIM2_Int_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<0;	//TIM2ʱ��ʹ��    
 	TIM2->ARR=arr;  	  //�趨�������Զ���װֵ 
	TIM2->PSC=psc;  	  //Ԥ��Ƶ��
		
	TIM2->DIER|=1<<0;   //��������ж�
	TIM2->SR&=~(1<<0);	//����жϱ�־λ
  MY_NVIC_Init(1,0,TIM2_IRQn,2);	//��ռ0�������ȼ�0����2	
}

//��ʱ��3�жϷ������	 
void TIM3_IRQHandler(void)
{ 		   
		TIM3->CR1&=0x00;	//�رն�ʱ��   By Bachman 2018 - 01 -18
		TIM3_count++;
	if(TIM3_count >= 2)
	{			
				TIM3->CNT = 0;    // ����������  By Bachman 2018 - 01 -18
				if(TIM3->SR&0X0001)//����ж�
				{
										TIM3->SR&=~(1<<0);//����жϱ�־λ 
										if(GlassArea == 0 && GlassAreaAssitSignal == 0)
										{
											printf("00"); 
											delay_ms(200);	 
											CAMERA_Image_Cut_Compress_6080(0,0);
											Light_On=0; //������Բ��ƹ�ر�
											Image_Sobel();	
											OPTATest();
											Hough();
											Image_Send();	
											HoughAfter();
											Image_Send();
											ThereIsACircle = 1;												
										}else if(GlassArea == 0 && GlassAreaAssitSignal == 1)
										{
											//��������ȡͼ�����������
												printf("11"); 
												delay_ms(200);
												CAMERA_Image_Cut_Compress_6080(0,0);
												Image_Sobel_Glass();
												Image_Sub_Background();	
												Hough();
												Image_Send();	
												Glass_Light_On=0; //������Բ��ƹ�ر�
												Light_On_Glass_Water = 1; //������ˮ�ƿ���
												ThereIsACircle = 1;
										}
											if(TIM3_count == 0xff)
											{
												TIM3_count = 1;
											}
				}		
	}	
		    
}


//��ʱ��2�жϷ������	 
void TIM2_IRQHandler(void)
{ 		   
		TIM2->CR1&=0x00;	//�رն�ʱ��   By Bachman 2018 - 01 -18
		TIM2_count++;
	if(TIM2_count >= 2)
	{
				TIM2->SR&=~(1<<0);//����жϱ�־λ 
				TIM2->CNT = 0;    // ����������  By Bachman 2018 - 01 -18
				if(TIM2->SR&0X0001)//����ж�
				{							
						timeIsUp = 1;
						On_Off = 0;//�ضϼ�ˮ�ź�
						if(TIM2_count == 0xff)
						{
							TIM2_count = 1;
						}
				}		
	}	
		    
}

