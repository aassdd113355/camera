#include "sys.h"
#include "delay.h"  
#include "usart.h"  
#include "ov7670.h"
#include "timer.h"	  		 
#include "sccb.h"	
#include "exti.h"
#include "Pic_Process.h"
#include "dma.h"
#include "led.h"

void dynamic_check(void);
extern u8 ov_sta;	//��exit.c���涨��
extern int flag;	//��exit.c���涨��
extern volatile u8 ThereIsACircle;
u8 timeIsUp; //��ʱ��ʱ���þ���־�����μ�ˮ��ʱ��־��
u16 AddWaterTime; //�ܼ�ˮʱ����
//��ʼ��һ�ν��о�̬��⣻����̬����ˮλΪ-1ʱ������⣻����ˮλ>6ʱ������⣻
//����̬����ˮλ��0-6�Ŀɼ�ˮ�ķ�Χ��ʱ��������̬���Ϳ�ˮ�źţ�
//����̬����ˮλΪ0-6ʱ������⣻����ˮλ>6ʱ���ؾ�̬��⣬���ضϼ�ˮ�źš�
int main(void)
{   
	cameraSysInit();	//ϵͳ��ʼ��
	 

 	while(1) 
	{		
		
			
		while(!ThereIsACircle);    			//û��Ɑ�ڵ�ʱ������������	
		
		

		if(GlassArea == 0)
		{
				CAMERA_Image_Cut_Compress_6080(0,0);//	��ȡһ֡ͼƬ�����Ҵ�������       2017-07-13 By BachMan			
				Image_Histeq();
				Sobel_After();
				eraseHorizon();
//			OPTA();
				Water_Level_Static();
				Image_Send_After_Static();			
						
				while(max>=0 && max< 6 && GlassArea == 0 && timeIsUp == 0 && AddWaterTime <= 50000)
				{
					On_Off = 1;//��ˮ�źſ�
					TIM2->CR1|=0x01; //������ʱ��
					dynamic_check();
				}		
		}						
	}
}	
//	//��̬��� ��̬������
void dynamic_check()
{
				CAMERA_Image_Cut_Compress_6080(0,0);
				Image_Histeq();
				Sobel_After();
				Water_Level_Dynamic();	
				Image_Send_Dynamic();
				AddWaterTime += TIM2->CNT;	 //�ۼ��ܼ�ˮʱ��
				TIM2->CNT = 0;               //����������
				if(max_dynamic > 5 || AddWaterTime > 50000)
				{
					On_Off = 0;//�ضϼ�ˮ�ź�
					max = 6;	//���Ӷ�̬��̬���		
					TIM2->CR1&=0x00;
				}				
}
