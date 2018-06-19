#include "LED_PWM.h"



void DelayMs(uint msec)
{ 
    uint i,j;
    
    for (i=0; i<msec; i++)
        for (j=0; j<535; j++);
}
/****************************************************************************
* 名    称: InitLed()
* 功    能: 设置LED灯相应的IO口
* 入口参数: 无
* 出口参数: 无
****************************************************************************/
void  InitLed(void)
{
   P1DIR |= 0x01;           //P1.0定义为输出
   LED1 = 1;                //使LED1灯上电默认为熄灭   
   
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
* 名    称: InitT1()
* 功    能: 定时器初始化，TICKSPD 是16 MHz系统不配置时默认是2分频，即16MHz
* 入口参数: 无
* 出口参数: 无
****************************************************************************/
void LedPwm_Init(uchar choice)
{
  
    CLKCONCMD &= ~0x40;      //设置系统时钟源为32MHZ晶振
    while(CLKCONSTA & 0x40); //等待晶振稳定为32M
    CLKCONCMD &= ~0x07;      //设置系统主时钟频率为32MHZ   
    CLKCONCMD |= 0x38;       //时钟速度32 MHz 定时器标记输出设置[5:3]250kHz

    PERCFG |= 0x40;          //定时器1 的IO位置   1:备用位置2 
    P2SEL &= ~0x10;          //定时器1优先
    P2DIR |= 0xC0;           //第1优先级：定时器1通道2-3
    if(choice==0)
    {
       P1DIR |= 0x01;           //端口0为输出    
       P1SEL |= 0x01;           //timer1 通道2映射口P1_0
    
       T1CC2H = 0x00;           //20%占空比为200us
       T1CC2L = 0x01;           //修改T1CC2L可调整led的亮度
       T1CCTL2 = 0x1c;          // 模式选择 通道2比较模式
    }
   
    else if(choice==1)
    {
      P1DIR |= 0x02;   //端口1为输出 
      P1SEL |= 0x02;   //timer1 通道1映射口P1_1
        
      T1CC1H = 0x00;           //20%占空比为200us
      T1CC1L = 0x01;           //修改T1CC2L可调整led的亮度
      T1CCTL1 = 0x1c;          // 模式选择 通道2比较模式
    
    }
    
    else
    {
       P1DIR |= 0x01;           //端口0为输出    
       P1SEL |= 0x01;           //timer1 通道2映射口P1_0
    
       T1CC2H = 0x00;           //20%占空比为200us
       T1CC2L = 0x01;           //修改T1CC2L可调整led的亮度
       T1CCTL2 = 0x1c;          // 模式选择 通道2比较模式
       
       P1DIR |= 0x02;   //端口1为输出 
       P1SEL |= 0x02;   //timer1 通道1映射口P1_1
        
      T1CC1H = 0x00;           //20%占空比为200us
      T1CC1L = 0x01;           //修改T1CC2L可调整led的亮度
      T1CCTL1 = 0x1c;          // 模式选择 通道2比较模式
    }
    T1CC0H = 0x00;           //1ms的周期时钟,频率为976.516HZ
    T1CC0L = 0xff; 
    T1CTL = 0x02;            //250KHz 1分频
}

/****************************************************************************
* 程序入口函数
****************************************************************************/
void Use_LedPwm(uchar choice,uchar flag)
{
   
    unsigned int counter=0;
    unsigned int counter_down=0;
   
    //InitLed();		         //调用初始化函数
    LedPwm_Init(choice);                //定时器初始化及pwm配置
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
    { //P1_0慢慢熄灭，P1_1慢慢变亮
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

