/******************************************
*              
*           �����޸��ߣ� jafy
*           �޸�ԭ��   ����ѧϰ
*
*******************************************/
#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"
#include <string.h>
#include "SampleApp.h"
#include "DebugTrace.h"

#if !defined( WIN32 )
#include "OnBoard.h"
#endif

#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_uart.h"
#include "LED_PWM.h"
#include  <stdio.h>
#include   <string.h>

const cId_t SampleApp_IN_ClusterList[ SAMPLEAPP_MAX_CLUSTERS ] =
{
  SAMPLEAPP_IN_CLUSTERID ,
  SAMPLEAPP_DATA_CLUSTERID 
};

const cId_t SampleApp_OUT_ClusterList[ SAMPLEAPP_MAX_CLUSTERS ] =
{
  SAMPLEAPP_OUT_CLUSTERID 
};

const SimpleDescriptionFormat_t  SampleApp_SimpleDesc =
{
  SAMPLEAPP_ENDPOINT ,
  SAMPLEAPP_PROFID ,
  SAMPLEAPP_DEVICEID,
  SAMPLEAPP_DEVICE_VERSION,
  SAMPLEAPP_FLAGS,
  SAMPLEAPP_MAX_CLUSTERS ,
  ( cId_t *)SampleApp_IN_ClusterList,
  SAMPLEAPP_MAX_CLUSTERS,
  ( cId_t *)SampleApp_OUT_ClusterList
};   //��������

endPointDesc_t SampleApp_epDesc; //�˵����������˵�������������������
byte SampleApp_TaskID;  //����ID
devStates_t SampleApp_NwkState; //�豸����״̬
byte SampleApp_TranskID;  //�������к�
unsigned char Uartbuf[20];  //���ڽ�����λ������
DeId NodeId[3];  //�洢�ն˽ڵ��ID��ip
uint8 NodeNum=0;
uint16 NodeId_arr[4];



void SampleApp_MessageMSGCB( afIncomingMSGPacket_t *pckt ); //��Ϣ������
//void SampleApp_SendTheMessage( void );   //���ݷ��ͺ���
void SampleApp_SendTheMessage(uint16 ShortAdr,uint8 command ); //���ݷ��ͺ���
//�����¼�����������
void SampleApp_HandleKeys( uint8 shift, uint8 keys );
static void rxCB(uint8 port,uint8 event); //���ڽ��մ�����
void To_string(uint8 *dest ,char *src,uint8 lenght);


/******************************************
*              
*     �������ƣ�SampleApp_Init
*     �������ܣ�Ӧ�ò��ʼ��
*
*******************************************/
void SampleApp_Init( uint8 task_id )
{ 
    halUARTCfg_t uartconfig;
    SampleApp_TaskID                  =task_id;
    SampleApp_TranskID                =0;              //�����ݰ�����ų�ʼ��Ϊ0��ÿ����һ�����ݰ���������Զ���1
    SampleApp_epDesc.endPoint         = SAMPLEAPP_ENDPOINT;
    SampleApp_epDesc.task_id          =& SampleApp_TaskID;
    SampleApp_epDesc.simpleDesc       =
          ( SimpleDescriptionFormat_t * )&SampleApp_SimpleDesc;
    SampleApp_epDesc.latencyReq      =noLatencyReqs;
    afRegister( &SampleApp_epDesc );    //���˵�����������ע��
 
    RegisterForKeys( task_id ); // �Ǽ����еİ����¼�
    //HalLedSet ( HAL_LED_ALL,HAL_LED_MODE_TOGGLE);
    
     //���ڳ�ʼ��
  uartconfig.configured             =TRUE;
  uartconfig.baudRate               =HAL_UART_BR_115200;
  uartconfig.flowControl            =FALSE;
  uartconfig.callBackFunc           =rxCB;
  HalUARTOpen (0,&uartconfig);
  osal_set_event(task_id,GET_ENDDEVICE_IFO);
}


/******************************************
*              
*     �������ƣ�SampleApp_ProcessEvent
*     �������ܣ�Ӧ�ò���������
*
*******************************************/
uint16 SampleApp_ProcessEvent( uint8 task_id, uint16 events )
{
    afIncomingMSGPacket_t *MSGpkt;
    if (events&SYS_EVENT_MSG)  
    {
       MSGpkt = ( afIncomingMSGPacket_t * )osal_msg_receive( SampleApp_TaskID ); //��ñ�����Ϣ�ĵ�ַ
      
            while ( MSGpkt )
            {
              switch ( MSGpkt->hdr.event )
              {        
                   case KEY_CHANGE://�����¼�
                        SampleApp_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
                   break;
                  /* case ZDO_STATE_CHANGE:
                       SampleApp_NwkState = ( devStates_t )(MSGpkt->hdr.status); 
                       if(SampleApp_NwkState ==DEV_ZB_COORD)
                       {
                         SampleApp_SendTheMessage(0xFFFF,1); //��ȡ�ն˽ڵ��ip
                         break;
                       }*/
                  case AF_INCOMING_MSG_CMD: 
                      SampleApp_MessageMSGCB( MSGpkt );
                       break;
                 default:
                       break;
              }

              // Release the memory 
              osal_msg_deallocate( (uint8 *)MSGpkt );

              // Next - if one is available 
              MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( task_id );
            }

            // return unprocessed events 
            return (events ^ SYS_EVENT_MSG);
      }
      if(events&GET_ENDDEVICE_IFO)
      {
        SampleApp_SendTheMessage(0xFFFF,1); //��ȡ�ն˽ڵ��ip
        return (events ^GET_ENDDEVICE_IFO);
      }
      
      return 0;
}

void SampleApp_MessageMSGCB( afIncomingMSGPacket_t *pkt )
{
  DeId *DeviceId;
  senor *sen_Data;
  switch( pkt->clusterId )
  {
     case SAMPLEAPP_IN_CLUSTERID :
           DeviceId=(DeId *)pkt->cmd.Data; //�����ݰ�ת��ΪDeId����
           NodeId_arr[DeviceId->Id]=DeviceId->NetworkId;
           osal_memcpy(&NodeId[NodeNum++],pkt->cmd.Data,3 );
          //HalLedBlink( HAL_LED_ALL, 0, 50, 200 );  
           HalLedSet ( HAL_LED_2,HAL_LED_MODE_TOGGLE);
           break;
       
     case  SAMPLEAPP_DATA_CLUSTERID : 
           
           sen_Data=(senor *)pkt->cmd.Data; //�����ݰ�ת��Ϊsenor����
           if(sen_Data->Id==3)
             sprintf(Uartbuf,"%d:%d:%d\n",sen_Data->Id,sen_Data->Data1,sen_Data->Data2);
           else
              sprintf(Uartbuf,"%d:%d\n",sen_Data->Id,sen_Data->Data1);
              HalUARTWrite(0,Uartbuf,strlen(Uartbuf));
           break;
          
  }
}

void SampleApp_SendTheMessage(uint16 ShortAdr,uint8 command )
{
  
  afAddrType_t  my_DstAddr;
  
  my_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
  if(command==10||command==1)
     my_DstAddr.addrMode=(afAddrMode_t)AddrBroadcast;  //13������ر����е�
  my_DstAddr.endPoint =  SAMPLEAPP_ENDPOINT;
  my_DstAddr.addr.shortAddr =ShortAdr;
  AF_DataRequest(&my_DstAddr, 
                 &SampleApp_epDesc,
                 SAMPLEAPP_OUT_CLUSTERID,
                 1,
                 (uint8 *)&command,
                 &SampleApp_TranskID,
                 AF_DISCV_ROUTE,
                 AF_DEFAULT_RADIUS );
  
   HalLedSet ( HAL_LED_1,HAL_LED_MODE_TOGGLE);
}

static void rxCB(uint8 port,uint8 event) //���ڽ��պ���
{ 
  //�����򴮿�д��̫�����ݣ�����ᷢ����ʧ
  uint8 i=0,j,choice=0;
  Send_Ur Send;
  HalUARTRead(0,Uartbuf,2);
  if(osal_memcmp(Uartbuf,"id",2))
  {
    /*HalUARTWrite(0,"A:\n",3);
     for(i=0;i<3;i++)
     {
       To_string(Send.id,(uint8*)&NodeId[i].Id,1);
       To_string(Send.NetworkId,(uint8*)&NodeId[i].NetworkId,2);
       HalUARTWrite(0,"id��",4);
       HalUARTWrite(0,Send.id,2);
       HalUARTWrite(0,"  ",2);
       HalUARTWrite(0,"NetworkId��",11);
       HalUARTWrite(0,Send.NetworkId,4);
       HalUARTWrite(0,"\n",1);
   
     }
     HalUARTWrite(0,"\n",1);
     HalUARTWrite(0,"B:\n",3);*/
     for(j=1;j<4;j++)
     {
       To_string(Send.id,(uint8*)&j,1);
       To_string(Send.NetworkId,(uint8*)&NodeId_arr[j],2);
       HalUARTWrite(0,"id��",4);
       HalUARTWrite(0,Send.id,2);
       HalUARTWrite(0,"  ",2);
       HalUARTWrite(0,"NetworkId��",11);
       HalUARTWrite(0,Send.NetworkId,4);
       HalUARTWrite(0,"\n",1);
   
     }
      HalUARTWrite(0,"\n",1);
    
   }
  else
  {
     choice=(Uartbuf[0]-48)*10;
     choice=choice+Uartbuf[1]-48;
     switch(choice)
     {
       
       case 10:SampleApp_SendTheMessage(0xFFFF,10);break; //���ģʽ,�ر����е�
       //����
       case 11:SampleApp_SendTheMessage(NodeId_arr[1],11);break; //��ͨģʽ
       case 12:SampleApp_SendTheMessage(NodeId_arr[1],12);break; //ӭ��ģʽ
       case 13:SampleApp_SendTheMessage(NodeId_arr[1],13);break; //ӰԺģʽ
       case 14:SampleApp_SendTheMessage(NodeId_arr[1],14);break; //�ۻ�ģʽ
       case 15:SampleApp_SendTheMessage(NodeId_arr[1],15);break; //�رտ������е�
       //����
       case 21:SampleApp_SendTheMessage(NodeId_arr[2],21);break; //��ͨģʽ
       case 22:SampleApp_SendTheMessage(NodeId_arr[2],22);break; //����ģʽ
       case 23:SampleApp_SendTheMessage(NodeId_arr[2],23);break; //�رճ������е�
       //����
       case 31:SampleApp_SendTheMessage(NodeId_arr[3],31);break; //����ģʽ
       case 32:SampleApp_SendTheMessage(NodeId_arr[3],32);break; //˯��ģʽ
       case 33:SampleApp_SendTheMessage(NodeId_arr[3],33);break; //��ģʽ
       
     }
  }
   osal_memset( Uartbuf, 0, 2); //�����������
}

/******************************************
*              
*     �������ƣ�SampleApp_HandleKeys
*     �������ܣ������¼�������
*
*******************************************/
void SampleApp_HandleKeys( uint8 shift, uint8 keys ) 
{
      (void)shift;  // Intentionally unreferenced parameter
      
      if ( keys & HAL_KEY_SW_6 ) //S1
      {
           P10_Normal_IO();
           HalLedSet ( HAL_LED_1,HAL_LED_MODE_TOGGLE);
             
      }
      
    
}

void To_string(uint8 *dest,char *src,uint8 lenght)
{
   uint8 *xad;
   uint8 i;
   uint8 ch;
   xad=src+lenght-1;
   for(i=0;i<lenght;i++,xad--)
   {
     ch=(*xad>>4)&0x0f;
     dest[i<<1]=ch+((ch<10) ? '0': '7');
     ch=*xad&0x0f;
     dest[(i<<1)+1]=ch+((ch<10)? '0' : '7');
   }
}
