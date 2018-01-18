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
extern u8 ov_sta;	//在exit.c里面定义
extern int flag;	//在exit.c里面定义
extern volatile u8 ThereIsACircle;

//初始第一次进行静态检测；当静态检测出水位为-1时继续检测；检测出水位>6时继续检测；
//当静态检测出水位在0-6的可加水的范围内时，开启动态检测和开水信号；
//当动态检测出水位为0-6时继续检测；检测出水位>6时返回静态检测，并关断加水信号。
int main(void)
{   
	cameraSysInit();	//系统初始化
	 

 	while(1) 
	{		
		
			
		while(!ThereIsACircle);    			//没检测杯口的时候阻塞在这里	
		
		

		if(GlassArea == 0)
		{


				CAMERA_Image_Cut_Compress_6080(0,0);//	获取一帧图片，并且存在数组       2017-07-13 By BachMan			
				Image_Histeq();
				Sobel_After();
//			OPTA();
				Water_Level_Static();
				Image_Send_After_Static();			
				
				while(max>=0 && max< 6 && GlassArea == 0)
				{
					On_Off = 1;//加水信号开
					dynamic_check();
				}
				
		}	

								
	}
	
	while(1);
}	
//	//动态检测 静态结束后
void dynamic_check()
{
				CAMERA_Image_Cut_Compress_6080(0,0);
				Image_Histeq();
				Sobel_After();
				Water_Level_Dynamic();	
				Image_Send_Dynamic();
				if(max_dynamic > 5)
				{
					On_Off = 0;//关断加水信号
					max = 6;	//连接动态静态检测
				}
}
