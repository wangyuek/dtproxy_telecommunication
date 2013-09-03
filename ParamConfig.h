#ifndef __PARAMCONFIG_H__
#define __PARAMCONFIG_H__

#define FORMAT_SIZE         32      // ��ʽ���ִ��ĳ���
#define FORMAT_NUM          128     // ÿ�ֱ��ĵ���������������
#define AUTO1860_CODE_NUM   255     // 1860�Զ�����������Ĳ��������
#define VP_SIZE				32		// �����ִ��ĳ���
#define MAX_VP_NUM			120		// ÿ�ֱ��ĵ������������
#define NAMELEN             50

// ÿ�ֱ��ĵĲ�������
typedef struct tagPARAM_CONFIG
{
    BOOL    bUsed;
    char    pICDCMD[NAMELEN];       // ICD�౨�ĵ�������
    char    pMIDCMD[NAMELEN];       // ������ǰ�û��౨�ĵ�������
    char    pFormat[FORMAT_NUM][FORMAT_SIZE];   // ICD�����������ʽ���Ӵ�
	char    pVPCode[MAX_VP_NUM][VP_SIZE];		//  ��Ҫ��ӵ���������
	char    pVPCode2[MAX_VP_NUM][VP_SIZE];		//  ��Ҫ��ӵĸ���������
	char	strTitle[2048];						// 200304007 LIUSHAOHUA
	char	strWidth[1024];						// 200304007
	int 	nVPCount;							//  �����������
}PARAM_CONFIG, *LPPARAM_CONFIG;

// 1860�Զ�������ҵ�������ձ�
typedef struct tagAUTO1860_CODE
{
    char    pAuto1860Type[NAMELEN]; // 1860�Զ����̲�����
    char    pDigitialCode[NAMELEN]; // �����ֻ���ҵ�����     
    char    pAnalogCode[NAMELEN];   // ģ���ֻ���ҵ�����
}AUTO1860_CODE, *LPAUTO1860_CODE;

typedef struct tagHLR_CONFIG
{
    char    pShiLi[128];
	char	pHuaWei[128];
}HLR_CONFIG, *LPHLR_CONFIG;

typedef struct tagHLR_MSG
{
	HLR_CONFIG	pHLRConfig[32];
	int 		iCount;		
}HLR_MSG, *LPHLR_MSG;

// ��ͨ��ϸ��Ϣ����
typedef struct tagCODE_CONFIG
{
    char    pIndex[8];
	char	pContent[2048];
}CODE_CONFIG, *LPCODE_CONFIG;

// ��ͨ������Ϣ������
typedef struct tagBRIEF_CONFIG
{
    BOOL    bUsed;
    char    pMIDCMD[NAMELEN];       // ������ǰ�û��౨�ĵ�������
	CODE_CONFIG    pCodeConfig[16];
	int 	iCount;		
}BRIEF_CONFIG, *LPBRIEF_CONFIG;

#endif