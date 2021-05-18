#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"
#include <string.h>
#include "Coordinator.h"
#include "DebugTrace.h"

#if !defined( WIN32 )
#include "OnBoard.h"
#endif

/* HAL */
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_uart.h"

const cId_t GenericApp_ClusterList[GENERICAPP_MAX_CLUSTERS] =  
//GENERICAPP_MAX_CLUSTERS是在Coordinator.h文件中定义的宏，为了跟协议栈里面的数据的定义格式保持一致，以下代码中的常量都是以宏定义的形式实现的
{
    GENERICAPP_CLUSTERID
};

const SimpleDescriptionFormat_t GenericApp_SimpleDesc =
{
    GENERICAPP_ENDPOINT,              //  int Endpoint;
    GENERICAPP_PROFID,                //  uint16 AppProfId[2];
    GENERICAPP_DEVICEID,              //  uint16 AppDeviceId[2];
    GENERICAPP_DEVICE_VERSION,        //  int   AppDevVer:4;
    GENERICAPP_FLAGS,                 //  int   AppFlags:4;
    GENERICAPP_MAX_CLUSTERS,          //  byte  AppNumInClusters;
    (cId_t *)GenericApp_ClusterList,  //  byte *pAppInClusterList;
    0,         
    (cId_t *)NULL   
};
//此数据结构用于描述一个ZigBee设备节点（简单设备描述符）

//定义三个变量
endPointDesc_t GenericApp_epDesc;  //节点描述符
byte GenericApp_TaskID;   //任务优先级
byte GenericApp_TransID;   //数据发送序列号

//函数声明
void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );  //消息处理函数
void GenericApp_SendTheMessage( void );  //数据发送函数

//任务初始化函数
void GenericApp_Init( byte task_id )
{
    GenericApp_TaskID = task_id;  //初始化任务优先级（任务优先级有协议栈的操作系统OSAL分配）
    GenericApp_TransID = 0;  //将发送数据包的序号初始化为0
    GenericApp_epDesc.endPoint = GENERICAPP_ENDPOINT;
    GenericApp_epDesc.task_id = &GenericApp_TaskID;
    GenericApp_epDesc.simpleDesc
        = (SimpleDescriptionFormat_t *)&GenericApp_SimpleDesc;
    GenericApp_epDesc.latencyReq = noLatencyReqs;    //对节点描述符进行的初始化
    afRegister( &GenericApp_epDesc );  //使用afRegister函数将节点描述符进行注册
    
}

//消息处理函数，完成对接收数据的处理
UINT16 GenericApp_ProcessEvent( byte task_id, UINT16 events )
{
    afIncomingMSGPacket_t *MSGpkt;  //定义一个指向接收消息结构体的指针MSGpkt
    if ( events & SYS_EVENT_MSG )
    {
        MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( GenericApp_TaskID );  //使用osal_msg_receive函数从消息队列上接收消息
        while ( MSGpkt )
        {
            switch ( MSGpkt->hdr.event )
            {
            case AF_INCOMING_MSG_CMD:   //对接收消息进行判断，若接收到无限数据，则调用下行函数对数据进行处理
                GenericApp_MessageMSGCB ( MSGpkt );
                break;
                
            default:
                break;
            }
            osal_msg_deallocate( (uint8 *)MSGpkt );  //释放消息所占据的存储空间
            MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( GenericApp_TaskID );  //对所有消息进行相应处理，直至处理完毕
        }
        
        return (events ^ SYS_EVENT_MSG);
    }
    return 0;
    
}

void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pkt )
{
    unsigned char buffer[4] = "   ";
    switch ( pkt->clusterId )
    {
    case GENERICAPP_CLUSTERID:
        osal_memcpy(buffer,pkt->cmd.Data,3);  //将收到的数据拷贝到缓冲区buffer中
        if((buffer[0] == 'L') || (buffer[1] =='E') || (buffer[2] == 'D'))  //判断接收的数据是否为LED
        {
            HalLedBlink(HAL_LED_2,0,50,500) ;  //LED2闪烁
        }
        else
        {
            HalLedSet(HAL_LED_2,HAL_LED_MODE_ON);   //点亮LED2
        }
        break;
    }
}