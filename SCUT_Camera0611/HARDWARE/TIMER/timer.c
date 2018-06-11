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
u8 TIM9_count = 0;
extern u8 isGlass; //exti.cÖÐ¶¨Òå
extern u8 processingTimeIsUp; //×Ü´¦ÀíÊ±¼ä³¬Ê±±êÖ¾ main¶¨Òå
u16 temp, length;
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

void TIM9_Int_Init(u16 arr,u16 psc)
{
	RCC->APB2ENR|=1<<4;	//TIM9Ê±ÖÓÊ¹ÄÜ    
 	TIM9->ARR=arr;  	  //Éè¶¨¼ÆÊýÆ÷×Ô¶¯ÖØ×°Öµ 
	TIM9->PSC=psc;  	  //Ô¤·ÖÆµÆ÷
	
	TIM9->DIER|=1<<0;   //ÔÊÐí¸üÐÂÖÐ¶Ï
	TIM9->SR&=~(1<<0);	//Çå³ýÖÐ¶Ï±êÖ¾Î»
  MY_NVIC_Init(1,0,TIM1_BRK_TIM9_IRQn,2);	//ÇÀÕ¼0£¬×ÓÓÅÏÈ¼¶0£¬×é2	
	TIM9->CR1|=0x01;
}

void TIM2_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<0;    //TIM2Ê±ÖÓÊ¹ÄÜ
	//LED_Init();
	TIM2->ARR=arr;         //Éè¶¨¼ÆÊýÆ÷×Ô¶¯ÖØ×°Öµ
	TIM2->PSC=psc;         //Ô¤·ÖÆµÆ÷
	TIM2->CCMR1|=1<<0;     //CC1S=01 Ñ¡ÔñÊäÈë¶ËIC1Ó³Éäµ½TI1
	TIM2->CCMR1|=1<<4;     //IC1F=0001 ÅäÖÃÂË²¨Æ÷ ÒÔFck_int²ÉÑù£¬Á½¸öÊÂ¼þºóÓÐÐ§
	TIM2->CCMR1|=0<<10;    //IC1PS=00 ÅäÖÃÊäÈë·ÖÆµ£¬²»·ÖÆµ
	TIM2->CCER|=0<<1;      //CC1P=0 ÉÏÉýÑØ²¶»ñ
	TIM2->CCER|=1<<0;      //CC1E=1 ÔÊÐí²¶»ñ¼ÆÊýÆ÷µÄÖµµ½²¶»ñ¼Ä´æÆ÷ÖÐ
	TIM2->DIER|=1<<1;      //ÔÊÐí²¶»ñÖÐ¶Ï
	TIM2->DIER|=1<<0;      //ÔÊÐí¸üÐÂÖÐ¶Ï
	TIM2->CR1|=0X01;       //Ê¹ÄÜ¶¨Ê±Æ÷2
	MY_NVIC_Init(2,0,TIM2_IRQn,2);//ÇÀÕ¼2£¬×ÓÓÅÏÈ¼¶0£¬×é2
}


void TIM5_Int_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<3;	//TIM5Ê±ÖÓÊ¹ÄÜ
	TIM5->ARR=arr;  	  //Éè¶¨¼ÆÊýÆ÷×Ô¶¯ÖØ×°Öµ
	TIM5->PSC=psc;  	  //Ô¤·ÖÆµÆ÷
	TIM5->DIER|=1<<0;   //ÔÊÐí¸üÐÂÖÐ¶Ï
	TIM5->CR1|=0x01; 		//¿ªÆô¶¨Ê±Æ÷
	TIM5->SR&=~(1<<0);	//Çå³ýÖÐ¶Ï±êÖ¾Î»
	MY_NVIC_Init(1,0,TIM5_IRQn,2);	//ÇÀÕ¼0£¬×ÓÓÅÏÈ¼¶0£¬×é2
}

//²¶»ñ×´Ì¬
//[7]:0,Ã»ÓÐ³É¹¦µÄ²¶»ñ;1,³É¹¦²¶»ñµ½Ò»´Î.
//[6]:0,»¹Ã»²¶»ñµ½¸ßµçÆ½;1,ÒÑ¾­²¶»ñµ½¸ßµçÆ½ÁË.
//[5:0]:²¶»ñ¸ßµçÆ½ºóÒç³öµÄ´ÎÊý
u8 TIM2CH1_CAPTURE_STA=0; //ÊäÈë²¶»ñ×´Ì¬
u16 TIM2CH1_CAPTURE_VAL;  //ÊäÈë²¶»ñÖµ
//¶¨Ê±Æ÷2ÖÐ¶Ï·þÎñ³ÌÐò
void TIM2_IRQHandler(void)
{
	u16 tsr;
	tsr=TIM2->SR;
	if ((TIM2CH1_CAPTURE_STA&0X80)==0)//»¹Î´³É¹¦²¶»ñ
	{
		if (tsr&0X01)//Òç³ö
		{
			if (TIM2CH1_CAPTURE_STA&0X40)//ÒÑ¾­²¶»ñµ½¸ßµçÆ½ÁË
			{
				if ((TIM2CH1_CAPTURE_STA&0X3F)==0X3F)//¸ßµçÆ½Ì«³¤ÁË
				{
					TIM2CH1_CAPTURE_STA|=0X80;//±ê¼Ç³É¹¦²¶»ñÁËÒ»´Î
					TIM2CH1_CAPTURE_VAL=0XFFFF;
				}else TIM2CH1_CAPTURE_STA++;
			}
		}
		if (tsr&0x02)//²¶»ñ1·¢Éú²¶»ñÊÂ¼þ
		{
			if (TIM2CH1_CAPTURE_STA&0X40) //²¶»ñµ½Ò»¸öÏÂ½µÑØ
			{
				TIM2CH1_CAPTURE_STA|=0X80; //±ê¼Ç³É¹¦²¶»ñµ½Ò»´Î¸ßµçÆ½Âö¿í
				TIM2CH1_CAPTURE_VAL=TIM2->CCR1;//»ñÈ¡µ±Ç°µÄ²¶»ñÖµ
				TIM2->CCER&=~(1<<1);    //CC1P=0 ÉèÖÃÎªÉÏÉýÑØ²¶»ñ
			}else                       //»¹Î´¿ªÊ¼£¬µÚÒ»´Î²¶»ñÉÏÉýÑØ
			{
				TIM2CH1_CAPTURE_VAL=0;
				TIM2CH1_CAPTURE_STA=0X40; //±ê¼Ç²¶»ñµ½ÁËÉÏÉýÑØ4
				TIM2->CNT=0;             //¼ÆÊýÆ÷Çå¿Õ
				TIM2->CCER|=1<<1;        //CC1P=1 ÉèÖÃÎªÏÂ½µÑØ²¶»ñ
			}
		}
	}
	TIM2->SR=0;//Çå³ýÖÐ¶Ï±êÖ¾Î»
}

//¶¨Ê±Æ÷5ÖÐ¶Ï·þÎñ³ÌÐò
void TIM5_IRQHandler(void)
{

	UltrasonicTrigger();   //´¥·¢³¬Éù²¨,led.h¶¨Òå

    if (TIM2CH1_CAPTURE_STA&0X80)//³É¹¦²¶»ñµ½ÁËÒ»´Î¸ßµçÆ½
		{
			temp=TIM2CH1_CAPTURE_STA&0X3F;
			temp*=65536;               //Òç³öÊ±¼ä×ÜºÍ
			temp+=TIM2CH1_CAPTURE_VAL; //µÃµ½×ÜµÄ¸ßµçÆ½Ê±¼ä
			length=temp*0.017;         //¼ÆËã³¤¶È

			if ((length<21.0f) && (length>4.0f))
			{
				  GlassAreaOut=0;       	
			}else{
					GlassAreaOut=1;     
			}
			TIM2CH1_CAPTURE_STA=0;     //¿ªÆôÏÂÒ»´Î²¶»ñ
		}
	
	TIM5->SR&=~(1<<0);//Çå³ýÖÐ¶Ï±êÖ¾Î»
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

											On_Off = 1; //¿ªÆô¼ÓË®ÐÅºÅ
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
												On_Off = 1; //¿ªÆô¼ÓË®ÐÅºÅ
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
										}else{
											Light_On=0; //ËÑË÷ÍêÔ²ºóµÆ¹â¹Ø±Õ
											Glass_Light_On=0; //ËÑË÷ÍêÔ²ºóµÆ¹â¹Ø±Õ
										}
											if(TIM3_count == 0xff)
											{
												TIM3_count = 1;
											}
				}		
	}	
		    
}


//¶¨Ê±Æ÷9ÖÐ¶Ï·þÎñ³ÌÐò	 
void TIM9_IRQHandler(void)
{ 		   
		TIM9->CR1&=0x00;	//¹Ø±Õ¶¨Ê±Æ÷   By Bachman 2018 - 01 -18
		TIM9_count++;
	
	if(TIM9_count < 2 ){
			TIM9->SR&=~(1<<0);//Çå³ýÖÐ¶Ï±êÖ¾Î»    // By Bachman 2018 - 05 -109  Ïû³ý¸Õ¿ª»ú»òÕß¸ÕÏÂÔØ³ÌÐò¾Í½øÈëÖÐ¶ÏµÄÎÊÌâ£¬²»Çå³ýÖÐ¶Ï±êÖ¾¾Í»áÒ»Ö±·´¸´½øÈëÖÐ¶Ï
	}
	if(TIM9_count >= 2)
	{
				
				TIM9->CNT = 0;    // ¼ÆÊýÆ÷ÇåÁã  By Bachman 2018 - 01 -18
			if(TIM9->SR&0X0001)//Òç³öÖÐ¶Ï
			{		
				
									if(GlassAreaAssitSignal == 1) //·ÇÍ¸Ã÷±­
									{					
														isGlass = 0;
														Light_On=1; 			//µÆ¹â´ò¿ª
														printf("0"); 
									}else if(GlassAreaAssitSignal == 0) //Í¸Ã÷±­
									{					
														isGlass = 1;
														//Light_On = 1;
														Glass_Light_On = 1;
														printf("1"); 
									}
									TransmitSig = 0;
										
										TIM3->CR1|=0x01;	//Ê¹ÄÜ¶¨Ê±Æ÷				

												if(TIM9_count == 0xff)
												{
													TIM9_count = 1;
												}
				}		
	}	
	TIM9->SR&=~(1<<0);//Çå³ýÖÐ¶Ï±êÖ¾Î» 	    
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

