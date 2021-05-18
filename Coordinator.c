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
//GENERICAPP_MAX_CLUSTERS����Coordinator.h�ļ��ж���ĺ꣬Ϊ�˸�Э��ջ��������ݵĶ����ʽ����һ�£����´����еĳ��������Ժ궨�����ʽʵ�ֵ�
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
//�����ݽṹ��������һ��ZigBee�豸�ڵ㣨���豸��������

//������������
endPointDesc_t GenericApp_epDesc;  //�ڵ�������
byte GenericApp_TaskID;   //�������ȼ�
byte GenericApp_TransID;   //���ݷ������к�

//��������
void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );  //��Ϣ������
void GenericApp_SendTheMessage( void );  //���ݷ��ͺ���

//�����ʼ������
void GenericApp_Init( byte task_id )
{
    GenericApp_TaskID = task_id;  //��ʼ���������ȼ����������ȼ���Э��ջ�Ĳ���ϵͳOSAL���䣩
    GenericApp_TransID = 0;  //���������ݰ�����ų�ʼ��Ϊ0
    GenericApp_epDesc.endPoint = GENERICAPP_ENDPOINT;
    GenericApp_epDesc.task_id = &GenericApp_TaskID;
    GenericApp_epDesc.simpleDesc
        = (SimpleDescriptionFormat_t *)&GenericApp_SimpleDesc;
    GenericApp_epDesc.latencyReq = noLatencyReqs;    //�Խڵ����������еĳ�ʼ��
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
            case AF_INCOMING_MSG_CMD:   //�Խ�����Ϣ�����жϣ������յ��������ݣ���������к��������ݽ��д���
                GenericApp_MessageMSGCB ( MSGpkt );
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

void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pkt )
{
    unsigned char buffer[4] = "   ";
    switch ( pkt->clusterId )
    {
    case GENERICAPP_CLUSTERID:
        osal_memcpy(buffer,pkt->cmd.Data,3);  //���յ������ݿ�����������buffer��
        if((buffer[0] == 'L') || (buffer[1] =='E') || (buffer[2] == 'D'))  //�жϽ��յ������Ƿ�ΪLED
        {
            HalLedBlink(HAL_LED_2,0,50,500) ;  //LED2��˸
        }
        else
        {
            HalLedSet(HAL_LED_2,HAL_LED_MODE_ON);   //����LED2
        }
        break;
    }
}