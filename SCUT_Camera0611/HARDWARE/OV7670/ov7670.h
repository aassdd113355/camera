#ifndef _OV7670_H
#define _OV7670_H
#include "sys.h"
#include "sccb.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ο�������guanfu_wang���롣
//ALIENTEKս��STM32������
//OV7670 ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/14
//�汾��V1.0		    							    							  
//////////////////////////////////////////////////////////////////////////////////

#define OV7670_VSYNC  	PFin(9)			//ͬ���źż��IO
#define OV7670_WRST		PEout(3)		//дָ�븴λ
#define OV7670_WREN		PCout(3)		//д��FIFOʹ��
#define OV7670_RCK		PCout(1)		//������ʱ��
#define OV7670_RRST		PEout(5)  		//��ָ�븴λ
#define OV7670_CS		PCout(13)  		//Ƭѡ�ź�(OE)
															  					 
#define OV7670_DATA   GPIOF->IDR&0x00FF  					//��������˿�
/////////////////////////////////////////									
	    				 
extern const u8 ov7670_init_reg_tbl_YUV[174][2];		//�Ĵ����������ñ�
							 
							 
u8   OV7670_Init(void);		  	   		 
void OV7670_Light_Mode(u8 mode);
void OV7670_Color_Saturation(u8 sat);
void OV7670_Brightness(u8 bright);
void OV7670_Contrast(u8 contrast);
void OV7670_Special_Effects(u8 eft);
void OV7670_Window_Set(u16 sx,u16 sy,u16 width,u16 height);


#endif





















