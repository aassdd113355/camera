#include "sys.h"
#include "delay.h"  
#include "usart.h"  
#include "ov7670.h"
#include "timer.h"	  		 
#include "sccb.h"	
#include "exti.h"
#include "Pic_Process.h"
#include "dma.h"
#include "led.h"
void dynamic_check_Glass(void);
void dynamic_check(void);
u8 timeIsUp; //¶¨Ê±Æ÷Ê±¼äÓÃ¾¡±êÖ¾£¨µ¥´Î¼ÓË®³¬Ê±±êÖ¾£©
u16 AddWaterTime; //×Ü¼ÓË®Ê±¼äÊý
extern u8 ov_sta;	//ÔÚexit.cÀïÃæ¶¨Òå
extern int flag;	//ÔÚexit.cÀïÃæ¶¨Òå
extern volatile u8 ThereIsACircle;
extern int static_flag1, static_flag2; //ÔÚpro.cÖÐ¶¨Òå

//³õÊ¼µÚÒ»´Î½øÐÐ¾²Ì¬¼ì²â£»µ±¾²Ì¬¼ì²â³öË®Î»Îª-1Ê±¼ÌÐø¼ì²â£»¼ì²â³öË®Î»>6Ê±¼ÌÐø¼ì²â£»
//µ±¾²Ì¬¼ì²â³öË®Î»ÔÚ0-6µÄ¿É¼ÓË®µÄ·¶Î§ÄÚÊ±£¬¿ªÆô¶¯Ì¬¼ì²âºÍ¿ªË®ÐÅºÅ£»
//µ±¶¯Ì¬¼ì²â³öË®Î»Îª0-6Ê±¼ÌÐø¼ì²â£»¼ì²â³öË®Î»>6Ê±·µ»Ø¾²Ì¬¼ì²â£¬²¢¹Ø¶Ï¼ÓË®ÐÅºÅ¡£
int main(void)
{   
	cameraSysInit();	//ÏµÍ³³õÊ¼»¯
	 

 	while(1) 
	{		
		
			
		while(!ThereIsACircle);    			//Ã»¼ì²â±­¿ÚµÄÊ±ºò×èÈûÔÚÕâÀï	
		
		

			//·Ç²£Á§±­
		if(GlassArea == 0 && GlassAreaAssitSignal == 0)
		{
				CAMERA_Image_Cut_Compress_6080(0,0);//	»ñÈ¡Ò»Ö¡Í¼Æ¬£¬²¢ÇÒ´æÔÚÊý×é       2017-07-13 By BachMan			
				Image_Histeq();
				Sobel_After();
				eraseHorizon();
//			OPTA();
				Water_Level_Static();
				Image_Send_After_Static();			
						
//				while(max>=0 && max< 6 && GlassArea == 0 && timeIsUp == 0 && AddWaterTime <= 50000 && AddWaterNum < 10)
//				while(max>=0 && max< 6 && GlassArea == 0 && timeIsUp == 0 && AddWaterTime <= 50000)

				while(max>=0 && max< 6 && GlassArea == 0)
				{
					On_Off = 1;//¼ÓË®ÐÅºÅ¿ª
					
					TIM2->CR1|=0x01; //¿ªÆô¶¨Ê±Æ÷
					dynamic_check();
				}		

					
		}else if(GlassArea == 0 && GlassAreaAssitSignal == 1){
							CAMERA_Image_Cut_Compress_6080(0,0);//	»ñÈ¡Ò»Ö¡Í¼Æ¬£¬²¢ÇÒ´æÔÚÊý×é       2017-07-13 By BachMan			
							Image_Histeq();
							Sobel_After_Glass();
							eraseHorizon();
//						  OPTA();
							Glass_Water_Level_Static();
							Image_Send_After_Static();	

				while(max>=0 && max< 6 && GlassArea == 0)
				{
					On_Off = 1;//¼ÓË®ÐÅºÅ¿ª
					
					TIM2->CR1|=0x01; //¿ªÆô¶¨Ê±Æ÷
					dynamic_check_Glass();
				}					
		}
	}	
}
//	//¶¯Ì¬¼ì²â ¾²Ì¬½áÊøºó
void dynamic_check()
{
				CAMERA_Image_Cut_Compress_6080(0,0);
				Image_Histeq();
				Sobel_After();
				Water_Level_Dynamic();	
				Image_Send_Dynamic();
				AddWaterTime += TIM2->CNT;	 //ÀÛ¼Æ×Ü¼ÓË®Ê±¼ä
				TIM2->CNT = 0;               //¼ÆÊýÆ÷ÇåÁã
				if(max_dynamic > 5 || AddWaterTime > 50000)
				{
					static_flag2 = -1;			//Çå³ý¾²Ì¬¼ÓË®ÖÐµÄ»º´æ
					static_flag1 = -1;
					//AddWaterNum = AddWaterNum + 1;
					On_Off = 0;//¹Ø¶Ï¼ÓË®ÐÅºÅ
					max = 6;	//Á¬½Ó¶¯Ì¬¾²Ì¬¼ì²
					static_flag1 = 6;		
					TIM2->CR1&=0x00;
				}				
}

void dynamic_check_Glass(){
				CAMERA_Image_Cut_Compress_6080(0,0);
				Image_Histeq();
				Sobel_After_Glass();
				Glass_Water_Level_Dynamic();	
				Image_Send_Dynamic();
				AddWaterTime += TIM2->CNT;	 //ÀÛ¼Æ×Ü¼ÓË®Ê±¼ä
				TIM2->CNT = 0;               //¼ÆÊýÆ÷ÇåÁã
				if(max_dynamic > 5 || AddWaterTime > 50000)
				{
					static_flag2 = -1;				//Çå³ý¾²Ì¬¼ÓË®ÖÐµÄ»º´æ
					static_flag1 = -1;
					//AddWaterNum = AddWaterNum + 1;
					On_Off = 0;//¹Ø¶Ï¼ÓË®ÐÅºÅ
					max = 6;	//Á¬½Ó¶¯Ì¬¾²Ì¬¼ì²
					static_flag1 = 6;		
					TIM2->CR1&=0x00;
				}				
}
