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
#include "dht11.h"
#include "hal_adc.h"

#define EndDevice_ID 3
const cId_t SampleApp_IN_ClusterList[ SAMPLEAPP_MAX_CLUSTERS ] =
{
  SAMPLEAPP_OUT_CLUSTERID 
};

const cId_t SampleApp_OUT_ClusterList[ SAMPLEAPP_MAX_CLUSTERS ] =
{
  SAMPLEAPP_IN_CLUSTERID ,
  SAMPLEAPP_DATA_CLUSTERID
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
  SAMPLEAPP_MAX_CLUSTERS ,
  ( cId_t *)SampleApp_OUT_ClusterList,
  
  
};   //简单描述符

endPointDesc_t SampleApp_epDesc; //端点描述符，端点描述符包含简单描述符
byte SampleApp_TaskID;
byte SampleApp_TranskID;
devStates_t SampleApp_NwkState;
#if EndDevice_ID!=3
    uint16  adc_val=0; //adc值
#else
    uint8 r_val;  //判断是否采集到温湿度
    uint8 temp;
    uint8 humi;
#endif

void SampleApp_MessageMSGCB( afIncomingMSGPacket_t *pckt ); //消息处理函数
void SampleApp_SendTheMessage(uint8 out_clusterID,uint16 value1,uint16 value2);   //数据发送函数
//void SampleApp_SendTheMessage(void) ;//数据接收函数
void SampleApp_HandleKeys( uint8 shift, uint8 keys );  //按键处理
void Per_operation(uint8 command); //执行协调器发出的命令
void Obtain_tm_hum_val(uint16 *tm_val,uint16  *hum_val);

void SampleApp_Init( byte task_id )
{
  SampleApp_TaskID                  =task_id;
  SampleApp_NwkState                =DEV_INIT;//表示该节点没有连接到zigbee网络
  SampleApp_TranskID                =0;              //将数据包的序号初始化为0，每发送一个数据包，该序号自动加1
  SampleApp_epDesc.endPoint         =  SAMPLEAPP_ENDPOINT ;
  SampleApp_epDesc.task_id          =&SampleApp_TaskID;
  SampleApp_epDesc.simpleDesc       =
      ( SimpleDescriptionFormat_t * )&SampleApp_SimpleDesc;
  SampleApp_epDesc.latencyReq      =noLatencyReqs;
  afRegister( &SampleApp_epDesc );    //将端点描述符进行注册
  
  RegisterForKeys( task_id ); // 登记所有的按键事件
  
#if EndDevice_ID!=3
     HalAdcInit();
#endif
  
 osal_start_timerEx(task_id,SEND_SENOR_DATA,500); //定时采集传感器数据

}

UINT16 SampleApp_ProcessEvent( byte task_id, UINT16 events)
{
  afIncomingMSGPacket_t *MSGpkt;
  if (events&SYS_EVENT_MSG)  
  {
    MSGpkt = ( afIncomingMSGPacket_t * )osal_msg_receive( SampleApp_TaskID ); //获得保存消息的地址
    while ( MSGpkt )
    {
      switch( MSGpkt->hdr.event)
      {
         case KEY_CHANGE://按键事件
              SampleApp_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
         case ZDO_STATE_CHANGE:
              SampleApp_NwkState = ( devStates_t )(MSGpkt->hdr.status); 
              if(SampleApp_NwkState ==DEV_END_DEVICE)
              {
                //HalLedBlink(HAL_LED_2,0,50,500);
                  SampleApp_SendTheMessage(SAMPLEAPP_IN_CLUSTERID,0,0);
              }
         case AF_INCOMING_MSG_CMD: 
              SampleApp_MessageMSGCB( MSGpkt );
              break;
            
              break;
         default:
           break;
      }
   
      osal_msg_deallocate( (uint8 *)MSGpkt );   //释放保存消息的储存空间
      MSGpkt=(afIncomingMSGPacket_t *)osal_msg_receive( SampleApp_TaskID);//再从消息队列接收消息
    }
      
      return (events ^ SYS_EVENT_MSG); //返回未处理的事件
    }
    if(events&SEND_SENOR_DATA)
    {
#if EndDevice_ID!=3
       adc_val = HalAdcRead(HAL_ADC_CHANNEL_6  , HAL_ADC_RESOLUTION_12);
       SampleApp_SendTheMessage(SAMPLEAPP_DATA_CLUSTERID,adc_val,0);
         
#else
      r_val = dht11_value(&temp , &humi , DHT11_UINT8);     
      if(r_val == 0)
      {  
        SampleApp_SendTheMessage(SAMPLEAPP_DATA_CLUSTERID,temp,humi);
       }
#endif
       
       osal_start_timerEx(task_id,SEND_SENOR_DATA,500); //定时采集传感器数据
       return (events ^ SEND_SENOR_DATA); //返回未处理的事件
    
  }
  return 0;
}
void SampleApp_SendTheMessage(uint8 out_clusterID,uint16 value1,uint16 value2)
{
  
  afAddrType_t  my_DstAddr;
  senor sen_Data;
  sen_Data.Id= EndDevice_ID;  //设备型ID
  if(out_clusterID==SAMPLEAPP_IN_CLUSTERID)
    sen_Data.Data1=NLME_GetShortAddr();
  else if (EndDevice_ID==3)
  {
      sen_Data.Data1=value1;
      sen_Data.Data2=value2;
  }
  else
     sen_Data.Data1=value1;
   
 
    
  my_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
  my_DstAddr.endPoint =  SAMPLEAPP_ENDPOINT;
  my_DstAddr.addr.shortAddr =0x0000;
  AF_DataRequest(&my_DstAddr, 
                 &SampleApp_epDesc,
                 out_clusterID,
                 sizeof(senor),
                 (uint8 *)&sen_Data,
                 &SampleApp_TranskID,
                 AF_DISCV_ROUTE,
                 AF_DEFAULT_RADIUS );
  
 // HalLedBlink(HAL_LED_1,0,50,500);
}
void SampleApp_MessageMSGCB( afIncomingMSGPacket_t *pkt )
{
  switch( pkt->clusterId )
  {
     case SAMPLEAPP_OUT_CLUSTERID :
          Per_operation(*((uint8 *)pkt->cmd.Data)); //
          
         //HalLedSet ( HAL_LED_2,HAL_LED_MODE_TOGGLE);
          break;
  }
}

void SampleApp_HandleKeys( uint8 shift, uint8 keys ) 
{
      (void)shift;  // Intentionally unreferenced parameter
      
      if ( keys & HAL_KEY_SW_6 ) //S1
      {
           P10_Normal_IO();
          // HalLedSet ( HAL_LED_1,HAL_LED_MODE_ON);
           HalLedSet ( HAL_LED_1,HAL_LED_MODE_TOGGLE);
             
      }
      
    
}
void Per_operation(uint8 command)
{
   P10_Normal_IO(); //设置p1.0为通用IO
   P11_Normal_IO(); //设置p1.0为通用IO
   HalLedSet ( HAL_LED_ALL,HAL_LED_MODE_ON);
   HalLedSet ( HAL_LED_ALL,HAL_LED_MODE_TOGGLE); 
   switch(command)
   {
      case 1  :  SampleApp_SendTheMessage(SAMPLEAPP_IN_CLUSTERID,0,0);
                 break;
      case 10 :  HalLedSet ( HAL_LED_ALL,HAL_LED_MODE_ON);
                 HalLedSet ( HAL_LED_ALL,HAL_LED_MODE_TOGGLE); 
                 break;  //离家模式
#if EndDevice_ID==1
     
      case 11 :  HalLedBlink( HAL_LED_ALL, 0, 10, 15 );
                 break; //普通模式
      case 12 :  HalLedSet ( HAL_LED_ALL,HAL_LED_MODE_ON);
                 break; //迎宾模式
    
      case 13 :  HalLedSet ( HAL_LED_2,HAL_LED_MODE_OFF);;   //影院模式
                 Use_LedPwm(0,0);
                 break; //关闭大灯，小灯慢慢变暗
      case 14 :  Use_LedPwm(0,1);
                 P10_Normal_IO(); //设置p1.0为通用IO
                 //DelayMs(20000);
                 HalLedBlink( HAL_LED_2, 0, 50, 300 );
                 HalLedBlink( HAL_LED_1, 0, 50, 200 );
                 HalLedBlink( HAL_LED_3, 0, 50, 400 );
                 break;
     case 15 :   HalLedSet ( HAL_LED_ALL,HAL_LED_MODE_ON);
                 HalLedSet ( HAL_LED_ALL,HAL_LED_MODE_TOGGLE); 
                 break;  //关闭客厅灯
#elif EndDevice_ID==2
     
      case 21 :  HalLedBlink( HAL_LED_ALL, 0, 10, 15 );
                 break; //普通模式
      case 22 :  HalLedSet ( HAL_LED_ALL,HAL_LED_MODE_ON);
                 break; //迎宾模式
      case 23 :  HalLedSet ( HAL_LED_ALL,HAL_LED_MODE_ON);
                 HalLedSet ( HAL_LED_ALL,HAL_LED_MODE_TOGGLE); 
                 break; //关闭厨房灯
#elif EndDevice_ID==3
      case 31 : HalLedSet ( HAL_LED_1,HAL_LED_MODE_ON);
                break; //普通模式
      case 32 :  Use_LedPwm(2,0);
                 P10_Normal_IO(); //设置p1.0为通用IO
                 //P11_Normal_IO(); //设置p1.0为通用IO
                 HalLedSet ( HAL_LED_1,HAL_LED_MODE_OFF);
                 break; //睡眠模式
      case 33 :  HalLedSet ( HAL_LED_ALL,HAL_LED_MODE_ON);
                 HalLedSet ( HAL_LED_ALL,HAL_LED_MODE_TOGGLE); 
                 break;  //关闭卧室灯
#endif
   }
}