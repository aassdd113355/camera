#ifndef __PIC_PROCESS_H
#define __PIC_PROCESS_H
#include "sys.h"

#define HEIGHT 60     // 2017-06-22 By BachMan ���ǵ������У�������ͷ����������������෴��
#define WIDTH 80
#define THRESHOLD 0x6a
#define SEND_BUF_SIZE 4804
#define PI 3.1415926
#define maxR 30
#define minR 20
#define stepR 1
#define cntR (((maxR-minR)/stepR)+1)

extern u8 ov_sta;	//��exit.c���涨��
extern u8 Pic_Buff[HEIGHT][WIDTH];//��pic_process.c ����
extern int max;
extern int max_dynamic;


void cameraSysInit(void);
void Image_Histeq(void);
void Image_Send(void);
void Image_Sobel(void);
void Hough(void);
void CAMERA_Image_Cut_Compress_6080(u16 x_pos, u16 y_pos);        // 2017-07-13 �ɼ�ͼƬ��ѹ��ͼƬ�ͽ�ͼ
void Water_Level_Dynamic(void);
void Sobel_After(void);
void Image_Send_Dynamic(void);
void Image_Send_After_Static(void);
void Water_Level_Static(void);
int creatYuzhi(float x);
void water_Level_Helper(void);
void send_Image(u8 originPic[][80]);
#endif

