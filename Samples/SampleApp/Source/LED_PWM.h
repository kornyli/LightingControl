#ifndef _LED_PWM_H
#define _LED_PWM_H
#include <ioCC2530.h>
typedef unsigned char uchar;
typedef unsigned int  uint;
#define LED1 P1_0       // P1.0�ڿ���LED1
void DelayMs(uint msec);
void P10_Normal_IO(void); //��p1.0��Ϊͨ��IO����ʹ��PWMʱ����Ϊ��ʱ��T1������
void P11_Normal_IO(void) ;  ////��p1.1��Ϊͨ��IO����ʹ��PWMʱ����Ϊ��ʱ��T1������
void  InitLed(void);
void LedPwm_Init(uchar choice);
/*******************************************************************************
choice:ѡ��IO��choice=0ΪP1_0,choice=1ΪP1_1,,choice=2ΪP1_1��P1_0
flag:flag=1,�ɰ�������flag=0�������䰵; 
*******************************************************************************/
void Use_LedPwm(uchar choice,uchar flag); 
#endif