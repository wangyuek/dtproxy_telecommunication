/////////////////////////////////
//
//misc.h
//
///////////////////////////////
#ifndef _MISC_H_
#define _MISC_H_

#include "alertapi.h"

#define CHECK_SHAKEHAND			5	//说明握手已经中断的最大未检测到的握手次数

#define LOCAL_PORT				22
#define CCS_PORT				20
#define IVR_PORT				4
#define CONFIG_PORT				6
#define ALARM_PORT				24

#define MAXNAMELEN				255
#define MAX_DESC_LEN			255
#define INIT_OPR_THREAD_NUM		100		//初始化的处理线程数
#define MAX_OPR_THREAD_NUM		101		//最大的处理线程数

#define DEFAULT_OVERTIME		3000
#define DEFAULT_CMD_SEP			";"
#define DEFAULT_PARA_SEP		"~"
#define DEFAULT_DATE_FMT		"%4d%2d%2d"
#define DEFAULT_TIME_FMT		"%2d%2d%2d"

#define MAX_PORT_USEDIN_ICD		50		//ICD中使用的最大端口号
#define MAX_LINE_IN_EDIT		1000
#define NAMELEN					50		//定义存储过程中使用名称的最大长度
#define MSG_PARA_LEN			10240
#define MAX_LINE_IN_LISTCTRL	MAX_LINE_IN_EDIT

const CString strConfigName = "配置";
const CString strOprThreadName = "ICD处理";
const CString strSocketOprThreadName = "Socket处理";
const CString strReceiverName = "ICD接收";
const CString strSenderName = "ICD发送";
const CString strSocketReceiverName = "Socket接收";
const CString strSocketSenderName = "Socket发送";
const CString strRecvManageName = "ICD接收管理";
const CString strSendManageName = "ICD发送管理";
const CString strSocketRecvManageName = "Socket接收管理";
const CString strSocketSendManageName = "Socket发送管理";
const CString strStatisticName = "统计";
const CString strAlarmName = "告警";
const CString strCenterName = "协议代理";

//**********************************************************
//** Message Type 
//**********************************************************

//握手消息
#define MSG_CCS_SHAKEHAND			650		//代理向CCS发
#define MSG_IVR_SHAKEHAND			93		//IVR向代理发
#define MSG_ALARM_SHAKEHAND			1061	//代理向告警台发，可以调用AlertApi的ShakeHand()来发送

//加载配置消息
#define MSG_ICD_RELOADCONFIG		1498	//配置台向代理发

//连接CCS消息
#define MSG_PROXY_CONNECT_CCS		651		//代理向CCS发
#define MSG_PROXY_CONNECT_CCS_ACK	652		//CCS向代理发

//向IVR上报数据源消息
#define MSG_IVR_REPORT_DATASRC		113		//IVR向代理发
#define MSG_IVR_REPORT_DATASRC_ACK	114		//代理向IVR发

//调用存储过程消息
#define MSG_SCP_PROXY_PROC			88		//IVR向代理发
#define MSG_PROXY_SCP_PROC_ACK		89		//代理向IVR发
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
	USHORT		ucMsgType;	//	消息类型	enum ICDCCS_MSGTYPE
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
		char  cSourceName[MAXNAMELEN];	//告警的线程名，模块名,应用程序名等
		ULONG lAlertIP  ;
		ULONG lAlertPort;
		ALERT_DEVICE  cAlertDevice;		//告警灯位，定义了五个缺省告警设备。
										//这五个告警设备分与相应的灯位号相对应
										//......平台组
										//0：CCS
										//1：ALERT
										//2：排队机
										//3：IVR
										//4：PROXY
										//5：FEP
										//......业务组
										//6: 
										//7:
										//8:
										//9:
										//10:
										//11:
		char  AlertLevel;				//告警级别
		ULONG lAlertTime;				//告警时长							
										//0：表示无限期告警
										//其它：以毫秒为单位

		ULONG lAlertEvent;				//告警事件
		LONG lAlertFlow;				//告警信息的序号
		WORD  wCataory;					//事件类别
		char  nMod;						//模块号
		char  strDesc[MAX_DESC_LEN];	//描述
		long  lRawLen;					//长度
		char  strRawData[1];			//告警二进制信息，在程序的运行过程中，产生的非法的数据
										//应该按照实际的程度来分配，以减少网络负担
		DWORD dwStay;    				//保留字段
}ALERTSTRUCT;

struct UnregistMsg{
		MESSAGE_HEAD  msg_head	;	//消息头
		DWORD		  dwSrcIP;		//告警源的IP地址
		DWORD         dwPort;		//告警源的端口号
};

struct RegistMsg{
	MESSAGE_HEAD  msg_head	;		//消息头
	REGISTINFO RegistInfo	;		//登记数据
};

struct AlertMsg{
	MESSAGE_HEAD  msg_head	;		//消息头
	ALERTSTRUCT AlertInfo	;		//告警信息
};

struct StopAlertMsg{
	MESSAGE_HEAD  msg_head	;		//消息头
	ULONG lAlertIP  ;				//告警源IP地址
	ULONG lAlertPort;				//告警源端口号
	LONG lAlertFlow;                //告警信息的序号
	DWORD dwStay;                   //保留字段
};

struct  ShakeHand
{
	MESSAGE_HEAD head;	//消息头
};

struct  ReloadConfig
{
	MESSAGE_HEAD	head;	//消息头
	UC		ucTaskHandle;	//管理台加载命令编号,以区分发往不同模块的命令
	US		wTaskDsn;		//保留未用
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

struct ReportDataSrcAck		//从IVR获得的消息包为标准的消息头结构MESSAGE_HEAD，返回为本结构
{
	IVR_MESSAGE_HEAD head;
    UC para[1];			//上报的数据源名称串，用"!"分隔
};

struct ProxySqlProcPara
{
	UC dataType;	//字段数据类型
					//与DATATYPE中定义一致
	UC paraType;	//2:输出输入参数
					//1:输出参数
					//0及其它：输入参数
	US dataLen;		//字段数据长度
};

struct  ProxySqlProcWithTimeOutMsg
{
	IVR_MESSAGE_HEAD	head;		//消息头
	char	dataSource[NAMELEN];	//数据源名
	UL		ulTimeOut;				//执行存储过程的超时值以毫秒为单位
	char	quanlifier[NAMELEN-sizeof(UL)];//用户名
	UC		paraNum;				//参数个数
	char	procName[NAMELEN];		//存储过程名
	ProxySqlProcPara	para;		//存储过程的参数
	char	dataArea;				//存储过程的数据区
};

struct ProxySqlProcAckMsg
{
	IVR_MESSAGE_HEAD	head;		//消息头
	US		status; 				//存储过程执行状态1：执行失败1, 0：执行成功
									//该值仅仅表示调用存储过程成功或失败
									//并不表示用户存储过程执行结果正确
	char		dataSource[NAMELEN];//数据源名
	char		quanlifier[NAMELEN];//用户名
	char		paraNum;			//参数个数
	char		procName[NAMELEN];	//存储过程名
	ProxySqlProcPara	para;		//存储过程的参数
	char		dataArea;			//存储过程的数据区
};

//中间件指令协议包
//
//end of 中间件指令协议包


#endif