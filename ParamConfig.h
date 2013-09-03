#ifndef __PARAMCONFIG_H__
#define __PARAMCONFIG_H__

#define FORMAT_SIZE         32      // 格式化字串的长度
#define FORMAT_NUM          128     // 每种报文的最多输入参数个数
#define AUTO1860_CODE_NUM   255     // 1860自动流程所定义的操作码个数
#define VP_SIZE				32		// 语音字串的长度
#define MAX_VP_NUM			120		// 每种报文的最多语音个数
#define NAMELEN             50

// 每种报文的参数配置
typedef struct tagPARAM_CONFIG
{
    BOOL    bUsed;
    char    pICDCMD[NAMELEN];       // ICD侧报文的命令字
    char    pMIDCMD[NAMELEN];       // 第三方前置机侧报文的命令字
    char    pFormat[FORMAT_NUM][FORMAT_SIZE];   // ICD侧输入参数格式化子串
	char    pVPCode[MAX_VP_NUM][VP_SIZE];		//  需要添加的语音代码
	char    pVPCode2[MAX_VP_NUM][VP_SIZE];		//  需要添加的负语音代码
	char	strTitle[2048];						// 200304007 LIUSHAOHUA
	char	strWidth[1024];						// 200304007
	int 	nVPCount;							//  语音代码个数
}PARAM_CONFIG, *LPPARAM_CONFIG;

// 1860自动流程新业务代码对照表
typedef struct tagAUTO1860_CODE
{
    char    pAuto1860Type[NAMELEN]; // 1860自动流程操作码
    char    pDigitialCode[NAMELEN]; // 数字手机新业务代码     
    char    pAnalogCode[NAMELEN];   // 模拟手机新业务代码
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

// 联通详细信息配置
typedef struct tagCODE_CONFIG
{
    char    pIndex[8];
	char	pContent[2048];
}CODE_CONFIG, *LPCODE_CONFIG;

// 联通代码信息配置项
typedef struct tagBRIEF_CONFIG
{
    BOOL    bUsed;
    char    pMIDCMD[NAMELEN];       // 第三方前置机侧报文的命令字
	CODE_CONFIG    pCodeConfig[16];
	int 	iCount;		
}BRIEF_CONFIG, *LPBRIEF_CONFIG;

#endif