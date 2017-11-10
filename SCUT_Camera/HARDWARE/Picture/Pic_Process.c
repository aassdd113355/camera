#include "Pic_Process.h"
#include "ov7670.h"
#include "exti.h"
#include "timer.h"
#include "usart.h"
#include "dma.h"
#include "math.h"
#include "lcd.h"
#include "String.h"
#include "led.h"
#include "delay.h"  
u8 Pic_Buff_Dup[HEIGHT][WIDTH];
u8 Pic_Buff_Temp[HEIGHT][WIDTH];
u8 Pic_Buff[HEIGHT][WIDTH]; 
u8 SendBuff[SEND_BUF_SIZE] = {0};
u16 hough_space[HEIGHT][WIDTH][cntR]={0}; 
int x_circle, y_circle, r_circle;
float gray_density_array[256];      														 // 用于存放各个灰度级的密度（百分比）
float gray_density_sum_array[256];   														 //用于存放各个灰度级的密度（百分比）的累加值   
int max_level,max = -1;
int flag1,flag2,flag;
int pixel_count[9]={0};
int cI,cJ;




 /* 函数名：CAMERA_Get_ImageCut
 * 描述  ：将从摄像头FIFO发过来的一张图片进行剪切存在数组中。
 *         读240*320次，但是只根据参数存储图的一部分                2017-06-20 By BachMan
 * 输入  ：u16 x_pos, u16 y_pos  开始的位置， 宽度和高度则取决于WIDTH和HIGHT
 *				 应该确保	x_pos+width 不超320， y_pos + hight 不超过240																				
 * 输出  ：无
 */ 
void CAMERA_Get_ImageCut(u16 x_pos, u16 y_pos)        // 2017-06-22 By BachMan 我们的行与列，跟摄像头的输出行与列正好相反。
{
	unsigned char color;
	u16 col, row;
	u16 c_count = 0, r_count = 0;

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
//			color|=GPIOF->IDR&0XFF;	//读数据   因为时YUYV格式，第二个字节不用读取
				OV7670_RCK=1; 
				
				if((col >= x_pos) && (row >= y_pos) && (col < (x_pos + WIDTH)) && (row < (y_pos + HEIGHT)))
				{
					if(c_count < WIDTH)          												// 如果c_count未到最后一列，则c_count增加 
					{
						Pic_Buff[r_count][c_count] = color;
						c_count++;
					}
					else								        								//	如果c_count到了最后一列，则转到else中执行换行
					{
						c_count = 0;
						r_count++;
						Pic_Buff[r_count][c_count] = color;
						c_count++;
					}
				}
			}
		}
		
		EXTI->PR=1<<9;     	 //清除LINE9上的中断标志位
		ov_sta=0;					   //开始下一次采集
}

 /* 函数名：CAMERA_Image_Cut_Compress_120160
 * 描述  ：将从摄像头FIFO发过来的一张图片进行剪切存在数组中。
 *         读240*320次，但是只根据参数存储图的一部分                2017-07-12 By BachMan
 * 输入  ：u16 x_pos, u16 y_pos  开始的位置， 宽度和高度则取决于WIDTH和HIGHT
 *				 应该确保	x_pos+width 不超160， y_pos + hight 不超过120																				
 * 输出  ：无
 */ 
void CAMERA_Image_Cut_Compress_120160(u16 x_pos, u16 y_pos)        // 2017-07-13 采集图片和压缩图片和截图
{
	unsigned char color;
	u16 col, row;
	u16 c_count = 0, r_count = 0;

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
//			color|=GPIOF->IDR&0XFF;	//读数据   因为时YUYV格式，第二个字节不用读取
				OV7670_RCK=1; 
				if((row%2 == 0) && (col%2 == 0))
				{
						if((col/2 >= x_pos) && (row/2 >= y_pos) && (col/2 < (x_pos + WIDTH)) && (row/2 < (y_pos + HEIGHT)))
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
		
		EXTI->PR=1<<9;     	 //清除LINE9上的中断标志位
		ov_sta=0;					   //开始下一次采集
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
		EXTI->PR=1<<9;     	 																	//清除LINE9上的中断标志位
		ov_sta=0;					  														    //开始下一次采集
}


 /* 函数名：Image_Histeq
 * 描述  ：对图像进行直方图均衡化，以累积分布函数作为基础的直方图进行修改   2017-06-22 By BachMan
 * 输入  ：无
 * 输出  ：无
 */
void Image_Histeq(void)
{
	u8 histEQU[256];
	u16 gray_array[256];              															 // 用于存放各个灰度级的数量
	u16 r, c;
	u16  i, j, k, n;
	
	// 清零数组
	for(i = 0; i < 256; i++)    
	{
		gray_array[i] = 0;
		histEQU[i] = 0;
		gray_density_array[i] = 0;
		gray_density_sum_array[i] = 0;
	}
	
	// 统计各灰度级的数量
	for(r = 0; r < HEIGHT; r++) 
	{
		for(c = 0; c < WIDTH; c++)
		{
			if((r-y_circle)*(r-y_circle) + (c-x_circle)*(c-x_circle) < (r_circle-r_circle/8)*(r_circle-r_circle/8))
				{
					gray_array[Pic_Buff_Dup[r][c]] = gray_array[Pic_Buff_Dup[r][c]] + 1; 
				}				
		}
	}	
	
	// 计算并且记录各灰度的密度（百分比）
	for(j = 0; j < 256; j++)    
	{
		gray_density_array[j] = ((float)gray_array[j])/(HEIGHT*WIDTH);  
	}
	
	// 累加各灰度值的密度（百分比累加）
	for (k = 0; k < 256; k++)
	{
		if(k == 0)
		{
			gray_density_sum_array[k] = gray_density_array[k];		
		}
		else
		{
			gray_density_sum_array[k] = gray_density_sum_array[k - 1] + gray_density_array[k];				
		}	
	}
	
	//
	for(n = 0; n < 256; n++)
	{
		histEQU[n] = (u8)(gray_density_sum_array[n] * 255.0+0.5);
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

	
//		for(i = 1; i < HEIGHT - 1; i++) 
//	{
//		for(j = 1; j < WIDTH - 1; j++)
//		{
//			Pic_Buff[i][j] = (u8)((Pic_Buff[i-1][j-1] + 2*Pic_Buff[i-1][j] + Pic_Buff[i-1][j+1] + 2*Pic_Buff[i][j-1] + 4*Pic_Buff[i][j] + 2*Pic_Buff[i][j+1] + Pic_Buff[i+1][j-1] + 2*Pic_Buff[i+1][j] + Pic_Buff[i+1][j+1])/16);
//		}
//	}
		//yuzhi = 0;
	memcpy(Pic_Buff_Temp,Pic_Buff,HEIGHT*WIDTH*sizeof(u8));
	
//		for(i = 1; i < HEIGHT - 1; i++) 
//	{
//		for(j = 1; j < WIDTH - 1; j++)
//		{
//				yuzhi += Pic_Buff[i][j];
//		}
//	}
//	
//	yuzhi = yuzhi /(HEIGHT * WIDTH) * 0.5;

	
	
	
	for(i = 1; i < HEIGHT - 1; i++) 
	{
		for(j = 1; j < WIDTH - 1; j++)
		{
			Gx = abs((Pic_Buff_Temp[i+1][j-1] + 2 * Pic_Buff_Temp[i+1][j] + Pic_Buff_Temp[i+1][j+1]) - (Pic_Buff_Temp[i-1][j-1] + 2 * Pic_Buff_Temp[i-1][j] + Pic_Buff_Temp[i-1][j+1]));
			Gy = abs((Pic_Buff_Temp[i-1][j-1] + 2 * Pic_Buff_Temp[i][j-1] + Pic_Buff_Temp[i+1][j-1]) - (Pic_Buff_Temp[i-1][j+1] + 2 * Pic_Buff_Temp[i][j+1] + Pic_Buff_Temp[i+1][j+1]));
		

			Pic_Buff[i][j] = Gx + Gy;
			
		}
	}
	
	

	 yuzhi = creatYuzhi(0.03);	//计算阈值
	
		for(i = 1; i < HEIGHT - 1; i++) 
	{
		for(j = 1; j < WIDTH - 1; j++)
		{

			if(Pic_Buff[i][j] > yuzhi){
				Pic_Buff[i][j] = 255;
			}else{
				Pic_Buff[i][j] = 0;
			}

			
		}
	}
	
	


	
	
	memset(Pic_Buff_Temp,0,HEIGHT*WIDTH*sizeof(u8));
		
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



void Sobel_After(void)
{		int i,j;
		u8 Gx,Gy;
	
		memcpy(Pic_Buff_Temp,Pic_Buff_Dup,HEIGHT*WIDTH*sizeof(u8));
	

	for(i = 1; i < HEIGHT - 1; i++) 
	{
		for(j = 1; j < WIDTH - 1; j++)
		{
			
			
			if((i-y_circle)*(i-y_circle) + (j-x_circle)*(j-x_circle) < (r_circle-r_circle/6)*(r_circle-r_circle/6))
			{
			Gx = abs((Pic_Buff_Temp[i+1][j-1] + 2 * Pic_Buff_Temp[i+1][j] + Pic_Buff_Temp[i+1][j+1]) - (Pic_Buff_Temp[i-1][j-1] + 2 * Pic_Buff_Temp[i-1][j] + Pic_Buff_Temp[i-1][j+1]));
			Gy = abs((Pic_Buff_Temp[i-1][j-1] + 2 * Pic_Buff_Temp[i][j-1] + Pic_Buff_Temp[i+1][j-1]) - (Pic_Buff_Temp[i-1][j+1] + 2 * Pic_Buff_Temp[i][j+1] + Pic_Buff_Temp[i+1][j+1]));
		

			if(Gy + Gx > 100){
				Pic_Buff_Dup[i][j] = 255;
			}else{
				Pic_Buff_Dup[i][j] = 0;
			}
			}
		}
	}
	
	
	memset(Pic_Buff_Temp,0,HEIGHT*WIDTH*sizeof(u8));
}


/* 函数名：Hough
 * 描述  ：Hough变换找圆 2017.07.09 Lee
 * 输入  ：无
 * 输出  ：无
 */
void Hough()
{	
		int i,j,k,l,a,b;
			double t = 0;
			int max_value=0;
		//	int x=0,y=0,r=0;
			
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
			
			for(i = 0; i < HEIGHT; i++) 
			{
				for(j = 0; j < WIDTH; j++)
				{
					for(k=0; k < cntR; k++)
					{
						if(hough_space[i][j][k] >= max_value)
						{
							max_value = hough_space[i][j][k];
							x_circle = j;
							y_circle = i;
							r_circle = minR + k*stepR;
							
						}
					}
				}
			}

			memset(hough_space,0,HEIGHT*WIDTH*cntR*sizeof(u16));
	
}


/* 函数名：Image_Send
 * 描述  ：将缩略图（60*80）DMA输出至串口 2017.06.28 Lee
 * 输入  ：无
 * 输出  ：无
 */
void Image_Send(void){
	int r;
	int c;
	
	
	SendBuff[0] = x_circle;
	SendBuff[1] = y_circle;
	SendBuff[2] = r_circle;
	
	for(r = 0; r < HEIGHT; r++) 
	{
		for(c = 0; c < WIDTH; c++)
		{
			SendBuff[WIDTH * r+ c + 4] = Pic_Buff[r][c];
		}
	}
	MYDMA_Config(DMA2_Stream7,4,(u32)&USART1->DR,(u32)SendBuff,SEND_BUF_SIZE);	//DMA2,STEAM7,CH4,外设为串口1,存储器为SendBuff,长度为:SEND_BUF_SIZE.
	USART1->CR3=1<<7;           												//使能串口1的DMA发送 
	MYDMA_Enable(DMA2_Stream7,SEND_BUF_SIZE);									//开始一次DMA传输！	
		if(DMA2->HISR&(1<<27)) 													//等待 DMA2_Steam7 传输完成
	{
		DMA2->HIFCR|=1<<27; 													//清除 DMA2_Steam7 传输完成标志	
	}
	
}


void Image_Send_After(void){
	int r;
	int c;
	
	
	SendBuff[0] = x_circle;
	SendBuff[1] = y_circle;
	SendBuff[2] = r_circle;
	SendBuff[3] = max;              
	for(r = 0; r < HEIGHT; r++) 
	{
		for(c = 0; c < WIDTH; c++)
		{
			SendBuff[WIDTH * r+ c + 4] = Pic_Buff_Dup[r][c];
		}
	}
	MYDMA_Config(DMA2_Stream7,4,(u32)&USART1->DR,(u32)SendBuff,SEND_BUF_SIZE);	//DMA2,STEAM7,CH4,外设为串口1,存储器为SendBuff,长度为:SEND_BUF_SIZE.
	USART1->CR3=1<<7;           												//使能串口1的DMA发送 
	MYDMA_Enable(DMA2_Stream7,SEND_BUF_SIZE);									//开始一次DMA传输！	
		if(DMA2->HISR&(1<<27)) 													//等待 DMA2_Steam7 传输完成
	{
		DMA2->HIFCR|=1<<27; 													//清除 DMA2_Steam7 传输完成标志	
	}
	
}


void Image_Send_After_Static(void){
	int r;
	int c;
	
	
	SendBuff[0] = 0;
	SendBuff[1] = 0;
	SendBuff[2] = 0;
	SendBuff[3] = max;              
	for(r = 0; r < HEIGHT; r++) 
	{
		for(c = 0; c < WIDTH; c++)
		{
			SendBuff[WIDTH * r+ c + 4] = Pic_Buff_Dup[r][c];
		}
	}
	MYDMA_Config(DMA2_Stream7,4,(u32)&USART1->DR,(u32)SendBuff,SEND_BUF_SIZE);	//DMA2,STEAM7,CH4,外设为串口1,存储器为SendBuff,长度为:SEND_BUF_SIZE.
	USART1->CR3=1<<7;           												//使能串口1的DMA发送 
	MYDMA_Enable(DMA2_Stream7,SEND_BUF_SIZE);									//开始一次DMA传输！	
		if(DMA2->HISR&(1<<27)) 													//等待 DMA2_Steam7 传输完成
	{
		DMA2->HIFCR|=1<<27; 													//清除 DMA2_Steam7 传输完成标志	
	}
	
}



/* 函数名：Water_Level     动态水位检测：计算白色像素占总量的百分比
 * 描述  ：统计图片水位像素个数，判断水位 2017.07.17 Lee
 * 输入  ：无
 * 输出  ：无
 */
void Water_Level(void)
{
	
	int pixel_count_sort[9]={0};
	int pixel_max[3]={0};
	int pixel_max_position[3] = {0};
	int i,j,k;
	int key;
	int level_sum=0,count=0,count_level=0;
	int sum_white=0;
	LED1=1;



	for(j=0;j<WIDTH;j++)
		{
			for(i=0;i<HEIGHT;i++)
			{
				if((i-y_circle)*(i-y_circle) + (j-x_circle)*(j-x_circle) < (r_circle-r_circle/7)*(r_circle-r_circle/7) && (abs(i-y_circle) < (r_circle/2)))
				{					
					level_sum++;
				if(Pic_Buff_Dup[i][j] == 255)
						{
							sum_white++;
						}
				}				
			}
		}
		level_sum = level_sum/9;	
				
		k=8;


		for(j=0;j<WIDTH;j++)
		{
			for(i=0;i<HEIGHT;i++)
			{
				if((i-y_circle)*(i-y_circle) + (j-x_circle)*(j-x_circle) < (r_circle-r_circle/7)*(r_circle-r_circle/7) && (abs(i-y_circle) < (r_circle/2)) )	
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
		
		


		for(i=0;i<9;i++)
	{
		if(pixel_count[i] < 15)                   //绿25
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
	
	if(flag1 == max_level && flag2 == max_level)
	{
		max = max_level;
	}else{
		max = -1;
	}
	
	flag2 = flag1;
	flag1 = max_level;
	
	
}

//在直方图均衡化的范围内截取一个类矩形，统计每层像素（仔细想有没有必要） ：计算最高层是第几层
void Water_Level_Static(void)
{
	int i,j,k;
	int level_sum=0,count=0,count_level=0;
	
	
		//动态检测
	
	//每层像素
		for(j=0;j<WIDTH;j++)
		{
			for(i=0;i<HEIGHT;i++)
			{
				if((i-y_circle)*(i-y_circle) + (j-x_circle)*(j-x_circle) < (r_circle-r_circle/7)*(r_circle-r_circle/7) && (abs(i-y_circle) < (r_circle/2)))
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
				if((i-y_circle)*(i-y_circle) + (j-x_circle)*(j-x_circle) < (r_circle-r_circle/7)*(r_circle-r_circle/7) && (abs(i-y_circle) < (r_circle/2)) )	
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
		
	
	
	
	for(i=0;i<9;i++)
	{

			if(pixel_count[i] > 15)
			{
				max = i;
			}else{
			break;
			}
			
	}
	
	if(max >= 6)
	{
		//发送停止信号
		max = -1;
		delay_ms(10000);
	}
}




