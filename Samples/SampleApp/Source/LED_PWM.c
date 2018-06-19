#include "LED_PWM.h"



void DelayMs(uint msec)
{ 
    uint i,j;
    
    for (i=0; i<msec; i++)
        for (j=0; j<535; j++);
}
/****************************************************************************
* ��    ��: InitLed()
* ��    ��: ����LED����Ӧ��IO��
* ��ڲ���: ��
* ���ڲ���: ��
****************************************************************************/
void  InitLed(void)
{
   P1DIR |= 0x01;           //P1.0����Ϊ���
   LED1 = 1;                //ʹLED1���ϵ�Ĭ��ΪϨ��   
   
}
void P10_Normal_IO(void) 
{
    P1SEL &=~ 0x01;  
   
}
void P11_Normal_IO(void) 
{
    P1SEL &=~ 0x02;  
   
}
/****************************************************************************
* ��    ��: InitT1()
* ��    ��: ��ʱ����ʼ����TICKSPD ��16 MHzϵͳ������ʱĬ����2��Ƶ����16MHz
* ��ڲ���: ��
* ���ڲ���: ��
****************************************************************************/
void LedPwm_Init(uchar choice)
{
  
    CLKCONCMD &= ~0x40;      //����ϵͳʱ��ԴΪ32MHZ����
    while(CLKCONSTA & 0x40); //�ȴ������ȶ�Ϊ32M
    CLKCONCMD &= ~0x07;      //����ϵͳ��ʱ��Ƶ��Ϊ32MHZ   
    CLKCONCMD |= 0x38;       //ʱ���ٶ�32 MHz ��ʱ������������[5:3]250kHz

    PERCFG |= 0x40;          //��ʱ��1 ��IOλ��   1:����λ��2 
    P2SEL &= ~0x10;          //��ʱ��1����
    P2DIR |= 0xC0;           //��1���ȼ�����ʱ��1ͨ��2-3
    if(choice==0)
    {
       P1DIR |= 0x01;           //�˿�0Ϊ���    
       P1SEL |= 0x01;           //timer1 ͨ��2ӳ���P1_0
    
       T1CC2H = 0x00;           //20%ռ�ձ�Ϊ200us
       T1CC2L = 0x01;           //�޸�T1CC2L�ɵ���led������
       T1CCTL2 = 0x1c;          // ģʽѡ�� ͨ��2�Ƚ�ģʽ
    }
   
    else if(choice==1)
    {
      P1DIR |= 0x02;   //�˿�1Ϊ��� 
      P1SEL |= 0x02;   //timer1 ͨ��1ӳ���P1_1
        
      T1CC1H = 0x00;           //20%ռ�ձ�Ϊ200us
      T1CC1L = 0x01;           //�޸�T1CC2L�ɵ���led������
      T1CCTL1 = 0x1c;          // ģʽѡ�� ͨ��2�Ƚ�ģʽ
    
    }
    
    else
    {
       P1DIR |= 0x01;           //�˿�0Ϊ���    
       P1SEL |= 0x01;           //timer1 ͨ��2ӳ���P1_0
    
       T1CC2H = 0x00;           //20%ռ�ձ�Ϊ200us
       T1CC2L = 0x01;           //�޸�T1CC2L�ɵ���led������
       T1CCTL2 = 0x1c;          // ģʽѡ�� ͨ��2�Ƚ�ģʽ
       
       P1DIR |= 0x02;   //�˿�1Ϊ��� 
       P1SEL |= 0x02;   //timer1 ͨ��1ӳ���P1_1
        
      T1CC1H = 0x00;           //20%ռ�ձ�Ϊ200us
      T1CC1L = 0x01;           //�޸�T1CC2L�ɵ���led������
      T1CCTL1 = 0x1c;          // ģʽѡ�� ͨ��2�Ƚ�ģʽ
    }
    T1CC0H = 0x00;           //1ms������ʱ��,Ƶ��Ϊ976.516HZ
    T1CC0L = 0xff; 
    T1CTL = 0x02;            //250KHz 1��Ƶ
}

/****************************************************************************
* ������ں���
****************************************************************************/
void Use_LedPwm(uchar choice,uchar flag)
{
   
    unsigned int counter=0;
    unsigned int counter_down=0;
   
    //InitLed();		         //���ó�ʼ������
    LedPwm_Init(choice);                //��ʱ����ʼ����pwm����
    if(choice==0)
    {
       if(flag)
       {
          counter=1;
          while(counter<0xF7)
          {
            T1CC2H = 0x00;
            T1CC2L = counter; 
            counter++;
            DelayMs(25000);
          }
       }
       else
       {
         counter=0XF7;
         while(counter>1)
         {
            T1CC2H = 0x00;
            T1CC2L = counter; 
            counter--;
            DelayMs(20000);
         }
       
       }
    }
    
   else if(choice==1)
    {
       if(flag)
       {
          counter=1;
          while(counter<0xF7)
          {
            T1CC1H = 0x00;
            T1CC1L = counter; 
            counter++;
            DelayMs(25000);
          }
       }
       else
       {
         counter=0XF7;
         while(counter>1)
         {
            T1CC1H = 0x00;
            T1CC1L = counter; 
            counter--;
            DelayMs(20000);
         }
       
       }
    }
    
    else
    { //P1_0����Ϩ��P1_1��������
          counter=1;
          counter_down=0xF7;
          while( counter_down>1)
          {
            if(counter<0xF7/3)
            {
               T1CC1H = 0x00;
               T1CC1L = counter; 
            }
          
            T1CC2H = 0x00;
            T1CC2L = counter_down;
            counter_down--;
            counter++;
            DelayMs(25000);
          }
    }
   
}

