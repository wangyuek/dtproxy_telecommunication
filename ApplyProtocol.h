#ifndef __APPLYPROTOCOL_H__
#define __APPLYPROTOCOL_H__

#define CMD_SIZE    10
#define SEP_SIZE			5

typedef struct tagAPPLY_MSG
{
	DWORD	dwLen;          		// �����ܳ��ȣ�Ϊ��������Ϣ��ʵ�ʴ������ݳ���֮��
	BYTE	byFactory;				// ���̱���, ��Ϊ��168
	BYTE	byProgID;				// ���̺�
	BYTE	byMorePkt; 				// �Ƿ��к�������1�У�0��
	char	pCMD[CMD_SIZE];			// �����ִ���NULL����
	DWORD	dwStartNum;				// ��ʼ��¼��(Ĭ��0)
	DWORD	dwEndNum;				// ��ֹ��¼��(Ϊ0ʱ��ʾ����ȫ������)
	DWORD	dwRequestID;  			// ����ID
	DWORD	dwAnswerID;    			// Ӧ��ID
	DWORD	dwSeq;					// �����
	char	pRecSep[SEP_SIZE];		// ��¼�ָ����ִ���NULL����
	char	pFieldSep[SEP_SIZE];	// �ֶηָ����ִ���NULL����
	DWORD	dwReserved1;			// ����
	DWORD	dwReserved2;			// ����

    short	nErrCode;		        // 0: �ɹ�,  ����: ʧ��
	char	pData[1];		        // 
}APPLY_MSG, *LPAPPLY_MSG;

#endif