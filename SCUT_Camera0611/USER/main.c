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
void dynamic_check_Glass(void);
void dynamic_check(void);
void glass_static_check(void);
void nonGlass_static_check(void);
u8 volatile timeIsUp; //定时器时间用尽标志（单次加水超时标志）
u8 volatile processingTimeIsUp; //总处理时间超时标志
u16 AddWaterTime; //总加水时间数
extern u8 ov_sta;	//在exit.c里面定义
extern int flag;	//在exit.c里面定义
extern u8 ThereIsACircle;
extern int static_flag1, static_flag2; //在pro.c中定义
extern u8 isGlass; //exti.c中定义
//初始第一次进行静态检测；当静态检测出水位为-1时继续检测；检测出水位>6时继续检测；
//当静态检测出水位在0-6的可加水的范围内时，开启动态检测和开水信号；
//当动态检测出水位为0-6时继续检测；检测出水位>6时返回静态检测，并关断加水信号。
int main(void)
{   
	cameraSysInit();	//系统初始化
	 

 	while(1) 
	{		
		
			
		while(!ThereIsACircle);    			//没检测杯口的时候阻塞在这里	
		
		

			//非玻璃杯
		if(ThereIsACircle && !isGlass)
		{
				
			
						 //当max_dynamic<=5 时 进入动态检测，否则关水进入静态检测
				while(max_dynamic <= 5  && !processingTimeIsUp && ThereIsACircle) 
				{
					dynamic_check();
				}
				//关断加水信号
				On_Off = 0;
				
				while(!processingTimeIsUp && ThereIsACircle)
				{
					nonGlass_static_check();
				}
				
		}else if(ThereIsACircle && isGlass){
			 //当max_dynamic<=5 时 进入动态检测，否则关水进入静态检测
				while(max_dynamic <= 5  && !processingTimeIsUp && ThereIsACircle) 
				{
					dynamic_check_Glass();
				}
				
				On_Off = 0;
				
				while(!processingTimeIsUp && ThereIsACircle)
				{
					glass_static_check();
				}
		}
		
		//确定的停止状态
			On_Off = 0;
	}	
}

void nonGlass_static_check()
{
				CAMERA_Image_Cut_Compress_6080(0,0);//	获取一帧图片，并且存在数组       2017-07-13 By BachMan			
				Image_Histeq();
				Sobel_After();
				eraseHorizon();
				Water_Level_Static();
				Image_Send_After_Static();			
						
//				while(max>=0 && max< 6 && GlassArea == 0 && timeIsUp == 0 && AddWaterTime <= 50000 && AddWaterNum < 10)
//				while(max>=0 && max< 6 && GlassArea == 0 && timeIsUp == 0 && AddWaterTime <= 50000)

				while(max>=0 && max< 5 && ThereIsACircle)
				{
					On_Off = 1;//加水信号开

					dynamic_check();
				}		
}

void glass_static_check()
{
				CAMERA_Image_Cut_Compress_6080(0,0);//	获取一帧图片，并且存在数组       2017-07-13 By BachMan			
				Image_Histeq();
				Sobel_After_Glass();
				Glass_Water_Level_Static();
				Image_Send_After_Static();	
							
				while(max>=0 && max< 6 && ThereIsACircle && !processingTimeIsUp)
				{
					if(ThereIsACircle)
					{
						On_Off = 1;//加水信号开		
					}
								
					dynamic_check_Glass();
				}	
}

//	//动态检测 静态结束后
void dynamic_check()
{
				CAMERA_Image_Cut_Compress_6080(0,0);
				Image_Histeq();
				Sobel_After();
				Water_Level_Dynamic();	
				Image_Send_Dynamic();

				if(max_dynamic > 5 && TIM4->CNT > 25000) //|| AddWaterTime > 50000 3.20改 在TIM4->CNT <= 10000内屏蔽关水 5.22 TIM4->CNT > 25000 调试
				{
					static_flag2 = -1;			//清除静态加水中的缓存
					static_flag1 = -1;
					On_Off = 0;//关断加水信号
					max = 6;	//连接动态静态检�
					static_flag1 = 6;		
				}				
}

void dynamic_check_Glass(){
				CAMERA_Image_Cut_Compress_6080(0,0);
				Image_Histeq();
				Sobel_After_Glass();
				Glass_Water_Level_Dynamic();	
				Image_Send_Dynamic();

				if(max_dynamic > 5 && TIM4->CNT > 10000) 	//5.11改 在TIM4->CNT <= 10000内屏蔽关水 5.22 
				{
					static_flag2 = -1;				//清除静态加水中的缓存
					static_flag1 = -1;
					On_Off = 0;//关断加水信号
					max = 6;	//连接动态静态检�
					static_flag1 = 6;		
				}				
}
