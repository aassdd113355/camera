#include "exti.h"
#include "delay.h" 
#include "ov7670.h"
#include "led.h"
#include "Pic_Process.h"
#include "usart.h"	
u8 ov_sta;
extern volatile u8 ThereIsACircle; //Pic_Process.c中定义
extern volatile u8 timeIsUp;
extern u16 AddWaterTime;
extern u8 AddWaterNum;
u8 Processing = 0;
 //外部中断5~9服务程序，读取最新一帧的图像
void EXTI9_5_IRQHandler(void)
{		 		
	if(EXTI->PR&(1<<9))			//是9线的中断
	{     
		EXTI->PR=1<<9;     //清除LINE9上的中断标志位	

				OV7670_WRST=0;	 	//复位写指针	
				delay_us(1);
				OV7670_WRST=1;	
				OV7670_WREN=1;		//允许写入FIFO
				ov_sta++;
				delay_us(200);
	}
						  
} 
 //3线中断服务函数
void EXTI3_IRQHandler(void)
{		
	if(EXTI->PR&(1<<3))			//是3线的中断
	{ 
		EXTI->PR=1<<3;     //清除LINE3上的中断标志位	
		delay_ms(500);
		if(GlassArea == 1)
		{
			Glass_Light_On = 0;
			Light_On=0; //灯光关闭
			On_Off = 0; 	//关
			ThereIsACircle = 0;
//			AddWaterNum = 0;
			Processing = 0;
			Light_On_Glass_Water = 0; //水灯关闭
		}
		else if(Processing == 0 && GlassArea == 0)
		{		
				
//				TIM4->CR1&=0x00;							//关闭定时器
//				TIM4->CNT = 0;               //计数器清零			
				TIM2->CNT = 0;
			  timeIsUp = 0;
			  AddWaterTime = 0;
			if(GlassAreaAssitSignal == 0) //非透明杯
			{
								Light_On=1; 			//灯光打开
							  printf("0"); 
			}else if(GlassAreaAssitSignal == 1) //透明杯
			{
								Glass_Light_On = 1;
								printf("1"); 
			}

				TIM3->CR1|=0x01;	//使能定时器

				//delay_ms(100);		//清除第一次中断
				//TIM3->SR&=~(1<<0);	//清除中断标志位	

			
//				TIM3->CNT = 0;
//				TIM3->EGR = 1;
//				while((TIM3->SR&0x01) == 0)
//				;
//		  	TIM3->SR&=~(1<<0);	//清除中断标志位	
//				TIM3->DIER|=1<<0;   //允许更新中断
		}
	} 
}




//外部中断9初始化
void EXTI9_Init(void)
{												  
	Ex_NVIC_Config(GPIO_F,9,RTIR); 			//上升沿触发			  
	MY_NVIC_Init(0,0,EXTI9_5_IRQn,2);		//抢占0,子优先级0，组2	   
}

void EXTIX_Init(void)	//外部中断初始化	
{	
	Ex_NVIC_Config(GPIO_D,3,3); 			//2018.1.17改
	MY_NVIC_Init(1,0,EXTI3_IRQn,2);		//抢占1,子优先级0，组2	
}

