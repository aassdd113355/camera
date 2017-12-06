#include "exti.h"
#include "delay.h" 
#include "ov7670.h"
#include "led.h"
#include "Pic_Process.h"
u8 ov_sta;
int flag = 0; //标志是否已经进行了杯口检测
int flag_onlyone = 1;


 //外部中断5~9服务程序，读取最新一帧的图像
void EXTI9_5_IRQHandler(void)
{		 		
	if(EXTI->PR&(1<<9))//是9线的中断
	{     

				OV7670_WRST=0;	 	//复位写指针	
				delay_us(1);
				OV7670_WRST=1;	
				OV7670_WREN=1;		//允许写入FIFO
				ov_sta++;
	}
	EXTI->PR=1<<9;     //清除LINE9上的中断标志位						  
} 
 //3线中断服务函数
void EXTI3_IRQHandler(void)
{		
		delay_ms(60);
		if(GlassArea == 1)
		{
			flag = 0;
			flag_onlyone = 1;
			On_Off = 0; 	//关
		}
		else
		{		
				TIM3->CR1|=0x01;	//使能定时器	
				delay_ms(20);		//清除第一次中断
				TIM3->SR&=~(1<<0);	//清除中断标志位				
		}
			
	
	EXTI->PR=1<<3;     //清除LINE3上的中断标志位						  
} 




//外部中断9初始化
void EXTI9_Init(void)
{												  
	Ex_NVIC_Config(GPIO_F,9,RTIR); 			//上升沿触发			  
	MY_NVIC_Init(0,0,EXTI9_5_IRQn,2);		//抢占0,子优先级0，组2	   
}

void EXTIX_Init(void)	//外部中断初始化	
{	
	Ex_NVIC_Config(GPIO_D,3,RTIR); 			//任意沿触发	
	MY_NVIC_Init(1,0,EXTI3_IRQn,2);		//抢占1,子优先级0，组2	
}

