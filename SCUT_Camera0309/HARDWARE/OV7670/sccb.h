#ifndef __SCCB_H
#define __SCCB_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//OVϵ������ͷ SCCB ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/14
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 


//IO��������
#define SCCB_SDA_IN()  {GPIOE->MODER&=~(3<<(6*2));GPIOE->MODER|=0<<6*2;}	//PE6 ����
#define SCCB_SDA_OUT() {GPIOE->MODER&=~(3<<(6*2));GPIOE->MODER|=1<<6*2;} 	//PE6 ���


//IO��������	 
#define SCCB_SCL    		PEout(4)	 	//SCL
#define SCCB_SDA    		PEout(6) 		//SDA	 

#define SCCB_READ_SDA    	PEin(6)  		//����SDA    
#define SCCB_ID   			0x42  			//OV7670��ID

///////////////////////////////////////////
void SCCB_Init(void);
void SCCB_Start(void);
void SCCB_Stop(void);
void SCCB_No_Ack(void);
u8 SCCB_WR_Byte(u8 dat);
u8 SCCB_RD_Byte(void);
u8 SCCB_WR_Reg(u8 reg,u8 data);
u8 SCCB_RD_Reg(u8 reg);
#endif













