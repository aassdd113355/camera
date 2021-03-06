#include "Pic_Process.h"
#include "ov7670.h"
#include "exti.h"
#include "timer.h"
#include "usart.h"
#include "dma.h"
#include "math.h"
#include "lcd.h"
#include "String.h"
#include "delay.h"  
#include "led.h"

u8 Pic_Buff_Background[HEIGHT][WIDTH];
u8 Pic_Buff_Dup[HEIGHT][WIDTH];
u8 Pic_Buff_Temp[HEIGHT][WIDTH];
u8 Pic_Buff[HEIGHT][WIDTH]; 
u8 SendBuff[SEND_BUF_SIZE] = {0};
u8 hough_space[HEIGHT][WIDTH][cntR]={0}; 
float gray_density_array[257];      														 // 用于存放各个灰度级的密度（百分比）
u8 histEQU[256];
u16 gray_array[256];              															 // 用于存放各个灰度级的数量
volatile int x_circle=2, y_circle=2, r_circle=2; 
int max = -1;
int max_dynamic = -1;
int pixel_count[9]={0};
int static_flag1=-1,static_flag2=-1;
volatile u8 ThereIsACircle = 0; 
extern u8 Processing;


 /* 函数名：cameraSysInit
  *  描述  ：系统初始化
  */ 
void cameraSysInit()
{
	Stm32_Clock_Init(336,8,2,7);//设置时钟,168Mhz ， APB1 为 42Mhz， APB2 为 84Mhz， USB/SDIO/随机数发生器时钟为 48Mhz。
	delay_init(168);			//延时初始化  
	uart_init(84,115200);		//初始化串口波特率为115200 
  OV7670_Init();
	delay_ms(1000);	 
	OV7670_Light_Mode(0);
	OV7670_Window_Set(10,174,240,320);	//设置窗口	  
	OV7670_CS=0;
	EXTI9_Init();						//使能定时器捕获	
	GpioInit();
	EXTIX_Init();
	TIM2_Init(0XFFFF,84-1);        			//以1Mhz的频率计数
	TIM3_Int_Init(20000-1,8399);				//10Khz计数频率,2秒钟中断	
  TIM9_Int_Init(5000-1,16800-1);			//500ms延时
	TIM4_Int_Init(40000-1,33600-1);			//5Khz计数频率,16秒钟中断	
	TIM5_Int_Init(500-1, 8400-1);  			//500=50ms
}

void EXTI3_ClearAndForbid(u8 forbid)
{
		EXTI->PR=1<<3;     //清除LINE3上的中断标志位
		if(forbid)
		{
			EXTI->IMR&=~(1<<3);//屏蔽3线中断
		}else
		{
			EXTI->IMR|=1<<3;//不屏蔽3线中断
		}
}



 /* 函数名：CAMERA_Image_Cut_Compress_6080
 * 描述  ：将从摄像头FIFO发过来的一张图片进行剪切存在数组中。
 *         读240*320次，但是只根据参数存储图的一部分                2017-07-12 By BachMan
 * 输入  ：u16 x_pos, u16 y_pos  开始的位置， 宽度和高度则取决于WIDTH和HIGHT
 *				 应该确保	x_pos+width 不超80， y_pos + hight 不超过60																				
 * 输出  ：无
 */ 
void CAMERA_Image_Cut_Compress_6080(u16 x_pos, u16 y_pos)        // 2017-07-13 采集图片和压缩图片和截图
{
	unsigned char color;
	u16 col, row;
	u16 c_count = 0, r_count = 0;
	__disable_irq();																			//关中断

		OV7670_RRST=0;																			//开始复位读指针 
		OV7670_RCK=0;
		OV7670_RCK=1;
		OV7670_RCK=0;
		OV7670_RRST=1;																			//复位读指针结束 
		OV7670_RCK=1;  

		for(row = 0; row <240; row++)
		{
			for(col = 0; col < 320; col++)
			{
				OV7670_RCK=0;
				color=GPIOF->IDR&0X00FF;														//读数据
//			YUYV_Y=GPIOF->IDR&0X00FF;
				OV7670_RCK=1; 
//			color<<=8;  
		    OV7670_RCK=0;
//			color|=GPIOF->IDR&0XFF;																//读数据   因为时YUYV格式，第二个字节不用读取
				OV7670_RCK=1; 
				if((row%4 == 0) && (col%4 == 0))
				{
						if((col/4 >= x_pos) && (row/4 >= y_pos) && (col/4 < (x_pos + WIDTH)) && (row/4 < (y_pos + HEIGHT)))
						{
							if(c_count < WIDTH)          										// 如果c_count未到最后一列，则c_count增加 
							{
								Pic_Buff[r_count][c_count] = color;
								c_count++;
							}
							else								        						//	如果c_count到了最后一列，则转到else中执行换行
							{
								c_count = 0;
								r_count++;
								Pic_Buff[r_count][c_count] = color;
								c_count++;
							}
						}
				}
			}
		}
		memcpy(Pic_Buff_Dup,Pic_Buff,HEIGHT*WIDTH*sizeof(u8));
		EXTI->PR=1<<9;     	 																//清除LINE9上的中断标志位
		ov_sta=0;					  														    //开始下一次采集
		__enable_irq();																			//开中断
}

void CAMERA_Image_Cut_Compress_6080_Background(u16 x_pos, u16 y_pos)        // 2017-07-13 采集图片和压缩图片和截图
{
	unsigned char color;
	u16 col, row;
	u16 c_count = 0, r_count = 0;
	__disable_irq();																			//关中断

		OV7670_RRST=0;																			//开始复位读指针 
		OV7670_RCK=0;
		OV7670_RCK=1;
		OV7670_RCK=0;
		OV7670_RRST=1;																			//复位读指针结束 
		OV7670_RCK=1;  

		for(row = 0; row <240; row++)
		{
			for(col = 0; col < 320; col++)
			{
				OV7670_RCK=0;
				color=GPIOF->IDR&0X00FF;														//读数据
//			YUYV_Y=GPIOF->IDR&0X00FF;
				OV7670_RCK=1; 
//			color<<=8;  
		    OV7670_RCK=0;
//			color|=GPIOF->IDR&0XFF;																//读数据   因为时YUYV格式，第二个字节不用读取
				OV7670_RCK=1; 
				if((row%4 == 0) && (col%4 == 0))
				{
						if((col/4 >= x_pos) && (row/4 >= y_pos) && (col/4 < (x_pos + WIDTH)) && (row/4 < (y_pos + HEIGHT)))
						{
							if(c_count < WIDTH)          										// 如果c_count未到最后一列，则c_count增加 
							{
								Pic_Buff_Background[r_count][c_count] = color;
								c_count++;
							}
							else								        						//	如果c_count到了最后一列，则转到else中执行换行
							{
								c_count = 0;
								r_count++;
								Pic_Buff_Background[r_count][c_count] = color;
								c_count++;
							}
						}
				}
			}
		}
//		memcpy(Pic_Buff_Dup,Pic_Buff,HEIGHT*WIDTH*sizeof(u8));
		EXTI->PR=1<<9;     	 																//清除LINE9上的中断标志位
		ov_sta=0;					  														    //开始下一次采集
		__enable_irq();																			//开中断
}



 /* 函数名：Image_Histeq
 * 描述  ：对图像进行直方图均衡化，以累积分布函数作为基础的直方图进行修改   2017-06-22 By BachMan
 * 输入  ：无
 * 输出  ：无
 */
void Image_Histeq(void)
{

	u16 countSum = 0;
	u16 r, c;
	u16  i, j, k, n;
	
	// 清零数组
	for(i = 0; i < 256; i++)    
	{
		gray_array[i] = 0;
		histEQU[i] = 0;
		gray_density_array[i] = 0;
	}
	
	// 统计各灰度级的数量
	for(r = 0; r < HEIGHT; r++) 
	{
		for(c = 0; c < WIDTH; c++)
		{
			if((r-y_circle)*(r-y_circle) + (c-x_circle)*(c-x_circle) < (r_circle-r_circle/8)*(r_circle-r_circle/8))
				{
					gray_array[Pic_Buff_Dup[r][c]] = gray_array[Pic_Buff_Dup[r][c]] + 1; 
					countSum = countSum + 1;
				}				
		}
	}	
	
	// 计算并且记录各灰度的密度（百分比）
	for(j = 0; j < 256; j++)    
	{
		gray_density_array[j] = gray_array[j]/(float)countSum;  
	}
	
	// 累加各灰度值的密度（百分比累加）
	for (k = 1; k < 257; k++)
	{
	gray_density_array[k] = gray_density_array[k-1] + gray_density_array[k];
	}
	
	//
	for(n = 0; n < 256; n++)
	{
		histEQU[n] = (u8)((gray_density_array[n+1] * 255.0)+0.5);
	}
	
	// 将图片数组	Pic_Buff 的灰度值转化为均衡化后新的灰度值
	for(r = 0; r < HEIGHT; r++) 
	{
		for(c = 0; c < WIDTH; c++)
		{
			if((r-y_circle)*(r-y_circle) + (c-x_circle)*(c-x_circle) < (r_circle-r_circle/8)*(r_circle-r_circle/8))
			{
			Pic_Buff_Dup[r][c] = histEQU[Pic_Buff_Dup[r][c]];
			}				
		}
	}
}




/* 函数名：Image_Sobel
 * 描述  ：Sobel边缘检测 2017.07.05 Lee
 * 输入  ：无
 * 输出  ：无
 */
void Image_Sobel(void)
{	int i,j;
	u8 Gx,Gy;
	int yuzhi;
	u16 tempGray;
	
	Processing = 1;

	memcpy(Pic_Buff_Temp,Pic_Buff,HEIGHT*WIDTH*sizeof(u8));
	
	yuzhi = creatYuzhi(0.1);	//计算阈值

	for(i = 1; i < HEIGHT - 1; i++) 
	{
		for(j = 1; j < WIDTH - 1; j++)
		{
			Gx = abs((Pic_Buff_Temp[i+1][j-1] + 2 * Pic_Buff_Temp[i+1][j] + Pic_Buff_Temp[i+1][j+1]) - (Pic_Buff_Temp[i-1][j-1] + 2 * Pic_Buff_Temp[i-1][j] + Pic_Buff_Temp[i-1][j+1]));
			Gy = abs((Pic_Buff_Temp[i-1][j-1] + 2 * Pic_Buff_Temp[i][j-1] + Pic_Buff_Temp[i+1][j-1]) - (Pic_Buff_Temp[i-1][j+1] + 2 * Pic_Buff_Temp[i][j+1] + Pic_Buff_Temp[i+1][j+1]));
		

			tempGray = Gx + Gy;
			if(tempGray > yuzhi){
				Pic_Buff[i][j] = 255;
			}else{
				Pic_Buff[i][j] = 0;
			}
		}
	}
	
	 
	
//	for(i = 1; i < HEIGHT - 1; i++) 
//	{
//		for(j = 1; j < WIDTH - 1; j++)
//		{
//			if(tempGray > yuzhi){
//				Pic_Buff[i][j] = 255;
//			}else{
//				Pic_Buff[i][j] = 0;
//			}
//		}
//	}
	

	memset(Pic_Buff_Temp,0,HEIGHT*WIDTH*sizeof(u8));
		
}

void Image_Sobel_Glass(void)
{
	int i,j;
	u8 Gx,Gy;
//	int yuzhi;
	u16 tempGray;
	
//	Processing = 1;

	memcpy(Pic_Buff_Temp,Pic_Buff,HEIGHT*WIDTH*sizeof(u8));
	
//	yuzhi = creatYuzhi(0.1);	//计算阈值

	for(i = 1; i < HEIGHT - 1; i++) 
	{
		for(j = 1; j < WIDTH - 1; j++)
		{
			Gx = abs((Pic_Buff_Temp[i+1][j-1] + 2 * Pic_Buff_Temp[i+1][j] + Pic_Buff_Temp[i+1][j+1]) - (Pic_Buff_Temp[i-1][j-1] + 2 * Pic_Buff_Temp[i-1][j] + Pic_Buff_Temp[i-1][j+1]));
			Gy = abs((Pic_Buff_Temp[i-1][j-1] + 2 * Pic_Buff_Temp[i][j-1] + Pic_Buff_Temp[i+1][j-1]) - (Pic_Buff_Temp[i-1][j+1] + 2 * Pic_Buff_Temp[i][j+1] + Pic_Buff_Temp[i+1][j+1]));
		

			tempGray = Gx + Gy;
			if(tempGray > 100){
				Pic_Buff[i][j] = 255;
			}else{
				Pic_Buff[i][j] = 0;
			}
		}
	}
	
	 
	
//	for(i = 1; i < HEIGHT - 1; i++) 
//	{
//		for(j = 1; j < WIDTH - 1; j++)
//		{
//			if(tempGray > yuzhi){
//				Pic_Buff[i][j] = 255;
//			}else{
//				Pic_Buff[i][j] = 0;
//			}
//		}
//	}
	

	memset(Pic_Buff_Temp,0,HEIGHT*WIDTH*sizeof(u8));
}

void Image_Sobel_Background(void)
{	int i,j;
	u8 Gx,Gy;
	u16 tempGray;

	memcpy(Pic_Buff_Temp,Pic_Buff_Background,HEIGHT*WIDTH*sizeof(u8));
	
//	yuzhi = creatYuzhi(0.1);	//计算阈值

	for(i = 1; i < HEIGHT - 1; i++) 
	{
		for(j = 1; j < WIDTH - 1; j++)
		{
			Gx = abs((Pic_Buff_Temp[i+1][j-1] + 2 * Pic_Buff_Temp[i+1][j] + Pic_Buff_Temp[i+1][j+1]) - (Pic_Buff_Temp[i-1][j-1] + 2 * Pic_Buff_Temp[i-1][j] + Pic_Buff_Temp[i-1][j+1]));
			Gy = abs((Pic_Buff_Temp[i-1][j-1] + 2 * Pic_Buff_Temp[i][j-1] + Pic_Buff_Temp[i+1][j-1]) - (Pic_Buff_Temp[i-1][j+1] + 2 * Pic_Buff_Temp[i][j+1] + Pic_Buff_Temp[i+1][j+1]));
		

			tempGray = Gx + Gy;
			if(tempGray > 60){
				Pic_Buff_Background[i][j] = 255;
			}else{
				Pic_Buff_Background[i][j] = 0;
			}
		}
	}
	memset(Pic_Buff_Temp,0,HEIGHT*WIDTH*sizeof(u8));	
}

void Image_Sub_Background(void)
{
	int i,j;
	for(i = 1; i < HEIGHT - 1; i++) 
	{
		for(j = 1; j < WIDTH - 1; j++)
		{
			if(Pic_Buff_Background[i][j] == 255)
			{
				Pic_Buff[i][j] = 0;
			}
		}
	}
}

//计算阈值 ,参数x 为百分比，  例如：0.03 为百分之三
int creatYuzhi(float x)
{	u16 yuzhiIndex, yuzhuleiji;
	int i,j,countTo255;
	u16 sobel_gray_array[256];
	
	yuzhiIndex = (HEIGHT-1)*(WIDTH-1) * x;  
	for(i = 0; i < 256; i++)    
	{
		sobel_gray_array[i] = 0;
	}
	
	// 统计各灰度级的数量
	for(i = 1; i < HEIGHT-1; i++) 
	{
		for(j = 1; j < WIDTH-1; j++)
		{
			sobel_gray_array[Pic_Buff[i][j]] = sobel_gray_array[Pic_Buff[i][j]] + 1; 	
		}
	}	
	
	yuzhuleiji = 0;
	countTo255 = 0;
	for(i=255; yuzhuleiji<yuzhiIndex ; i--)
	{
		yuzhuleiji = yuzhuleiji + sobel_gray_array[i];
		countTo255 = countTo255 + 1;
	}
	
	
	return (255 - countTo255);
}



//void Sobel_After(void)
//{		int i,j;
//		u8 Gx,Gy;
////		int count=0;//统计区域像素个数
////		int yuzhi;
//	
//	memcpy(Pic_Buff_Temp,Pic_Buff_Dup,HEIGHT*WIDTH*sizeof(u8));
//	

//	for(i = 1; i < HEIGHT - 1; i++) 
//	{
//		for(j = 1; j < WIDTH - 1; j++)
//		{
//			
//			if((i-y_circle)*(i-y_circle) + (j-x_circle)*(j-x_circle) < (r_circle-r_circle/5)*(r_circle-r_circle/5)) //第二次边缘检测区域应比直方图均衡化更小一些
//			{
//			Gx = abs((Pic_Buff_Temp[i+1][j-1] + 2 * Pic_Buff_Temp[i+1][j] + Pic_Buff_Temp[i+1][j+1]) - (Pic_Buff_Temp[i-1][j-1] + 2 * Pic_Buff_Temp[i-1][j] + Pic_Buff_Temp[i-1][j+1]));
//			Gy = abs((Pic_Buff_Temp[i-1][j-1] + 2 * Pic_Buff_Temp[i][j-1] + Pic_Buff_Temp[i+1][j-1]) - (Pic_Buff_Temp[i-1][j+1] + 2 * Pic_Buff_Temp[i][j+1] + Pic_Buff_Temp[i+1][j+1]));
//			if(Gy + Gx > 35){
//				Pic_Buff_Dup[i][j] = 255;
//			}else{
//				Pic_Buff_Dup[i][j] = 0;
//			}
//			}
//		}
//	}	
//	memset(Pic_Buff_Temp,0,HEIGHT*WIDTH*sizeof(u8));
//}

void Sobel_After(void)
{		int i,j;
		u8 Gx,Gy;
		u16 count=0;//统计区域像素个数
		u8 yuzhi;
//	  u8 glassYuzhi;
		u16 tempGray;
		
		
		
		memcpy(Pic_Buff_Temp,Pic_Buff_Dup,HEIGHT*WIDTH*sizeof(u8));
	
	for(i = 1; i < HEIGHT - 1; i++) 
	{
		for(j = 1; j < WIDTH - 1; j++)
		{		
			if((i-y_circle)*(i-y_circle) + (j-x_circle)*(j-x_circle) < (r_circle-r_circle/6)*(r_circle-r_circle/6))
			{
				count=count+1;
			}
		}
	}
	
	yuzhi = creatYuzhi_After(0.1, count);
//	glassYuzhi = creatYuzhi_Glass(count);

	for(i = 1; i < HEIGHT - 1; i++) 
	{
		for(j = 1; j < WIDTH - 1; j++)
		{
			
			
			if((i-y_circle)*(i-y_circle) + (j-x_circle)*(j-x_circle) < (r_circle-r_circle/6)*(r_circle-r_circle/6)) //第二次边缘检测区域应比直方图均衡化更小一些
			{
			Gx = abs((Pic_Buff_Temp[i+1][j-1] + 2 * Pic_Buff_Temp[i+1][j] + Pic_Buff_Temp[i+1][j+1]) - (Pic_Buff_Temp[i-1][j-1] + 2 * Pic_Buff_Temp[i-1][j] + Pic_Buff_Temp[i-1][j+1]));
			Gy = abs((Pic_Buff_Temp[i-1][j-1] + 2 * Pic_Buff_Temp[i][j-1] + Pic_Buff_Temp[i+1][j-1]) - (Pic_Buff_Temp[i-1][j+1] + 2 * Pic_Buff_Temp[i][j+1] + Pic_Buff_Temp[i+1][j+1]));
			tempGray = Gy + Gx;
				if(tempGray > yuzhi && tempGray > 200){ //&& Pic_Buff_Dup[i][j] > 150
					Pic_Buff_Dup[i][j] = 255;
				}else{
					Pic_Buff_Dup[i][j] = 0;
				}
			}
		}
	}
	

	
//		for(i = 1; i < HEIGHT - 1; i++) 
//	{
//		for(j = 1; j < WIDTH - 1; j++)
//		{
//			
//			
//			if((i-y_circle)*(i-y_circle) + (j-x_circle)*(j-x_circle) < (r_circle-r_circle/6)*(r_circle-r_circle/6))
//			{
//				if(Pic_Buff_Dup[i][j] > yuzhi ){ //&& Pic_Buff_Dup[i][j] > 100
//					Pic_Buff_Dup[i][j] = 255;
//				}else{
//					Pic_Buff_Dup[i][j] = 0;
//			}
//			}
//		}
//	}
	
	memset(Pic_Buff_Temp,0,HEIGHT*WIDTH*sizeof(u8));
}

void Sobel_After_Glass(void)
{		int i,j;
		u8 Gx,Gy;
		u16 count=0;//统计区域像素个数
		u8 yuzhi;
//	  u8 glassYuzhi;
		u16 tempGray;
		
		
		
		memcpy(Pic_Buff_Temp,Pic_Buff_Dup,HEIGHT*WIDTH*sizeof(u8));
	
	for(i = 1; i < HEIGHT - 1; i++) 
	{
		for(j = 1; j < WIDTH - 1; j++)
		{		
			if((i-y_circle)*(i-y_circle) + (j-x_circle)*(j-x_circle) < (r_circle-r_circle/6)*(r_circle-r_circle/6))
			{
				count=count+1;
			}
		}
	}
	
	yuzhi = creatYuzhi_After(0.1, count);
//	glassYuzhi = creatYuzhi_Glass(count);

	for(i = 1; i < HEIGHT - 1; i++) 
	{
		for(j = 1; j < WIDTH - 1; j++)
		{
			
			
			if((i-y_circle)*(i-y_circle) + (j-x_circle)*(j-x_circle) < (r_circle-r_circle/6)*(r_circle-r_circle/6)) //第二次边缘检测区域应比直方图均衡化更小一些
			{
			Gx = abs((Pic_Buff_Temp[i+1][j-1] + 2 * Pic_Buff_Temp[i+1][j] + Pic_Buff_Temp[i+1][j+1]) - (Pic_Buff_Temp[i-1][j-1] + 2 * Pic_Buff_Temp[i-1][j] + Pic_Buff_Temp[i-1][j+1]));
			Gy = abs((Pic_Buff_Temp[i-1][j-1] + 2 * Pic_Buff_Temp[i][j-1] + Pic_Buff_Temp[i+1][j-1]) - (Pic_Buff_Temp[i-1][j+1] + 2 * Pic_Buff_Temp[i][j+1] + Pic_Buff_Temp[i+1][j+1]));
			tempGray = Gy + Gx;
				if(tempGray > yuzhi){ //&& Pic_Buff_Dup[i][j] > 150
					Pic_Buff_Dup[i][j] = 255;
				}else{
					Pic_Buff_Dup[i][j] = 0;
				}
			}
		}
	}
	

	
//		for(i = 1; i < HEIGHT - 1; i++) 
//	{
//		for(j = 1; j < WIDTH - 1; j++)
//		{
//			
//			
//			if((i-y_circle)*(i-y_circle) + (j-x_circle)*(j-x_circle) < (r_circle-r_circle/6)*(r_circle-r_circle/6))
//			{
//				if(Pic_Buff_Dup[i][j] > yuzhi ){ //&& Pic_Buff_Dup[i][j] > 100
//					Pic_Buff_Dup[i][j] = 255;
//				}else{
//					Pic_Buff_Dup[i][j] = 0;
//			}
//			}
//		}
//	}
	
	memset(Pic_Buff_Temp,0,HEIGHT*WIDTH*sizeof(u8));
}

u8 creatYuzhi_Glass(u16 count)
{
	u16 tempSum = 0;
	int i, j;
	for(i = 1; i < HEIGHT - 1; i++) 
	{
		for(j = 1; j < WIDTH - 1; j++)
		{
			if((i-y_circle)*(i-y_circle) + (j-x_circle)*(j-x_circle) < (r_circle-r_circle/6)*(r_circle-r_circle/6)) //第二次边缘检测区域应比直方图均衡化更小一些
			{
				tempSum = tempSum + Pic_Buff_Dup[i][j];
			}
		}
	}
	return (u8)(tempSum/count);
}

//计算阈值 ,参数x 为百分比，参数num为区域内的像素数，  例如：0.03 为百分之三
int creatYuzhi_After(float x, int num)
{	
	u16 yuzhiIndex, yuzhuleiji, glassPixelNum;
	int i,j;
	u16 sobel_gray_array[256];
	
	//yuzhiIndex = num * x;
	for(i = 0; i < 256; i++)    
	{
		sobel_gray_array[i] = 0;
	}
	
	// 统计各灰度级的数量
	for(i = 1; i < HEIGHT-1; i++) 
	{
		for(j = 1; j < WIDTH-1; j++)
		{
			if((i-y_circle)*(i-y_circle) + (j-x_circle)*(j-x_circle) < (r_circle-r_circle/3)*(r_circle-r_circle/3))
			{
				sobel_gray_array[Pic_Buff_Dup[i][j]] = sobel_gray_array[Pic_Buff_Dup[i][j]] + 1; 
			}
		}
	}	
	
	//亮度30以下认为是杯子
	for(i = 0; i<0; i++)
	{
		glassPixelNum = glassPixelNum + sobel_gray_array[i];
	}
	
	yuzhiIndex = (num-glassPixelNum) * x;
	
	yuzhuleiji = 0;
	for(i=255; yuzhuleiji<yuzhiIndex ; i--)
	{
		yuzhuleiji = yuzhuleiji + sobel_gray_array[i];
	}
	
	
	return i;
}


void eraseHorizon()
{
	int i,j;
		for(i = 1; i < HEIGHT - 1; i++) 
	{
		for(j = 1; j < WIDTH - 1; j++)
		{		
			if((i-y_circle)*(i-y_circle) + (j-x_circle)*(j-x_circle) < (r_circle-r_circle/6)*(r_circle-r_circle/6) && Pic_Buff_Dup[i][j] == 255)
			{
				 if(Pic_Buff_Dup[i-1][j] == 0 && Pic_Buff_Dup[i+1][j] == 0)
				 {
					 Pic_Buff_Dup[i][j] = 0; 
				 }
			}
		}
	}
}



/* 函数名：Hough
 * 描述  ：Hough变换找圆 2017.07.09 Lee
 * 输入  ：无
 * 输出  ：无
 */
void Hough()
{	
			int i,j,k;
			int max_value=0;
			int tempI=0,tempJ=0,tempK=0;
//			Processing = 1;
//			__disable_irq();																			//关中断
			HoughHelper();
	
			for(k=cntR-1; k >= 0; k--)
			{
				for(i = 0; i < HEIGHT; i++) 
				{
					for(j = 0; j < WIDTH; j++)
					{

						if(hough_space[i][j][k] > max_value)
						{
							max_value = hough_space[i][j][k];

							tempI = i;
							tempJ = j;
							tempK = k;
						}
					}
				}
			}
			

							x_circle = tempJ;
							y_circle = tempI;
							r_circle = minR + tempK*stepR;
							
			


			memset(hough_space,0,HEIGHT*WIDTH*cntR*sizeof(u8));
//			__enable_irq();																			//开中断
			Processing = 0;
}

//第二次搜索圆，步长为1
void HoughAfter()
{			
			int tempI=0,tempJ=0,tempK=0, tempR = r_circle;
			int i,j,k;
			int max_value=0;
//			__disable_irq();																			//关中断
//				Processing = 1;
			HoughAfterHelper();
			for(k=9; k >= 0; k--)
			{
			for(i = 0; i < HEIGHT; i++) 
			{
				for(j = 0; j < WIDTH; j++)
				{
					if((abs(i-y_circle) < (r_circle/4)) && (abs(j-x_circle) < (r_circle/3)))
					{

							if(hough_space[i][j][k] > max_value)
							{
								max_value = hough_space[i][j][k];
								tempJ = j;
								tempI = i;
								tempK = k;
							}	
						}
					}
				}
			}
			
							x_circle = tempJ;
							y_circle = tempI;
							r_circle = tempR - 9 + tempK;
			
			memset(hough_space,0,HEIGHT*WIDTH*cntR*sizeof(u8));
//			__enable_irq();																			//开中断
			Processing = 0;
			
}

void HoughAfterHelper()
{
		int i,j,k,l,a,b;
		double t = 0;
				for(i = 0; i < HEIGHT; i++) 
			{
				for(j = 0; j < WIDTH; j++)
				{
					if((i-y_circle)*(i-y_circle) + (j-x_circle)*(j-x_circle) <= (r_circle+3)*(r_circle+3) && (abs(i-y_circle) < ((r_circle+3)/2)))
					{
						if(Pic_Buff[i][j] == 255)
						{
							for(k = 0;k< 10;k++)
							{
								for(l=0;l<361;l=l+10)
									{
									t = (l * PI) / 180;
									a = i-(r_circle-9+k)*cos(t);
									b = j-(r_circle-9+k)*sin(t);
										if(a>0 && a<HEIGHT && b>0 && b<WIDTH)
											{
												hough_space[a][b][k]++;
											}
									}
							}
						}
					}
				}
			}
}

void HoughHelper()
{				
				int i,j,k,l,a,b;
				double t = 0;
			for(i = 0; i < HEIGHT; i++) 
			{
				for(j = 0; j < WIDTH; j++)
				{
				if(Pic_Buff[i][j] == 255)
					{
					for(k = 0;k<cntR;k++)
						{
							for(l=0;l<361;l=l+10)
								{
								t = (l * PI) / 180;
								a = i-(minR+k*stepR)*cos(t);
								b = j-(minR+k*stepR)*sin(t);
									if(a>0 && a<HEIGHT && b>0 && b<WIDTH)
										{
											hough_space[a][b][k]++;
										}
								}
						}
					}
				}
			}
}



/* 函数名：Image_Send
 * 描述  ：将缩略图（60*80）DMA输出至串口 2017.06.28 Lee
 * 输入  ：无
 * 输出  ：无
 */
void Image_Send(void){

	
	
	SendBuff[0] = 0xAA;
	SendBuff[1] = x_circle;
	SendBuff[2] = y_circle;
	SendBuff[3] = r_circle;
	SendBuff[4] = 0;
	send_Image(Pic_Buff);
}


void Image_Send_Dynamic(void){
	
	
	
	SendBuff[0] = 0xAA;
	SendBuff[1] = 0;
	SendBuff[2] = 0;
	SendBuff[3] = 0;
	SendBuff[4] = max_dynamic;
	send_Image(Pic_Buff_Dup);
	
}


void Image_Send_After_Static(void){

	
	
	SendBuff[0] = 0xAA;
	SendBuff[1] = x_circle;
	SendBuff[2] = y_circle;
	SendBuff[3] = r_circle;
	SendBuff[4] = max;              
	send_Image(Pic_Buff_Dup);
	
}

void send_Image(u8 originPic[][80])
{
		int r;
		int c;
		
	for(r = 0; r < HEIGHT; r++) 
	{
		for(c = 0; c < WIDTH; c++)
		{
			if(originPic[r][c] == 0xAA)
			{
				SendBuff[WIDTH * r+ c + 5] = 0xAB;
			}else if(originPic[r][c] == 0x55)
			{
				SendBuff[WIDTH * r+ c + 5] = 0x56;
			}else
			{
				SendBuff[WIDTH * r+ c + 5] = originPic[r][c];
			}
		}
	}
	SendBuff[0] = 0xAA;
	SendBuff[SEND_BUF_SIZE-1] = 0x55;
	MYDMA_Config(DMA2_Stream7,4,(u32)&USART1->DR,(u32)SendBuff,SEND_BUF_SIZE);	//DMA2,STEAM7,CH4,外设为串口1,存储器为SendBuff,长度为:SEND_BUF_SIZE.
	USART1->CR3=1<<7;           												//使能串口1的DMA发送 
	MYDMA_Enable(DMA2_Stream7,SEND_BUF_SIZE);						//开始一次DMA传输！	
	if(DMA2->HISR&(1<<27)) 														//等待 DMA2_Steam7 传输完成
	{
		DMA2->HIFCR|=1<<27; 															//清除 DMA2_Steam7 传输完成标志	
	}
}



/* 函数名：Water_Level     静态水位检测：计算白色像素占总量的百分比
 * 描述  ：统计图片水位像素个数，判断水位 2017.07.17 Lee
 * 输入  ：
 * 输出  ：液位稳定时输出液位（用flag1,flag2判断），否则输出-1，max表示
 */
void Water_Level_Static(void)
{
	int max_level=0;
	int i, j;
	int pixel_count_sort[9]={0};
	int pixel_max[3]={0};
	int pixel_max_position[3] = {0};	
	int key;
	
	//int sum_white=0;

	water_Level_Helper();
		

	for(i=0;i<9;i++)
	{
			if(pixel_count[i] < r_circle * r_circle * 0.03)                   //绿25
			{
			pixel_count[i] = 0;
			}
		pixel_count_sort[i] = pixel_count[i];											//		静态检测									
	}
	
		for(i=0;i<9;i++)
	{
		key = pixel_count_sort[i];
		j=i-1;
		
		while(j>=0 && (pixel_count_sort[j] < key))
		{
			pixel_count_sort[j+1] = pixel_count_sort[j];										
			j=j-1;
		}
		pixel_count_sort[j+1]=key;
	}
	
	for(i=0;i<3;i++)
	{
		pixel_max[i] = pixel_count_sort[i];														//找出前三个最大值并记录编号
		for(j=8;j>=0;j--)
		{
			if(pixel_max[i] == pixel_count[j])
			{
				pixel_max_position[i] = j;
			}
		}
	}
	

	for(i=0;i<3;i++)
	{
		if(pixel_max_position[i] > max_level)															//找出最大的编号
		{
			max_level = pixel_max_position[i];
		}
	}
	

	
	if(static_flag1 == max_level && static_flag2 == max_level)											//判断输出最高液位
	{
		max = max_level;
	}else{
		max = -1;
	}
	
		static_flag2 = static_flag1;																	//更新缓存液位
		static_flag1 = max_level;
	
	
}



void Glass_Water_Level_Static(void)
{
	int max_level=0;
	int i, j;
	int pixel_count_sort[9]={0};
	int pixel_max[3]={0};
	int pixel_max_position[3] = {0};	
	int key;
	
	//int sum_white=0;

	water_Level_Helper();
		

	for(i=0;i<9;i++)
	{
			if(pixel_count[i] < r_circle * r_circle * 0.04)                   //调整
			{
			pixel_count[i] = 0;
			}
		pixel_count_sort[i] = pixel_count[i];											//		静态检测									
	}
	
		for(i=0;i<9;i++)
	{
		key = pixel_count_sort[i];
		j=i-1;
		
		while(j>=0 && (pixel_count_sort[j] < key))
		{
			pixel_count_sort[j+1] = pixel_count_sort[j];										
			j=j-1;
		}
		pixel_count_sort[j+1]=key;
	}
	
	for(i=0;i<3;i++)
	{
		pixel_max[i] = pixel_count_sort[i];														//找出前三个最大值并记录编号
		for(j=8;j>=0;j--)
		{
			if(pixel_max[i] == pixel_count[j])
			{
				pixel_max_position[i] = j;
			}
		}
	}
	

	for(i=0;i<3;i++)
	{
		if(pixel_max_position[i] > max_level)															//找出最大的编号
		{
			max_level = pixel_max_position[i];
		}
	}
	

	
	if(static_flag1 == max_level && static_flag2 == max_level)											//判断输出最高液位
	{
		max = max_level;
	}else{
		max = -1;
	}
	
		static_flag2 = static_flag1;																	//更新缓存液位
		static_flag1 = max_level;
}

//在直方图均衡化的范围内截取一个类矩形，统计每层像素 ：计算最高层是第几层
void Water_Level_Dynamic(void)
{	
//	int flag1,flag2;
	int i;
	int max_level=0;
		
	water_Level_Helper();
		
	for(i=0;i<9;i++)
	{

			if(pixel_count[i] > r_circle * r_circle * 0.04)
			{
				max_level = i;
			}			
	}
	
//	if(flag1 == max_level && flag2 == max_level)
//	{
		max_dynamic = max_level;
//	}else{
//		max = -1;
//	}
//	
//	flag2 = flag1;
//	flag1 = max;
	

}


void Glass_Water_Level_Dynamic(void)
{	
//	int flag1,flag2;
	int i;
	int max_level=0;
		
	water_Level_Helper();
		
	for(i=0;i<9;i++)
	{

			if(pixel_count[i] > r_circle * r_circle * 0.06)    //调整参数
			{
				max_level = i;
			}			
	}
	
//	if(flag1 == max_level && flag2 == max_level)
//	{
		max_dynamic = max_level;
//	}else{
//		max = -1;
//	}
//	
//	flag2 = flag1;
//	flag1 = max;
	

}

//统计每层的像素记录在数组pixel_count中
void water_Level_Helper()
{	
	int i,j,k;
	int level_sum=0,count=0,count_level=0;
	
	
	
	//每层像素
		for(j=0;j<WIDTH;j++)
		{
			for(i=0;i<HEIGHT;i++)
			{
				if((i-y_circle)*(i-y_circle) + (j-x_circle)*(j-x_circle) < (r_circle-r_circle/3)*(r_circle-r_circle/3) && (abs(i-y_circle) < (r_circle*2/3)))
				{					
					level_sum++;
				}				
			}
		}
		level_sum = level_sum/9;	
				
		k=8;

		//每过level_sum个像素将其中灰度为255的像素倒序存入pixel_count数组中（重写，逻辑乱）
		for(j=0;j<WIDTH;j++)
		{
			for(i=0;i<HEIGHT;i++)
			{
				if((i-y_circle)*(i-y_circle) + (j-x_circle)*(j-x_circle) < (r_circle-r_circle/3)*(r_circle-r_circle/3))	
					{
						count++;
						if(Pic_Buff_Dup[i][j] == 255)
						{
							count_level++;
						}
						if(count > level_sum)
						{
							count=1;
							pixel_count[k] = count_level;
							k--;
							count_level=1;
						}
				}
			}
		}
}

void OPTATest()
{
	int i, j, loopflag=1;
	while(loopflag)
	{ 
		loopflag = 0;
		for(i = 1; i < HEIGHT-2; i++) 
		{
			for(j = 1; j < WIDTH-2; j++)
			{
				if( Pic_Buff[i][j] == 255)
				{
					//8个消除模板和2个保留模板
					if(  Pic_Buff[i-1][j-1] == 0
						&& Pic_Buff[i-1][j] == 0
						&& Pic_Buff[i-1][j+1] == 0
						&& Pic_Buff[i][j-1] == 255	
						&& Pic_Buff[i][j+1] == 255
						&& Pic_Buff[i+1][j] == 255)
					{
						if( Pic_Buff[i-1][j] != 0
							||Pic_Buff[i+1][j] != 255
						  ||Pic_Buff[i+2][j] != 0)
						{
							Pic_Buff[i][j] = 0;
							loopflag = 1;
						}
					}else if(Pic_Buff[i-1][j-1] == 0
								&& Pic_Buff[i][j-1] == 0
								&& Pic_Buff[i+1][j-1] == 0
								&& Pic_Buff[i-1][j] == 255
								&& Pic_Buff[i+1][j] == 255
								&& Pic_Buff[i][j+1] == 255)
					{
						if( Pic_Buff[i][j-1] != 0
							||Pic_Buff[i][j+1] != 255
						  ||Pic_Buff[i][j+2] != 0)
						{
							Pic_Buff[i][j] = 0;
							loopflag = 1;
						}
					}else if(Pic_Buff[i+1][j-1] == 0
								&& Pic_Buff[i+1][j] == 0
								&& Pic_Buff[i+1][j+1] == 0
								&& Pic_Buff[i][j-1] == 255
								&& Pic_Buff[i][j+1] == 255
								&& Pic_Buff[i-1][j] == 255)
					{
							Pic_Buff[i][j] = 0;
							loopflag = 1;
					}else if(Pic_Buff[i-1][j+1] == 0
								&& Pic_Buff[i][j+1] == 0
								&& Pic_Buff[i+1][j+1] == 0
								&& Pic_Buff[i-1][j] == 255
								&& Pic_Buff[i+1][j] == 255
								&& Pic_Buff[i][j-1] == 255)
					{
							Pic_Buff[i][j] = 0;
							loopflag = 1;
					}else if(Pic_Buff[i-1][j] == 0
								&& Pic_Buff[i-1][j+1] == 0
								&& Pic_Buff[i][j+1] == 0
								&& Pic_Buff[i+1][j] == 255
								&& Pic_Buff[i][j-1] == 255)
					{
						if( Pic_Buff[i-1][j] != 0
							||Pic_Buff[i+1][j] != 255
						  ||Pic_Buff[i+2][j] != 0)
						{
							Pic_Buff[i][j] = 0;
							loopflag = 1;
						}
					}else if(Pic_Buff[i-1][j-1] == 0
								&& Pic_Buff[i-1][j] == 0
								&& Pic_Buff[i][j-1] == 0
								&& Pic_Buff[i][j+1] == 255
								&& Pic_Buff[i+1][j] == 255)
					{
						if( Pic_Buff[i-1][j] != 0
							||Pic_Buff[i+1][j] != 255
						  ||Pic_Buff[i+2][j] != 0)
						{
							Pic_Buff[i][j] = 0;
							loopflag = 1;
						}else if( Pic_Buff[i][j-1] != 0
										||Pic_Buff[i][j+1] != 255
										||Pic_Buff[i][j+2] != 0)
						{
							Pic_Buff[i][j] = 0;
							loopflag = 1;
						}
					}else if(Pic_Buff[i][j-1] == 0
								&& Pic_Buff[i+1][j-1] == 0
								&& Pic_Buff[i+1][j] == 0
								&& Pic_Buff[i-1][j] == 255
								&& Pic_Buff[i][j+1] == 1)
					{
						if( Pic_Buff[i][j-1] != 0
							||Pic_Buff[i][j+1] != 255
							||Pic_Buff[i][j+2] != 0)
						{
							Pic_Buff[i][j] = 0;
							loopflag = 1;
						}
					}else if(Pic_Buff[i][j+1] == 0
								&& Pic_Buff[i+1][j] == 0
								&& Pic_Buff[i+1][j+1] == 0
								&& Pic_Buff[i-1][j] == 255
								&& Pic_Buff[i][j-1] == 255)
					{
							Pic_Buff[i][j] = 0;
							loopflag = 1;						
					}
				
				}
			}
		}
	}
}

void OPTA()
{
	int i, j, loopflag=1;
	while(loopflag)
	{ 
		loopflag = 0;
		for(i = 1; i < HEIGHT-2; i++) 
		{
			for(j = 1; j < WIDTH-2; j++)
			{
				if(((i-y_circle)*(i-y_circle) + (j-x_circle)*(j-x_circle) < (r_circle-r_circle/3)*(r_circle-r_circle/3)) && Pic_Buff_Dup[i][j] == 255)
				{
					//8个消除模板和2个保留模板
					if(  Pic_Buff_Dup[i-1][j-1] == 0
						&& Pic_Buff_Dup[i-1][j] == 0
						&& Pic_Buff_Dup[i-1][j+1] == 0
						&& Pic_Buff_Dup[i][j-1] == 255	
						&& Pic_Buff_Dup[i][j+1] == 255
						&& Pic_Buff_Dup[i+1][j] == 255)
					{
						if( Pic_Buff_Dup[i-1][j] != 0
							||Pic_Buff_Dup[i+1][j] != 255
						  ||Pic_Buff_Dup[i+2][j] != 0)
						{
							Pic_Buff_Dup[i][j] = 0;
							loopflag = 1;
						}
					}else if(Pic_Buff_Dup[i-1][j-1] == 0
								&& Pic_Buff_Dup[i][j-1] == 0
								&& Pic_Buff_Dup[i+1][j-1] == 0
								&& Pic_Buff_Dup[i-1][j] == 255
								&& Pic_Buff_Dup[i+1][j] == 255
								&& Pic_Buff_Dup[i][j+1] == 255)
					{
						if( Pic_Buff_Dup[i][j-1] != 0
							||Pic_Buff_Dup[i][j+1] != 255
						  ||Pic_Buff_Dup[i][j+2] != 0)
						{
							Pic_Buff_Dup[i][j] = 0;
							loopflag = 1;
						}
					}else if(Pic_Buff_Dup[i+1][j-1] == 0
								&& Pic_Buff_Dup[i+1][j] == 0
								&& Pic_Buff_Dup[i+1][j+1] == 0
								&& Pic_Buff_Dup[i][j-1] == 255
								&& Pic_Buff_Dup[i][j+1] == 255
								&& Pic_Buff_Dup[i-1][j] == 255)
					{
							Pic_Buff_Dup[i][j] = 0;
							loopflag = 1;
					}else if(Pic_Buff_Dup[i-1][j+1] == 0
								&& Pic_Buff_Dup[i][j+1] == 0
								&& Pic_Buff_Dup[i+1][j+1] == 0
								&& Pic_Buff_Dup[i-1][j] == 255
								&& Pic_Buff_Dup[i+1][j] == 255
								&& Pic_Buff_Dup[i][j-1] == 255)
					{
							Pic_Buff_Dup[i][j] = 0;
							loopflag = 1;
					}else if(Pic_Buff_Dup[i-1][j] == 0
								&& Pic_Buff_Dup[i-1][j+1] == 0
								&& Pic_Buff_Dup[i][j+1] == 0
								&& Pic_Buff_Dup[i+1][j] == 255
								&& Pic_Buff_Dup[i][j-1] == 255)
					{
						if( Pic_Buff_Dup[i-1][j] != 0
							||Pic_Buff_Dup[i+1][j] != 255
						  ||Pic_Buff_Dup[i+2][j] != 0)
						{
							Pic_Buff_Dup[i][j] = 0;
							loopflag = 1;
						}
					}else if(Pic_Buff_Dup[i-1][j-1] == 0
								&& Pic_Buff_Dup[i-1][j] == 0
								&& Pic_Buff_Dup[i][j-1] == 0
								&& Pic_Buff_Dup[i][j+1] == 255
								&& Pic_Buff_Dup[i+1][j] == 255)
					{
						if( Pic_Buff_Dup[i-1][j] != 0
							||Pic_Buff_Dup[i+1][j] != 255
						  ||Pic_Buff_Dup[i+2][j] != 0)
						{
							Pic_Buff_Dup[i][j] = 0;
							loopflag = 1;
						}else if( Pic_Buff_Dup[i][j-1] != 0
										||Pic_Buff_Dup[i][j+1] != 255
										||Pic_Buff_Dup[i][j+2] != 0)
						{
							Pic_Buff_Dup[i][j] = 0;
							loopflag = 1;
						}
					}else if(Pic_Buff_Dup[i][j-1] == 0
								&& Pic_Buff_Dup[i+1][j-1] == 0
								&& Pic_Buff_Dup[i+1][j] == 0
								&& Pic_Buff_Dup[i-1][j] == 255
								&& Pic_Buff_Dup[i][j+1] == 1)
					{
						if( Pic_Buff_Dup[i][j-1] != 0
							||Pic_Buff_Dup[i][j+1] != 255
							||Pic_Buff_Dup[i][j+2] != 0)
						{
							Pic_Buff_Dup[i][j] = 0;
							loopflag = 1;
						}
					}else if(Pic_Buff_Dup[i][j+1] == 0
								&& Pic_Buff_Dup[i+1][j] == 0
								&& Pic_Buff_Dup[i+1][j+1] == 0
								&& Pic_Buff_Dup[i-1][j] == 255
								&& Pic_Buff_Dup[i][j-1] == 255)
					{
							Pic_Buff_Dup[i][j] = 0;
							loopflag = 1;						
					}
				
				}
			}
		}
	}
}


void clearPic(void)
{
	int i, j;
						for(i = 60; i < WIDTH; i++) 
						{
							for(j = 0; j < HEIGHT; j++)
							{
								Pic_Buff[j][i] = 0;
							}
						}
}



