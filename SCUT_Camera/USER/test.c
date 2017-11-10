#include "sys.h"
#include "delay.h"  
#include "usart.h"  
#include "led.h"
#include "lcd.h"
#include "ov7670.h"
//#include "ov7670cfg.h"
#include "timer.h"	  		 
#include "sccb.h"	
#include "exti.h"
#include "Pic_Process.h"
#include "dma.h"
//ALIENTEK ̽����STM32F407������ ʵ��13
//TFTLCD��ʾʵ��  
//����֧�֣�www.openedv.com
//������������ӿƼ����޹�˾
extern u8 ov_sta;	//��exit.c���涨��
int flag_onlyone = 1;

int main(void)
{   
	Stm32_Clock_Init(336,8,2,7);//����ʱ��,168Mhz �� APB1 Ϊ 42Mhz�� APB2 Ϊ 84Mhz�� USB/SDIO/�����������ʱ��Ϊ 48Mhz��
	delay_init(168);			//��ʱ��ʼ��  
	uart_init(84,115200);		//��ʼ�����ڲ�����Ϊ115200 
    OV7670_Init();
	delay_ms(3500);	 
	LED_Init();
	OV7670_Light_Mode(0);
	//TIM3_Int_Init(9999,8399);			//10Khz����Ƶ��,1�����ж�	
	EXTI9_Init();						//ʹ�ܶ�ʱ������
	OV7670_Window_Set(10,174,240,320);	//���ô���	  
    OV7670_CS=0;
	
	
	
	
	
	
	
	
 	while(1) 
	{	

	if(max < 6 && max >= 0)
	{
	if(ov_sta==2)
	{
		
		
				
				CAMERA_Image_Cut_Compress_6080(0,0);//	��ȡһ֡ͼƬ�����Ҵ�������       2017-07-13 By BachMan
				if(flag_onlyone == 1){
						Image_Sobel();												// ȷ��ˮλ�����½�
						Hough();
						Image_Send();
					flag_onlyone--;
			}
		
		
	
				Image_Histeq();
				Sobel_After();
				Water_Level();				// Һλ�ж�
				Image_Send_After();
	}					
								
	}else {max=-1;}
	
	if(max == -1)
	{
		//��ˮ�����ź�
		//��ʱ
		if(ov_sta==2)
	{
		
		
				
				CAMERA_Image_Cut_Compress_6080(0,0);
				Image_Histeq();
				Sobel_After();
				Water_Level_Static();				// Һλ�ж�
				Image_Send_After_Static();
		
				
	}
	
	

	}
}
}

