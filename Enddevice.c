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
//GENERICAPP_MAX_CLUSTERS��Coordinator.h�ļ��ж���ĺ�
{
    GENERICAPP_CLUSTERID
};

//�������ݽṹ������������һ��ZigBee�豸�ڵ�
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

//�����ĸ�����
endPointDesc_t GenericApp_epDesc;  //�ڵ�������
byte GenericApp_TaskID;   //�������ȼ�
byte GenericApp_TransID;  //���ݷ������к�
devStates_t GenericApp_NwkState;  //����ڵ�״̬

//��������
void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );  //��Ϣ������
void GenericApp_SendTheMessage( void );  //���ݷ��ͺ���

//����Ϊ�����ʼ������
void GenericApp_Init( byte task_id )
{
    GenericApp_TaskID = task_id;  //��ʼ���������ȼ�
    GenericApp_NwkState = DEV_INIT;  //���豸״̬��ʼ��ΪDEV_INIT,��ʾ�ýڵ�û�����ӵ�ZigBee����
    GenericApp_TransID = 0;  //�����ݰ�����ų�ʼ��Ϊ0,
    GenericApp_epDesc.endPoint = GENERICAPP_ENDPOINT;
    GenericApp_epDesc.task_id = &GenericApp_TaskID;
    GenericApp_epDesc.simpleDesc
        = (SimpleDescriptionFormat_t *)&GenericApp_SimpleDesc;
    GenericApp_epDesc.latencyReq = noLatencyReqs;  //�Խڵ����������г�ʼ��
    afRegister( &GenericApp_epDesc );  //ʹ��afRegister�������ڵ�����������ע��
}


//��Ϣ����������ɶԽ������ݵĴ���
UINT16 GenericApp_ProcessEvent( byte task_id, UINT16 events )
{
    afIncomingMSGPacket_t *MSGpkt;  //����һ��ָ�������Ϣ�ṹ���ָ��MSGpkt
    if ( events & SYS_EVENT_MSG )
    {
        MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( GenericApp_TaskID );  //ʹ��osal_msg_receive��������Ϣ�����Ͻ�����Ϣ
        while ( MSGpkt )
        {
            switch ( MSGpkt->hdr.event )
            {
            case ZDO_STATE_CHANGE:   //�Խ�����Ϣ�����жϣ������յ��������ݣ���������к��������ݽ��д���
                GenericApp_NwkState = (devStates_t)(MSGpkt->hdr.status);  //��ȡ�ڵ���豸����
                if (GenericApp_NwkState == DEV_END_DEVICE)  //�Խڵ��豸���ͽ����жϣ�������ն˽ڵ㣬��ִ����һ�д��룬ʵ���������ݷ���
                {
                    GenericApp_SendTheMessage() ;
                }
                break;
                
            default:
                break;
            }
            osal_msg_deallocate( (uint8 *)MSGpkt );  //�ͷ���Ϣ��ռ�ݵĴ洢�ռ�
            MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( GenericApp_TaskID );  //��������Ϣ������Ӧ����ֱ���������
        }
        
        return (events ^ SYS_EVENT_MSG);
    }
    return 0;
    
}

//ʵ�����ݷ���
void GenericApp_SendTheMessage( void )
{
    unsigned char theMessageData[4] = "LED";  //�������飬ʵ�����ݷ���
    afAddrType_t my_DstAddr;  //�������
    my_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;//�����͵�ַģʽ����Ϊ������Addr16Bit��ʾ������
    my_DstAddr.endPoint = GENERICAPP_ENDPOINT;  // ��ʼ���˿ں�
    my_DstAddr.addr.shortAddr = 0x0000;  //0x0000ΪЭ���������ַ����ZigBee�������ǹ̶���
    AF_DataRequest( &my_DstAddr, &GenericApp_epDesc,  //�������ݷ��ͺ���AF_DataRequest�����������ݵķ���
                   GENERICAPP_CLUSTERID,
                   3,
                   theMessageData,
                   &GenericApp_TransID,
                   AF_DISCV_ROUTE,
                   AF_DEFAULT_RADIUS );
    HalLedBlink(HAL_LED_2,0,50,500) ;  //���ú�����ʹ�ն˽ڵ��LED2��˸
}
