#include "dht11.h"

//32MHZ us��ʱ������
#pragma optimize=none
void dht11_delay_us(unsigned int n)
{
    n>>=1;
    while(n--)
    {
          asm("NOP");
          asm("NOP");
          asm("NOP");
          asm("NOP");
          asm("NOP");
          asm("NOP");
          asm("NOP");
          asm("NOP");
          asm("NOP");
          asm("NOP");
          asm("NOP");
          asm("NOP");
          asm("NOP");
          asm("NOP");
          asm("NOP");
    }
}


unsigned char dht11_read_byte(void)
{
    unsigned char r_val = 0; 
    unsigned char t_count = 0; //��ʱ������ֹ��ʱ��
    unsigned char i;
    
    for(i = 0 ; i < 8 ; i++)
    {
        t_count = 0;
      
        //�͵�ƽ50us��ʼһ������λ��ȡ��
        while( !DHT11_PIN )
        {
              asm("NOP");
              t_count++;
              if(t_count > 250) //��ʱ��
                  return 100;
        } 
        t_count = 0;
                             
        dht11_delay_us(32);  //32us
        
        //�ߵ�ƽ26~28us��ʾ'0',70us��ʾ'1'
        if( DHT11_PIN == 1 )
        {      
            r_val <<= 1;
            r_val |= 1;
        }
        else
        {
            r_val <<= 1;
            continue;
        }
               
        //�ȴ�DHT11�������������      
        while( DHT11_PIN == 1)
        {
            asm("NOP");
            t_count++;
            if(t_count>250)
            {
                return 100;
            }
        }            
    }
    return r_val;  
}


char dht11_value(unsigned char *temp , unsigned char *humi , unsigned char flag)
{
     unsigned char t_count = 0; //��ʱ����
     unsigned char h_i = 0 , h_f = 0;
     unsigned char t_i = 0 , t_f = 0;
     unsigned char check_sum = 0;
     
     DHT11_PIN_OUT();
     DHT11_PIN_L();  //����͵�ƽ��
     
     //�͵�ƽ����ʱ��������18ms;
     dht11_delay_us(20000); //20ms;
     
     DHT11_PIN_H();  //���������ź�,�ߵ�ƽ��
     
     //�����ȴ�20us~40us����ȡDHT11��Ӧ�����
     dht11_delay_us(30);
     
     DHT11_PIN_IN();
     if(DHT11_PIN == 0) //��ȷ����Ӧ�����
     {        
            while( !DHT11_PIN )
            {
                  asm("NOP");
                  t_count++;
            
                  if(t_count > 250) //��ʱ��
                    return -1;
            }   
            
            t_count = 0;
            
            dht11_delay_us(50); //DHT11������Ӧ��������������80us;
            while( DHT11_PIN ); //�ȴ����գ�
            {
                  asm("NOP");
                  t_count++;
            
                  if(t_count > 250) //��ʱ��
                     return -1;
            }  
            
            h_i = dht11_read_byte(); //ʪ���������֣�
            h_f = dht11_read_byte(); //ʪ��С�����֣�
            t_i = dht11_read_byte(); //�¶��������֣�
            t_f = dht11_read_byte(); //�¶�С�����֣�
            check_sum = dht11_read_byte(); //У��ͣ�  
              
            //У�����ȷ������ʪ���������ֻ�ȡ��ȷ����ʾ��ȡ�ɹ���
            if(check_sum == ( h_i + h_f + t_i + t_f ) || (h_i != 100 && t_i != 100) )
            {            
                  if(flag == DHT11_STRING)
                  {
                      temp[0] = t_i/10+0x30;
                      temp[1] = t_i%10+0x30;
                      humi[0] = h_i/10+0x30;
                      humi[1] = h_i%10+0x30;
                  }
                  else 
                  {
                      *temp = t_i;
                      *humi = h_i;
                  }                                                       
            }
            else
            {          
                if(flag == DHT11_STRING)
                {
                    temp[0] = '0';
                    temp[1] = '0';
                    humi[0] = '0';
                    humi[1] = '0';
                }
                else 
                {
                    *temp = 0;
                    *humi = 0;
                }     
    
                return -1;
            }
     }
     else
     {
           if(flag == DHT11_STRING)
            {
                temp[0] = '0';
                temp[1] = '0';
                humi[0] = '0';
                humi[1] = '0';
            }
            else 
            {
                *temp = 0;
                *humi = 0;
            }     

            return -1; 
     }     
     return 0;
}



