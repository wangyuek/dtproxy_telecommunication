#ifndef __APPLYPROTOCOL_H__
#define __APPLYPROTOCOL_H__

#define CMD_SIZE    10
#define SEP_SIZE			5

typedef struct tagAPPLY_MSG
{
	DWORD	dwLen;          		// 包的总长度，为包控制信息和实际传送内容长度之和
	BYTE	byFactory;				// 厂商编码, 华为＝168
	BYTE	byProgID;				// 进程号
	BYTE	byMorePkt; 				// 是否还有后续包，1有，0无
	char	pCMD[CMD_SIZE];			// 命令字串，NULL结束
	DWORD	dwStartNum;				// 起始记录号(默认0)
	DWORD	dwEndNum;				// 终止记录号(为0时表示其余全部数据)
	DWORD	dwRequestID;  			// 请求ID
	DWORD	dwAnswerID;    			// 应答ID
	DWORD	dwSeq;					// 包序号
	char	pRecSep[SEP_SIZE];		// 记录分隔符字串，NULL结束
	char	pFieldSep[SEP_SIZE];	// 字段分隔符字串，NULL结束
	DWORD	dwReserved1;			// 保留
	DWORD	dwReserved2;			// 保留

    short	nErrCode;		        // 0: 成功,  其它: 失败
	char	pData[1];		        // 
}APPLY_MSG, *LPAPPLY_MSG;

#endif