#include "sys.h"
#include "delay.h"  
#include "usart.h"  
#include "led.h"
#include "lcd.h"
#include "ov7670.h"
#include "timer.h"	  		 
#include "sccb.h"	
#include "exti.h"
#include "Pic_Process.h"
#include "dma.h"

void dynamic_check(void);
extern u8 ov_sta;	//��exit.c���涨��
int flag_onlyone = 1;


//����̬����ˮλ��0-6�Ŀɼ�ˮ�ķ�Χ��ʱ��������̬���Ϳ�ˮ�źţ�
//����̬����ˮλΪ-1ʱ������⣻����ˮλ>6ʱ�������;
//����̬����ˮλΪ0-6ʱ������⣻����ˮλ>6ʱ���ؾ�̬��⣬���ضϼ�ˮ�źš�
int main(void)
{   
	cameraSysInit();	//ϵͳ��ʼ��

 	while(1) 
	{	


	if(ov_sta==2)
	{
		
		
				
				CAMERA_Image_Cut_Compress_6080(0,0);//	��ȡһ֡ͼƬ�����Ҵ�������       2017-07-13 By BachMan	
		
//				if(flag_onlyone == 1){					//ȷ������
//						Image_Sobel();												
//						Hough();
//						Image_Send();
//					flag_onlyone--;
//			}
					
				Image_Histeq();
				Sobel_After();
//				Water_Level_Static();
				Image_Send_After_Static();			
//				
//				while(max>=0 && max<=6)
//				{
//					//��ˮ�źſ�
//					dynamic_check();
//				}
	}					
								
	}
	
}	
	//��̬��� ��̬������
void dynamic_check()
{
		if(ov_sta==2)
	{
		
		
				
				CAMERA_Image_Cut_Compress_6080(0,0);
				Image_Histeq();
				Sobel_After();
				Water_Level_Dynamic();	
				Image_Send_Dynamic();
				if(max > 6)
				{
					//�ضϼ�ˮ�ź�
					//��ʱ1s����
				}
				
	}
}
	
	
	

	
	






