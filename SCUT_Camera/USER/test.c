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
//ALIENTEK 探索者STM32F407开发板 实验13
//TFTLCD显示实验  
//技术支持：www.openedv.com
//广州市星翼电子科技有限公司
extern u8 ov_sta;	//在exit.c里面定义
int flag_onlyone = 1;

int main(void)
{   
	Stm32_Clock_Init(336,8,2,7);//设置时钟,168Mhz ， APB1 为 42Mhz， APB2 为 84Mhz， USB/SDIO/随机数发生器时钟为 48Mhz。
	delay_init(168);			//延时初始化  
	uart_init(84,115200);		//初始化串口波特率为115200 
    OV7670_Init();
	delay_ms(3500);	 
	LED_Init();
	OV7670_Light_Mode(0);
	//TIM3_Int_Init(9999,8399);			//10Khz计数频率,1秒钟中断	
	EXTI9_Init();						//使能定时器捕获
	OV7670_Window_Set(10,174,240,320);	//设置窗口	  
    OV7670_CS=0;
	
	
	
	
	
	
	
	
 	while(1) 
	{	

	if(max < 6 && max >= 0)
	{
	if(ov_sta==2)
	{
		
		
				
				CAMERA_Image_Cut_Compress_6080(0,0);//	获取一帧图片，并且存在数组       2017-07-13 By BachMan
				if(flag_onlyone == 1){
						Image_Sobel();												// 确定水位的上下界
						Hough();
						Image_Send();
					flag_onlyone--;
			}
		
		
	
				Image_Histeq();
				Sobel_After();
				Water_Level();				// 液位判断
				Image_Send_After();
	}					
								
	}else {max=-1;}
	
	if(max == -1)
	{
		//打开水开关信号
		//延时
		if(ov_sta==2)
	{
		
		
				
				CAMERA_Image_Cut_Compress_6080(0,0);
				Image_Histeq();
				Sobel_After();
				Water_Level_Static();				// 液位判断
				Image_Send_After_Static();
		
				
	}
	
	

	}
}
}

