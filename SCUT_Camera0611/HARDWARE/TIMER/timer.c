#include "timer.h"
#include "Pic_Process.h"
#include "delay.h" 
#include "led.h"
#include "usart.h"
extern int flag;
extern u8 ThereIsACircle; //Pic_Process.c�ж���
u8 TIM3_count = 0;
u8 TIM2_count = 0;
u8 TIM4_count = 0;
u8 TIM9_count = 0;
extern u8 isGlass; //exti.c�ж���
extern u8 processingTimeIsUp; //�ܴ���ʱ�䳬ʱ��־ main����
u16 temp, length;
void TIM4_Int_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<2;	//TIM3ʱ��ʹ��    
 	TIM4->ARR=arr;  	  //�趨�������Զ���װֵ 
	TIM4->PSC=psc;  	  //Ԥ��Ƶ��
		
	TIM4->DIER|=1<<0;   //��������ж�
	TIM4->SR&=~(1<<0);	//����жϱ�־λ
  MY_NVIC_Init(1,0,TIM4_IRQn,2);	//��ռ0�������ȼ�0����2	
	TIM4->CR1|=0x01;	//����ʱ��   By Bachman 2018 - 01 -19
}

//ͨ�ö�ʱ��3�жϳ�ʼ��
//����ʱ��ѡ��ΪAPB1��2������APB1Ϊ42M
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//��ʱ�����ʱ����㷽��:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=��ʱ������Ƶ��,��λ:Mhz
//����ʹ�õ��Ƕ�ʱ��3
void TIM3_Int_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<1;	//TIM3ʱ��ʹ��    
 	TIM3->ARR=arr;  	  //�趨�������Զ���װֵ 
	TIM3->PSC=psc;  	  //Ԥ��Ƶ��
		
	TIM3->DIER|=1<<0;   //��������ж�
	TIM3->SR&=~(1<<0);	//����жϱ�־λ
  MY_NVIC_Init(1,0,TIM3_IRQn,2);	//��ռ0�������ȼ�0����2	
	TIM3->CR1|=0x01;	//����ʱ��   By Bachman 2018 - 01 -19
}

void TIM9_Int_Init(u16 arr,u16 psc)
{
	RCC->APB2ENR|=1<<4;	//TIM9ʱ��ʹ��    
 	TIM9->ARR=arr;  	  //�趨�������Զ���װֵ 
	TIM9->PSC=psc;  	  //Ԥ��Ƶ��
	
	TIM9->DIER|=1<<0;   //��������ж�
	TIM9->SR&=~(1<<0);	//����жϱ�־λ
  MY_NVIC_Init(1,0,TIM1_BRK_TIM9_IRQn,2);	//��ռ0�������ȼ�0����2	
	TIM9->CR1|=0x01;
}

void TIM2_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<0;    //TIM2ʱ��ʹ��
	//LED_Init();
	TIM2->ARR=arr;         //�趨�������Զ���װֵ
	TIM2->PSC=psc;         //Ԥ��Ƶ��
	TIM2->CCMR1|=1<<0;     //CC1S=01 ѡ�������IC1ӳ�䵽TI1
	TIM2->CCMR1|=1<<4;     //IC1F=0001 �����˲��� ��Fck_int�����������¼�����Ч
	TIM2->CCMR1|=0<<10;    //IC1PS=00 ���������Ƶ������Ƶ
	TIM2->CCER|=0<<1;      //CC1P=0 �����ز���
	TIM2->CCER|=1<<0;      //CC1E=1 �������������ֵ������Ĵ�����
	TIM2->DIER|=1<<1;      //�������ж�
	TIM2->DIER|=1<<0;      //��������ж�
	TIM2->CR1|=0X01;       //ʹ�ܶ�ʱ��2
	MY_NVIC_Init(2,0,TIM2_IRQn,2);//��ռ2�������ȼ�0����2
}


void TIM5_Int_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<3;	//TIM5ʱ��ʹ��
	TIM5->ARR=arr;  	  //�趨�������Զ���װֵ
	TIM5->PSC=psc;  	  //Ԥ��Ƶ��
	TIM5->DIER|=1<<0;   //��������ж�
	TIM5->CR1|=0x01; 		//������ʱ��
	TIM5->SR&=~(1<<0);	//����жϱ�־λ
	MY_NVIC_Init(1,0,TIM5_IRQn,2);	//��ռ0�������ȼ�0����2
}

//����״̬
//[7]:0,û�гɹ��Ĳ���;1,�ɹ�����һ��.
//[6]:0,��û���񵽸ߵ�ƽ;1,�Ѿ����񵽸ߵ�ƽ��.
//[5:0]:����ߵ�ƽ������Ĵ���
u8 TIM2CH1_CAPTURE_STA=0; //���벶��״̬
u16 TIM2CH1_CAPTURE_VAL;  //���벶��ֵ
//��ʱ��2�жϷ������
void TIM2_IRQHandler(void)
{
	u16 tsr;
	tsr=TIM2->SR;
	if ((TIM2CH1_CAPTURE_STA&0X80)==0)//��δ�ɹ�����
	{
		if (tsr&0X01)//���
		{
			if (TIM2CH1_CAPTURE_STA&0X40)//�Ѿ����񵽸ߵ�ƽ��
			{
				if ((TIM2CH1_CAPTURE_STA&0X3F)==0X3F)//�ߵ�ƽ̫����
				{
					TIM2CH1_CAPTURE_STA|=0X80;//��ǳɹ�������һ��
					TIM2CH1_CAPTURE_VAL=0XFFFF;
				}else TIM2CH1_CAPTURE_STA++;
			}
		}
		if (tsr&0x02)//����1���������¼�
		{
			if (TIM2CH1_CAPTURE_STA&0X40) //����һ���½���
			{
				TIM2CH1_CAPTURE_STA|=0X80; //��ǳɹ�����һ�θߵ�ƽ����
				TIM2CH1_CAPTURE_VAL=TIM2->CCR1;//��ȡ��ǰ�Ĳ���ֵ
				TIM2->CCER&=~(1<<1);    //CC1P=0 ����Ϊ�����ز���
			}else                       //��δ��ʼ����һ�β���������
			{
				TIM2CH1_CAPTURE_VAL=0;
				TIM2CH1_CAPTURE_STA=0X40; //��ǲ�����������4
				TIM2->CNT=0;             //���������
				TIM2->CCER|=1<<1;        //CC1P=1 ����Ϊ�½��ز���
			}
		}
	}
	TIM2->SR=0;//����жϱ�־λ
}

//��ʱ��5�жϷ������
void TIM5_IRQHandler(void)
{

	UltrasonicTrigger();   //����������,led.h����

    if (TIM2CH1_CAPTURE_STA&0X80)//�ɹ�������һ�θߵ�ƽ
		{
			temp=TIM2CH1_CAPTURE_STA&0X3F;
			temp*=65536;               //���ʱ���ܺ�
			temp+=TIM2CH1_CAPTURE_VAL; //�õ��ܵĸߵ�ƽʱ��
			length=temp*0.017;         //���㳤��

			if ((length<21.0f) && (length>4.0f))
			{
				  GlassAreaOut=0;       	
			}else{
					GlassAreaOut=1;     
			}
			TIM2CH1_CAPTURE_STA=0;     //������һ�β���
		}
	
	TIM5->SR&=~(1<<0);//����жϱ�־λ
}

//��ʱ��3�жϷ������	 
void TIM3_IRQHandler(void)
{ 		   
		TIM3->CR1&=0x00;	//�رն�ʱ��   By Bachman 2018 - 01 -18
		TIM3_count++;
	
	if(TIM3_count < 2 ){
			TIM3->SR&=~(1<<0);//����жϱ�־λ    // By Bachman 2018 - 05 -109  �����տ������߸����س���ͽ����жϵ����⣬������жϱ�־�ͻ�һֱ���������ж�
	}
	
	if(TIM3_count >= 2)
	{			

				TIM3->CNT = 0;    // ����������  By Bachman 2018 - 01 -18
				if(TIM3->SR&0X0001)//����ж�
				{	
										TIM3->SR&=~(1<<0);//����жϱ�־λ 
									
										if(GlassArea == 0 && isGlass == 0)
										{

											On_Off = 1; //������ˮ�ź�
											TIM4->CR1|=0x01;	//ʹ�ܶ�ʱ��
											printf("00"); 
											delay_ms(200);	 
											CAMERA_Image_Cut_Compress_6080(0,0);
											Light_On=0; //������Բ��ƹ�ر�
											
											Image_Sobel();	
											OPTATest();
											Hough();
											Image_Send();	
											HoughAfter();
											Image_Send();
											ThereIsACircle = 1;												
										}else if(GlassArea == 0 && isGlass == 1)
										{

											//��������ȡͼ����������
												On_Off = 1; //������ˮ�ź�
												TIM4->CR1|=0x01;	//ʹ�ܶ�ʱ���
												printf("11"); 
												delay_ms(200);
												CAMERA_Image_Cut_Compress_6080(0,0);
												Image_Sobel_Glass();
												clearPic();
												Hough();
												Image_Send();	
//												HoughAfter();
//												Image_Send();
												Light_On=0; //������Բ��ƹ�ر�
												Glass_Light_On=0; //������Բ��ƹ�ر�
												Light_On_Glass_Water = 1; //������ˮ�ƿ���
												ThereIsACircle = 1;
										}else{
											Light_On=0; //������Բ��ƹ�ر�
											Glass_Light_On=0; //������Բ��ƹ�ر�
										}
											if(TIM3_count == 0xff)
											{
												TIM3_count = 1;
											}
				}		
	}	
		    
}


//��ʱ��9�жϷ������	 
void TIM9_IRQHandler(void)
{ 		   
		TIM9->CR1&=0x00;	//�رն�ʱ��   By Bachman 2018 - 01 -18
		TIM9_count++;
	
	if(TIM9_count < 2 ){
			TIM9->SR&=~(1<<0);//����жϱ�־λ    // By Bachman 2018 - 05 -109  �����տ������߸����س���ͽ����жϵ����⣬������жϱ�־�ͻ�һֱ���������ж�
	}
	if(TIM9_count >= 2)
	{
				
				TIM9->CNT = 0;    // ����������  By Bachman 2018 - 01 -18
			if(TIM9->SR&0X0001)//����ж�
			{		
				
									if(GlassAreaAssitSignal == 1) //��͸����
									{					
														isGlass = 0;
														Light_On=1; 			//�ƹ��
														printf("0"); 
									}else if(GlassAreaAssitSignal == 0) //͸����
									{					
														isGlass = 1;
														//Light_On = 1;
														Glass_Light_On = 1;
														printf("1"); 
									}
									TransmitSig = 0;
										
										TIM3->CR1|=0x01;	//ʹ�ܶ�ʱ��				

												if(TIM9_count == 0xff)
												{
													TIM9_count = 1;
												}
				}		
	}	
	TIM9->SR&=~(1<<0);//����жϱ�־λ 	    
}

//��ʱ��4�жϷ������	 
void TIM4_IRQHandler(void)
{ 		   
		TIM4->CR1&=0x00;	//�رն�ʱ��   By Bachman 2018 - 01 -18
		TIM4_count++;
	
	if(TIM4_count < 2 ){
			TIM4->SR&=~(1<<0);//����жϱ�־λ    // By Bachman 2018 - 05 -109  �����տ������߸����س���ͽ����жϵ����⣬������жϱ�־�ͻ�һֱ���������ж�
	}
	if(TIM4_count >= 2)
	{
				
				TIM4->CNT = 0;    // ����������  By Bachman 2018 - 01 -18
				if(TIM4->SR&0X0001)//����ж�
				{		
											
						processingTimeIsUp = 1;
						if(TIM2_count == 0xff)
						{
							TIM2_count = 1;
						}
				}		
	}	
	TIM4->SR&=~(1<<0);//����жϱ�־λ 	    
}

