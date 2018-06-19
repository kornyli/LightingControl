/******************************************
*              
*           程序修改者： jafy
*           修改原因：   方便学习
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
};   //简单描述符

endPointDesc_t SampleApp_epDesc; //端点描述符，端点描述符包含简单描述符
byte SampleApp_TaskID;  //任务ID
devStates_t SampleApp_NwkState; //设备网络状态
byte SampleApp_TranskID;  //发送序列号
unsigned char Uartbuf[20];  //串口接收上位机命令
DeId NodeId[3];  //存储终端节点的ID和ip
uint8 NodeNum=0;
uint16 NodeId_arr[4];



void SampleApp_MessageMSGCB( afIncomingMSGPacket_t *pckt ); //消息处理函数
//void SampleApp_SendTheMessage( void );   //数据发送函数
void SampleApp_SendTheMessage(uint16 ShortAdr,uint8 command ); //数据发送函数
//按键事件处理函数声明
void SampleApp_HandleKeys( uint8 shift, uint8 keys );
static void rxCB(uint8 port,uint8 event); //串口接收处理函数
void To_string(uint8 *dest ,char *src,uint8 lenght);


/******************************************
*              
*     函数名称：SampleApp_Init
*     函数功能：应用层初始化
*
*******************************************/
void SampleApp_Init( uint8 task_id )
{ 
    halUARTCfg_t uartconfig;
    SampleApp_TaskID                  =task_id;
    SampleApp_TranskID                =0;              //将数据包的序号初始化为0，每发送一个数据包，该序号自动加1
    SampleApp_epDesc.endPoint         = SAMPLEAPP_ENDPOINT;
    SampleApp_epDesc.task_id          =& SampleApp_TaskID;
    SampleApp_epDesc.simpleDesc       =
          ( SimpleDescriptionFormat_t * )&SampleApp_SimpleDesc;
    SampleApp_epDesc.latencyReq      =noLatencyReqs;
    afRegister( &SampleApp_epDesc );    //将端点描述符进行注册
 
    RegisterForKeys( task_id ); // 登记所有的按键事件
    //HalLedSet ( HAL_LED_ALL,HAL_LED_MODE_TOGGLE);
    
     //串口初始化
  uartconfig.configured             =TRUE;
  uartconfig.baudRate               =HAL_UART_BR_115200;
  uartconfig.flowControl            =FALSE;
  uartconfig.callBackFunc           =rxCB;
  HalUARTOpen (0,&uartconfig);
  osal_set_event(task_id,GET_ENDDEVICE_IFO);
}


/******************************************
*              
*     函数名称：SampleApp_ProcessEvent
*     函数功能：应用层任务处理函数
*
*******************************************/
uint16 SampleApp_ProcessEvent( uint8 task_id, uint16 events )
{
    afIncomingMSGPacket_t *MSGpkt;
    if (events&SYS_EVENT_MSG)  
    {
       MSGpkt = ( afIncomingMSGPacket_t * )osal_msg_receive( SampleApp_TaskID ); //获得保存消息的地址
      
            while ( MSGpkt )
            {
              switch ( MSGpkt->hdr.event )
              {        
                   case KEY_CHANGE://按键事件
                        SampleApp_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
                   break;
                  /* case ZDO_STATE_CHANGE:
                       SampleApp_NwkState = ( devStates_t )(MSGpkt->hdr.status); 
                       if(SampleApp_NwkState ==DEV_ZB_COORD)
                       {
                         SampleApp_SendTheMessage(0xFFFF,1); //获取终端节点的ip
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
        SampleApp_SendTheMessage(0xFFFF,1); //获取终端节点的ip
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
           DeviceId=(DeId *)pkt->cmd.Data; //将数据包转换为DeId类型
           NodeId_arr[DeviceId->Id]=DeviceId->NetworkId;
           osal_memcpy(&NodeId[NodeNum++],pkt->cmd.Data,3 );
          //HalLedBlink( HAL_LED_ALL, 0, 50, 200 );  
           HalLedSet ( HAL_LED_2,HAL_LED_MODE_TOGGLE);
           break;
       
     case  SAMPLEAPP_DATA_CLUSTERID : 
           
           sen_Data=(senor *)pkt->cmd.Data; //将数据包转换为senor类型
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
     my_DstAddr.addrMode=(afAddrMode_t)AddrBroadcast;  //13号命令关闭所有灯
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

static void rxCB(uint8 port,uint8 event) //串口接收函数
{ 
  //不能向串口写入太多数据，否则会发生丢失
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
       HalUARTWrite(0,"id：",4);
       HalUARTWrite(0,Send.id,2);
       HalUARTWrite(0,"  ",2);
       HalUARTWrite(0,"NetworkId：",11);
       HalUARTWrite(0,Send.NetworkId,4);
       HalUARTWrite(0,"\n",1);
   
     }
     HalUARTWrite(0,"\n",1);
     HalUARTWrite(0,"B:\n",3);*/
     for(j=1;j<4;j++)
     {
       To_string(Send.id,(uint8*)&j,1);
       To_string(Send.NetworkId,(uint8*)&NodeId_arr[j],2);
       HalUARTWrite(0,"id：",4);
       HalUARTWrite(0,Send.id,2);
       HalUARTWrite(0,"  ",2);
       HalUARTWrite(0,"NetworkId：",11);
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
       
       case 10:SampleApp_SendTheMessage(0xFFFF,10);break; //离家模式,关闭所有灯
       //客厅
       case 11:SampleApp_SendTheMessage(NodeId_arr[1],11);break; //普通模式
       case 12:SampleApp_SendTheMessage(NodeId_arr[1],12);break; //迎宾模式
       case 13:SampleApp_SendTheMessage(NodeId_arr[1],13);break; //影院模式
       case 14:SampleApp_SendTheMessage(NodeId_arr[1],14);break; //聚会模式
       case 15:SampleApp_SendTheMessage(NodeId_arr[1],15);break; //关闭客厅所有灯
       //厨房
       case 21:SampleApp_SendTheMessage(NodeId_arr[2],21);break; //普通模式
       case 22:SampleApp_SendTheMessage(NodeId_arr[2],22);break; //宾客模式
       case 23:SampleApp_SendTheMessage(NodeId_arr[2],23);break; //关闭厨房所有灯
       //卧室
       case 31:SampleApp_SendTheMessage(NodeId_arr[3],31);break; //正常模式
       case 32:SampleApp_SendTheMessage(NodeId_arr[3],32);break; //睡觉模式
       case 33:SampleApp_SendTheMessage(NodeId_arr[3],33);break; //起床模式
       
     }
  }
   osal_memset( Uartbuf, 0, 2); //清除接收数据
}

/******************************************
*              
*     函数名称：SampleApp_HandleKeys
*     函数功能：按键事件处理函数
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
