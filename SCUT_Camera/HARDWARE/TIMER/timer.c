#include "timer.h"
#include "Pic_Process.h"

extern int flag_onlyone;
extern int flag;

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
 	TIM3->ARR=arr;  	//设定计数器自动重装值 
	TIM3->PSC=psc;  	//预分频器	  
	TIM3->DIER|=1<<0;   //允许更新中断
	TIM3->SR&=0<<0;		//中断状态清零	
	//TIM3->CR1|=0x01;    //使能定时器3
  	MY_NVIC_Init(0,0,TIM3_IRQn,2);	//抢占0，子优先级0，组2									 
}

//定时器3中断服务程序	 
void TIM3_IRQHandler(void)
{ 		    		  			    
	if(TIM3->SR&0X0001)//溢出中断
	{
		
				
				while(flag_onlyone == 1){					//确定杯口
						if(ov_sta>1)
						{
							CAMERA_Image_Cut_Compress_6080(0,0);
	//						Image_Send();
							Image_Sobel();												
							Hough();	
							flag_onlyone--;
						}
					}
				flag = 1;
				TIM3->CR1&=0x00;	//关闭定时器
	}				   
	TIM3->SR&=~(1<<0);//清除中断标志位 	    
}

