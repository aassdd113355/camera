#include "timer.h"
#include "Pic_Process.h"
#include "delay.h" 
#include "led.h"
#include "usart.h"
extern int flag;
extern u8 ThereIsACircle; //Pic_Process.c中定义
u8 TIM3_count = 0;
u8 TIM2_count = 0;
u8 TIM4_count = 0;
u8 TIM9_count = 0;
extern u8 isGlass; //exti.c中定义
extern u8 processingTimeIsUp; //总处理时间超时标志 main定义
u16 temp, length;
void TIM4_Int_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<2;	//TIM3时钟使能    
 	TIM4->ARR=arr;  	  //设定计数器自动重装值 
	TIM4->PSC=psc;  	  //预分频器
		
	TIM4->DIER|=1<<0;   //允许更新中断
	TIM4->SR&=~(1<<0);	//清除中断标志位
  MY_NVIC_Init(1,0,TIM4_IRQn,2);	//抢占0，子优先级0，组2	
	TIM4->CR1|=0x01;	//开定时器   By Bachman 2018 - 01 -19
}

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

void TIM9_Int_Init(u16 arr,u16 psc)
{
	RCC->APB2ENR|=1<<4;	//TIM9时钟使能    
 	TIM9->ARR=arr;  	  //设定计数器自动重装值 
	TIM9->PSC=psc;  	  //预分频器
	
	TIM9->DIER|=1<<0;   //允许更新中断
	TIM9->SR&=~(1<<0);	//清除中断标志位
  MY_NVIC_Init(1,0,TIM1_BRK_TIM9_IRQn,2);	//抢占0，子优先级0，组2	
	TIM9->CR1|=0x01;
}

void TIM2_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<0;    //TIM2时钟使能
	//LED_Init();
	TIM2->ARR=arr;         //设定计数器自动重装值
	TIM2->PSC=psc;         //预分频器
	TIM2->CCMR1|=1<<0;     //CC1S=01 选择输入端IC1映射到TI1
	TIM2->CCMR1|=1<<4;     //IC1F=0001 配置滤波器 以Fck_int采样，两个事件后有效
	TIM2->CCMR1|=0<<10;    //IC1PS=00 配置输入分频，不分频
	TIM2->CCER|=0<<1;      //CC1P=0 上升沿捕获
	TIM2->CCER|=1<<0;      //CC1E=1 允许捕获计数器的值到捕获寄存器中
	TIM2->DIER|=1<<1;      //允许捕获中断
	TIM2->DIER|=1<<0;      //允许更新中断
	TIM2->CR1|=0X01;       //使能定时器2
	MY_NVIC_Init(2,0,TIM2_IRQn,2);//抢占2，子优先级0，组2
}


void TIM5_Int_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<3;	//TIM5时钟使能
	TIM5->ARR=arr;  	  //设定计数器自动重装值
	TIM5->PSC=psc;  	  //预分频器
	TIM5->DIER|=1<<0;   //允许更新中断
	TIM5->CR1|=0x01; 		//开启定时器
	TIM5->SR&=~(1<<0);	//清除中断标志位
	MY_NVIC_Init(1,0,TIM5_IRQn,2);	//抢占0，子优先级0，组2
}

//捕获状态
//[7]:0,没有成功的捕获;1,成功捕获到一次.
//[6]:0,还没捕获到高电平;1,已经捕获到高电平了.
//[5:0]:捕获高电平后溢出的次数
u8 TIM2CH1_CAPTURE_STA=0; //输入捕获状态
u16 TIM2CH1_CAPTURE_VAL;  //输入捕获值
//定时器2中断服务程序
void TIM2_IRQHandler(void)
{
	u16 tsr;
	tsr=TIM2->SR;
	if ((TIM2CH1_CAPTURE_STA&0X80)==0)//还未成功捕获
	{
		if (tsr&0X01)//溢出
		{
			if (TIM2CH1_CAPTURE_STA&0X40)//已经捕获到高电平了
			{
				if ((TIM2CH1_CAPTURE_STA&0X3F)==0X3F)//高电平太长了
				{
					TIM2CH1_CAPTURE_STA|=0X80;//标记成功捕获了一次
					TIM2CH1_CAPTURE_VAL=0XFFFF;
				}else TIM2CH1_CAPTURE_STA++;
			}
		}
		if (tsr&0x02)//捕获1发生捕获事件
		{
			if (TIM2CH1_CAPTURE_STA&0X40) //捕获到一个下降沿
			{
				TIM2CH1_CAPTURE_STA|=0X80; //标记成功捕获到一次高电平脉宽
				TIM2CH1_CAPTURE_VAL=TIM2->CCR1;//获取当前的捕获值
				TIM2->CCER&=~(1<<1);    //CC1P=0 设置为上升沿捕获
			}else                       //还未开始，第一次捕获上升沿
			{
				TIM2CH1_CAPTURE_VAL=0;
				TIM2CH1_CAPTURE_STA=0X40; //标记捕获到了上升沿4
				TIM2->CNT=0;             //计数器清空
				TIM2->CCER|=1<<1;        //CC1P=1 设置为下降沿捕获
			}
		}
	}
	TIM2->SR=0;//清除中断标志位
}

//定时器5中断服务程序
void TIM5_IRQHandler(void)
{

	UltrasonicTrigger();   //触发超声波,led.h定义

    if (TIM2CH1_CAPTURE_STA&0X80)//成功捕获到了一次高电平
		{
			temp=TIM2CH1_CAPTURE_STA&0X3F;
			temp*=65536;               //溢出时间总和
			temp+=TIM2CH1_CAPTURE_VAL; //得到总的高电平时间
			length=temp*0.017;         //计算长度

			if ((length<21.0f) && (length>4.0f))
			{
				  GlassAreaOut=0;       	
			}else{
					GlassAreaOut=1;     
			}
			TIM2CH1_CAPTURE_STA=0;     //开启下一次捕获
		}
	
	TIM5->SR&=~(1<<0);//清除中断标志位
}

//定时器3中断服务程序	 
void TIM3_IRQHandler(void)
{ 		   
		TIM3->CR1&=0x00;	//关闭定时器   By Bachman 2018 - 01 -18
		TIM3_count++;
	
	if(TIM3_count < 2 ){
			TIM3->SR&=~(1<<0);//清除中断标志位    // By Bachman 2018 - 05 -109  消除刚开机或者刚下载程序就进入中断的问题，不清除中断标志就会一直反复进入中断
	}
	
	if(TIM3_count >= 2)
	{			

				TIM3->CNT = 0;    // 计数器清零  By Bachman 2018 - 01 -18
				if(TIM3->SR&0X0001)//溢出中断
				{	
										TIM3->SR&=~(1<<0);//清除中断标志位 
									
										if(GlassArea == 0 && isGlass == 0)
										{

											On_Off = 1; //开启加水信号
											TIM4->CR1|=0x01;	//使能定时器
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
										}else if(GlassArea == 0 && isGlass == 1)
										{

											//玻璃杯的取图函数及其参�
												On_Off = 1; //开启加水信号
												TIM4->CR1|=0x01;	//使能定时器�
												printf("11"); 
												delay_ms(200);
												CAMERA_Image_Cut_Compress_6080(0,0);
												Image_Sobel_Glass();
												clearPic();
												Hough();
												Image_Send();	
//												HoughAfter();
//												Image_Send();
												Light_On=0; //搜索完圆后灯光关闭
												Glass_Light_On=0; //搜索完圆后灯光关闭
												Light_On_Glass_Water = 1; //玻璃杯水灯开启
												ThereIsACircle = 1;
										}else{
											Light_On=0; //搜索完圆后灯光关闭
											Glass_Light_On=0; //搜索完圆后灯光关闭
										}
											if(TIM3_count == 0xff)
											{
												TIM3_count = 1;
											}
				}		
	}	
		    
}


//定时器9中断服务程序	 
void TIM9_IRQHandler(void)
{ 		   
		TIM9->CR1&=0x00;	//关闭定时器   By Bachman 2018 - 01 -18
		TIM9_count++;
	
	if(TIM9_count < 2 ){
			TIM9->SR&=~(1<<0);//清除中断标志位    // By Bachman 2018 - 05 -109  消除刚开机或者刚下载程序就进入中断的问题，不清除中断标志就会一直反复进入中断
	}
	if(TIM9_count >= 2)
	{
				
				TIM9->CNT = 0;    // 计数器清零  By Bachman 2018 - 01 -18
			if(TIM9->SR&0X0001)//溢出中断
			{		
				
									if(GlassAreaAssitSignal == 1) //非透明杯
									{					
														isGlass = 0;
														Light_On=1; 			//灯光打开
														printf("0"); 
									}else if(GlassAreaAssitSignal == 0) //透明杯
									{					
														isGlass = 1;
														//Light_On = 1;
														Glass_Light_On = 1;
														printf("1"); 
									}
									TransmitSig = 0;
										
										TIM3->CR1|=0x01;	//使能定时器				

												if(TIM9_count == 0xff)
												{
													TIM9_count = 1;
												}
				}		
	}	
	TIM9->SR&=~(1<<0);//清除中断标志位 	    
}

//定时器4中断服务程序	 
void TIM4_IRQHandler(void)
{ 		   
		TIM4->CR1&=0x00;	//关闭定时器   By Bachman 2018 - 01 -18
		TIM4_count++;
	
	if(TIM4_count < 2 ){
			TIM4->SR&=~(1<<0);//清除中断标志位    // By Bachman 2018 - 05 -109  消除刚开机或者刚下载程序就进入中断的问题，不清除中断标志就会一直反复进入中断
	}
	if(TIM4_count >= 2)
	{
				
				TIM4->CNT = 0;    // 计数器清零  By Bachman 2018 - 01 -18
				if(TIM4->SR&0X0001)//溢出中断
				{		
											
						processingTimeIsUp = 1;
						if(TIM2_count == 0xff)
						{
							TIM2_count = 1;
						}
				}		
	}	
	TIM4->SR&=~(1<<0);//清除中断标志位 	    
}

