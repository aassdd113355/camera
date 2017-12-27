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

u8 Pic_Buff_Dup[HEIGHT][WIDTH];
u8 Pic_Buff_Temp[HEIGHT][WIDTH];
u8 Pic_Buff[HEIGHT][WIDTH]; 
u8 SendBuff[SEND_BUF_SIZE] = {0};
u8 hough_space[HEIGHT][WIDTH][cntR]={0}; 
int x_circle, y_circle, r_circle;
float gray_density_array[256];      														 // ���ڴ�Ÿ����Ҷȼ����ܶȣ��ٷֱȣ�
float gray_density_sum_array[256];   														 //���ڴ�Ÿ����Ҷȼ����ܶȣ��ٷֱȣ����ۼ�ֵ   
int max = -1;
int max_dynamic = -1;
int pixel_count[9]={0};
int static_flag1=-1,static_flag2=-1;
u8 ThereIsACircle = 0;

 /* ��������cameraSysInit
  *  ����  ��ϵͳ��ʼ��
  */ 
void cameraSysInit()
{
	Stm32_Clock_Init(336,8,2,7);//����ʱ��,168Mhz �� APB1 Ϊ 42Mhz�� APB2 Ϊ 84Mhz�� USB/SDIO/�����������ʱ��Ϊ 48Mhz��
	delay_init(168);			//��ʱ��ʼ��  
	uart_init(84,115200);		//��ʼ�����ڲ�����Ϊ115200 
  OV7670_Init();
	delay_ms(1000);	 
	OV7670_Light_Mode(0);
	OV7670_Window_Set(10,174,240,320);	//���ô���	  
	OV7670_CS=0;
	EXTI9_Init();						//ʹ�ܶ�ʱ������	
	GpioInit();
	EXTIX_Init();
	TIM3_Int_Init(49999,3500);			//10Khz����Ƶ��,5�����ж�	

}



 /* ��������CAMERA_Image_Cut_Compress_6080
 * ����  ����������ͷFIFO��������һ��ͼƬ���м��д��������С�
 *         ��240*320�Σ�����ֻ���ݲ����洢ͼ��һ����                2017-07-12 By BachMan
 * ����  ��u16 x_pos, u16 y_pos  ��ʼ��λ�ã� ��Ⱥ͸߶���ȡ����WIDTH��HIGHT
 *				 Ӧ��ȷ��	x_pos+width ����80�� y_pos + hight ������60																				
 * ���  ����
 */ 
void CAMERA_Image_Cut_Compress_6080(u16 x_pos, u16 y_pos)        // 2017-07-13 �ɼ�ͼƬ��ѹ��ͼƬ�ͽ�ͼ
{
	unsigned char color;
	u16 col, row;
	u16 c_count = 0, r_count = 0;
	__disable_irq();																			//���ж�

		OV7670_RRST=0;																			//��ʼ��λ��ָ�� 
		OV7670_RCK=0;
		OV7670_RCK=1;
		OV7670_RCK=0;
		OV7670_RRST=1;																			//��λ��ָ����� 
		OV7670_RCK=1;  

		for(row = 0; row <240; row++)
		{
			for(col = 0; col < 320; col++)
			{
				OV7670_RCK=0;
				color=GPIOF->IDR&0X00FF;														//������
//			YUYV_Y=GPIOF->IDR&0X00FF;
				OV7670_RCK=1; 
//			color<<=8;  
		    OV7670_RCK=0;
//			color|=GPIOF->IDR&0XFF;																//������   ��ΪʱYUYV��ʽ���ڶ����ֽڲ��ö�ȡ
				OV7670_RCK=1; 
				if((row%4 == 0) && (col%4 == 0))
				{
						if((col/4 >= x_pos) && (row/4 >= y_pos) && (col/4 < (x_pos + WIDTH)) && (row/4 < (y_pos + HEIGHT)))
						{
							if(c_count < WIDTH)          										// ���c_countδ�����һ�У���c_count���� 
							{
								Pic_Buff[r_count][c_count] = color;
								c_count++;
							}
							else								        						//	���c_count�������һ�У���ת��else��ִ�л���
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
		EXTI->PR=1<<9;     	 																	//���LINE9�ϵ��жϱ�־λ
		ov_sta=0;					  														    //��ʼ��һ�βɼ�
		__enable_irq();																			//���ж�
}


 /* ��������Image_Histeq
 * ����  ����ͼ�����ֱ��ͼ���⻯�����ۻ��ֲ�������Ϊ������ֱ��ͼ�����޸�   2017-06-22 By BachMan
 * ����  ����
 * ���  ����
 */
void Image_Histeq(void)
{
	u8 histEQU[256];
	u16 gray_array[256];              															 // ���ڴ�Ÿ����Ҷȼ�������
	u16 r, c;
	u16  i, j, k, n;
	
	// ��������
	for(i = 0; i < 256; i++)    
	{
		gray_array[i] = 0;
		histEQU[i] = 0;
		gray_density_array[i] = 0;
		gray_density_sum_array[i] = 0;
	}
	
	// ͳ�Ƹ��Ҷȼ�������
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
	
	// ���㲢�Ҽ�¼���Ҷȵ��ܶȣ��ٷֱȣ�
	for(j = 0; j < 256; j++)    
	{
		gray_density_array[j] = ((float)gray_array[j])/(HEIGHT*WIDTH);  
	}
	
	// �ۼӸ��Ҷ�ֵ���ܶȣ��ٷֱ��ۼӣ�
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
	
	// ��ͼƬ����	Pic_Buff �ĻҶ�ֵת��Ϊ���⻯���µĻҶ�ֵ
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




/* ��������Image_Sobel
 * ����  ��Sobel��Ե��� 2017.07.05 Lee
 * ����  ����
 * ���  ����
 */
void Image_Sobel(void)
{	int i,j;
	u8 Gx,Gy;
	int yuzhi;

	memcpy(Pic_Buff_Temp,Pic_Buff,HEIGHT*WIDTH*sizeof(u8));

	for(i = 1; i < HEIGHT - 1; i++) 
	{
		for(j = 1; j < WIDTH - 1; j++)
		{
			Gx = abs((Pic_Buff_Temp[i+1][j-1] + 2 * Pic_Buff_Temp[i+1][j] + Pic_Buff_Temp[i+1][j+1]) - (Pic_Buff_Temp[i-1][j-1] + 2 * Pic_Buff_Temp[i-1][j] + Pic_Buff_Temp[i-1][j+1]));
			Gy = abs((Pic_Buff_Temp[i-1][j-1] + 2 * Pic_Buff_Temp[i][j-1] + Pic_Buff_Temp[i+1][j-1]) - (Pic_Buff_Temp[i-1][j+1] + 2 * Pic_Buff_Temp[i][j+1] + Pic_Buff_Temp[i+1][j+1]));
		

			Pic_Buff[i][j] = Gx + Gy;
			
		}
	}
	
	 yuzhi = creatYuzhi(0.05);	//������ֵ
	
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
//������ֵ ,����x Ϊ�ٷֱȣ�  ���磺0.03 Ϊ�ٷ�֮��
int creatYuzhi(float x)
{	u16 yuzhiIndex, yuzhuleiji;
	int i,j,countTo255;
	u16 sobel_gray_array[256];
	
	yuzhiIndex = (HEIGHT-1)*(WIDTH-1) * x;  
	for(i = 0; i < 256; i++)    
	{
		sobel_gray_array[i] = 0;
	}
	
	// ͳ�Ƹ��Ҷȼ�������
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
//		int count=0;//ͳ���������ظ���
//		int yuzhi;
	
	memcpy(Pic_Buff_Temp,Pic_Buff_Dup,HEIGHT*WIDTH*sizeof(u8));
	

	for(i = 1; i < HEIGHT - 1; i++) 
	{
		for(j = 1; j < WIDTH - 1; j++)
		{
			
			if((i-y_circle)*(i-y_circle) + (j-x_circle)*(j-x_circle) < (r_circle-r_circle/5)*(r_circle-r_circle/5)) //�ڶ��α�Ե�������Ӧ��ֱ��ͼ���⻯��СһЩ
			{
			Gx = abs((Pic_Buff_Temp[i+1][j-1] + 2 * Pic_Buff_Temp[i+1][j] + Pic_Buff_Temp[i+1][j+1]) - (Pic_Buff_Temp[i-1][j-1] + 2 * Pic_Buff_Temp[i-1][j] + Pic_Buff_Temp[i-1][j+1]));
			Gy = abs((Pic_Buff_Temp[i-1][j-1] + 2 * Pic_Buff_Temp[i][j-1] + Pic_Buff_Temp[i+1][j-1]) - (Pic_Buff_Temp[i-1][j+1] + 2 * Pic_Buff_Temp[i][j+1] + Pic_Buff_Temp[i+1][j+1]));
			if(Gy + Gx > 0.16 * 225){
				Pic_Buff_Dup[i][j] = 255;
			}else{
				Pic_Buff_Dup[i][j] = 0;
			}
			}
		}
	}	
	memset(Pic_Buff_Temp,0,HEIGHT*WIDTH*sizeof(u8));
}



/* ��������Hough
 * ����  ��Hough�任��Բ 2017.07.09 Lee
 * ����  ����
 * ���  ����
 */
void Hough()
{	
			int i,j,k;
			int max_value=0;
			int tempI=0,tempJ=0,tempK=0;
			
			HoughHelper();
			
			for(i = 0; i < HEIGHT; i++) 
			{
				for(j = 0; j < WIDTH; j++)
				{
					for(k=cntR-1; k >= 0; k--)
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
			
			if(hough_space[tempI][tempJ][tempK] >= 12)
			{
							x_circle = tempJ;
							y_circle = tempI;
							r_circle = minR + tempK*stepR;
							ThereIsACircle = 1;
			}else
			{
				x_circle = 0;
				y_circle = 0;
				r_circle = 0;
				ThereIsACircle = 0;
			}
			


			memset(hough_space,0,HEIGHT*WIDTH*cntR*sizeof(u8));
	
}

//�ڶ�������Բ������Ϊ1
void HoughAfter()
{			
			int tempI=0,tempJ=0,tempK=0;
			int i,j,k,l,a,b;
			double t = 0;
			int max_value=0;
			for(i = 0; i < HEIGHT; i++) 
			{
				for(j = 0; j < WIDTH; j++)
				{
					if((i-y_circle)*(i-y_circle) + (j-x_circle)*(j-x_circle) <= (r_circle+5)*(r_circle+5) && (abs(i-y_circle) < ((r_circle+5)/5 * 3)))
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
			for(k=9; k >= 0; k--)
			{
			for(i = 0; i < HEIGHT; i++) 
			{
				for(j = 0; j < WIDTH; j++)
				{
					if((abs(i-y_circle) < (r_circle/4)))
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
							r_circle = r_circle - 9 + tempK;
			
			memset(hough_space,0,HEIGHT*WIDTH*10*sizeof(u8));
			
			
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



/* ��������Image_Send
 * ����  ��������ͼ��60*80��DMA��������� 2017.06.28 Lee
 * ����  ����
 * ���  ����
 */
void Image_Send(void){

	
	
	SendBuff[0] = x_circle;
	SendBuff[1] = y_circle;
	SendBuff[2] = r_circle;

	send_Image(Pic_Buff);
	
}


void Image_Send_Dynamic(void){
	
	
	
	SendBuff[0] = 0;
	SendBuff[1] = 0;
	SendBuff[2] = 0;
	SendBuff[3] = max_dynamic;
	send_Image(Pic_Buff_Dup);
	
}


void Image_Send_After_Static(void){

	
	
	SendBuff[0] = x_circle;
	SendBuff[1] = y_circle;
	SendBuff[2] = r_circle;
	SendBuff[3] = max;              
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
			SendBuff[WIDTH * r+ c + 4] = originPic[r][c];
		}
	}
	MYDMA_Config(DMA2_Stream7,4,(u32)&USART1->DR,(u32)SendBuff,SEND_BUF_SIZE);	//DMA2,STEAM7,CH4,����Ϊ����1,�洢��ΪSendBuff,����Ϊ:SEND_BUF_SIZE.
	USART1->CR3=1<<7;           												//ʹ�ܴ���1��DMA���� 
	MYDMA_Enable(DMA2_Stream7,SEND_BUF_SIZE);						//��ʼһ��DMA���䣡	
		if(DMA2->HISR&(1<<27)) 														//�ȴ� DMA2_Steam7 �������
	{
		DMA2->HIFCR|=1<<27; 															//��� DMA2_Steam7 ������ɱ�־	
	}
}



/* ��������Water_Level     ��̬ˮλ��⣺�����ɫ����ռ�����İٷֱ�
 * ����  ��ͳ��ͼƬˮλ���ظ������ж�ˮλ 2017.07.17 Lee
 * ����  ��
 * ���  ��Һλ�ȶ�ʱ���Һλ����flag1,flag2�жϣ����������-1��max��ʾ
 */
void Water_Level_Static(void)
{	int max_level=0;
	int i, j;
	int pixel_count_sort[9]={0};
	int pixel_max[3]={0};
	int pixel_max_position[3] = {0};	
	int key;
	
	//int sum_white=0;

	water_Level_Helper();
		

		for(i=0;i<9;i++)
	{
		if(pixel_count[i] < r_circle * r_circle * 0.055)                   //��25
			{
				pixel_count[i] = 0;
			}
		pixel_count_sort[i] = pixel_count[i];											//		��̬���									
		
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
		pixel_max[i] = pixel_count_sort[i];														//�ҳ�ǰ�������ֵ����¼���
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
		if(pixel_max_position[i] > max_level)															//�ҳ����ı��
		{
			max_level = pixel_max_position[i];
		}
	}
	
		static_flag2 = static_flag1;																	//���»���Һλ
		static_flag1 = max_level;
	
	if(static_flag1 == max_level && static_flag2 == max_level)											//�ж�������Һλ
	{
		max = max_level;
	}else{
		max = -1;
	}
	

	
	
}

//��ֱ��ͼ���⻯�ķ�Χ�ڽ�ȡһ������Σ�ͳ��ÿ������ ��������߲��ǵڼ���
void Water_Level_Dynamic(void)
{	
//	int flag1,flag2;
	int i;
	int max_level=0;
		
	water_Level_Helper();
		
	for(i=0;i<9;i++)
	{

			if(pixel_count[i] > r_circle * r_circle * 0.08)
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

//ͳ��ÿ������ؼ�¼������pixel_count��
void water_Level_Helper()
{	
	int i,j,k;
	int level_sum=0,count=0,count_level=0;
	
	
	
	//ÿ������
		for(j=0;j<WIDTH;j++)
		{
			for(i=0;i<HEIGHT;i++)
			{
				if((i-y_circle)*(i-y_circle) + (j-x_circle)*(j-x_circle) < (r_circle-r_circle/4)*(r_circle-r_circle/4) && (abs(i-y_circle) < (r_circle*2/3)))
				{					
					level_sum++;
				}				
			}
		}
		level_sum = level_sum/9;	
				
		k=8;

		//ÿ��level_sum�����ؽ����лҶ�Ϊ255�����ص������pixel_count�����У���д���߼��ң�
		for(j=0;j<WIDTH;j++)
		{
			for(i=0;i<HEIGHT;i++)
			{
				if((i-y_circle)*(i-y_circle) + (j-x_circle)*(j-x_circle) < (r_circle-r_circle/5)*(r_circle-r_circle/5) && (abs(j-x_circle) < (r_circle/2)) )	
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


