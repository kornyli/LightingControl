#ifndef _LED_PWM_H
#define _LED_PWM_H
#include <ioCC2530.h>
typedef unsigned char uchar;
typedef unsigned int  uint;
#define LED1 P1_0       // P1.0口控制LED1
void DelayMs(uint msec);
void P10_Normal_IO(void); //将p1.0设为通用IO，在使用PWM时是作为定时器T1的外设
void P11_Normal_IO(void) ;  ////将p1.1设为通用IO，在使用PWM时是作为定时器T1的外设
void  InitLed(void);
void LedPwm_Init(uchar choice);
/*******************************************************************************
choice:选择IO；choice=0为P1_0,choice=1为P1_1,,choice=2为P1_1和P1_0
flag:flag=1,由暗变亮；flag=0，由亮变暗; 
*******************************************************************************/
void Use_LedPwm(uchar choice,uchar flag); 
#endif