#ifndef __SPCALLPROTOCOL_H__
#define __SPCALLPROTOCOL_H__

#define NAMELEN                 50
#define OUTPUT_PARAM	        1		// �������(��1, 2֮��ȱʡΪ�������)
#define INOUT_PARAM		        2		// ����, �������

#define MSG_SPCALL              88
#define MSG_SPCALL_ACK          89
#define MSG_IVR_SHAKEHAND		93		// IVR������������Ϣ��
#define MSG_REPORT_DATASRC		113		// Э�������IVR�ϱ�����Դ
#define MSG_REPORT_DATASRC_ACK	114		// Э�������IVR�ϱ�����Դ
#define MSG_CCS_SHAKEHAND		650		// CCS��������Ϣ

// ��Ϣ����Դ
typedef struct tagMSG_ADDRESS
{
    BOOL    bApply;             // TRUE: ������Ϣ(ICD); FALSE: Ӧ����Ϣ(MID)
    BOOL    bIVRMsg;            // TRUE: IVR������Ϣ; FALSE: AppSvr������Ϣ
    DWORD   dwSrcIP;            // ICDվ��IP
    DWORD   dwSrcID;            // ICDվ��ID
    DWORD   dwConnIndex;        // MID��Ӧ������
    char    pSPName[NAMELEN];   // ��Ϣ����
    BOOL    bValid;             // ����Ϣ����Ч��
}MSG_ADDRESS, *LPMSG_ADDRESS;

// IVR���͸�PROXY��������Ϣ�������еİ�ͷ�ṹ
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

// �洢���̵���ÿ��������˵�����߷��صĽ������ÿ���ֶε�˵��
typedef struct tagSP_PARAM
{
	BYTE byDataType;			// �ֶ���������
		    					// ��DATATYPE�ж���һ��
	BYTE byParaType;			// 2: ����������
			    				// 1: �������
				    			// 0���������������
	WORD wDataLen;				// �ֶ����ݳ���
}SP_PARAM, *LPSP_PARAM;

// ��PROXY���ʹ洢���̵�����Ϣ
typedef struct tagSP_CALL
{
	IVR_MSG_HEAD	IVRHead;						//��Ϣͷ
	char			pDataSource[NAMELEN];			//����Դ��
	DWORD			dwTimeout;						//ִ�д洢���̵ĳ�ʱֵ�Ժ���Ϊ��λ
	char			pUserID[NAMELEN-sizeof(DWORD)];	//�û���
	BYTE			byParaNum;						//��������
	char			pProcName[NAMELEN];				//�洢������
	SP_PARAM		SPPara;							//�洢���̵Ĳ���
//	char			pDataArea;						//�洢���̵�������
}SP_CALL, *LPSP_CALL;

// ���洢���̵��õĽ��������ԭʼ������
typedef struct tagSP_CALL_ACK
{
    IVR_MSG_HEAD	IVRHead;						//��Ϣͷ
	WORD		    wStatus; 		                //�洢����ִ��״̬1��ִ��ʧ��1, 0��ִ�гɹ�
									                //��ֵ������ʾ���ô洢���̳ɹ���ʧ��
									                //������ʾ�û��洢����ִ�н����ȷ
	char			pDataSource[NAMELEN];			//����Դ��
	char			pUserID[NAMELEN];	            //�û���
	BYTE			byParaNum;						//��������
	char			pProcName[NAMELEN];				//�洢������
	SP_PARAM		SPPara;							//�洢���̵Ĳ���
//	char			pDataArea;						//�洢���̵�������
}SP_CALL_ACK, *LPSP_CALL_ACK;

// Э�������IVR�ϱ�����Դ��Ϣ
typedef struct tagREPORT_DATA_SOURCE
{
	IVR_MSG_HEAD	IVRHead;
	char			pDataSource[1];		// ����Դ����, ��!����
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