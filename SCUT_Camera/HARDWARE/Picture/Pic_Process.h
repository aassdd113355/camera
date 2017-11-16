#ifndef __PIC_PROCESS_H
#define __PIC_PROCESS_H
#include "sys.h"

#define HEIGHT 60     // 2017-06-22 By BachMan 我们的行与列，跟摄像头的输出行与列正好相反。
#define WIDTH 80
#define THRESHOLD 0x6a
#define SEND_BUF_SIZE 4804
#define PI 3.1415926
#define maxR 25
#define minR 15
#define stepR 1
#define cntR (((maxR-minR)/stepR)+1)

extern u8 ov_sta;	//在exit.c里面定义
extern u8 Pic_Buff[HEIGHT][WIDTH];//在pic_process.c 定义
extern int max;



void cameraSysInit(void);
void Image_Binaryzation(void);
void Image_Filter(void);
void Get_Black_Line(void);
void Black_Line_handle(void);
//void CAMERA_Get_Image(void);
void CAMERA_Get_ImageCut(u16 x_pos, u16 y_pos);
void Image_Histeq(void);
void Image_Send(void);
void Image_Sobel(void);
void Hough(void);
void PrintCircle(int x1,int y1,int r1);
void Image_Tran(void);
void CAMERA_Image_Cut_Compress_120160(u16 x_pos, u16 y_pos);        // 2017-07-13 采集图片和压缩图片和截图 
void CAMERA_Image_Cut_Compress_6080(u16 x_pos, u16 y_pos);        // 2017-07-13 采集图片和压缩图片和截图
void Water_Level_Dynamic(void);
void Sobel_After(void);
void Image_Send_Dynamic(void);
void Image_Send_After_Static(void);
void Water_Level_Static(void);
void findCenter(void);
int creatYuzhi(float x);
int creatYuzhi_After(float, int);
void water_Level_Helper(void);
void send_Image(u8 originPic[][80]);
#endif

