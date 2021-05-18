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
//GENERICAPP_MAX_CLUSTERS在Coordinator.h文件中定义的宏
{
    GENERICAPP_CLUSTERID
};

//以下数据结构可以用来描述一个ZigBee设备节点
const SimpleDescriptionFormat_t GenericApp_SimpleDesc =
{
    GENERICAPP_ENDPOINT,              //  int Endpoint;
    GENERICAPP_PROFID,                //  uint16 AppProfId[2];
    GENERICAPP_DEVICEID,              //  uint16 AppDeviceId[2];
    GENERICAPP_DEVICE_VERSION,        //  int   AppDevVer:4;
    GENERICAPP_FLAGS,                 //  int   AppFlags:4;
    0,
    (cId_t *)NULL,
    GENERICAPP_MAX_CLUSTERS,          //  byte  AppNumInClusters;
    (cId_t *)GenericApp_ClusterList  //  byte *pAppInClusterList;
};

//定义四个变量
endPointDesc_t GenericApp_epDesc;  //节点描述符
byte GenericApp_TaskID;   //任务优先级
byte GenericApp_TransID;  //数据发送序列号
devStates_t GenericApp_NwkState;  //保存节点状态

//函数声明
void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );  //消息处理函数
void GenericApp_SendTheMessage( void );  //数据发送函数

//以下为任务初始化函数
void GenericApp_Init( byte task_id )
{
    GenericApp_TaskID = task_id;  //初始化任务优先级
    GenericApp_NwkState = DEV_INIT;  //将设备状态初始化为DEV_INIT,表示该节点没有连接到ZigBee网络
    GenericApp_TransID = 0;  //将数据包的序号初始化为0,
    GenericApp_epDesc.endPoint = GENERICAPP_ENDPOINT;
    GenericApp_epDesc.task_id = &GenericApp_TaskID;
    GenericApp_epDesc.simpleDesc
        = (SimpleDescriptionFormat_t *)&GenericApp_SimpleDesc;
    GenericApp_epDesc.latencyReq = noLatencyReqs;  //对节点描述符进行初始化
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
            case ZDO_STATE_CHANGE:   //对接收消息进行判断，若接收到无限数据，则调用下行函数对数据进行处理
                GenericApp_NwkState = (devStates_t)(MSGpkt->hdr.status);  //读取节点的设备类型
                if (GenericApp_NwkState == DEV_END_DEVICE)  //对节点设备类型进行判断，如果是终端节点，则执行下一行代码，实现无线数据发送
                {
                    GenericApp_SendTheMessage() ;
                }
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

//实现数据发送
void GenericApp_SendTheMessage( void )
{
    unsigned char theMessageData[4] = "LED";  //定义数组，实现数据发送
    afAddrType_t my_DstAddr;  //定义变量
    my_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;//将发送地址模式设置为单播（Addr16Bit表示单播）
    my_DstAddr.endPoint = GENERICAPP_ENDPOINT;  // 初始化端口号
    my_DstAddr.addr.shortAddr = 0x0000;  //0x0000为协调器网络地址，在ZigBee网络中是固定的
    AF_DataRequest( &my_DstAddr, &GenericApp_epDesc,  //调用数据发送函数AF_DataRequest进行无线数据的发送
                   GENERICAPP_CLUSTERID,
                   3,
                   theMessageData,
                   &GenericApp_TransID,
                   AF_DISCV_ROUTE,
                   AF_DEFAULT_RADIUS );
    HalLedBlink(HAL_LED_2,0,50,500) ;  //调用函数，使终端节点的LED2闪烁
}
