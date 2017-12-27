#include "timer.h"
#include "Pic_Process.h"
#include "delay.h" 
#include "led.h"
extern int flag;
extern volatile u8 ThereIsACircle; //Pic_Process.c�ж���

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
 	TIM3->ARR=arr;  	//�趨�������Զ���װֵ 
	TIM3->PSC=psc;  	//Ԥ��Ƶ��
		
	TIM3->DIER|=1<<0;   //��������ж�
	TIM3->SR&=~(1<<0);	//����жϱ�־λ
  	MY_NVIC_Init(1,0,TIM3_IRQn,2);	//��ռ0�������ȼ�0����2	

}

//��ʱ��3�жϷ������	 
void TIM3_IRQHandler(void)
{ 		    		  
	if(TIM3->SR&0X0001)//����ж�
	{
		
				
																		//ȷ������
					while(ThereIsACircle == 0 && GlassArea == 0)
						{
						if(ov_sta>1)
							{
								CAMERA_Image_Cut_Compress_6080(0,0);
								//Image_Send();
								Image_Sobel();
								//Image_Send();				
								Hough();
								Image_Send();
								delay_ms(5000);
							}
						}
						HoughAfter();
						Image_Send();

				TIM3->CR1&=0x00;	//�رն�ʱ��
	}				   
	TIM3->SR&=~(1<<0);//����жϱ�־λ 	    
}

