#include "timer.h"
#include "Pic_Process.h"
#include "delay.h" 
#include "led.h"
#include "usart.h"
extern int flag;
extern volatile u8 ThereIsACircle; //Pic_Process.c中定义
u8 TIM3_count = 0;
u8 TIM2_count = 0;
extern u8 timeIsUp;//定时器时间用尽标志 main定义
//通用定时器3中断初始化
//这里时钟选择为APB1的2倍，而APB1为42M
//arr：自动重装值。
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz
//这里使用的是定时器3
void TIM3_Int_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<1;	//TIM3时钟使能    
 	TIM3->ARR=arr;  	  //设定计数器自动重装值 
	TIM3->PSC=psc;  	  //预分频器
		
	TIM3->DIER|=1<<0;   //允许更新中断
	TIM3->SR&=~(1<<0);	//清除中断标志位
  MY_NVIC_Init(1,0,TIM3_IRQn,2);	//抢占0，子优先级0，组2	
	
	TIM3->CR1|=0x01;	//开定时器   By Bachman 2018 - 01 -19

}

void TIM2_Int_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<0;	//TIM2时钟使能    
 	TIM2->ARR=arr;  	  //设定计数器自动重装值 
	TIM2->PSC=psc;  	  //预分频器
		
	TIM2->DIER|=1<<0;   //允许更新中断
	TIM2->SR&=~(1<<0);	//清除中断标志位
  MY_NVIC_Init(1,0,TIM2_IRQn,2);	//抢占0，子优先级0，组2	
}

//定时器3中断服务程序	 
void TIM3_IRQHandler(void)
{ 		   
		TIM3->CR1&=0x00;	//关闭定时器   By Bachman 2018 - 01 -18
		TIM3_count++;
	if(TIM3_count >= 2)
	{			
				TIM3->CNT = 0;    // 计数器清零  By Bachman 2018 - 01 -18
				if(TIM3->SR&0X0001)//溢出中断
				{
										TIM3->SR&=~(1<<0);//清除中断标志位 
										if(GlassArea == 0 && GlassAreaAssitSignal == 0)
										{
											printf("00"); 
											delay_ms(200);	 
											CAMERA_Image_Cut_Compress_6080(0,0);
											Light_On=0; //搜索完圆后灯光关闭
											Image_Sobel();	
											OPTATest();
											Hough();
											Image_Send();	
											HoughAfter();
											Image_Send();
											ThereIsACircle = 1;												
										}else if(GlassArea == 0 && GlassAreaAssitSignal == 1)
										{
											//玻璃杯的取图函数及其参数
												printf("11"); 
												delay_ms(200);
												CAMERA_Image_Cut_Compress_6080(0,0);
												Image_Sobel_Glass();
												Image_Sub_Background();	
												Hough();
												Image_Send();	
												Glass_Light_On=0; //搜索完圆后灯光关闭
												Light_On_Glass_Water = 1; //玻璃杯水灯开启
												ThereIsACircle = 1;
										}
											if(TIM3_count == 0xff)
											{
												TIM3_count = 1;
											}
				}		
	}	
		    
}


//定时器2中断服务程序	 
void TIM2_IRQHandler(void)
{ 		   
		TIM2->CR1&=0x00;	//关闭定时器   By Bachman 2018 - 01 -18
		TIM2_count++;
	if(TIM2_count >= 2)
	{
				TIM2->SR&=~(1<<0);//清除中断标志位 
				TIM2->CNT = 0;    // 计数器清零  By Bachman 2018 - 01 -18
				if(TIM2->SR&0X0001)//溢出中断
				{							
						timeIsUp = 1;
						On_Off = 0;//关断加水信号
						if(TIM2_count == 0xff)
						{
							TIM2_count = 1;
						}
				}		
	}	
		    
}

