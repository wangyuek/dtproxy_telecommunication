#ifndef __COMMONFUNC_H__
#define __COMMONFUNC_H__

#include "ParamConfig.h"
#include "SPCallProtocol.h"
#include "STQProtocol.h"
#include "ApplyProtocol.h"

#include "Misc.h"
#include <afxsock.h>		// MFC socket extensions
#include <io.h>

#define DTPROXYDLL_INI		    ".\\DtProxyDll.ini"
#define CONFIG_NUM			    120
#define AUTO1860_TYPE_NUM	    50

LPPARAM_CONFIG GetParamConfigFromICDName(CString strICDCMD);
CString TransParamToString(LPSP_PARAM pParam, char *pData);
BOOL TransStringToParam(LPSP_PARAM pParam, char *pData, DWORD &dwDataLen, CString strField);
CString GetField(CString &strRecord, CString strFieldSep, CString strRecSep);
CString GetMIDCodeFromConfig(DWORD dwAuto1860Type, BOOL bDigital);

LPBRIEF_CONFIG GetBriefConfigFromMIDName(CString strMIDCMD);
CString GetBriefCode(CString strSource, CString strPriCode);

int  ReadCfgFile();
int  ReadBriefCfgFile();
void GetSPNameFromMsgType(DWORD dwMsgType, CString &strSPName);
BOOL MakeSequence(char *pSeq, DWORD &dwLen);
void InitDll(void);
BOOL TransToAppSvrMsg(char *pBuf, DWORD dwLen, char *pApply, DWORD dwApplyLen, 
					  char *pMsg, DWORD &dwMsgLen, BOOL &bEndReply);

CString GetFee(CString strFee);
CString GetVPCode(CString strFee);
CString GetFeeVP(CString strFee);
CString GetAuto1860Code(CString strPriCode);

//add by xuwenfu 20081015 判断C网号码
bool IsMobileNumber(CString);
int JudgeHandsetNo(CString);

//added by wangyue 20130326
CString YuanToFen(CString strYuan);
CString FenToYuan(CString strFen);
//在strParentStr中查找strSubStr的数量
int GetSubStringNoCase(CString &strParentStr,CString &strSubStr);
CString UTF8ToUnicode(char* UTF8);
CString GetElementValue(CString &strXML,CString strStartTag);
CString GetElement(CString &strXML,CString strTag);
CString GetElementAttr(CString &strXML,CString strTag,CString strAttrName);
CString UTF8ToANSI(CString strUTF8);

//费用item
class BILLITEM
{
public:
	CString strName;
	CString strID;
	CString strFee;
public:
	BILLITEM(CString tmpName,CString tmpID,CString tmpFee) : strName(tmpName),strID(tmpID),strFee(tmpFee)
	{

	}
};

extern BRIEF_CONFIG		g_BriefConfig[CONFIG_NUM];


void NewWriteLog(CString strMsg, CString strTip, CString strCallName, DWORD dwRequestId);
void NewWriteLog(CString strMsg, CString strTip, CString strCallName, char* pRequestId);
void MyTrimSm2(int nLenMax, CString strSmsTotal, CString strSmsSub[], int nCount);   // Add by xuzhenquan 20100624
#endif