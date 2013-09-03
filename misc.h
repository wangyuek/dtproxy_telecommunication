/////////////////////////////////
//
//misc.h
//
///////////////////////////////
#ifndef _MISC_H_
#define _MISC_H_

#include "alertapi.h"

#define CHECK_SHAKEHAND			5	//˵�������Ѿ��жϵ����δ��⵽�����ִ���

#define LOCAL_PORT				22
#define CCS_PORT				20
#define IVR_PORT				4
#define CONFIG_PORT				6
#define ALARM_PORT				24

#define MAXNAMELEN				255
#define MAX_DESC_LEN			255
#define INIT_OPR_THREAD_NUM		100		//��ʼ���Ĵ����߳���
#define MAX_OPR_THREAD_NUM		101		//���Ĵ����߳���

#define DEFAULT_OVERTIME		3000
#define DEFAULT_CMD_SEP			";"
#define DEFAULT_PARA_SEP		"~"
#define DEFAULT_DATE_FMT		"%4d%2d%2d"
#define DEFAULT_TIME_FMT		"%2d%2d%2d"

#define MAX_PORT_USEDIN_ICD		50		//ICD��ʹ�õ����˿ں�
#define MAX_LINE_IN_EDIT		1000
#define NAMELEN					50		//����洢������ʹ�����Ƶ���󳤶�
#define MSG_PARA_LEN			10240
#define MAX_LINE_IN_LISTCTRL	MAX_LINE_IN_EDIT

const CString strConfigName = "����";
const CString strOprThreadName = "ICD����";
const CString strSocketOprThreadName = "Socket����";
const CString strReceiverName = "ICD����";
const CString strSenderName = "ICD����";
const CString strSocketReceiverName = "Socket����";
const CString strSocketSenderName = "Socket����";
const CString strRecvManageName = "ICD���չ���";
const CString strSendManageName = "ICD���͹���";
const CString strSocketRecvManageName = "Socket���չ���";
const CString strSocketSendManageName = "Socket���͹���";
const CString strStatisticName = "ͳ��";
const CString strAlarmName = "�澯";
const CString strCenterName = "Э�����";

//**********************************************************
//** Message Type 
//**********************************************************

//������Ϣ
#define MSG_CCS_SHAKEHAND			650		//������CCS��
#define MSG_IVR_SHAKEHAND			93		//IVR�����
#define MSG_ALARM_SHAKEHAND			1061	//������澯̨�������Ե���AlertApi��ShakeHand()������

//����������Ϣ
#define MSG_ICD_RELOADCONFIG		1498	//����̨�����

//����CCS��Ϣ
#define MSG_PROXY_CONNECT_CCS		651		//������CCS��
#define MSG_PROXY_CONNECT_CCS_ACK	652		//CCS�����

//��IVR�ϱ�����Դ��Ϣ
#define MSG_IVR_REPORT_DATASRC		113		//IVR�����
#define MSG_IVR_REPORT_DATASRC_ACK	114		//������IVR��

//���ô洢������Ϣ
#define MSG_SCP_PROXY_PROC			88		//IVR�����
#define MSG_PROXY_SCP_PROC_ACK		89		//������IVR��
//**************************************************************
//** end of Message Type
//**************************************************************

//define some type
typedef unsigned char		UC;
typedef unsigned short		US;
typedef unsigned long		UL;
//end of define 

const   long MAXTIMERS				  = 1024;

typedef struct
{
	USHORT		usSend;			
	USHORT		usReceive;		
	USHORT		ucMsgType;	//	��Ϣ����	enum ICDCCS_MSGTYPE
	UCHAR       ucMid;
}MESSAGE_HEAD;

typedef struct 
{
		char  strComputerName[MAXNAMELEN];
		char  strAppName[MAXNAMELEN];
		char  strServiceName[MAXNAMELEN];
		char  nAlarmLevel;
		ULONG lAlertIP;
		ULONG lAlertPort;
		char  nMod;
		char  nPos;
}REGISTINFO;

typedef struct 
{
		char  cSourceName[MAXNAMELEN];	//�澯���߳�����ģ����,Ӧ�ó�������
		ULONG lAlertIP  ;
		ULONG lAlertPort;
		ALERT_DEVICE  cAlertDevice;		//�澯��λ�����������ȱʡ�澯�豸��
										//������澯�豸������Ӧ�ĵ�λ�����Ӧ
										//......ƽ̨��
										//0��CCS
										//1��ALERT
										//2���Ŷӻ�
										//3��IVR
										//4��PROXY
										//5��FEP
										//......ҵ����
										//6: 
										//7:
										//8:
										//9:
										//10:
										//11:
		char  AlertLevel;				//�澯����
		ULONG lAlertTime;				//�澯ʱ��							
										//0����ʾ�����ڸ澯
										//�������Ժ���Ϊ��λ

		ULONG lAlertEvent;				//�澯�¼�
		LONG lAlertFlow;				//�澯��Ϣ�����
		WORD  wCataory;					//�¼����
		char  nMod;						//ģ���
		char  strDesc[MAX_DESC_LEN];	//����
		long  lRawLen;					//����
		char  strRawData[1];			//�澯��������Ϣ���ڳ�������й����У������ķǷ�������
										//Ӧ�ð���ʵ�ʵĳ̶������䣬�Լ������縺��
		DWORD dwStay;    				//�����ֶ�
}ALERTSTRUCT;

struct UnregistMsg{
		MESSAGE_HEAD  msg_head	;	//��Ϣͷ
		DWORD		  dwSrcIP;		//�澯Դ��IP��ַ
		DWORD         dwPort;		//�澯Դ�Ķ˿ں�
};

struct RegistMsg{
	MESSAGE_HEAD  msg_head	;		//��Ϣͷ
	REGISTINFO RegistInfo	;		//�Ǽ�����
};

struct AlertMsg{
	MESSAGE_HEAD  msg_head	;		//��Ϣͷ
	ALERTSTRUCT AlertInfo	;		//�澯��Ϣ
};

struct StopAlertMsg{
	MESSAGE_HEAD  msg_head	;		//��Ϣͷ
	ULONG lAlertIP  ;				//�澯ԴIP��ַ
	ULONG lAlertPort;				//�澯Դ�˿ں�
	LONG lAlertFlow;                //�澯��Ϣ�����
	DWORD dwStay;                   //�����ֶ�
};

struct  ShakeHand
{
	MESSAGE_HEAD head;	//��Ϣͷ
};

struct  ReloadConfig
{
	MESSAGE_HEAD	head;	//��Ϣͷ
	UC		ucTaskHandle;	//����̨����������,�����ַ�����ͬģ�������
	US		wTaskDsn;		//����δ��
};

typedef enum _DATA_TYPE
{
    D_NULL          = 0,
    D_CHAR          = 1,
    D_CHAR_END      = 199,
    D_DOUBLE        = 200,
    D_INT_1         = 201,
    D_INT_2              ,
    D_INT_3              ,
    D_INT_4              ,
    D_BCD           = 205,
    D_BCD_END       = 250,
    D_DATE          = 253,  
    D_TIME          = 254,  
    D_DATETIME      = 255,  
}MACRO_DATATYPE;

/*
struct	DATE_TYPE
{
	WORD	year;		//a
	WORD	month;		//b
	WORD	day;		//c
	WORD    Reserved[5];
};
*/

/*
struct	TIME_TYPE
{
	WORD	hour;		//d
	WORD	minute;		//e
	WORD	second;		//f
	WORD    Reserved[5];
};
*/

struct	DATE_TIMETYPE
{
	WORD	year;		//a
	WORD	month;		//b
	WORD	day;		//c
	WORD	hour;		//d
	WORD	minute;		//e
	WORD	second;		//f
	DWORD  fraction;	//g
};

typedef struct
{
	UC sender_node;
	UC sender_port;

	UC receiver_node;
	UC receiver_port;

    US usMsgType;
	UC ucMid;
    UC ucTaskHandle;
    US usTaskDsn;

}IVR_MESSAGE_HEAD;

struct ReportDataSrcAck		//��IVR��õ���Ϣ��Ϊ��׼����Ϣͷ�ṹMESSAGE_HEAD������Ϊ���ṹ
{
	IVR_MESSAGE_HEAD head;
    UC para[1];			//�ϱ�������Դ���ƴ�����"!"�ָ�
};

struct ProxySqlProcPara
{
	UC dataType;	//�ֶ���������
					//��DATATYPE�ж���һ��
	UC paraType;	//2:����������
					//1:�������
					//0���������������
	US dataLen;		//�ֶ����ݳ���
};

struct  ProxySqlProcWithTimeOutMsg
{
	IVR_MESSAGE_HEAD	head;		//��Ϣͷ
	char	dataSource[NAMELEN];	//����Դ��
	UL		ulTimeOut;				//ִ�д洢���̵ĳ�ʱֵ�Ժ���Ϊ��λ
	char	quanlifier[NAMELEN-sizeof(UL)];//�û���
	UC		paraNum;				//��������
	char	procName[NAMELEN];		//�洢������
	ProxySqlProcPara	para;		//�洢���̵Ĳ���
	char	dataArea;				//�洢���̵�������
};

struct ProxySqlProcAckMsg
{
	IVR_MESSAGE_HEAD	head;		//��Ϣͷ
	US		status; 				//�洢����ִ��״̬1��ִ��ʧ��1, 0��ִ�гɹ�
									//��ֵ������ʾ���ô洢���̳ɹ���ʧ��
									//������ʾ�û��洢����ִ�н����ȷ
	char		dataSource[NAMELEN];//����Դ��
	char		quanlifier[NAMELEN];//�û���
	char		paraNum;			//��������
	char		procName[NAMELEN];	//�洢������
	ProxySqlProcPara	para;		//�洢���̵Ĳ���
	char		dataArea;			//�洢���̵�������
};

//�м��ָ��Э���
//
//end of �м��ָ��Э���


#endif