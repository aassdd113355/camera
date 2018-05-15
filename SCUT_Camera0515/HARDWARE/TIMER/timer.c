#include "timer.h"
#include "Pic_Process.h"
#include "delay.h" 
#include "led.h"
#include "usart.h"
extern int flag;
extern u8 ThereIsACircle; //Pic_Process.cÖÐ¶¨Òå
u8 TIM3_count = 0;
u8 TIM2_count = 0;
u8 TIM4_count = 0;
extern u8 timeIsUp;//¶¨Ê±Æ÷Ê±¼äÓÃ¾¡±êÖ¾ main¶¨Òå
extern u8 isGlass; //exti.cÖÐ¶¨Òå
extern u8 processingTimeIsUp; //×Ü´¦ÀíÊ±¼ä³¬Ê±±êÖ¾ main¶¨Òå
void TIM4_Int_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<2;	//TIM3Ê±ÖÓÊ¹ÄÜ    
 	TIM4->ARR=arr;  	  //Éè¶¨¼ÆÊýÆ÷×Ô¶¯ÖØ×°Öµ 
	TIM4->PSC=psc;  	  //Ô¤·ÖÆµÆ÷
		
	TIM4->DIER|=1<<0;   //ÔÊÐí¸üÐÂÖÐ¶Ï
	TIM4->SR&=~(1<<0);	//Çå³ýÖÐ¶Ï±êÖ¾Î»
  MY_NVIC_Init(1,0,TIM4_IRQn,2);	//ÇÀÕ¼0£¬×ÓÓÅÏÈ¼¶0£¬×é2	
	TIM4->CR1|=0x01;	//¿ª¶¨Ê±Æ÷   By Bachman 2018 - 01 -19
}

//Í¨ÓÃ¶¨Ê±Æ÷3ÖÐ¶Ï³õÊ¼»¯
//ÕâÀïÊ±ÖÓÑ¡ÔñÎªAPB1µÄ2±¶£¬¶øAPB1Îª42M
//arr£º×Ô¶¯ÖØ×°Öµ¡£
//psc£ºÊ±ÖÓÔ¤·ÖÆµÊý
//¶¨Ê±Æ÷Òç³öÊ±¼ä¼ÆËã·½·¨:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=¶¨Ê±Æ÷¹¤×÷ÆµÂÊ,µ¥Î»:Mhz
//ÕâÀïÊ¹ÓÃµÄÊÇ¶¨Ê±Æ÷3
void TIM3_Int_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<1;	//TIM3Ê±ÖÓÊ¹ÄÜ    
 	TIM3->ARR=arr;  	  //Éè¶¨¼ÆÊýÆ÷×Ô¶¯ÖØ×°Öµ 
	TIM3->PSC=psc;  	  //Ô¤·ÖÆµÆ÷
		
	TIM3->DIER|=1<<0;   //ÔÊÐí¸üÐÂÖÐ¶Ï
	TIM3->SR&=~(1<<0);	//Çå³ýÖÐ¶Ï±êÖ¾Î»
  MY_NVIC_Init(1,0,TIM3_IRQn,2);	//ÇÀÕ¼0£¬×ÓÓÅÏÈ¼¶0£¬×é2	
	TIM3->CR1|=0x01;	//¿ª¶¨Ê±Æ÷   By Bachman 2018 - 01 -19
}

void TIM2_Int_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<0;	//TIM2Ê±ÖÓÊ¹ÄÜ    
 	TIM2->ARR=arr;  	  //Éè¶¨¼ÆÊýÆ÷×Ô¶¯ÖØ×°Öµ 
	TIM2->PSC=psc;  	  //Ô¤·ÖÆµÆ÷
	TIM2->CNT = 0;	
	TIM2->DIER|=1<<0;   //ÔÊÐí¸üÐÂÖÐ¶Ï
	TIM2->SR&=~(1<<0);	//Çå³ýÖÐ¶Ï±êÖ¾Î»
  MY_NVIC_Init(1,0,TIM2_IRQn,2);	//ÇÀÕ¼0£¬×ÓÓÅÏÈ¼¶0£¬×é2	
}

//¶¨Ê±Æ÷3ÖÐ¶Ï·þÎñ³ÌÐò	 
void TIM3_IRQHandler(void)
{ 		   
		TIM3->CR1&=0x00;	//¹Ø±Õ¶¨Ê±Æ÷   By Bachman 2018 - 01 -18
		TIM3_count++;
	
	if(TIM3_count < 2 ){
			TIM3->SR&=~(1<<0);//Çå³ýÖÐ¶Ï±êÖ¾Î»    // By Bachman 2018 - 05 -109  Ïû³ý¸Õ¿ª»ú»òÕß¸ÕÏÂÔØ³ÌÐò¾Í½øÈëÖÐ¶ÏµÄÎÊÌâ£¬²»Çå³ýÖÐ¶Ï±êÖ¾¾Í»áÒ»Ö±·´¸´½øÈëÖÐ¶Ï
	}
	
	if(TIM3_count >= 2)
	{			

				TIM3->CNT = 0;    // ¼ÆÊýÆ÷ÇåÁã  By Bachman 2018 - 01 -18
				if(TIM3->SR&0X0001)//Òç³öÖÐ¶Ï
				{	
										TIM3->SR&=~(1<<0);//Çå³ýÖÐ¶Ï±êÖ¾Î» 

										if(GlassArea == 0 && isGlass == 0)
										{
											On_Off = 1;
											TIM4->CR1|=0x01;	//Ê¹ÄÜ¶¨Ê±Æ÷
											printf("00"); 
											delay_ms(200);	 
											CAMERA_Image_Cut_Compress_6080(0,0);
											Light_On=0; //ËÑË÷ÍêÔ²ºóµÆ¹â¹Ø±Õ
											
											Image_Sobel();	
											OPTATest();
											Hough();
											Image_Send();	
											HoughAfter();
											Image_Send();
											ThereIsACircle = 1;												
										}else if(GlassArea == 0 && isGlass == 1)
										{
											//²£Á§±­µÄÈ¡Í¼º¯Êý¼°Æä²ÎÊ
												On_Off = 1;
												TIM4->CR1|=0x01;	//Ê¹ÄÜ¶¨Ê±Æ÷ý
												printf("11"); 
												delay_ms(200);
												CAMERA_Image_Cut_Compress_6080(0,0);
												Image_Sobel_Glass();
												clearPic();
												Hough();
												Image_Send();	
//												HoughAfter();
//												Image_Send();
												Light_On=0; //ËÑË÷ÍêÔ²ºóµÆ¹â¹Ø±Õ
												Glass_Light_On=0; //ËÑË÷ÍêÔ²ºóµÆ¹â¹Ø±Õ
												Light_On_Glass_Water = 1; //²£Á§±­Ë®µÆ¿ªÆô
												ThereIsACircle = 1;
										}
											if(TIM3_count == 0xff)
											{
												TIM3_count = 1;
											}
				}		
	}	
		    
}


//¶¨Ê±Æ÷2ÖÐ¶Ï·þÎñ³ÌÐò	 
void TIM2_IRQHandler(void)
{ 		   
		TIM2->CR1&=0x00;	//¹Ø±Õ¶¨Ê±Æ÷   By Bachman 2018 - 01 -18
		TIM2_count++;
	
	if(TIM2_count < 2 ){
			TIM2->SR&=~(1<<0);//Çå³ýÖÐ¶Ï±êÖ¾Î»    // By Bachman 2018 - 05 -109  Ïû³ý¸Õ¿ª»ú»òÕß¸ÕÏÂÔØ³ÌÐò¾Í½øÈëÖÐ¶ÏµÄÎÊÌâ£¬²»Çå³ýÖÐ¶Ï±êÖ¾¾Í»áÒ»Ö±·´¸´½øÈëÖÐ¶Ï
	}
	if(TIM2_count >= 2)
	{
				
				TIM2->CNT = 0;    // ¼ÆÊýÆ÷ÇåÁã  By Bachman 2018 - 01 -18
				if(TIM2->SR&0X0001)//Òç³öÖÐ¶Ï
				{		
											
						timeIsUp = 1;
						//On_Off = 0;//¹Ø¶Ï¼ÓË®ÐÅºÅ
						if(TIM2_count == 0xff)
						{
							TIM2_count = 1;
						}
				}		
	}	
	TIM2->SR&=~(1<<0);//Çå³ýÖÐ¶Ï±êÖ¾Î» 	    
}

//¶¨Ê±Æ÷4ÖÐ¶Ï·þÎñ³ÌÐò	 
void TIM4_IRQHandler(void)
{ 		   
		TIM4->CR1&=0x00;	//¹Ø±Õ¶¨Ê±Æ÷   By Bachman 2018 - 01 -18
		TIM4_count++;
	
	if(TIM4_count < 2 ){
			TIM4->SR&=~(1<<0);//Çå³ýÖÐ¶Ï±êÖ¾Î»    // By Bachman 2018 - 05 -109  Ïû³ý¸Õ¿ª»ú»òÕß¸ÕÏÂÔØ³ÌÐò¾Í½øÈëÖÐ¶ÏµÄÎÊÌâ£¬²»Çå³ýÖÐ¶Ï±êÖ¾¾Í»áÒ»Ö±·´¸´½øÈëÖÐ¶Ï
	}
	if(TIM4_count >= 2)
	{
				
				TIM4->CNT = 0;    // ¼ÆÊýÆ÷ÇåÁã  By Bachman 2018 - 01 -18
				if(TIM4->SR&0X0001)//Òç³öÖÐ¶Ï
				{		
											
						processingTimeIsUp = 1;
						if(TIM2_count == 0xff)
						{
							TIM2_count = 1;
						}
				}		
	}	
	TIM4->SR&=~(1<<0);//Çå³ýÖÐ¶Ï±êÖ¾Î» 	    
}

