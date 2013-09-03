#ifndef __STQPROTOCOL_H__
#define __STQPROTOCOL_H__

#define STQ_CMD_SIZE        6
#define ADDR_SIZE           9
#define MAX_DATA_LEN        216

enum ePACKET_TYPE
{
    APPLY_PACKET    = '1',            // 报文请求
    REPLY_PACKET    = '2',            // 报文应答
    APPLY_FILE      = '3',            // 文件请求
    REPLY_FILE      = '4',            // 文件应答
};

// 思特奇公司的协议报文
typedef struct tagSTQ_PACKET
{
    BYTE    byMorePkt;              // 后续包标志
    BYTE    byPktType;              // 包类型
    BYTE    byEnd;                  // 请求报文结束标志
    BYTE    bySave;                 // 保留字段
    WORD    wSeq;                   // 包序号
    WORD    wLen;                   // pData字段的长度(未用)
    DWORD   dwApplyID;              // 请求ID
    char    pCMD[STQ_CMD_SIZE];     // 交易命令字
    char    pDstAddr[ADDR_SIZE];    // 交易目的地址
    char    pSrcAddr[ADDR_SIZE];    // 交易发起地址
    char    pData[MAX_DATA_LEN];    // 交易数据区
}STQ_PACKET, *LPSTQ_PACKET;

#endif