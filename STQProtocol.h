#ifndef __STQPROTOCOL_H__
#define __STQPROTOCOL_H__

#define STQ_CMD_SIZE        6
#define ADDR_SIZE           9
#define MAX_DATA_LEN        216

enum ePACKET_TYPE
{
    APPLY_PACKET    = '1',            // ��������
    REPLY_PACKET    = '2',            // ����Ӧ��
    APPLY_FILE      = '3',            // �ļ�����
    REPLY_FILE      = '4',            // �ļ�Ӧ��
};

// ˼���湫˾��Э�鱨��
typedef struct tagSTQ_PACKET
{
    BYTE    byMorePkt;              // ��������־
    BYTE    byPktType;              // ������
    BYTE    byEnd;                  // �����Ľ�����־
    BYTE    bySave;                 // �����ֶ�
    WORD    wSeq;                   // �����
    WORD    wLen;                   // pData�ֶεĳ���(δ��)
    DWORD   dwApplyID;              // ����ID
    char    pCMD[STQ_CMD_SIZE];     // ����������
    char    pDstAddr[ADDR_SIZE];    // ����Ŀ�ĵ�ַ
    char    pSrcAddr[ADDR_SIZE];    // ���׷����ַ
    char    pData[MAX_DATA_LEN];    // ����������
}STQ_PACKET, *LPSTQ_PACKET;

#endif