#include "timer.h"
#include "Pic_Process.h"
#include "delay.h" 
#include "led.h"
extern int flag;
extern volatile u8 ThereIsACircle; //Pic_Process.c中定义
int TIM3_count = 0;

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

//定时器3中断服务程序	 
void TIM3_IRQHandler(void)
{ 		   
		TIM3->CR1&=0x00;	//关闭定时器   By Bachman 2018 - 01 -18
		TIM3_count++;
	if(TIM3_count > 2)
	{
				TIM3->CNT = 0;    // 计数器清零  By Bachman 2018 - 01 -18
				if(TIM3->SR&0X0001)//溢出中断
				{							

//EXTI3_ClearAndForbid(1);
										if(GlassArea == 0)
										{
											delay_ms(200);	 
											CAMERA_Image_Cut_Compress_6080(0,0);
											Light_On=0; //搜索完圆后灯光关闭
											Image_Sobel();	
											OPTATest();
											Hough();
											Image_Send();	
											HoughAfter();
											Image_Send();
											
										}
//EXTI3_ClearAndForbid(0);
				}		
	}	
	TIM3->SR&=~(1<<0);//清除中断标志位 	    
}

