#ifndef __SPCALLPROTOCOL_H__
#define __SPCALLPROTOCOL_H__

#define NAMELEN                 50
#define OUTPUT_PARAM	        1		// 输出参数(除1, 2之外缺省为输入参数)
#define INOUT_PARAM		        2		// 输入, 输出参数

#define MSG_SPCALL              88
#define MSG_SPCALL_ACK          89
#define MSG_IVR_SHAKEHAND		93		// IVR发来的握手消息包
#define MSG_REPORT_DATASRC		113		// 协议代理向IVR上报数据源
#define MSG_REPORT_DATASRC_ACK	114		// 协议代理向IVR上报数据源
#define MSG_CCS_SHAKEHAND		650		// CCS的握手消息

// 消息的来源
typedef struct tagMSG_ADDRESS
{
    BOOL    bApply;             // TRUE: 请求消息(ICD); FALSE: 应答消息(MID)
    BOOL    bIVRMsg;            // TRUE: IVR类型消息; FALSE: AppSvr类型消息
    DWORD   dwSrcIP;            // ICD站点IP
    DWORD   dwSrcID;            // ICD站点ID
    DWORD   dwConnIndex;        // MID对应的连接
    char    pSPName[NAMELEN];   // 消息类型
    BOOL    bValid;             // 该消息的有效性
}MSG_ADDRESS, *LPMSG_ADDRESS;

// IVR发送给PROXY的所有消息包所共有的包头结构
typedef struct tagIVR_MSG_HEAD
{
	BYTE bySendNode;
	BYTE bySendPort;

	BYTE byRecvNode;
	BYTE byRecvPort;

    WORD wMsgType;
	BYTE byMID;
    BYTE byTaskHandle;
    WORD wTaskDSN;

}IVR_MSG_HEAD, *LPIVR_MSG_HEAD;

// 存储过程调用每个参数的说明或者返回的结果集的每个字段的说明
typedef struct tagSP_PARAM
{
	BYTE byDataType;			// 字段数据类型
		    					// 与DATATYPE中定义一致
	BYTE byParaType;			// 2: 输出输入参数
			    				// 1: 输出参数
				    			// 0及其它：输入参数
	WORD wDataLen;				// 字段数据长度
}SP_PARAM, *LPSP_PARAM;

// 向PROXY发送存储过程调用消息
typedef struct tagSP_CALL
{
	IVR_MSG_HEAD	IVRHead;						//消息头
	char			pDataSource[NAMELEN];			//数据源名
	DWORD			dwTimeout;						//执行存储过程的超时值以毫秒为单位
	char			pUserID[NAMELEN-sizeof(DWORD)];	//用户名
	BYTE			byParaNum;						//参数个数
	char			pProcName[NAMELEN];				//存储过程名
	SP_PARAM		SPPara;							//存储过程的参数
//	char			pDataArea;						//存储过程的数据区
}SP_CALL, *LPSP_CALL;

// 将存储过程调用的结果返还给原始调用者
typedef struct tagSP_CALL_ACK
{
    IVR_MSG_HEAD	IVRHead;						//消息头
	WORD		    wStatus; 		                //存储过程执行状态1：执行失败1, 0：执行成功
									                //该值仅仅表示调用存储过程成功或失败
									                //并不表示用户存储过程执行结果正确
	char			pDataSource[NAMELEN];			//数据源名
	char			pUserID[NAMELEN];	            //用户名
	BYTE			byParaNum;						//参数个数
	char			pProcName[NAMELEN];				//存储过程名
	SP_PARAM		SPPara;							//存储过程的参数
//	char			pDataArea;						//存储过程的数据区
}SP_CALL_ACK, *LPSP_CALL_ACK;

// 协议代理向IVR上报数据源消息
typedef struct tagREPORT_DATA_SOURCE
{
	IVR_MSG_HEAD	IVRHead;
	char			pDataSource[1];		// 数据源名称, 用!隔开
}REPORT_DATA_SOURCE, *LPREPORT_DATA_SOURCE;

typedef struct	tagDATE_TYPE
{
	WORD	wYear;	
	WORD	wMonth;	
	WORD	wDay;
	WORD    wReserved[5];
}DATE_TYPE, *LPDATE_TYPE;

typedef struct	tagTIME_TYPE
{
	WORD	wHour;
	WORD	wMinute;
	WORD	wSecond;
	WORD    wReserved[5];
}TIME_TYPE, *LPTIME_TYPE;

typedef struct	tagDATETIME_TYPE
{
	WORD	wYear;
	WORD	wMonth;
	WORD	wDay;
	WORD	wHour;
	WORD	wMinute;
	WORD	wSecond;
	DWORD   dwFraction;	
}DATETIME_TYPE, *LPDATETIME_TYPE;

/*
enum eDATA_TYPE
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
};
*/

#endif