// DtProxyDll.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "DtProxyDll.h"
//#include "Misc.h"
#include "ParamConfig.h"
#include "SPCallProtocol.h"
#include "STQProtocol.h"
#include "CommonFunc.h"
#include "ApplyProtocol.h"
#include "cmarkup/Markup.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


char g_pLoginUserName[128];
char g_pLoginPWD[128];

BRIEF_CONFIG	g_BriefConfig[CONFIG_NUM]; //for Brief.ini

CString g_strSrcAddr;
CString g_strDstAddr;

AUTO1860_CODE   g_Auto1860Code[AUTO1860_CODE_NUM];
PARAM_CONFIG    g_ParamConfig[CONFIG_NUM];
HLR_MSG			g_HLR_ZMIO;
HLR_MSG			g_HLR_ZMSO;
HLR_MSG			g_HLR_ZMSO_Repalce;

CString g_strFSep;
CString g_strRSep;
CString g_strFeeSep;
CString g_strVPCodePositive;    // 费用为正数时的VP语音编码集
CString g_strVPCodeNegative;    // 费用为负数时的VP语音编码集
CString g_strAuto1860Code;      // 1860操作码映射集
CString g_strPathOfFaxFile;

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

/////////////////////////////////////////////////////////////////////////////
// CDtProxyDllApp

BEGIN_MESSAGE_MAP(CDtProxyDllApp, CWinApp)
//{{AFX_MSG_MAP(CDtProxyDllApp)
// NOTE - the ClassWizard will add and remove mapping macros here.
//    DO NOT EDIT what you see in these blocks of generated code!
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDtProxyDllApp construction

CDtProxyDllApp::CDtProxyDllApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CDtProxyDllApp object

CDtProxyDllApp theApp;



///////////////////////////////////////////////////////////////////////////////
//  Func:       
//  Describe:   将IVR类型的消息转换为外部接口协议
//  Input:      
//              pBuf:       消息指针
//              dwLen:      消息长度
//              dwTransLen: pTrans缓冲区的大小
//              dwIndexLen: pSeqIndex缓冲区的大小
//  Output:
//              pTrans:     转换后的消息指针
//              dwTransLen: 转换后的消息长度
//              pSeqIndex:  生成的流水号指针
//              dwIndexLen: 生成的流水号的长度
//  Return:
//              TRUE:       转换成功
//              FALSE:      转换失败
//  Note:       dwTransLen, dwIndexLen都属于输入输出参数. 
//              这里我们默认为请求包可以在一个报文中实现,无需分割成两个报文
///////////////////////////////////////////////////////////////////////////////
BOOL __stdcall
DllTransIVRMsg(char *pBuf, DWORD dwLen, char *pTrans, DWORD &dwTransLen, 
               char *pSeqIndex, DWORD &dwIndexLen)
{
    LPSP_CALL       pSPCall;
    LPAPPLY_MSG     pApply;
    LPSP_PARAM      pParam;
    LPPARAM_CONFIG  pConfig;
    char            *pSrcData;
    DWORD           dwSeq;
    CString         strData;
    CString         strMIDCMD;
	
	InitDll();
    pSPCall = (LPSP_CALL)&pBuf[sizeof(MSG_ADDRESS)];
    pApply  = (LPAPPLY_MSG)pTrans;
	
	memset(pTrans, 0, sizeof(APPLY_MSG));
	
    // 获取参数的配置说明
    pConfig = GetParamConfigFromICDName(pSPCall->pProcName);    
	if (!pConfig)
    {						
        return FALSE;   // 未知命令字
    }
    strMIDCMD = pConfig->pMIDCMD;
	
    // 生成流水号
    if (!MakeSequence(pSeqIndex, dwIndexLen))
    {
        return FALSE;   // 流水号生成失败
    }
    dwSeq = *(DWORD *)pSeqIndex;
	
    // 参数转换(通用转换方式)
    pParam = &pSPCall->SPPara;
    pSrcData = (char *)pParam + sizeof(SP_PARAM) * pSPCall->byParaNum;
    for (int ii = 0; ii < pSPCall->byParaNum; ii++)
    {
        // 不转换输出参数, 按照默认格式转换
        if (pParam->byParaType != OUTPUT_PARAM)
        {
            strData += TransParamToString(pParam, pSrcData);                
            strData += g_strFSep;
        }
		
        pSrcData += pParam->wDataLen;
        pParam++;
    }
	
	//写日志
	NewWriteLog(strData, "DllTransIVRMsg", pConfig->pICDCMD, dwSeq);
	
	if(!_stricmp(pConfig->pICDCMD, "P_SCEGetUserPaytype")) //获取用户付费类型
	{
		CString strServiceNo; //受理号码
		CString strCallerNo;  //主叫号码  暂时不用
		CString strCalleeNo;  //被叫号码  暂时不用
		strServiceNo = GetField(strData, g_strFSep, g_strRSep);
		GetField(strData, g_strFSep, g_strRSep);
		GetField(strData, g_strFSep, g_strRSep);
		strData = strServiceNo + g_strFSep;
	}

	else if (!_stricmp(pConfig->pICDCMD, "P_SCEHandsetNoTypeJudg")) //手机号码验证
	{
		CString strHandsetNo;
		CString strUserType;   //预留
		strHandsetNo = GetField(strData, g_strFSep, g_strRSep);
		strUserType = GetField(strData, g_strFSep, g_strRSep);

		strData = strHandsetNo + g_strFSep;
	}

	//modified by huguangwen 20090309  (发给BOSS的参数中加入用户类型)
	//modified by wangyue 20130318,将I_SCE_Login/P_SCELogin两个接口合并
	//else if (!_stricmp(pConfig->pICDCMD, "P_SCELogin"))  //密码验证
	//{
	//	CString strHandsetNo;
	//	CString strPassword;
	//	CString strUserType;  //用户类型（预留）
	//	
	//	
	//	strHandsetNo = GetField(strData, g_strFSep, g_strRSep);
	//	strPassword  = GetField(strData, g_strFSep, g_strRSep);
	//	strUserType  = GetField(strData, g_strFSep, g_strRSep);

	//	bool bMobileNum = false;
	//	bMobileNum = IsMobileNumber(strHandsetNo);
	//	if (true == bMobileNum)   //c网
	//	{
	//		strUserType = "4";
	//	}
	//	else					  //固网
	//	{
	//		strUserType = "2";
	//	}

	//	strData = strHandsetNo + g_strFSep + strUserType + g_strFSep + strPassword + g_strFSep;
	//}

	// add by tongyufeng,20101228,增加SP自动业务办理接口
	//modified by wangyue,20130506,因为接口改造亚联修改入参，增加地市和渠道两个入参
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_AutoService"))  //程控业务办理
	{
		CString strCityCode;//地市代码
		CString strChannelID;//渠道ID，1002是10000号系统
		CString strHandsetNo; //用户号码
		CString strServiceType;  //业务类型，0-七彩铃音  1-来电显示 2-呼叫保持 3-呼叫等待
		CString strOptType;  //操作类型，0开通，1退订
		CString strUserType; //用户类型，1-手机，10-固话
		
		strCityCode = GetField(strData, g_strFSep, g_strRSep);
		strChannelID = GetField(strData, g_strFSep, g_strRSep);
		strHandsetNo = GetField(strData, g_strFSep, g_strRSep);
		strUserType = GetField(strData, g_strFSep, g_strRSep);
		strServiceType = GetField(strData, g_strFSep, g_strRSep);
		strOptType = GetField(strData, g_strFSep, g_strRSep);
		
		strData = strCityCode + g_strFSep + strChannelID + g_strFSep + strHandsetNo + g_strFSep + strUserType + g_strFSep + strServiceType + g_strFSep 
			+ strOptType + g_strFSep;
	}

	// add by tongyufeng,2011-02-21,增加话费短信命令字
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_AllFeeSms"))
	{
		CString strHandsetNo; //用户号码
		CString strUserType; //用户类型，1-手机，10-固话
		CString strCitycode; //用户地市
		
		strHandsetNo = GetField(strData, g_strFSep, g_strRSep);
		strUserType = GetField(strData, g_strFSep, g_strRSep);
        //strCitycode = GetField(strData, g_strFSep, g_strRSep);
		
		strData = strHandsetNo + g_strFSep + _T("1~")/*querytype*/ + strUserType + g_strFSep;
	}
	
	else if (!_stricmp(pConfig->pICDCMD, "P_SCEChangePassword")
		|| !_stricmp(pConfig->pICDCMD, "I_SCE_PasswdRest"))  //修改密码\密码重置
	{
		CString strHandsetNo;
		CString strOldPWD;  //旧密码
		CString strNewPWd;  //新密码
		CString strUserType; //nOption（1合同，2固定，3移动，4托收号码）
		CString strCityCode;//地市代码

		strHandsetNo = GetField(strData, g_strFSep, g_strRSep);
		strOldPWD = GetField(strData, g_strFSep, g_strRSep);
		strNewPWd = GetField(strData, g_strFSep, g_strRSep);
		strUserType = GetField(strData, g_strFSep, g_strRSep);
		strCityCode = GetField(strData, g_strFSep, g_strRSep);

		bool bMobileNum = false;
		bMobileNum = IsMobileNumber(strHandsetNo);
		if (true == bMobileNum)   //c网
		{
			strUserType = "4";
		}
		else					  //固网
		{
			strUserType = "2";
		}
		if(!_stricmp(pConfig->pICDCMD, "P_SCEChangePassword"))//修改密码
		{
			strData = strUserType + g_strFSep + strHandsetNo + g_strFSep + strCityCode + g_strFSep +_T("")/*消息流水号*/
				+g_strFSep + _T("1002")/*系统标识 1002:10000号*/ +g_strFSep + _T("")/*接触ID*/ + g_strFSep + strOldPWD + g_strFSep + strNewPWd + g_strFSep + _T("1")/*1:密码修改，2：密码重置*/
				+ g_strFSep + _T("2")/*1:查询密码，2:交易密码,亚联反馈CRM2.0以后只有业务密码，不区分查询密码*/+g_strFSep;
		}
		else if(!_stricmp(pConfig->pICDCMD, "I_SCE_PasswdReset"))//密码重置
		{
			strData = strUserType + g_strFSep + strHandsetNo + g_strFSep + strCityCode + g_strFSep +_T("")/*消息流水号*/
				+g_strFSep + _T("1002")/*系统标识 1002:10000号*/ +g_strFSep + _T("")/*接触ID*/ + g_strFSep + strOldPWD + g_strFSep + strNewPWd + g_strFSep + _T("2")/*1:密码修改，2：密码重置*/
				+ g_strFSep + _T("2")/*1:查询密码，2:交易密码*/+g_strFSep;
		}
	}

	else if (!_stricmp(pConfig->pICDCMD, "P_SCEResetPassword"))  //重置密码
	{
		CString strHandsetNo;
		CString strPassWord;
		CString strUserType;
		strHandsetNo = GetField(strData, g_strFSep, g_strRSep);
		strPassWord = GetField(strData, g_strFSep, g_strRSep);
		strUserType = GetField(strData, g_strFSep, g_strRSep);

		strData = strHandsetNo + g_strFSep + strPassWord + g_strFSep;
	}

	//modified by huguangwen 20090309  (发给BOSS的参数中加入用户类型,和编码类型两个参数)
	else if (!_stricmp(pConfig->pICDCMD, "P_SCEGetTotalFee"))  //月结话费查询(后付费)支持５个月
	{
		CString strHandsetNo;
		CString striType;    //付费类型(暂无用)
		CString strUserType;
		CString strStartTime; //查询月份
		CString strEndtime; //暂时不用
		
		strHandsetNo = GetField(strData, g_strFSep, g_strRSep);
		GetField(strData, g_strFSep, g_strRSep);
		strUserType = GetField(strData, g_strFSep, g_strRSep);
		strStartTime = GetField(strData, g_strFSep, g_strRSep);
		GetField(strData, g_strFSep, g_strRSep);

		bool bMobileNum = false;
		bMobileNum = IsMobileNumber(strHandsetNo);
		if (true == bMobileNum)   //c网
		{
			strUserType = "1";
		}
		else					  //固网
		{
			strUserType = "10";
		}

		strData = strHandsetNo + g_strFSep + strUserType + g_strFSep
			+ strStartTime + g_strFSep + "0" + g_strFSep;
	}

	else if (!_stricmp(pConfig->pICDCMD, "P_SCEPhoneIDCheck"))  //证件号码验证
	{
		CString strHandsetNo;  //用户手机号码
		CString strIDNo;       //证件号码
		CString strUserType;
		strHandsetNo = GetField(strData, g_strFSep, g_strRSep);
		strIDNo = GetField(strData, g_strFSep, g_strRSep);
		strUserType = GetField(strData, g_strFSep, g_strRSep);

		strData = strHandsetNo + g_strFSep + strIDNo + g_strFSep;
	}
	else if (!_stricmp(pConfig->pICDCMD, "P_SCEAppealOpenClose")) //停开机申请
	{
		CString strHandsetNo;
		CString strAppealType;//申请类型
		CString strUserType;
		strHandsetNo = GetField(strData, g_strFSep, g_strRSep);
		strAppealType = GetField(strData, g_strFSep, g_strRSep);
		strUserType = GetField(strData, g_strFSep, g_strRSep);

		strData = strHandsetNo + g_strFSep + strAppealType + g_strFSep;
	}

	//modified by huguangwen 20090309  (发给BOSS的参数中加入用户类型,和编码类型两个参数)
	else if (!_stricmp(pConfig->pICDCMD, "P_SCEGetRealTimeFee"))  //实时话费(后付费)
	{
		CString strHandsetNo;
		CString strUserType;
		strHandsetNo = GetField(strData, g_strFSep, g_strRSep);
		strUserType = GetField(strData, g_strFSep, g_strRSep);
		//20130402，wangyue：流程传给dtproxy查询开始时间和结束时间，但是新的接口不需要这两个参数
		GetField(strData, g_strFSep, g_strRSep);
		GetField(strData, g_strFSep, g_strRSep);

		bool bMobileNum = false;
		bMobileNum = IsMobileNumber(strHandsetNo);
		if (true == bMobileNum)   //c网
		{
			strUserType = "1";
		}
		else					  //固网
		{
			strUserType = "10";
		}

		strData = strHandsetNo + g_strFSep + "1"/*接口协议的querytype字段，魏晓春反馈不能为空，可以默认为1*/ + g_strFSep + strUserType  + g_strFSep;
	}


	else if (!_stricmp(pConfig->pICDCMD, "P_SceQueryScore"))  //积分查询
	{
		CString strHandsetNo;
		CString strUserType;
		CString strQueryType;
		CString strYearMonth;

		strHandsetNo = GetField(strData, g_strFSep, g_strRSep);
		strUserType = GetField(strData, g_strFSep, g_strRSep);
		strQueryType = GetField(strData, g_strFSep, g_strRSep);
		strYearMonth = GetField(strData, g_strFSep, g_strRSep);

		strData = strHandsetNo + g_strFSep;
	}

	else if (!_stricmp(pConfig->pICDCMD, "P_SCEVAGetTotalFee"))  //余额查询(后付费)
	{
		CString strHandsetNo;
		CString strUserType;
		strHandsetNo = GetField(strData, g_strFSep, g_strRSep);
		strUserType = GetField(strData, g_strFSep, g_strRSep);
		//20130402，wangyue：流程传给dtproxy查询开始时间和结束时间，但是新的接口不需要这两个参数
		GetField(strData, g_strFSep, g_strRSep);
		GetField(strData, g_strFSep, g_strRSep);

		bool bMobileNum = false;
		bMobileNum = IsMobileNumber(strHandsetNo);
		if (true == bMobileNum)   //c网
		{
			strUserType = "1";
		}
		else					  //固网
		{
			strUserType = "10";
		}

		strData = strHandsetNo + g_strFSep + "1"/*接口协议的querytype字段，魏晓春反馈不能为空，可以默认为1*/ + g_strFSep + strUserType  + g_strFSep;
	}

	//原固网部分

	
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_GetUserinfor"))  //1.1.1  用户级别I_SCE_GetUserinfor
	{
		CString strServiceNo;
		CString strCallerNo;
		CString strCalleeNo;
		CString strCityCode;

		strServiceNo = GetField(strData, g_strFSep, g_strRSep);
		strCallerNo = GetField(strData, g_strFSep, g_strRSep);
		strCalleeNo = GetField(strData, g_strFSep, g_strRSep);
		strCityCode = GetField(strData, g_strFSep, g_strRSep);

		CString strUserType;
		bool bMobileNum = false;
		bMobileNum = IsMobileNumber(strServiceNo);
		if (true == bMobileNum)   //c网
		{
			strUserType = "1";
		}
		else					  //固网
		{
			strUserType = "10";
		}

		strData = strServiceNo + g_strFSep + strUserType + g_strFSep;
	}
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_ChangePassword"))  //1.1.2  修改密码I_SCE_ChangePassword
	{
		CString strServiceNo;
		CString strCityCode;
		CString strOldPassWord;
		CString strPassWord;
		CString strPassType;

		strServiceNo = GetField(strData, g_strFSep, g_strRSep);
		strCityCode = GetField(strData, g_strFSep, g_strRSep);
		strOldPassWord = GetField(strData, g_strFSep, g_strRSep);
		strPassWord = GetField(strData, g_strFSep, g_strRSep);
		strPassType = GetField(strData, g_strFSep, g_strRSep);

		CString strUserType;
		bool bMobileNum = false;
		bMobileNum = IsMobileNumber(strServiceNo);
		if (true == bMobileNum)   //c网
		{
			strUserType = "1";
		}
		else					  //固网
		{
			strUserType = "10";
		}

		strData = strServiceNo + g_strFSep + strUserType + g_strFSep + strOldPassWord + g_strFSep + strPassWord + g_strFSep;		
	}

	//modify by tongyufeng 20111124，修改欠费查询接口，IVR入参为4个参数，同时查询账户类型使用IVR传入,去掉接口判断
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_GetDebtFee"))   //1.1.3  查询欠费I_SCE_GetDebtFee
	{
		CString strHandsetNo;
		CString strCityCode;
		//CString strNumFlag;
		CString strUserType;
		CString strAccType;
		
		strHandsetNo = GetField(strData, g_strFSep, g_strRSep);
		strCityCode = GetField(strData, g_strFSep, g_strRSep);
		//strNumFlag = GetField(strData, g_strFSep, g_strRSep);
		strUserType = GetField(strData, g_strFSep, g_strRSep);
		strAccType = GetField(strData, g_strFSep, g_strRSep);

		/*CString strUserType;
		bool bMobileNum = false;
		bMobileNum = IsMobileNumber(strHandsetNo);
		if (true == bMobileNum)   //c网
		{
			strUserType = "1";
		}
		else					  //固网
		{
			strUserType = "10";
		}

		if (!strNumFlag.Compare("1")) //query by tel number
		{	
			strData = strHandsetNo + g_strFSep + strUserType + g_strFSep + "0" + g_strFSep;
		}

		else
		{
		strData = strHandsetNo + g_strFSep + strUserType + g_strFSep + "1" + g_strFSep;
	}*/	
		
		strData = strHandsetNo + g_strFSep + strUserType + g_strFSep + strAccType + g_strFSep;
		
	}

	/************************************************************************/
	/*begin: added by tongyufeng,wangyue,2011-11-24,I_SCE_PasswdReset,I_SCE_QueryBroadNoByAccount,I_SCE_QueryBrandNoByIDCard,
	  I_SCE_QueryFreeResource  (宽带能力提升专用接口)                                                                   */
	/************************************************************************/
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_QueryBrandNoByIDCard"))  //10047 查询同身份证办理的宽带号码
	{
		CString strIDCard;   //号码
		CString strcitycode;   //地市区号
		
		strIDCard = GetField(strData,g_strFSep,g_strRSep);
		strIDCard.Replace(_T("*"),_T("X"));
		strcitycode = GetField(strData,g_strFSep,g_strRSep);
		
		strData = strIDCard + g_strFSep;
	}
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_QueryFreeResource"))  //10048 查询用户免费资源
	{
		CString strMobileNum;	//手机号码
		CString strUserType;	//用户类型
		
		strMobileNum = GetField(strData,g_strFSep,g_strRSep);
		strUserType = GetField(strData,g_strFSep,g_strRSep);
		
		strData = strMobileNum + g_strFSep + strUserType + g_strFSep;
	}
	
	/************************************************************************/
	/*end: added by tongyufeng,wangyue,2011-11-24                                                                     */
	/************************************************************************/
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_GetMonthFee"))  //1.1.5  某月帐单查询I_SCE_GetMonthFee
	{
		CString strHandsetNo;
		CString strCityCode;
		CString strNumFlag;
		CString strStartTime;
		CString strEndTime;

		strHandsetNo = GetField(strData, g_strFSep, g_strRSep);
		strCityCode = GetField(strData, g_strFSep, g_strRSep);
		strNumFlag = GetField(strData, g_strFSep, g_strRSep);
		strStartTime = GetField(strData, g_strFSep, g_strRSep);
		strEndTime = GetField(strData, g_strFSep, g_strRSep);

		CString strUserType;
		bool bMobileNum = false;
		bMobileNum = IsMobileNumber(strHandsetNo);
		if (true == bMobileNum)   //c网
		{
			strUserType = "1";
		}
		else					  //固网
		{
			strUserType = "10";
		}

		if (!strNumFlag.Compare("1"))
		{
			strData = strHandsetNo + g_strFSep + strUserType + g_strFSep + strStartTime + g_strFSep + "0" + g_strFSep; 	
		}
		else if(!strNumFlag.Compare("2"))
		{
			strData = strHandsetNo + g_strFSep + strUserType + g_strFSep + strStartTime + g_strFSep + "1" + g_strFSep; 	
		}
	}
	
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_GetBalance"))   //1.1.6  余额查询I_SCE_GetBalance
	{
		CString strHandsetNo;
		CString strCityCode;
		CString strNumFlag;

		strHandsetNo = GetField(strData, g_strFSep, g_strRSep);
		strCityCode = GetField(strData, g_strFSep, g_strRSep);
		strNumFlag = GetField(strData, g_strFSep, g_strRSep);

		CString strUserType;
		bool bMobileNum = false;
		bMobileNum = IsMobileNumber(strHandsetNo);
		if (true == bMobileNum)   //c网
		{
			strUserType = "1";
		}
		else					  //固网
		{
			strUserType = "10";
		}

		if (!strNumFlag.Compare("1"))
		{
			strData = strHandsetNo + g_strFSep + strUserType + g_strFSep + "0" + g_strFSep; 	
		}
		else if (!strNumFlag.Compare("2"))
		{
			strData = strHandsetNo + g_strFSep + strUserType + g_strFSep + "1" + g_strFSep; 	
		}		
	}
	
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_HandsetNoTypeJudg"))  //1.1.8  号码有效性校验I_SCE_HandsetNoTypeJudg
	{
		CString strHandsetNo;
		CString strCityCode;
		CString strNumFlag;

		strHandsetNo = GetField(strData, g_strFSep, g_strRSep);
		strCityCode = GetField(strData, g_strFSep, g_strRSep);
		strNumFlag = GetField(strData, g_strFSep, g_strRSep);

		CString strUserType;
		bool bMobileNum = false;
		bMobileNum = IsMobileNumber(strHandsetNo);
		if (true == bMobileNum)   //c网
		{
			strUserType = "1";
		}
		else					  //固网
		{
			strUserType = "10";
		}
		
		if (!strNumFlag.Compare("1"))
		{
			strData = strHandsetNo + g_strFSep + strUserType + g_strFSep ; 	
		}		
	}
	
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_CHKNumMatch"))  //1.1.9  帐号和号码匹配校验：I_SCE_CHKNumMatch
	{
		CString strHandsetNo;
		CString strCityCode;
		CString strContractNo;

		strHandsetNo = GetField(strData, g_strFSep, g_strRSep);
		strCityCode = GetField(strData, g_strFSep, g_strRSep);
		strContractNo = GetField(strData, g_strFSep, g_strRSep);
		
		CString strUserType;
		bool bMobileNum = false;//对bMobileNum的初始化
		bMobileNum = IsMobileNumber(strHandsetNo);
		if (true == bMobileNum)   //c网
		{
			strUserType = "1";
		}
		else					  //固网
		{
			strUserType = "10";
		}

		strData = strHandsetNo + g_strFSep + strUserType + g_strFSep + strContractNo + g_strFSep;		
	}

	
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_RealTimeFee"))  //1.1.10  实时费用查询I_SCE_RealTimeFee
	{
		CString strHandsetNo;
		CString strCityCode;
		CString strNumFlag;

		strHandsetNo = GetField(strData, g_strFSep, g_strRSep);
		strCityCode = GetField(strData, g_strFSep, g_strRSep);
		strNumFlag = GetField(strData, g_strFSep, g_strRSep);

		CString strUserType;
		bool bMobileNum = false;
		bMobileNum = IsMobileNumber(strHandsetNo);
		if (true == bMobileNum)   //c网
		{
			strUserType = "1";
		}
		else					  //固网
		{
			strUserType = "10";
		}

		if (!strNumFlag.Compare("1"))
		{
			strData = strHandsetNo + g_strFSep + strUserType + g_strFSep + "0" + g_strFSep; 	
		}
		else if (!strNumFlag.Compare("2"))
		{
			strData = strHandsetNo + g_strFSep + strUserType + g_strFSep + "1" + g_strFSep; 	
		}		
	}

	
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_Login")
		|| !_stricmp(pConfig->pICDCMD, "P_SCELogin"))  //1.1.11  用户密码验证I_SCE_Login
	{
		CString strHandsetNo;
		CString strCityCode;
		CString strPassword;
		CString strUserType;

		strHandsetNo = GetField(strData, g_strFSep, g_strRSep);
		strCityCode = GetField(strData, g_strFSep, g_strRSep);
		strPassword = GetField(strData, g_strFSep, g_strRSep);
		strUserType = GetField(strData, g_strFSep, g_strRSep);

		bool bMobileNum = false;
		bMobileNum = IsMobileNumber(strHandsetNo);
		if (true == bMobileNum)   //c网
		{
			strUserType = "4";
		}
		else					  //固网
		{
			strUserType = "2";
			//如果固话号码长度<=8，则加上区号
			if (strHandsetNo.GetLength()<=8)
			{
				strHandsetNo = strCityCode + strHandsetNo;
			}
		}

		strData = strHandsetNo + g_strFSep + strUserType + g_strFSep + strPassword + g_strFSep + strCityCode + g_strFSep;
	}
	//10033和10046接口合并
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_GetSubscriberNCustomer")
		|| !_stricmp(pConfig->pICDCMD, "I_SCE_QueryBroadNoByAccount"))
	{
		CString strServiceNoo;
		CString strCityCode;
		CString strQueryType;

		strServiceNoo = GetField(strData, g_strFSep, g_strRSep);
		strCityCode = GetField(strData, g_strFSep, g_strRSep);
		strQueryType = GetField(strData, g_strFSep, g_strRSep);

		CString strUserType;
		bool bMobileNum = false;
		bMobileNum = IsMobileNumber(strServiceNoo);
		if (true == bMobileNum)   //c网
		{
			strUserType = "1";
		}
		else					  //固网
		{
			strUserType = "10";
		}

		strData = strServiceNoo + g_strFSep + strUserType + g_strFSep;		
	}

	//add by lilong 20090430 begin
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_GetUserinfor1") || !_stricmp(pConfig->pICDCMD, "I_SCE_GetUserinfor2"))  //1.1.1  用户级别I_SCE_GetUserinfor
	{
		CString strServiceNo;
		CString strCallerNo;
		CString strCalleeNo;
		CString strCityCode;
		
		if (!_stricmp(pConfig->pICDCMD, "I_SCE_GetUserinfor1"))
		{
			strServiceNo = GetField(strData, g_strFSep, g_strRSep);
			strCallerNo = GetField(strData, g_strFSep, g_strRSep);
			strCalleeNo = GetField(strData, g_strFSep, g_strRSep);
			strCityCode = GetField(strData, g_strFSep, g_strRSep);
		}
		else
		{
			strServiceNo = GetField(strData, g_strFSep, g_strRSep);
			strCallerNo = strServiceNo;
			strCalleeNo = GetField(strData, g_strFSep, g_strRSep);
			strCityCode = GetField(strData, g_strFSep, g_strRSep);
		}
		
		CString strUserType;
		bool bMobileNum = false;
		bMobileNum = IsMobileNumber(strServiceNo);
		
		if (true == bMobileNum)   //c网
		{
			strUserType = "1";
		}
		else					  //固网
		{
			strUserType = "10";
		}
		
		strData = strServiceNo + g_strFSep + strUserType + g_strFSep;
 
	}
    //add by lilong 20090430 end


    else
    {
        // 默认为IVR发来的存储过程的调用参数个数以及类别都和MID所定义的一致
        CString strOldData;
        int     ii = 0;
		
        strOldData = strData;
        strData.Empty();
        while (strOldData.GetLength())
        {
            CString strTmp;
			CString strField;
			
			// 取出每一个字段按照指定的格式格式化
            strField = GetField(strOldData, g_strFSep, g_strRSep);
			strTmp.Format(pConfig->pFormat[ii], strField);
            strData += strTmp;
            strData += g_strFSep;
            ii++;
        }
    }
	
    // 将末尾的字段分割符替换称为记录分割符
    int nCount = strData.GetLength() - g_strFSep.GetLength();
    strData.Delete(nCount, g_strFSep.GetLength());
    strData += g_strRSep;
	
    // 填写报文结构
    pApply->dwLen       = htonl(sizeof(APPLY_MSG) + strData.GetLength());
    pApply->byFactory   = 168;
    pApply->byProgID    = 12;
    pApply->byMorePkt   = 0;
    strcpy(pApply->pCMD, strMIDCMD);
    pApply->dwStartNum  = 0;
    pApply->dwEndNum    = 0;
    pApply->dwRequestID = dwSeq;
    pApply->dwAnswerID  = 0;
    pApply->dwSeq       = 0;
    strcpy(pApply->pRecSep, g_strRSep);
    strcpy(pApply->pFieldSep, g_strFSep);
    pApply->dwReserved1 = 0;
    pApply->dwReserved2 = 0;
    pApply->nErrCode    = 0;
    strcpy(pApply->pData, strData);
	
    dwTransLen = htonl(pApply->dwLen);
    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//  Func:       
//  Describe:   将AppSvr类型的消息转换为外部接口协议
//  Input:      
//              pBuf:       消息指针
//              dwLen:      消息长度
//              dwTransLen: pTrans缓冲区的大小
//              dwIndexLen: pSeqIndex缓冲区的大小
//  Output:
//              pTrans:     转换后的消息指针
//              dwTransLen: 转换后的消息长度
//              pSeqIndex:  生成的流水号指针
//              dwIndexLen: 生成的流水号的长度
//  Return:
//              TRUE:       转换成功
//              FALSE:      转换失败
//  Note:       dwTransLen, dwIndexLen都属于输入输出参数. 
///////////////////////////////////////////////////////////////////////////////
BOOL __stdcall
DllTransAppSvrMsg(char *pBuf, DWORD dwLen, char *pTrans, DWORD &dwTransLen, 
				  char *pSeqIndex, DWORD &dwIndexLen)
{
	LPAPPLY_MSG     pDst;
	LPAPPLY_MSG		pAppSvr;
	LPPARAM_CONFIG	pConfig;
	DWORD			dwSeq;
	
    InitDll();
    pAppSvr = (LPAPPLY_MSG)&pBuf[sizeof(MSG_ADDRESS)];
    pDst    = (LPAPPLY_MSG)pTrans;
	
    if (dwTransLen < dwLen - sizeof(MSG_ADDRESS))    
    {
        return FALSE;   // 输出缓冲区太小
    }
	
    // 获取参数的配置说明
    pConfig = GetParamConfigFromICDName(pAppSvr->pCMD);
	if (!pConfig)
    {						
        return FALSE;   // 未知命令字
    }
	
    // 生成流水号
    if (!MakeSequence(pSeqIndex, dwIndexLen))
    {
        return FALSE;   // 流水号生成失败
    }
    dwSeq = *(DWORD *)pSeqIndex;
	
    // 增加返回格式
    CString strData;
    CString strISDN;
    CString strWorkNo;
	CString strSrcData;
	CString strDstData;
	char    pData[24*1024];
	DWORD	dwDataLen;
    
    // 取出数据段
	dwDataLen = htonl(pAppSvr->dwLen) - sizeof(APPLY_MSG) + 1;
	if (dwDataLen >= 24*1024)
	{
		return FALSE;
	}
	
	
	memcpy(pData, pAppSvr->pData, dwDataLen);
	pData[dwDataLen] = 0;
    strSrcData = pData;
	
	//写日志
	NewWriteLog(strSrcData, "DllTransAppSvrMsg", pAppSvr->pCMD, dwSeq);
	


	//原固网部分

	if (!_stricmp(pAppSvr->pCMD, "10022"))//10022 用户基本信息查询
	{
		CString strHandsetNo;//用户手机号码
		strHandsetNo = GetField(strSrcData, g_strFSep, g_strRSep);

		CString strUserType;
		bool bMobileNum = false;
		bMobileNum = IsMobileNumber(strHandsetNo);
		if (true == bMobileNum)   //c网
		{
			strUserType = "1";
		}
		else					  //固网
		{
			strUserType = "10";
		}
		if (strlen(strHandsetNo)<9)
		{
			strHandsetNo="0371"+strHandsetNo;			
		}

		//modify by tongyufeng 20100827,为了宽带号码可以查询，去掉了usertype，CRM已经支持！
		strDstData = strHandsetNo + g_strFSep + strUserType + g_strRSep;
		//strDstData = strHandsetNo + g_strFSep;

	}

	else if (!_stricmp(pAppSvr->pCMD, "10033"))//(CRM接口)+10033获取产品ID、帐户ID和客户ID
	{
		CString strHandsetNo;//用户手机号码
		strHandsetNo = GetField(strSrcData, g_strFSep, g_strRSep);

		CString strUserType;
		bool bMobileNum = false;
		bMobileNum = IsMobileNumber(strHandsetNo);
		if (true == bMobileNum)   //c网
		{
			strUserType = "1";
		}
		else					  //固网
		{
			strUserType = "10";
		}

		strDstData = strHandsetNo + g_strFSep + strUserType + g_strRSep;
	}

	else if (!_stricmp(pAppSvr->pCMD, "10035"))//(CRM接口)+10035用户产品资料查询
	{
		CString strUserFlag;//用户标识
		strUserFlag = GetField(strSrcData, g_strFSep, g_strRSep);
		
		strDstData = strUserFlag + g_strRSep;
	}

	// add by tongyufeng,20101228,增加SP自动业务办理接口
	else if (!_stricmp(pAppSvr->pCMD, "10041"))//自动业务办理功能
	{

		CString strHandsetNo; //用户号码
		CString strServiceType;  //业务类型，0-七彩铃音  1-来电显示 2-呼叫保持 3-呼叫等待
		CString strOptType;  //操作类型，1-登记   3-取消
		CString strUserType; //用户类型，1-手机，10-固话
		
		strHandsetNo = GetField(strSrcData, g_strFSep, g_strRSep);
        strUserType = GetField(strSrcData, g_strFSep, g_strRSep);
		strServiceType = GetField(strSrcData, g_strFSep, g_strRSep);
		strOptType = GetField(strSrcData, g_strFSep, g_strRSep);
		
		strDstData = strHandsetNo + g_strFSep + strUserType + g_strFSep + strServiceType + g_strFSep 
			+ strOptType + g_strRSep;
	}
    //add by tongyufeng,2011-02-21
	else if (!_stricmp(pAppSvr->pCMD, "10045"))//所有话费一次返回，短信使用 
	{
		
		CString strHandsetNo; //用户号码
		CString strServiceType;  //业务类型
		
		strHandsetNo = GetField(strSrcData, g_strFSep, g_strRSep);
		strServiceType = GetField(strSrcData, g_strFSep, g_strRSep);
		
		strDstData = strHandsetNo + g_strFSep + strServiceType + g_strRSep;
	}

	else if (!_stricmp(pAppSvr->pCMD, "10034"))//2.9	 (CRM接口)+10034 修改客户密码
	{
		CString strClientFlag;//客户标识
		CString strOldPassword;//旧密码
		CString strNewPassword;//新密码

		strClientFlag = GetField(strSrcData, g_strFSep, g_strRSep);
		strOldPassword = GetField(strSrcData, g_strFSep, g_strRSep);
		strNewPassword = GetField(strSrcData, g_strFSep, g_strRSep);
		
		strDstData = strClientFlag + g_strFSep + strOldPassword + g_strFSep + strNewPassword + g_strRSep;
	}

	else if (!_stricmp(pAppSvr->pCMD, "10015"))//2.4	(CRM接口)10015 修改用户密码
	{
		CString strNumber;//电话号码
		CString strOldPassword;//旧密码
		CString strNewPassword;//新密码

		strNumber = GetField(strSrcData, g_strFSep, g_strRSep);
		strOldPassword = GetField(strSrcData, g_strFSep, g_strRSep);
		strNewPassword = GetField(strSrcData, g_strFSep, g_strRSep);

		CString strUserType;//用户类型
		bool bMobileNum = false;
		bMobileNum = IsMobileNumber(strNumber);
		if (true == bMobileNum)   //c网
		{
			strUserType = "1";
		}
		else					  //固网
		{
			strUserType = "10";
		}
		
		strDstData = strNumber + g_strFSep 
				   + strUserType + g_strFSep 
				   + strOldPassword + g_strFSep
				   + strNewPassword + g_strRSep;
	}

	else if (!_stricmp(pAppSvr->pCMD, "10036"))//(CRM接口)+10036客户产品资料查询
	{
		CString strUserFlag;//用户标识
		
		strUserFlag = GetField(strSrcData, g_strFSep, g_strRSep);

		strDstData = strUserFlag + g_strRSep;
	}

	else if (!_stricmp(pAppSvr->pCMD, "10037"))//2.12	 (CRM接口)+10037 帐户信息查询
	{
		CString strUserFlag;//用户标识
		
		strUserFlag = GetField(strSrcData, g_strFSep, g_strRSep);
		
		strDstData = strUserFlag + g_strRSep;
	}

	else if (!_stricmp(pAppSvr->pCMD, "10038"))//2.13	 (CRM接口)+10038 套餐信息查询
	{
		CString strUserFlag;//用户标识
		
		strUserFlag = GetField(strSrcData, g_strFSep, g_strRSep);
		
		strDstData = strUserFlag + g_strRSep;
	}
	/************************************************************************/
	/* added by wangyue,2011-11-30 10046、10047的人工接口                                                                     */
	/************************************************************************/
	else if (!_stricmp(pAppSvr->pCMD, "10046"))//2.18	10046查询同账户下的宽带号码
	{
		CString strAccountID;
		strAccountID = GetField(strSrcData,g_strFSep,g_strRSep);

		strDstData = strAccountID + g_strRSep;
	}
	else if (!_stricmp(pAppSvr->pCMD, "10047"))//2.19	10047查询同身份证办理的宽带号码
	{
		CString strIDCard;
		strIDCard = GetField(strSrcData,g_strFSep,g_strRSep);
		
		strDstData = strIDCard + g_strRSep;
	}
	else                          
    {
        strDstData = strSrcData;
    }
	
    // 替换流水号, 更新数据段, 长度重新计算
    memcpy(pDst, pAppSvr, sizeof(APPLY_MSG));
	strcpy(pDst->pCMD, pConfig->pMIDCMD);		//20030420 LIUSHAOHUA
    pDst->dwRequestID = dwSeq;
    strcpy(pDst->pData, strDstData);
    pDst->dwLen = htonl(sizeof(APPLY_MSG) + strDstData.GetLength());
    dwTransLen = htonl(pDst->dwLen);
	
    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//  Func:       
//  Describe:   判断是请求应答消息还是无请求应答消息
//  Input:      
//              pBuf:       消息指针
//              dwLen:      消息长度
//  Output:
//  Return:
//              TRUE:       请求应答消息
//              FALSE:      无请求应答消息
//  Note:       
///////////////////////////////////////////////////////////////////////////////
BOOL __stdcall
DllIsApplyReplyMsg(char *pBuf, DWORD dwLen)
{
    InitDll();
    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//  Func:       
//  Describe:   根据原始请求消息将第三方前置机发来的应答消息转换成IVR类型或者
//              AppSvr类型的消息
//  Input:      
//              pBuf:       消息指针
//              dwLen:      消息长度
//              pApply:     原始请求消息指针
//              dwApplyLen: 原始请求消息长度
//              dwMsgLen:   pMsg缓冲区长度
//  Output:
//              pMsg:       转换后的消息指针
//              dwMsgLen:   转换后的消息长度
//              bEndReply:  应答结束标志
//  Return:
//              TRUE:       转换成功
//              FALSE:      转换失败
//  Note:       
///////////////////////////////////////////////////////////////////////////////
BOOL __stdcall
DllTransReplyMsg(char *pBuf, DWORD dwLen, char *pApply, DWORD dwApplyLen, char *pMsg, 
                 DWORD &dwMsgLen, BOOL &bEndReply)
{
    LPAPPLY_MSG	    pReply;
    LPSP_CALL       pSrcSPCall;
    LPSP_CALL_ACK   pSPCallAck;
   	LPPARAM_CONFIG  pConfig;	
	LPMSG_ADDRESS	pAddr;
	char            *pData;
    char			pAck[24*1024]; 
    char            pSrcReplyData[32*1024];
    CString         strSrcReplyData;
    CString         strTran;
	
    InitDll();
	
	pAddr = (LPMSG_ADDRESS)pApply;
	if (!pAddr->bIVRMsg)
	{
		return TransToAppSvrMsg(pBuf, dwLen, pApply, dwApplyLen, pMsg, dwMsgLen, bEndReply);
	}
	
	pReply = (LPAPPLY_MSG)&pBuf[sizeof(MSG_ADDRESS)];
	pSrcSPCall = (LPSP_CALL)&pApply[sizeof(MSG_ADDRESS)];
    pSPCallAck  = (LPSP_CALL_ACK)pAck;
	
		  // 复制参数描述区以及参数区
	//=============高传俊20081201为处理短信增加=========
	//	LPSP_CALL pSPCall2;
    LPSP_PARAM pParam2;
	LPSP_CALL pSrcSPCall2 = pSrcSPCall;
    char *pSrcData2;
    CString         strData2;
	pParam2 = &pSrcSPCall2->SPPara;
    pSrcData2 = (char *)pParam2 + sizeof(SP_PARAM) * pSrcSPCall2->byParaNum;
    for (int ii = 0; ii < pSrcSPCall2->byParaNum; ii++)
    {
        // 不转换输出参数, 按照默认格式转换
        if (pParam2->byParaType != OUTPUT_PARAM)
        {
            strData2 += TransParamToString(pParam2, pSrcData2);                
            strData2 += g_strFSep;
        }
		
        pSrcData2 += pParam2->wDataLen;
        pParam2++;
    }
	//=============
	
	memcpy(pSPCallAck, pSrcSPCall, sizeof(IVR_MSG_HEAD));	//复制消息包包头结构
	
    pSPCallAck->IVRHead.bySendNode = pSrcSPCall->IVRHead.byRecvNode;
    pSPCallAck->IVRHead.bySendPort = pSrcSPCall->IVRHead.byRecvPort;
	pSPCallAck->IVRHead.byRecvNode = pSrcSPCall->IVRHead.bySendNode;
	pSPCallAck->IVRHead.byRecvPort = pSrcSPCall->IVRHead.bySendPort;
	
	pSPCallAck->IVRHead.wMsgType   = MSG_SPCALL_ACK;	//由中间件返回的消息	
	
	pSPCallAck->wStatus = 0;	//执行成功
	
	memcpy(pSPCallAck->pDataSource, pSrcSPCall->pDataSource, sizeof(pSrcSPCall->pDataSource));
	memcpy(pSPCallAck->pUserID, pSrcSPCall->pUserID, sizeof(pSrcSPCall->pUserID));
	memcpy(pSPCallAck->pProcName, pSrcSPCall->pProcName, sizeof(pSrcSPCall->pProcName));
	
	pSPCallAck->byParaNum = pSrcSPCall->byParaNum;
	
	
	
	SP_PARAM* pPara = (SP_PARAM *)&pSrcSPCall->SPPara;
	
   	int iParaLen = pSrcSPCall->byParaNum * sizeof(SP_PARAM);	//参数定义长度
	for (int i = 0; i < pSrcSPCall->byParaNum; i++ )	        
	{
		iParaLen += pPara->wDataLen;                            //取得所有参数的长度
		pPara++;
	}	
	
	memcpy((LPSTR)&pSPCallAck->SPPara, (LPSTR)&pSrcSPCall->SPPara, iParaLen);
	
    // 获取返回的数据段内容
    DWORD dwDataLen = htonl(pReply->dwLen) - sizeof(APPLY_MSG) + 1;

	pConfig = GetParamConfigFromICDName(pSrcSPCall->pProcName);

    memcpy(pSrcReplyData, pReply->pData, dwDataLen);
    pSrcReplyData[dwDataLen] = 0;
    strSrcReplyData = pSrcReplyData;
	
	////////////////////////////////////
	if (!_stricmp(pConfig->pICDCMD, "P_SCEGetUserPaytype")) //获取用户付费类型
	{
		CString strRet;
		strRet = GetField(strSrcReplyData, g_strRSep, g_strFSep);

		strTran = strRet;
	}
	else if (!_stricmp(pConfig->pICDCMD, "P_SCEHandsetNoTypeJudg"))// 手机号码验证
	{
		CString strRet;
		strRet = GetField(strSrcReplyData, g_strRSep, g_strFSep);
		
		strTran = strRet;		
	}
	//modified by wangyue 20130318,将I_SCE_Login/P_SCELogin两个接口合并
	//else if (!_stricmp(pConfig->pICDCMD, "P_SCELogin")) //密码验证
	//{
	//	CString strRet;
	//	strRet = GetField(strSrcReplyData, g_strRSep, g_strFSep);

	//	strTran = strRet;
	//}

	// add by tongyufeng,20101228,增加SP自动业务办理接口 
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_AutoService")) //SP程控业务办理
	{
		CString strInnerXML;
		CString strResult;
		CMarkup xml;
		CString strANSIData;
		strANSIData = UTF8ToANSI(strSrcReplyData);

		//创建xml对象
		if (xml.SetDoc(strANSIData))
		{
			if (xml.FindElem_C(_T("/soap:Envelope/soap:Body/ns1:orderServiceFor10000Response/ns1:out")))
			{
				strInnerXML = xml.GetData();
				if (xml.SetDoc(strInnerXML))
				{
					if(xml.FindElem_C(_T("/orderServiceFor10000Response/result")))
					{
						strResult = xml.GetData();
						if (!strResult.Compare(_T("0")))
						{
							strSrcReplyData = _T("0;");
						}
						else
						{
							strSrcReplyData = _T("111;");
						}
					}
					else
					{
						strSrcReplyData = _T("111;");
					}
				}
				else
				{
					strSrcReplyData = _T("111;");
				}
			}
			else
			{
				strSrcReplyData = _T("111;");
			}
		}
		else
		{
			strSrcReplyData = _T("111;");
		}
		//////////////////////////////////////////////////////////////////////////
		CString strRet;
		strRet = GetField(strSrcReplyData, g_strRSep, g_strFSep);
		strTran = strRet;
	}

	// add by tongyufeng,2011-02-21,增加自动短信命令字接口
	/************************************************************************/
	/* date:    20130328
	   author:	wangyue
	   purpose: modified,convert xml messages to former format*/
	/************************************************************************/
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_AllFeeSms")) //SP程控业务办理
	{
		/*CString strInnerXML;
		CString strResult;
		CMarkup xml;
		CString strANSIData;
		strANSIData = UTF8ToANSI(strSrcReplyData);

		//创建xml对象
		if (xml.SetDoc(strANSIData))
		{
			if (xml.FindElem_C(_T("/soap:Envelope/soap:Body/ns1:PassWDMaintenanceResponse/ns1:out")))
			{
				strInnerXML = xml.GetData();
				if (xml.SetDoc(strInnerXML))
				{
					if(xml.FindElem_C(_T("/PassWDMaintenanceResponse/result")))
					{
						strResult = xml.GetData();
						if (!strResult.Compare(_T("0")))
						{
							strSrcReplyData = _T("0;");
						}
						else
						{
							strSrcReplyData = _T("111;");
						}
					}
					else
					{
						strSrcReplyData = _T("111;");
					}
				}
			}
			else
			{
				strSrcReplyData = _T("111;");
			}
		}
		else
		{
			strSrcReplyData = _T("111;");
		}*/
		//////////////////////////////////////////////////////////////////////////
		CString strResult;
		CString strCurrentFee;//当前话费
		CString strDiscount;//优惠金额
		CString strRealFee;//实收话费
		CString strBalance;//余额
		CString strOwed;//欠费
		//////////////////////////////////////////////////////////////////////////
		//去掉03d4..
		strSrcReplyData = strSrcReplyData.Right(strSrcReplyData.GetLength()-6);
		//取result
		strResult = GetElementValue(strSrcReplyData,_T("result"));
		if (!strResult.Compare(_T("0")))
		{
			//取currentFee
			strCurrentFee= GetElementValue(strSrcReplyData,_T("currentFee"));
			//取discount
			strDiscount = GetElementValue(strSrcReplyData,_T("discount"));
			//取realFee
			strRealFee = GetElementValue(strSrcReplyData,_T("realFee"));
			//取balance
			strBalance = GetElementValue(strSrcReplyData,_T("balance"));
			//取owedfee
			strOwed = GetElementValue(strSrcReplyData,_T("owedfee"));
			//组合成原来的报文格式
			strSrcReplyData = _T("0;") + strCurrentFee + _T("~") + strDiscount + _T("~") + strRealFee
				+ _T("~") + strBalance + _T("~") + strOwed;
		}
		else
		{
			strSrcReplyData = _T("111;");
		}
		//////////////////////////////////////////////////////////////////////////
		CString strRet;
		strRet = GetField(strSrcReplyData, g_strRSep, g_strFSep);

		if(!_stricmp(strRet,"0"))
		{
			strTran="0~";

			CString strMoney;    //话费信息

			for (int i=0;i<5;i++)
			{
				strMoney = GetField(strSrcReplyData, g_strRSep, g_strFSep);
				strMoney.TrimLeft(' ');
				strMoney.TrimRight(' ');

				double dYuan = 0.00;
				dYuan = atof(strMoney)/100.00;  //分变成元
				CString strMoneyYuan;
		     	strMoneyYuan.Format("%.2f",dYuan);

				strTran+=strMoneyYuan+g_strFSep;
			}
		}
		else
		{
			strTran = _T("111~");
		}
		
	}
	/************************************************************************/
	/* date:    20130328
	author:	wangyue
	purpose: modified,convert xml messages to former format*/
	/************************************************************************/
	else if (!_stricmp(pConfig->pICDCMD, "P_SCEChangePassword")
		|| !_stricmp(pConfig->pICDCMD, "I_SCE_PasswdReset")) //修改密码
	{
		CString strInnerXML;
		CString strResult;
		CMarkup xml;
		CString strANSIData;
		strANSIData = UTF8ToANSI(strSrcReplyData);

		//创建xml对象
		if (xml.SetDoc(strANSIData))
		{
			if (xml.FindElem_C(_T("/soap:Envelope/soap:Body/ns1:PassWDMaintenanceResponse/ns1:out")))
			{
				strInnerXML = xml.GetData();
				if (xml.SetDoc(strInnerXML))
				{
					if(xml.FindElem_C(_T("/PassWDMaintenanceResponse/result")))
					{
						strResult = xml.GetData();
						if (!strResult.Compare(_T("0")))
						{
							strSrcReplyData = _T("0;");
						}
						else
						{
							strSrcReplyData = _T("111;");
						}
					}
					else
					{
						strSrcReplyData = _T("111;");
					}
				}
			}
			else
			{
				strSrcReplyData = _T("111;");
			}
		}
		else
		{
			strSrcReplyData = _T("111;");
		}
		//////////////////////////////////////////////////////////////////////////
		CString strRet;
		strRet = GetField(strSrcReplyData, g_strRSep, g_strFSep);

		strTran = strRet;
	}
	else if (!_stricmp(pConfig->pICDCMD, "P_SCEResetPassword")) //重置密码
	{
		CString strRet;
		strRet = GetField(strSrcReplyData, g_strRSep, g_strFSep);

		strTran = strRet;
	}
	/************************************************************************/
	/* date:    20130328
	   author:	wangyue
	   purpose: modified,convert xml messages to former format*/
	/************************************************************************/
	else if (!_stricmp(pConfig->pICDCMD, "P_SCEGetTotalFee"))  //月结话费查询(后付费)
	{
		//业务号码
		CString strNo;
		//账务周期
		CString strBillingCycle;
		//错误代码
		CString strCode;
		//错误信息
		CString strMsg;
		//查询结果
		CString strResult;
		//总费用
		CString strTotalFee;
		BILLITEM ArrItem[78]={
			BILLITEM(_T("月基本费"),_T("251001"),_T("0")),BILLITEM(_T("语音通话费"),_T("251002"),_T("0")),
				BILLITEM(_T("综合信息服务费"),_T("251003"),_T("0")),BILLITEM(_T("代收费"),_T("251004"),_T("0")),
				BILLITEM(_T("补收费"),_T("251005"),_T("0")),BILLITEM(_T("个人代付"),_T("251006"),_T("0")),
				BILLITEM(_T("违约金"),_T("251007"),_T("0")),
				BILLITEM(_T("其他"),_T("251008"),_T("0")),BILLITEM(_T("固_11808费用"),_T("252001"),_T("0")),
				BILLITEM(_T("固_12361捐款费"),_T("252002"),_T("0")),BILLITEM(_T("固_4008长途费"),_T("252003"),_T("0")),
				BILLITEM(_T("固_4008市话费"),_T("252004"),_T("0")),BILLITEM(_T("固_800长途费"),_T("252005"),_T("0")),
				BILLITEM(_T("固_800市话费"),_T("252006"),_T("0")),BILLITEM(_T("固_IP港澳台长途"),_T("252007"),_T("0")),
				BILLITEM(_T("固_IP国际长途"),_T("252008"),_T("0")),BILLITEM(_T("固_IP国内长途"),_T("252009"),_T("0")),
				BILLITEM(_T("固_VOD点播费"),_T("252010"),_T("0")),BILLITEM(_T("固_彩铃下载费"),_T("252011"),_T("0")),
				BILLITEM(_T("固_超级一号通送话费"),_T("252012"),_T("0")),BILLITEM(_T("固_附加收费终端月租"),_T("252013"),_T("0")),
				BILLITEM(_T("固_港澳台长途费"),_T("252014"),_T("0")),BILLITEM(_T("固_国际长途费"),_T("252015"),_T("0")),
				BILLITEM(_T("固_国内长途费"),_T("252016"),_T("0")),BILLITEM(_T("固_宽带设备租用月租费"),_T("252017"),_T("0")),
				BILLITEM(_T("固_宽带通信费"),_T("252018"),_T("0")),BILLITEM(_T("固_宽带月租费"),_T("252019"),_T("0")),
				BILLITEM(_T("固_七彩铃音信息费"),_T("252020"),_T("0")),BILLITEM(_T("固_区间通话费"),_T("252021"),_T("0")),
				BILLITEM(_T("固_区内通话费"),_T("252022"),_T("0")),BILLITEM(_T("固_数据通信费"),_T("252023"),_T("0")),
				BILLITEM(_T("固_网络使用费"),_T("252024"),_T("0")),BILLITEM(_T("固_新业务开户费"),_T("252026"),_T("0")),
				BILLITEM(_T("固_信息费"),_T("252027"),_T("0")),BILLITEM(_T("固_预存款"),_T("252028"),_T("0")),
				BILLITEM(_T("固_预付费补足最低消费"),_T("252029"),_T("0")),BILLITEM(_T("固_预付费送话费"),_T("252030"),_T("0")),
				BILLITEM(_T("固_月租费"),_T("252031"),_T("0")),BILLITEM(_T("固_滞纳金"),_T("252032"),_T("0")),
				BILLITEM(_T("固_终端设备费"),_T("252033"),_T("0")),BILLITEM(_T("固_装移机费"),_T("252034"),_T("0")),
				BILLITEM(_T("固_优惠费用"),_T("252035"),_T("0")),BILLITEM(_T("商务领航业务费"),_T("252036"),_T("0")),
				BILLITEM(_T("长途包功能费"),_T("252037"),_T("0")),BILLITEM(_T("VPN业务使用费"),_T("252038"),_T("0")),
				BILLITEM(_T("来电显示费"),_T("252039"),_T("0")),BILLITEM(_T("呼出限制费"),_T("252040"),_T("0")),
				BILLITEM(_T("三方通话费"),_T("252041"),_T("0")),BILLITEM(_T("闹钟服务费"),_T("252042"),_T("0")),
				BILLITEM(_T("话费包天功能费"),_T("252043"),_T("0")),BILLITEM(_T("七彩铃音费"),_T("252044"),_T("0")),
				BILLITEM(_T("亲情包功能费"),_T("252045"),_T("0")),BILLITEM(_T("加装包功能费"),_T("252046"),_T("0")),
				BILLITEM(_T("协同通信费"),_T("252047"),_T("0")),BILLITEM(_T("本地语音包功能费"),_T("252048"),_T("0")),
				BILLITEM(_T("GC业务功能费"),_T("252049"),_T("0")),BILLITEM(_T("彩铃月租费"),_T("252050"),_T("0")),
				BILLITEM(_T("畅听功能使用费"),_T("252051"),_T("0")),BILLITEM(_T("话费提醒功能费"),_T("252052"),_T("0")),
				BILLITEM(_T("家家乐功能费"),_T("252053"),_T("0")),BILLITEM(_T("来电显示费"),_T("252054"),_T("0")),
				BILLITEM(_T("漫游包使用费"),_T("252055"),_T("0")),BILLITEM(_T("网络使用费"),_T("252056"),_T("0")),
				BILLITEM(_T("夜话聊吧功能费"),_T("252057"),_T("0")),BILLITEM(_T("短信包月功能费"),_T("252058"),_T("0")),
				BILLITEM(_T("彩信套餐费"),_T("252059"),_T("0")),BILLITEM(_T("销售管家月使用费"),_T("252060"),_T("0")),
				BILLITEM(_T("天翼对讲功能费"),_T("252061"),_T("0")),BILLITEM(_T("漫游包功能费"),_T("252062"),_T("0")),
				BILLITEM(_T("WLAN包月功能费"),_T("252063"),_T("0")),BILLITEM(_T("其它功能费"),_T("252064"),_T("0")),
				BILLITEM(_T("VPN业务使用费"),_T("252065"),_T("0")),BILLITEM(_T("短信包月功能费"),_T("252066"),_T("0")),
				BILLITEM(_T("漫游包功能费"),_T("252067"),_T("0")),BILLITEM(_T("电信e通功能费"),_T("252068"),_T("0")),
				BILLITEM(_T("司法e通功能费"),_T("252069"),_T("0")),BILLITEM(_T("亲情包功能费"),_T("252070"),_T("0")),
				BILLITEM(_T("优惠费用"),_T("260004"),_T("0"))
			
		};
		//去掉03d4..
		strSrcReplyData = strSrcReplyData.Right(strSrcReplyData.GetLength()-6);
		CMarkup xml;
		if (xml.SetDoc(strSrcReplyData))
		{
			//取result字段
			if(xml.FindElem_C(_T("/soapenv:Envelope/soapenv:Body/multiRef[@id='id0']/result")))
			{
				strResult = xml.GetData();
				if (!strResult.Compare(_T("0")))
				{
					//取accNbr字段
					if(xml.FindElem_C(_T("/soapenv:Envelope/soapenv:Body/multiRef[@id='id0']/accNbr")))
					{
						strNo = xml.GetData();
					}
					//取billingCycle字段
					if(xml.FindElem_C(_T("/soapenv:Envelope/soapenv:Body/multiRef[@id='id0']/billingCycle")))
					{
						strBillingCycle = xml.GetData();
					}
					//取totalAmount字段
					if(xml.FindElem_C(_T("/soapenv:Envelope/soapenv:Body/multiRef[@id='id0']/totalAmount")))
					{
						strTotalFee = xml.GetData();
						strTotalFee = FenToYuan(strTotalFee);
					}
					//取userBillInfoArray字段
					if(xml.FindElem_C(_T("/soapenv:Envelope/soapenv:Body/multiRef[@id='id0']/userBillInfoArray")))
					{
						int nAmountOfItem = 0;
						CString strAttr = xml.GetAttrib(_T("soapenc:arrayType"));
						strAttr = strAttr.Mid(strAttr.Find(_T("["))+1,strAttr.GetLength()-strAttr.Find(_T("["))-2);
						nAmountOfItem = atoi(strAttr);
						//循环取nAmountOfItem个账单项，并在BILLITEM ArrItem[76]数组中，将对应的账单项填充
						for (int i=1;i<=nAmountOfItem;i++)
						{
							CString strTmp_1,strTmp_2;
							CString strItemAmount;
							CString strItemID;
							strTmp_1.Format(_T("/soapenv:Envelope/soapenv:Body/multiRef[@id='id%d']/amount"),i);
							strTmp_2.Format(_T("/soapenv:Envelope/soapenv:Body/multiRef[@id='id%d']/billItemTypeId"),i);
							if(xml.FindElem_C(strTmp_1))
							{
								strItemAmount = xml.GetData();
							}
							if(xml.FindElem_C(strTmp_2))
							{
								strItemID = xml.GetData();
							}
							
							strItemAmount = FenToYuan(strItemAmount);

							//将取到的账单项金额根据账单ID，填入BILLITEM ArrItem[78]数组中
							for (int j=0;j<78;j++)
							{
								if (!ArrItem[j].strID.Compare(strItemID))
								{
									ArrItem[j].strFee = strItemAmount;
									break;
								}
							}
						}
						//////////////////////////////////////////////////////////////////////////
						//将解析出来的账单项组合成1~0~2形式，如果金额为0就填写0;
						strSrcReplyData=_T("0;")+strTotalFee;
						for (int k=0;k<78;k++)
						{
							strSrcReplyData += _T("~");
							strSrcReplyData += ArrItem[k].strFee;
						}
					}
				}
				else
				{
					strSrcReplyData = _T("111;");
				}
			}
			else
			{
				strSrcReplyData = _T("111;");
			}
		}
		else
		{
			strSrcReplyData = _T("111;");
		}
		//////////////////////////////////////////////////////////////////////////
		CString strRet;
		CString strMoney;
		CString strMoneyYuan;
	
		//短信中要用到的	
		CString strAllMsg;
		CString strHandsetNo;
		CString strStartTime;
		strHandsetNo = GetField(strData2, g_strRSep, g_strFSep);
		GetField(strData2, g_strRSep, g_strFSep);
		GetField(strData2, g_strRSep, g_strFSep);
		
		//短信中要用到的
		strStartTime = GetField(strData2, g_strRSep, g_strFSep);
		strAllMsg = "尊敬的中国电信" + strHandsetNo + "客户，";
		strAllMsg += "您" + strStartTime.Left(4) + "年";
		strAllMsg += strStartTime.Mid(4,2) + "月的";
		
		//短信中要用到的
		CString strTitle;
		CString strCurrentTitle;
		strTitle = pConfig->strTitle;
		
		strRet = GetField(strSrcReplyData, g_strRSep, g_strFSep);
		if(!_stricmp(strRet,"0"))
		{
			strTran = "0~";

			for (int iGuangwen = 0;iGuangwen < 79;iGuangwen ++)
			{
				strMoney = GetField(strSrcReplyData, g_strRSep, g_strFSep);
                				
				//短信中要用到的
				strCurrentTitle = GetField(strTitle, g_strRSep, g_strFSep);
				
				
				if( 0 == strMoney.GetLength() )
				{
					strTran += "0000000" + g_strFSep;
				}
				else
				{
					//添加短信内容
					if (_stricmp (pConfig->pVPCode[iGuangwen],"0000000"))
					{
						strAllMsg += strCurrentTitle + "为" + strMoney + "元，";
					}
					
					if(atof(strMoney) > 0)
					{
						strTran +=  pConfig->pVPCode[iGuangwen] + strMoney + g_strFSep;
					}
					else if(atof(strMoney) < 0)
					{
						strMoney.Remove('-');
						strTran +=  pConfig->pVPCode2[iGuangwen] + strMoney + g_strFSep;
					}
					else
					{						
						strTran +=  pConfig->pVPCode[iGuangwen];
						strTran += _T("0.00") + g_strFSep;
					}

				}

			}
			
			//将短信的内容最后面的“，”号变成“。”号
			strAllMsg = strAllMsg.Left(strAllMsg.GetLength() - 2);
			strAllMsg += "。";

			char pSMS[150];
			memset(pSMS, 0, 150);
			char *pSrc = (char*)(LPCSTR)strAllMsg;
			char *pDst = (char*)pSMS;
			int nCount = 0;
			int nNum = 0;
			while(*pSrc != '\0')
			{
				if((BYTE)(*pSrc) > 127)
				{
					memcpy(pDst, pSrc, 2);
					pSrc += 2;
					pDst += 2;
					nCount += 2;
				}
				else
				{
					memcpy(pDst, pSrc, 1);
					pSrc += 1;
					pDst += 1;
					nCount += 1;
				}
				if( (nCount > 138 && (BYTE)(*pSrc) > 127) || nCount >= 140)
				{
					CString strTemp = pSMS;
					strTran += strTemp + "~";
					memset(pSMS, 0, 150);
					pDst = (char*)pSMS;
					nCount = 0;
					nNum ++;
				}
			}

			if(nCount > 0)
			{
				CString strTemp = pSMS;
				strTran += strTemp + "~";
				nNum ++;
			}
			            
			for(int k = nNum; k <= 5; k++)
			{
				strTran += "~";
			}		
		}
		else
		{
			strTran = "111~";
		}
	}
	else if (!_stricmp(pConfig->pICDCMD, "P_SCEPhoneIDCheck"))// 证件号码验证
	{
		CString strRet;
		strRet = GetField(strSrcReplyData, g_strRSep, g_strFSep);

		strTran = strRet;
	}
	else if (!_stricmp(pConfig->pICDCMD, "P_SCEAppealOpenClose"))  // 停开机申请
	{
		CString strRet;
		strRet = GetField(strSrcReplyData, g_strRSep, g_strFSep);

		strTran = strRet;
	}
	/************************************************************************/
	/* date:    20130328
	   author:	wangyue
	   purpose: modified,convert xml messages to former format*/
	/************************************************************************/
	else if (!_stricmp(pConfig->pICDCMD, "P_SCEGetRealTimeFee")) //实时话费(后付费)
	{
		CString strResult;
		CString strRealFee;
		CString strBalance;
		CString strANSIData;
		strANSIData = UTF8ToANSI(strSrcReplyData);
		//////////////////////////////////////////////////////////////////////////
		//创建xml对象
		CMarkup xml;
		if (xml.SetDoc(strANSIData))
		{
			//取result
			if (xml.FindElem_C(_T("/soapenv:Envelope/soapenv:Body/multiRef/result")))
			{
				strResult = xml.GetData();
				if (!strResult.Compare(_T("0")))
				{
					if (xml.FindElem_C(_T("/soapenv:Envelope/soapenv:Body/multiRef/balance")))
					{
						strBalance = xml.GetData();
						if (xml.FindElem_C(_T("/soapenv:Envelope/soapenv:Body/multiRef/realFee")))
						{
							strRealFee = xml.GetData();
							strSrcReplyData = _T("0~")+strRealFee+g_strFSep+strBalance+g_strRSep;
						} 
						else
						{
							strSrcReplyData = _T("111~取realFee字段失败;");
						}
					}
					else
					{
						strSrcReplyData = _T("111~取balance字段失败;");
					}
				}
				else
				{
					strSrcReplyData = _T("111~result字段非0;");
				}
			}
			else
			{
				strSrcReplyData = _T("111~取result字段失败;");
			}
		}
		else
		{
			strSrcReplyData = _T("111~xml对象创建失败;");
		}
		//////////////////////////////////////////////////////////////////////////
		CString strRet;
		strRet = GetField(strSrcReplyData, g_strRSep, g_strFSep);
		if(!_stricmp(strRet,"0"))
		{
			//河南报音以元为单位
			strTran = "0~";
			CString strCurrentFee;  //实时话费
			CString strBalanceFee;  //余额	
			strCurrentFee = GetField(strSrcReplyData, g_strRSep, g_strFSep);
			//strBalanceFee = GetField(strSrcReplyData, g_strRSep, g_strFSep);

			strCurrentFee.TrimLeft(' ');
			strCurrentFee.TrimRight(' ');

			double dYuan = 0.00;
			dYuan = atof(strCurrentFee)/100.00;  //分变成元
			CString strCurrentFeeYuan;
			strCurrentFeeYuan.Format("%.2f",dYuan);
			
			//短信内容 BEGIN
			CString strMsg;
			CString strHandsetNo;
			strHandsetNo = GetField(strData2, g_strRSep, g_strFSep);
			strMsg = "尊敬的中国电信" + strHandsetNo + "客户，您的";
			strMsg += "实时话费为" + strCurrentFeeYuan + "元。~";
			//短信内容 END
			
			if(atof(strCurrentFee) > 0)
			{
				strTran += pConfig->pVPCode[0] + strCurrentFeeYuan + g_strFSep;	
			}
			else if(atof(strCurrentFee) < 0)
			{
			//modify by weiliwen begin 20090831
				//strCurrentFee.Remove('-');
				strCurrentFeeYuan.Remove('-');
			//modify by weiliwen end 20090831

				strTran += pConfig->pVPCode2[0] + strCurrentFeeYuan + g_strFSep;	
			}
			else
			{
				strTran += pConfig->pVPCode[0] + strCurrentFeeYuan + g_strFSep;
			}
			
			for ( int ii = 1; ii < 50; ii++)
			{
				strTran += "0000000~";
			}
			//=====增加短信处理begin 20081202

			strTran += strMsg;
			
			for (int ii = 1; ii < 5; ii++)
			{
				strTran += "";
			}
			//=====增加短信处理End 20081202
		}
		else 
		{
			strTran = strRet;
		}
	}
	/************************************************************************/
	/* date:    20130328
	   author:	wangyue
	   purpose: modified,convert xml messages to former format*/
	/************************************************************************/
	else if (!_stricmp(pConfig->pICDCMD, "P_SCEVAGetTotalFee"))  //余额查询(后付费)
	{
		CString strResult;
		CString strRealFee;
		CString strBalance;
		//////////////////////////////////////////////////////////////////////////
		//去掉03d4..
		strSrcReplyData = strSrcReplyData.Right(strSrcReplyData.GetLength()-6);
		//取result
		strResult = GetElementValue(strSrcReplyData,_T("result"));
		if (!strResult.Compare(_T("0")))
		{
			//取balance
			strBalance= GetElementValue(strSrcReplyData,_T("balance"));
			//取RealFee
			strRealFee = GetElementValue(strSrcReplyData,_T("realFee"));
			//组合成原来的报文格式
			strSrcReplyData = _T("0;") + strRealFee + _T("~") + strBalance;
		}
		else
		{
			strSrcReplyData = _T("111;");
		}
		//////////////////////////////////////////////////////////////////////////
		CString strRet;
		strRet = GetField(strSrcReplyData, g_strRSep, g_strFSep);
		if(!_stricmp(strRet,"0"))
		{
			strTran = "0~";
			//CString strstrCurrentFee;  //实时话费
			CString strBalanceFee;  //余额1

			GetField(strSrcReplyData, g_strRSep, g_strFSep);
			strBalanceFee = GetField(strSrcReplyData, g_strRSep, g_strFSep);

			strBalanceFee.TrimLeft(' ');
			strBalanceFee.TrimRight(' ');

			double dYuan = 0.00;
			dYuan = atof(strBalanceFee)/100.00;  //分变成元
			CString strBalanceFeeYuan;
			strBalanceFeeYuan.Format("%.2f",dYuan);
			
			//短信内容 BEGIN
			CString strMsg;
			CString strHandsetNo;
			strHandsetNo = GetField(strData2, g_strRSep, g_strFSep);
			strMsg = "尊敬的中国电信" + strHandsetNo + "客户，您的";
			strMsg += "余额为" + strBalanceFeeYuan + "元。~";
			//短信内容 END

			if(atof(strBalanceFee) > 0)
			{
				strTran += pConfig->pVPCode[0] + strBalanceFeeYuan + g_strFSep;	
			}
			else if(atof(strBalanceFee) < 0)
			{  
			//modify by weiliwen begin 20090831
				//strBalanceFee.Remove('-');
				strBalanceFeeYuan.Remove('-');
			//modify by weiliwen end 20090831
				strTran += pConfig->pVPCode2[0] + strBalanceFeeYuan + g_strFSep;	
			}
			else
			{				
				strTran += pConfig->pVPCode[0] + strBalanceFeeYuan + g_strFSep;
			}
			for ( int ii = 1; ii < 50; ii++)
			{
				strTran += "0000000~";
			}
			
			//=====增加短信处理begin 20081202
			strTran += strMsg;
			
			for (int n = 1; n < 5; n++)
			{
				strTran += "~";
			}
			//=====增加短信处理end 20081202
		}
		else 
		{
			strTran = strRet;
		}		
	}
	/************************************************************************/
	/* date:    20130328
	   author:	wangyue
	   purpose: modified,convert xml messages to former format*/
	/************************************************************************/
	else if (!_stricmp(pConfig->pICDCMD, "P_SceQueryScore"))  //积分查询
	{
		CString strInnerXML;
		CString strResult;
		CMarkup xml;
		CString strANSIData = UTF8ToANSI(strSrcReplyData);
		if (xml.SetDoc(strANSIData))
		{
			if (xml.FindElem_C(_T("/soap:Envelope/soap:Body/ns1:queryRemainPointResponse/ns1:out")))
			{
				strInnerXML = xml.GetData();
				if (xml.SetDoc(strInnerXML))
				{
					if(xml.FindElem_C(_T("/result/resultCode")))
					{
						strResult = xml.GetData();
						if (!strResult.Compare(_T("0")))
						{
							if(xml.FindElem_C(_T("/result/sumpoint")))
							{
								CString strSurplusPoint = xml.GetData();
								strSrcReplyData = _T("0~") + strSurplusPoint;
							}
						}
						else
						{
							strSrcReplyData = _T("111;");
						}
					}
					else
					{
						strSrcReplyData = _T("111;");
					}
				}
			}
			else
			{
				strSrcReplyData = _T("111;");
			}
		}
		else
		{
			strSrcReplyData = _T("111;");
		}
		//////////////////////////////////////////////////////////////////////
		CString strRet;
		strRet = GetField(strSrcReplyData, g_strRSep, g_strFSep);
		if(!_stricmp(strRet,"0"))
		{
			strTran = "0~";
			
			//CString strTotalPoint;  //用户所有累积积分
			CString strCurrentPoint;  //用户当前剩余积分
  
			//strTotalPoint = GetField(strSrcReplyData, g_strRSep, g_strFSep);
			strCurrentPoint = GetField(strSrcReplyData, g_strRSep, g_strFSep);
			
			//短信内容 BEGIN
			CString strMsg;
			CString strHandsetNo;
			strHandsetNo = GetField(strData2, g_strRSep, g_strFSep);
			//modify by tongyufeng,20100121,应客户要求，去掉了所有累积积分
			strMsg = "尊敬的中国电信" + strHandsetNo + "客户，您的当前剩余积分为"+strCurrentPoint + "。~";
			//strMsg += "所有累积积分为" + strTotalPoint + "，" + "当前剩余积分为" + strCurrentPoint + "。~" ;
			//短信内容 END

			//if (atof(strTotalPoint) > 0)
			//{				
			//	strTran +=  pConfig->pVPCode[0] + strTotalPoint + g_strFSep;
			//}
			/*else if (atof(strTotalPoint) < 0)
			{
				strTotalPoint.Remove('-');
				strTran +=  pConfig->pVPCode2[0] + strTotalPoint + g_strFSep;
			}
			else
			{				
				strTran +=  pConfig->pVPCode[0] + strTotalPoint + g_strFSep;
			}*/

			if (atof(strCurrentPoint) > 0)
			{
				strTran += pConfig->pVPCode[1] + strCurrentPoint + g_strFSep;
			}
			else if (atof(strCurrentPoint) < 0)
			{
				strCurrentPoint.Remove('-');
				strTran += pConfig->pVPCode2[1] + strCurrentPoint + g_strFSep;
			}
			else
			{				
				strTran += pConfig->pVPCode[1] + strCurrentPoint + g_strFSep;
			}
			
			for ( int ii = 2; ii < 50 ; ii++)
			{
				strTran += "0000000~";
			}
			
			//=====增加短信处理begin 20081202
			strTran += strMsg;
			
			for (int m = 1; m < 5; m++)
			{
				strTran += "";
			}
			//=====增加短信处理End 20081202
		}
		else 
		{
			strTran = strRet;
		}
		
	}
	
	//原来固网部分 BEGIN
	
	//BOSS未给出各个参数具体值，暂时无法实现
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_GetUserinfor"))//1.1.1  用户级别I_SCE_GetUserinfor
	{
		CString strRet;
		CString strUserLevel;
		strRet = GetField(strSrcReplyData, g_strRSep, g_strFSep);
		if(!_stricmp(strRet,"0"))
		{
			strTran = "0~";
			GetField(strSrcReplyData, g_strRSep, g_strFSep);
			GetField(strSrcReplyData, g_strRSep, g_strFSep);
			strUserLevel = GetField(strSrcReplyData, g_strRSep, g_strFSep);
			if (!_stricmp(strUserLevel,"8"))//
			{
				strTran += "40";
			}
			else if (!_stricmp(strUserLevel,"7"))
			{
				strTran += "20";
			}
			else
			{
				strTran +="10";
			}
		}
		//add by kewentao  20090420
		else
		{
		   strTran = "111";
		}
		//end by kewentao 20090420
		
	}
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_ChangePassword"))//1.1.2  修改密码I_SCE_ChangePassword
	{
		CString strRet;
		strRet = GetField(strSrcReplyData, g_strRSep, g_strFSep);
		
		strTran = strRet;		
	}
	
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_GetDebtFee"))//1.1.3  查询欠费I_SCE_GetDebtFee
	{
		CString strRet;
		strRet = GetField(strSrcReplyData, g_strRSep, g_strFSep);

		//CString strstrCurrentFee;  //实时话费
		CString strBalanceFee;  //余额
		
		if(!_stricmp(strRet,"0"))
		{
			GetField(strSrcReplyData, g_strRSep, g_strFSep);
			strBalanceFee = GetField(strSrcReplyData, g_strRSep, g_strFSep);
			
			if(atof(strBalanceFee) > 0)
			{
				strTran = "1";
			}
			else
			{
				strTran = "0~";

				strBalanceFee.TrimLeft(' ');
				strBalanceFee.TrimRight(' ');
				
				double dYuan = 0.00;
				dYuan = atof(strBalanceFee)/100.00;  //分变成元
				CString strBalanceFeeYuan;
				strBalanceFeeYuan.Format("%.2f",dYuan);
				
				strBalanceFee.Remove('-');
				strTran += pConfig->pVPCode[0] + strBalanceFeeYuan + g_strFSep;	
				
				for ( int ii = 1; ii < 20; ii++)
				{
					strTran += "0000000~";
				}
				
				//短信内容 BEGIN
				CString strMsg;
				CString strHandsetNo;
				strHandsetNo = GetField(strData2, g_strRSep, g_strFSep);
				strMsg = "尊敬的中国电信" + strHandsetNo + "客户，您的";
				strMsg += "欠费总额为" + strBalanceFeeYuan + "元。~";
				//短信内容 END
				//=====增加短信处理begin 20081202
				
				strTran += strMsg;
				
				for (int n = 1; n < 5; n++)
				{
					strTran += "~";
				}
			}			
		}
		else 
		{
			strTran = strRet;
		}		
	}

/************************************************************************/
/*begin: added by tongyufeng,wangyue,2011-11-24,I_SCE_QueryBroadNoByAccount,I_SCE_QueryBrandNoByIDCard,I_SCE_QueryFreeResource                                                                     */
/************************************************************************/
	/************************************************************************/
	/* date:    20130328
	   author:	wangyue
	   purpose: modified,convert xml messages to former format*/
	/************************************************************************/
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_GetSubscriberNCustomer"))
	{
		CString strInnerXML;
		CString strResult;
		CMarkup xml;
		CString strANSIData;
		strANSIData = UTF8ToANSI(strSrcReplyData);

		//创建xml对象
		if (xml.SetDoc(strANSIData))
		{
			if (xml.FindElem_C(_T("/soap:Envelope/soap:Body/ns1:queryUserInfoAndBandNumResponse/ns1:out")))
			{
				strInnerXML = xml.GetData();
				if (xml.SetDoc(strInnerXML))
				{
					if(xml.FindElem_C(_T("/returnXml/userInfo/resultCode")))
					{
						strResult = xml.GetData();
						if (!strResult.Compare(_T("0")))
						{
							if(xml.FindElem_C(_T("/returnXml/userInfo/productId")))
							{
								CString strProductID = xml.GetData();
								if(xml.FindElem_C(_T("/returnXml/userInfo/partyId")))
								{
									CString strPartyId = xml.GetData();
									if(xml.FindElem_C(_T("/returnXml/userInfo/accountId")))
									{
										CString stAccountId = xml.GetData();
										strSrcReplyData = _T("0~") + strProductID + g_strFSep + strPartyId + g_strFSep + stAccountId + g_strRSep;
									}
									else
									{
										strSrcReplyData = _T("111;");
									}
								}
								else
								{
									strSrcReplyData = _T("111;");
								}
							}
							else
							{
								strSrcReplyData = _T("111;");
							}
						}
						else
						{
							strSrcReplyData = _T("111;");
						}
					}
					else
					{
						strSrcReplyData = _T("111;");
					}
				}
			}
			else
			{
				strSrcReplyData = _T("111;");
			}
		}
		else
		{
			strSrcReplyData = _T("111;");
		}
		/////////////////////////////////////////////////////////////////
		CString strRet;
		CString strUserFlag;
		CString strClientFlag;
		CString strAccountFlag;
		
		strRet = GetField(strSrcReplyData, g_strFSep, g_strRSep);
		if (!strRet.Compare("0"))
		{
			strTran = "0~";	
			strUserFlag = GetField(strSrcReplyData, g_strFSep, g_strRSep);
			strClientFlag = GetField(strSrcReplyData, g_strFSep, g_strRSep);
			strAccountFlag = GetField(strSrcReplyData, g_strFSep, g_strRSep);
			strTran += (strUserFlag+"~");
			strTran += (strClientFlag+"~");
			strTran += strAccountFlag;
		}
		else
		{
			strTran = strRet;
		}
	}
	/************************************************************************/
	/* date:    20130328
	   author:	wangyue
	   purpose: modified,convert xml messages to former format*/
	/************************************************************************/
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_QueryBroadNoByAccount"))  //
	{
		CString strInnerXML;
		CString strResult;
		CMarkup xml;
		CString strANSIData;
		strANSIData = UTF8ToANSI(strSrcReplyData);

		//创建xml对象
		if (xml.SetDoc(strANSIData))
		{
			if (xml.FindElem_C(_T("/soap:Envelope/soap:Body/ns1:queryUserInfoAndBandNumResponse/ns1:out")))
			{
				strInnerXML = xml.GetData();
				if (xml.SetDoc(strInnerXML))
				{
					if(xml.FindElem_C(_T("/returnXml/broadNum/broadCode")))
					{
						strResult = xml.GetData();
						if (!strResult.Compare(_T("0")))
						{
							if(xml.FindElem_C(_T("/returnXml/broadNum/accessNum")))
							{
								CString strBroadNo = xml.GetData();
								strSrcReplyData = _T("0~1~")+strBroadNo+g_strRSep;
							}
							else
							{
								strSrcReplyData = _T("111;");
							}
						}
						else
						{
							strSrcReplyData = _T("111;");
						}
					}
					else
					{
						strSrcReplyData = _T("111;");
					}
				}
			}
			else
			{
				strSrcReplyData = _T("111;");
			}
		}
		else
		{
			strSrcReplyData = _T("111;");
		}
		//////////////////////////////////////////////////////////////////////////////
		//对端返回数据格式：返回码;序号~宽带号码;
		CString strRet=_T("");		//对端返回码
		CString strSN1=_T("");		//序号
		CString strSN2=_T("");		//序号
		CString strBroadNo=_T("");	//宽带号码
		
		strRet = GetField(strSrcReplyData,g_strFSep,g_strRSep);
		strSN1 = GetField(strSrcReplyData,g_strFSep,g_strRSep);
		strBroadNo = GetField(strSrcReplyData,g_strFSep,g_strRSep);
		strSN2 = GetField(strSrcReplyData,g_strFSep,g_strRSep);
		
		if(!_stricmp(strRet,"0"))
		{
			if (!_stricmp(strSN2,""))  
			{
				strTran = "0~"+strBroadNo;
			}
			else
			{
				strTran = "2~";  //如果返回了第2个号码，说明绑定了多个，则IVR不处理
			}
			
		}
		else
		{
			strTran = "111~";
		}
		
	//	strTran = strRet + g_strFSep + strBroadNo;
	}

	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_QueryBrandNoByIDCard"))  //
	{
		CString strInnerXML;
		CString strResult;
		CMarkup xml;
		CString strANSIData;
		strANSIData = UTF8ToANSI(strSrcReplyData);

		//创建xml对象
		if (xml.SetDoc(strANSIData))
		{
			if (xml.FindElem_C(_T("/soap:Envelope/soap:Body/ns1:queryBandNumByIDCardResponse/ns1:out")))
			{
				strInnerXML = xml.GetData();
				if (xml.SetDoc(strInnerXML))
				{
					if(xml.FindElem_C(_T("/result/resultCode")))
					{
						strResult = xml.GetData();
						if (!strResult.Compare(_T("0")))
						{
							if(xml.FindElem_C(_T("/result/accessNum")))
							{
								CString strNo = xml.GetData();
								strSrcReplyData = _T("0~1~")+strNo+g_strRSep;
							}
							else
							{
								strSrcReplyData = _T("111;");
							}
						}
						else
						{
							strSrcReplyData = _T("111;");
						}
					}
					else
					{
						strSrcReplyData = _T("111;");
					}
				}
			}
			else
			{
				strSrcReplyData = _T("111;");
			}
		}
		else
		{
			strSrcReplyData = _T("111;");
		}
		//////////////////////////////////////////////////////////////////////////
		//对端返回数据格式：返回码;序号~宽带号码;
		CString strRet=_T("");		//对端返回码
		CString strSN1=_T("");		//序号
		CString strSN2=_T("");		//序号
		CString strBroadNo=_T("");	//宽带号码
		
		strRet = GetField(strSrcReplyData,g_strFSep,g_strRSep);
		strSN1 = GetField(strSrcReplyData,g_strFSep,g_strRSep);
		strBroadNo = GetField(strSrcReplyData,g_strFSep,g_strRSep);
		strSN2 = GetField(strSrcReplyData,g_strFSep,g_strRSep);
		
		if(!_stricmp(strRet,"0"))
		{
			if (!_stricmp(strSN2,""))
			{
				strTran = "0~"+strBroadNo;
			}
			else
			{
				strTran = "2~";   //如果返回了第2个号码，说明绑定了多个，则IVR不处理
			}
			
		}
		else
		{
			strTran = "111~";
		}
	}
	/************************************************************************/
	/* date:    20130328
	   author:	wangyue
	   purpose: modified,convert xml messages to former format*/
	/************************************************************************/
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_QueryFreeResource"))  //10048 查询用户免费资源
	{
		CString strResult;
		
		//////////////////////////////////////////////////////////////////////////
		//去掉03d4..
		strSrcReplyData = strSrcReplyData.Right(strSrcReplyData.GetLength()-6);
		CMarkup xml;
		if (xml.SetDoc(strSrcReplyData))
		{
			//取result字段
			if(xml.FindElem_C(_T("/soapenv:Envelope/soapenv:Body/multiRef[@id='id0']/result")))
			{
				strResult = xml.GetData();
				if (!strResult.Compare(_T("0")))
				{
					strSrcReplyData = _T("0;");
					if(xml.FindElem_C(_T("/soapenv:Envelope/soapenv:Body/multiRef[@id='id0']/userFreeresList")))
					{
						int nAmountOfItem = 0;
						CString strAttr = xml.GetAttrib(_T("soapenc:arrayType"));
						strAttr = strAttr.Mid(strAttr.Find(_T("["))+1,strAttr.GetLength()-strAttr.Find(_T("["))-2);
						nAmountOfItem = atoi(strAttr);
						//资源名称，id，单位，总共可使用量，剩余量，已使用量
						CString strName,strType,strUnit,strFree,strUsable,strUsed;
						//循环取每个免费资源的内容
						for (int i=1;i<=nAmountOfItem;i++)
						{
							CString strTemp_1,strTemp_2,strTemp_3,strTemp_4,strTemp_5,strTemp_6;
							strTemp_1.Format(_T("/soapenv:Envelope/soapenv:Body/multiRef[@id='id%d']/res_free_name"),i);
							strTemp_2.Format(_T("/soapenv:Envelope/soapenv:Body/multiRef[@id='id%d']/res_free_type"),i);
							strTemp_3.Format(_T("/soapenv:Envelope/soapenv:Body/multiRef[@id='id%d']/res_unit"),i);
							strTemp_4.Format(_T("/soapenv:Envelope/soapenv:Body/multiRef[@id='id%d']/total_res_free"),i);
							strTemp_5.Format(_T("/soapenv:Envelope/soapenv:Body/multiRef[@id='id%d']/total_res_usable"),i);
							strTemp_6.Format(_T("/soapenv:Envelope/soapenv:Body/multiRef[@id='id%d']/total_res_used"),i);

							if(xml.FindElem_C(strTemp_1))
							{
								strName = xml.GetData();
							}
							if(xml.FindElem_C(strTemp_2))
							{
								strType = xml.GetData();
							}
							if(xml.FindElem_C(strTemp_3))
							{
								strUnit = xml.GetData();
							}
							if(xml.FindElem_C(strTemp_4))
							{
								strFree = xml.GetAttrib(_T("href"));
								strFree.Delete(0);
								strTemp_4.Format(_T("/soapenv:Envelope/soapenv:Body/multiRef[@id='%s']"),strFree);
								if (xml.FindElem_C(strTemp_4))
								{
									strFree = xml.GetData();
								}
							}
							if(xml.FindElem_C(strTemp_5))
							{
								strUsable =  xml.GetAttrib(_T("href"));
								strUsable.Delete(0);
								strTemp_5.Format(_T("/soapenv:Envelope/soapenv:Body/multiRef[@id='%s']"),strUsable);
								if (xml.FindElem_C(strTemp_5))
								{
									strUsable = xml.GetData();
								}
							}
							if(xml.FindElem_C(strTemp_6))
							{
								strUsed =  xml.GetAttrib(_T("href"));
								strUsed.Delete(0);
								strTemp_6.Format(_T("/soapenv:Envelope/soapenv:Body/multiRef[@id='%s']"),strUsed);
								if (xml.FindElem_C(strTemp_6))
								{
									strUsed = xml.GetData();
								}
							}
							//拼strSrcReplyData
							strSrcReplyData += strType + _T("~") + strName + _T("~") + strFree + _T("~") + 
								strUsed + _T("~") + strUsable + _T("~") + strUnit + _T(";");
						}
					}
				}
				else
				{
					strSrcReplyData = _T("111;");
				}
			}
			else
			{
				strSrcReplyData = _T("111;");
			}
		}
		else
		{
			strSrcReplyData = _T("111;");
		}
		
		//////////////////////////////////////////////////////////////////////////
		//对端返回数据格式：返回码;序号~资源名称~总共可使用资源数量~免费资源已使用量~剩余免费资源量~资源单位;
		//说明：如果用户存在多种免费资源，返回多个"
		//列如：0；0~免费资源A~300~40~260~分；1~免费资源B~100~10~90~分；
		/************************************************************************/
		/* 尊敬的客户,您本月的（资源名称）可使用（总共可使用资源数量）（资源单位），已用（免费资源已使用量）（资源单位）,
		剩余（剩余免费资源量）（资源单位）可用; 
		（资源名称）可使用（总共可使用资源数量）（资源单位），已用（免费资源已使用量）（资源单位）,剩余（剩余免费资源量）
		（资源单位）可用。感谢您使用中国电信*/
		/************************************************************************/
		CString strRet;	//对端返回码
		CString strSN;	//序号
		CString strResourceName;//资源名称
		CString strTotalResource;//总共可使用资源数量
		CString strFreeResource;//免费资源已使用量
		CString strSurplusFreeResource;//剩余免费资源量
		CString strMeasurement;	//资源单位
		
		strRet = GetField(strSrcReplyData,g_strFSep,g_strRSep);
		strRet.TrimLeft();
		strRet.TrimRight();

		CString strSM = _T("0~尊敬的客户,您本月的");

		if (!_stricmp(strRet,"0"))//返回成功
		{
			//取第一条记录
			strRet = GetField(strSrcReplyData,g_strRSep,g_strRSep);
			while (!strRet.IsEmpty())
			{
				//取record(记录)中的每一个field(字段)
				strSN = GetField(strRet,g_strFSep,g_strFSep);
				strResourceName = GetField(strRet,g_strFSep,g_strFSep);
				strTotalResource = GetField(strRet,g_strFSep,g_strFSep);
				strFreeResource = GetField(strRet,g_strFSep,g_strFSep);
				strSurplusFreeResource = GetField(strRet,g_strFSep,g_strFSep);
				strMeasurement = GetField(strRet,g_strFSep,g_strFSep);
				//date：2013-08-21 根据电信需求，需要将免费流量中的k转换成M
				if (!strMeasurement.CompareNoCase("k"))
				{
					strMeasurement = _T("M");
					float fTotal = atof(strTotalResource)/1024.00;
					float fUsed = atof(strFreeResource)/1024.00;
					float fSurplus = atof(strSurplusFreeResource)/1024.00;

					strTotalResource.Format(_T("%.2f"),fTotal);
					strFreeResource.Format(_T("%.2f"),fUsed);
					strSurplusFreeResource.Format(_T("%.2f"),fSurplus);
				}

				//组装短信内容
				strSM += strResourceName;
				strSM += _T("总可使用");
				strSM += strTotalResource;
				strSM += strMeasurement;
				strSM += _T("，");

				strSM += _T("已用");
				strSM += strFreeResource;
				strSM += strMeasurement;
				strSM += _T("，");

				strSM += _T("剩余");
				strSM += strSurplusFreeResource;
				strSM += strMeasurement;
				strSM += _T("；");

				strRet = GetField(strSrcReplyData,g_strRSep,g_strRSep);//取下一条record(记录)
			}

			strSM = strSM.Left(strSM.GetLength()-2);
			strSM += _T("。");
			strSM += _T("感谢您使用中国电信");
//			TRACE("dtproxy SM=%s",strSM);

			//由于可能短信过长，需要分割
			CString strSmsSub[5]; 
			CString	strSms;
			MyTrimSm2(140, strSM, strSmsSub, 5);
			for(int j =0; j < 5; j++)                     //5条短信
			{
				strSms += strSmsSub[j] + g_strFSep;
			}
			strTran += strSms + g_strRSep ;
	//		TRACE("dtproxy SM=%s",strTran);
		}
		else if (!_stricmp(strRet,"101"))//查询结果为空
		{
			strTran = "101~";
		}
		else//查询失败
		{
			strTran = "111~";
		}
	}

/************************************************************************/
/*end: added by tongyufeng,wangyue,2011-11-24                                                                     */
/************************************************************************/
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_GetMonthFee"))//1.1.5  某月帐单查询I_SCE_GetMonthFee
	{
		CString strRet;
		CString strMoney;
		CString strMoneyYuan;
		
		//短信中要用到的	
		CString strAllMsg;
		CString strHandsetNo;
		CString strStartTime;
		strHandsetNo = GetField(strData2, g_strRSep, g_strFSep);
		GetField(strData2, g_strRSep, g_strFSep);
		GetField(strData2, g_strRSep, g_strFSep);
		
		//短信中要用到的
		strStartTime = GetField(strData2, g_strRSep, g_strFSep);
		strAllMsg = "尊敬的中国电信" + strHandsetNo + "客户，";
		strAllMsg += "您" + strStartTime.Left(4) + "年";
		strAllMsg += strStartTime.Mid(4,2) + "月的";
		
		//短信中要用到的
		CString strTitle;
		CString strCurrentTitle;
		strTitle = pConfig->strTitle;
		
		strRet = GetField(strSrcReplyData, g_strRSep, g_strFSep);
		if(!_stricmp(strRet,"0"))
		{
			strTran = "0~";
						
			for (int iGuangwen = 0;iGuangwen < 50;iGuangwen ++)
			{
				strMoney = GetField(strSrcReplyData, g_strRSep, g_strFSep);
				
				//短信中要用到的
				strCurrentTitle = GetField(strTitle, g_strRSep, g_strFSep);
				
				
				if( 0 == strMoney.GetLength() )
				{
					strTran += "0000000" + g_strFSep;
				}
				else
				{
					//添加短信内容
					if (_stricmp (pConfig->pVPCode[iGuangwen],"0000000"))
					{
						strAllMsg += strCurrentTitle + "为" + strMoney + "元，";
					}
					
					if(atof(strMoney) > 0)
					{
						strTran +=  pConfig->pVPCode[iGuangwen] + strMoney + g_strFSep;
					}
					else if(atof(strMoney) < 0)
					{
						strMoney.Remove('-');
						strTran +=  pConfig->pVPCode2[iGuangwen] + strMoney + g_strFSep;
					}
					else
					{						
						strTran +=  pConfig->pVPCode[iGuangwen] + strMoney + g_strFSep;
					}
				}
				
			}
			
			//将短信的内容最后面的“，”号变成“。”号
			strAllMsg = strAllMsg.Left(strAllMsg.GetLength() - 2);
			strAllMsg += "。";


			char pSMS[150];
			memset(pSMS, 0, 150);
			char *pSrc = (char*)(LPCSTR)strAllMsg;
			char *pDst = (char*)pSMS;
			int nCount = 0;
			int nNum = 0;
			while(*pSrc != '\0')
			{
				if((BYTE)(*pSrc) > 127)
				{
					memcpy(pDst, pSrc, 2);
					pSrc += 2;
					pDst += 2;
					nCount += 2;
				}
				else
				{
					memcpy(pDst, pSrc, 1);
					pSrc += 1;
					pDst += 1;
					nCount += 1;
				}
				if( (nCount > 138 && (BYTE)(*pSrc) > 127) || nCount >= 140)
				{
					CString strTemp = pSMS;
					strTran += strTemp + "~";
					memset(pSMS, 0, 150);
					pDst = (char*)pSMS;
					nCount = 0;
					nNum ++;
				}
			}

			if(nCount > 0)
			{
				CString strTemp = pSMS;
				strTran += strTemp + "~";
				nNum ++;
			}
			            
			for(int k = nNum; k <= 5; k++)
			{
				strTran += "~";
			}		
		}
		else if (!_stricmp(strRet, "111"))// 失败
		{
			strTran = "111~";
		}	
	}

	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_GetBalance"))//1.1.6  余额查询I_SCE_GetBalance
	{
		CString strRet;
		strRet = GetField(strSrcReplyData, g_strRSep, g_strFSep);
		if(!_stricmp(strRet,"0"))
		{
			strTran = "0~";
			//CString strstrCurrentFee;  //实时话费
			CString strBalanceFee;  //余额
			
			GetField(strSrcReplyData, g_strRSep, g_strFSep);
			strBalanceFee = GetField(strSrcReplyData, g_strRSep, g_strFSep);
			
			strBalanceFee.TrimLeft(' ');
			strBalanceFee.TrimRight(' ');

			double dYuan;
			dYuan = 0.00;
			dYuan = atof(strBalanceFee)/100.00;  //补收补退
			CString strBalanceFeeYuan;
			strBalanceFeeYuan.Format("%.2f",dYuan);
			
			//短信 BEGIN
			CString strMsg;
			CString strHandsetNo;
			strHandsetNo = GetField(strData2, g_strRSep, g_strFSep);
			strMsg = "尊敬的中国电信" + strHandsetNo + "客户，您的";
			strMsg += "余额为" + strBalanceFeeYuan + "元。~";
			//短信 END
			

			if(atof(strBalanceFee) > 0)
			{
				strTran += pConfig->pVPCode[0] + strBalanceFeeYuan + g_strFSep;	
			}
			else if(atof(strBalanceFee) < 0)
			{
			//modify by weiliwen begin 20090831
				//strBalanceFee.Remove('-');
				strBalanceFeeYuan.Remove('-');
			//modify by weiliwen end 20090831

				strTran += pConfig->pVPCode2[0] + strBalanceFeeYuan + g_strFSep;	
			}
			else
			{				
				strTran += pConfig->pVPCode[0] + strBalanceFeeYuan + g_strFSep;
			}
			for ( int ii = 1; ii < 50; ii++)
			{
				strTran += "0000000~";
			}
			
			//=====增加短信处理begin 20081202

			strTran += strMsg;
			
			for (int n = 1; n < 5; n++)
			{
				strTran += "~";
			}
		}
		else 
		{
			strTran = strRet;
		}		
	}
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_HandsetNoTypeJudg"))//1.1.8  号码有效性校验I_SCE_HandsetNoTypeJudg
	{
		CString strRet;
		CString strFlag;

		strRet = GetField(strSrcReplyData, g_strRSep, g_strFSep);
		strFlag = GetField(strSrcReplyData, g_strRSep, g_strFSep);
		if (!_stricmp(strRet,"0"))
		{
			strTran = strFlag;
		}
		else 
		{
			strTran = "111";
		}			
	}
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_CHKNumMatch"))//1.1.9  帐号和号码匹配校验：I_SCE_CHKNumMatch
	{
		CString strRet;
		CString strFlag;
		
		strRet = GetField(strSrcReplyData, g_strRSep, g_strFSep);
		strFlag = GetField(strSrcReplyData, g_strRSep, g_strFSep);
		if (!_stricmp(strRet,"0"))
		{
			strTran = strFlag;
		}
		else 
		{
			strTran = "111";
		}
	}
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_RealTimeFe"))//1.1.10  实时费用查询I_SCE_RealTimeFee
	{
		CString strRet;
		strRet = GetField(strSrcReplyData, g_strRSep, g_strFSep);
		if(!_stricmp(strRet,"0"))
		{
			strTran = "0~";
			CString strstrCurrentFee;  //实时话费
			CString strBalanceFee;  //余额
			
			strstrCurrentFee = GetField(strSrcReplyData, g_strRSep, g_strFSep);
			//strBalanceFee = GetField(strSrcReplyData, g_strRSep, g_strFSep);
			
			strstrCurrentFee.TrimLeft(' ');
			strstrCurrentFee.TrimRight(' ');
			

			double dYuan;
			dYuan = atof(strstrCurrentFee)/100.00;  //补收补退
			CString strstrCurrentFeeYuan;
			strstrCurrentFeeYuan.Format("%.2f",dYuan);
			
			//短信 BEGIN
			CString strMsg;
			CString strHandsetNo;
			strHandsetNo = GetField(strData2, g_strRSep, g_strFSep);
			strMsg = "尊敬的中国电信" + strHandsetNo + "客户，您的";
			strMsg += "实时话费为" + strstrCurrentFeeYuan + "元。~";
			//短信 END
			
			
			if(atof(strBalanceFee) > 0)
			{
				strTran += pConfig->pVPCode[0] + strstrCurrentFeeYuan + g_strFSep;	
			}
			else if(atof(strBalanceFee) < 0)
			{
			//modify by weiliwen begin 20090831
			  //strBalanceFee.Remove('-');
				strstrCurrentFeeYuan.Remove('-');
			//modify by weiliwen end 20090831

				strTran += pConfig->pVPCode2[0] + strstrCurrentFeeYuan + g_strFSep;	
			}
			else
			{				
				strTran += pConfig->pVPCode[0] + strstrCurrentFeeYuan + g_strFSep;
			}
			for ( int ii = 1; ii < 50; ii++)
			{
				strTran += "0000000~";
			}
			
			//=====增加短信处理begin 20081202

			strTran += strMsg;
			
			for (int n = 1; n < 5; n++)
			{
				strTran += "~";
			}
		}
		else 
		{
			strTran = strRet;
		}		
	}
	/************************************************************************/
	/* date:    20130328
	   author:	wangyue
	   purpose: modified,convert xml messages to former format*/
	/************************************************************************/
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_Login")
		|| !_stricmp(pConfig->pICDCMD, "P_SCELogin"))//1.1.11  用户密码验证I_SCE_Login
	{
		CString strInnerXML;
		CString strResult;
		CMarkup xml;
		CString strANSIData;
		strANSIData = UTF8ToANSI(strSrcReplyData);

		//创建xml对象
		if (xml.SetDoc(strANSIData))
		{
			if (xml.FindElem_C(_T("/soap:Envelope/soap:Body/ns1:checkPasswordResponse/ns1:out")))
			{
				strInnerXML = xml.GetData();
				if (xml.SetDoc(strInnerXML))
				{
					if(xml.FindElem_C(_T("/CheckPasswordResponse/result")))
					{
						strResult = xml.GetData();
						if (!strResult.Compare(_T("0")))
						{
							strSrcReplyData = _T("0;");
						}
						else
						{
							strSrcReplyData = _T("111;");
						}
					}
					else
					{
						strSrcReplyData = _T("111;");
					}
				}
			}
			else
			{
				strSrcReplyData = _T("111;");
			}
		}
		else
		{
			strSrcReplyData = _T("111;");
		}
	//////////////////////////////////////////////////////////////////////////////
		CString strRet;
		
        strRet = GetField(strSrcReplyData, g_strFSep, g_strRSep);
		if (!strRet.Compare("0"))
		{
			strTran = "0~";
		}
		else if (!strRet.Compare("1"))
		{
			strTran = "1~"; 
		}
		else
		{
			strTran = "111";
		}
	}
	//add by lilong 20090430 begin
	/************************************************************************/
	/* date:    20130328
	   author:	wangyue
	   purpose: modified,convert xml messages to former format*/
	/************************************************************************/
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_GetUserinfor1"))//1.1.1  用户级别I_SCE_GetUserinfor
	{
		CString strResult;
		CString strCustId;//客户编号
		CString strCustName;//客户姓名
		CString strSegmentId;//品牌id
		CString strSegmentName;//品牌名称
		CString strAreaID;//地区代码
		CString strAreaName;//地区名称
		CString strCustGread;//客户级别
		CString strCustGread2;//政企客户服务等级 
		CString strProductStatusName;//用户状态
		CString strAddress;//宽带地址
		//////////////////////////////////////////////////////////////////////////
		CString strInnerXML;
		CMarkup xml;
		CString strANSIData;
		strANSIData = UTF8ToANSI(strSrcReplyData);

		//创建xml对象
		if (xml.SetDoc(strANSIData))
		{
			if (xml.FindElem_C(_T("/soap:Envelope/soap:Bodyns1:querycustinfo3Response/ns1:out")))
			{
				strInnerXML = xml.GetData();
				if (xml.SetDoc(strInnerXML))
				{
					if(xml.FindElem_C(_T("/custInfo/resultId")))
					{
						strResult = xml.GetData();
						if (!strResult.Compare(_T("0")))
						{
							if(xml.FindElem_C(_T("/custInfo/custGread")))
							{
								strCustGread = xml.GetData();
								strSrcReplyData = _T("0~")+strCustGread+g_strRSep;
							}
							else
							{
								strSrcReplyData = _T("111~取custGread字段出错");
							}
						}
						else
						{
							strSrcReplyData = _T("111~resultID为非0");
						}
					}
					else
					{
						strSrcReplyData = _T("111~取resultID字段出错;");
					}
				}
				else
				{
					strSrcReplyData = _T("111~创建内部xml对象出错;");
				}
			}
			else
			{
				strSrcReplyData = _T("111~获取内部xml字符串出错;");
			}
		}
		else
		{
			strSrcReplyData = _T("111~创建xml对象出错;");
		}
		//////////////////////////////////////////////////////////////////////////
		CString strRet;
		CString strUserClass;
		CString strVIPFLag;
		strRet = GetField(strSrcReplyData, g_strRSep, g_strFSep);
		if(!_stricmp(strRet,"0"))
		{
			strTran = "0~";
			/*
			GetField(strSrcReplyData, g_strRSep, g_strFSep);
						GetField(strSrcReplyData, g_strRSep, g_strFSep);
						GetField(strSrcReplyData, g_strRSep, g_strFSep);
						GetField(strSrcReplyData, g_strRSep, g_strFSep);
						GetField(strSrcReplyData, g_strRSep, g_strFSep);
						GetField(strSrcReplyData, g_strRSep, g_strFSep);*/
			
			strUserClass = GetField(strSrcReplyData, g_strRSep, g_strFSep);
			/*
			返回给IVR：
				10：普通用户 
				40：银卡用户
				50：金卡用户
				60：钻石卡用户
			老协议：1 钻石卡 2 金卡 3 银卡 4 普通卡 5 贵宾卡 0 普通用户
			新协议：1000 钻石卡 1100 金卡 1200 银卡  1300普通
			*/
			if(!_stricmp(strUserClass,"1000"))
			{
				strUserClass = "60";
				strVIPFLag = "1";
			}
			else if(!_stricmp(strUserClass,"1100"))
			{
				strUserClass = "50";
				strVIPFLag = "1";
			}
			else if(!_stricmp(strUserClass,"1200"))
			{
				strUserClass = "40";
				strVIPFLag = "1";				
			}
			else if(!_stricmp(strUserClass,"1300") || !_stricmp(strUserClass,"0"))
			{
				strUserClass = "10";
				strVIPFLag = "0";				
			}				
			else
			{
				//默认处理
			}
			strTran += strUserClass + "~0~" + strVIPFLag + "~";
			
		
		}		
		else
		{
			strTran = "111~";
		}		
	}
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_GetUserinfor2"))//夜间流程使用，20130507，wangyue
	{
		CString strResult;
		CString strCustId;//客户编号
		CString strCustName;//客户姓名
		CString strSegmentId;//品牌id
		CString strSegmentName;//品牌名称
		CString strAreaID;//地区代码
		CString strAreaName;//地区名称
		CString strCustGread;//客户级别
		CString strCustGread2;//政企客户服务等级 
		CString strProductStatusName;//用户状态
		CString strAddress;//宽带地址
		//////////////////////////////////////////////////////////////////////////
		CString strInnerXML;
		CMarkup xml;
		CString strANSIData;
		strANSIData = UTF8ToANSI(strSrcReplyData);

		//创建xml对象
		if (xml.SetDoc(strANSIData))
		{
			if (xml.FindElem_C(_T("/soap:Envelope/soap:Bodyns1:querycustinfo3Response/ns1:out")))
			{
				strInnerXML = xml.GetData();
				if (xml.SetDoc(strInnerXML))
				{
					if(xml.FindElem_C(_T("/custInfo/resultId")))
					{
						strResult = xml.GetData();
						if (!strResult.Compare(_T("0")))
						{
							if(xml.FindElem_C(_T("/custInfo/custGread")))
							{
								strCustGread = xml.GetData();
								strSrcReplyData = _T("0~")+strCustGread+g_strRSep;
							}
							else
							{
								strSrcReplyData = _T("111~取custGread字段出错");
							}
						}
						else
						{
							strSrcReplyData = _T("111~resultID为非0");
						}
					}
					else
					{
						strSrcReplyData = _T("111~取resultID字段出错;");
					}
				}
				else
				{
					strSrcReplyData = _T("111~创建内部xml对象出错;");
				}
			}
			else
			{
				strSrcReplyData = _T("111~获取内部xml字符串出错;");
			}
		}
		else
		{
			strSrcReplyData = _T("111~创建xml对象出错;");
		}
		//////////////////////////////////////////////////////////////////////////
		CString strRet;
		CString strUserClass;
		CString strVIPFLag;
		CString strGovFlag=_T("0");//政企标志
		strRet = GetField(strSrcReplyData, g_strRSep, g_strFSep);
		if(!_stricmp(strRet,"0"))
		{
			strTran = "0~";
			/*
			GetField(strSrcReplyData, g_strRSep, g_strFSep);
						GetField(strSrcReplyData, g_strRSep, g_strFSep);
						GetField(strSrcReplyData, g_strRSep, g_strFSep);
						GetField(strSrcReplyData, g_strRSep, g_strFSep);
						GetField(strSrcReplyData, g_strRSep, g_strFSep);
						GetField(strSrcReplyData, g_strRSep, g_strFSep);*/
			
			strUserClass = GetField(strSrcReplyData, g_strRSep, g_strFSep);
			/*
			返回给IVR：
				10：普通用户 
				40：银卡用户
				50：金卡用户
				60：钻石卡用户
			老协议：1 钻石卡 2 金卡 3 银卡 4 普通卡 5 贵宾卡 0 普通用户
			新协议：1000 钻石卡 1100 金卡 1200 银卡  1300普通
			*/
			if(!_stricmp(strUserClass,"1000"))
			{
				strUserClass = "60";
				strVIPFLag = "1";
			}
			else if(!_stricmp(strUserClass,"1100"))
			{
				strUserClass = "50";
				strVIPFLag = "1";
			}
			else if(!_stricmp(strUserClass,"1200"))
			{
				strUserClass = "40";
				strVIPFLag = "1";				
			}
			else if(!_stricmp(strUserClass,"1300") || !_stricmp(strUserClass,"0"))
			{
				strUserClass = "10";
				strVIPFLag = "0";				
			}				
			else
			{
				//默认处理
				strUserClass = _T("10");
				strVIPFLag = _T("0");
				strGovFlag=_T("1");
			}
			strTran += strUserClass + "~0~" + strVIPFLag + strGovFlag + "~";
		}		
		else
		{
			strTran = "111~";
		}	
	}
	//add by lilong 20090430 end
	else
	{
		strTran = strSrcReplyData;
	}

	pPara = (SP_PARAM *)&pSPCallAck->SPPara;	
	pData = (char *)pPara + pSPCallAck->byParaNum * sizeof(SP_PARAM);
    dwMsgLen = sizeof(SP_CALL_ACK) + (pSrcSPCall->byParaNum - 1) * sizeof(SP_PARAM);
	
	//写日志
	NewWriteLog(strTran, "DllTransReplyMsg", pConfig->pICDCMD, pReply->dwRequestID);
	
	
	for (int i = 0; i < pSPCallAck->byParaNum; i++ )
	{
		//处理输入输出参数和输出参数
		if ((pPara->byParaType == OUTPUT_PARAM) || (pPara->byParaType == INOUT_PARAM)) 
		{
            CString pChar;
			
			pChar = GetField(strTran, g_strFSep, g_strRSep);
			
			// 去掉前后的空格
			pChar.TrimLeft(' ');
			pChar.TrimRight(' ');			
			TransStringToParam(pPara, pData, (DWORD &)(pPara->wDataLen), pChar);
		}
		
		dwMsgLen += pPara->wDataLen;
		pData += pPara->wDataLen;
		pPara++;
	}
	
    memcpy(pMsg, pSPCallAck, dwMsgLen);
	bEndReply = TRUE;
	
    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//  Func:       
//  Describe:   将第三方前置机发来的无请求应答消息转换成IVR类型或AppSvr类型的消息
//  Input:      
//              pBuf:       消息指针
//              dwLen:      消息长度
//              dwMsgLen:   pMsg缓冲区长度
//  Output:
//              pMsg:       转换后的消息指针
//              dwMsgLen:   转换后的消息长度
//  Return:
//              TRUE:       转换成功
//              FALSE:      转换失败
//  Note:       由于没有原始请求, 只能根据配置进行转换
///////////////////////////////////////////////////////////////////////////////
BOOL __stdcall
DllTransNoApplyReplyMsg(char *pBuf, DWORD dwLen, char *pMsg, DWORD &dwMsgLen)
{
    LPAPPLY_MSG     pSrcReply;
	
    InitDll();
	
    pSrcReply = (LPAPPLY_MSG)&pBuf[sizeof(MSG_ADDRESS)];
	
    if (dwLen - sizeof(MSG_ADDRESS) > dwMsgLen)
    {
        // 输出缓冲区不足
        return FALSE;
    }
	
	dwMsgLen = htonl(pSrcReply->dwLen);
    memcpy(pMsg, pSrcReply, dwMsgLen);
	
    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//  Func:       
//  Describe:   取得应答包的流水号
//  Input:      
//              pBuf:       消息指针
//              dwLen:      消息长度
//              dwIndexLen: 流水号缓冲区的长度
//  Output:
//              pSeqIndex:  流水号指针
//              dwIndexLen: 流水号的长度
//  Return:
//              TRUE:       取得流水号
//              FALSE:      无法取得流水号
//  Note:       某种协议方式可能没有流水号的概念
///////////////////////////////////////////////////////////////////////////////
BOOL __stdcall
DllGetApplyIndex(char *pBuf, DWORD dwLen, char *pSeqIndex, DWORD &dwIndexLen)
{
    LPAPPLY_MSG     pReply;
    DWORD           dwSeq;
	
    InitDll();
	
	//    ASSERT(dwLen == sizeof(STQ_PACKET) + sizeof(MSG_ADDRESS));
    ASSERT(dwIndexLen >= sizeof(DWORD));    
	
	pReply = (LPAPPLY_MSG)&pBuf[sizeof(MSG_ADDRESS)];
    dwSeq = pReply->dwRequestID;
    memcpy(pSeqIndex, &dwSeq, sizeof(DWORD));
    dwIndexLen = sizeof(DWORD);
	
    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//  Func:       
//  Describe:   取得应答包的消息类型(ICD侧的命令字)
//  Input:      
//              pBuf:       消息指针
//              dwLen:      消息长度
//              dwNameLen:  pName缓冲区的长度
//  Output:
//              pName:      消息类型指针
//              dwNameLen:  消息类型的长度
//  Return:
//              TRUE:       取得命令字
//              FALSE:      无法取得命令字
//  Note:       
///////////////////////////////////////////////////////////////////////////////
BOOL __stdcall
DllGetReplyMsgType(char *pBuf, DWORD dwLen, char *pName, DWORD &dwNameLen)
{
    LPAPPLY_MSG     pReply;
    char            pMIDCMD[CMD_SIZE + 1];
    CString         strMIDCMD;
	
    InitDll();
	//    ASSERT(dwLen == sizeof(STQ_PACKET) + sizeof(MSG_ADDRESS));
	
    pReply = (LPAPPLY_MSG)&pBuf[sizeof(MSG_ADDRESS)];
    memcpy(pMIDCMD, pReply->pCMD, sizeof(pReply->pCMD));
    pMIDCMD[sizeof(pReply->pCMD)] = 0;
    strMIDCMD = pMIDCMD;             // MID侧命令字
	
    for (int ii = 0; ii < CONFIG_NUM; ii++)
    {
        if (!g_ParamConfig[ii].bUsed)
        {
            break;
        }
		
        // MID侧命令字匹配
        if (!strMIDCMD.CompareNoCase(g_ParamConfig[ii].pMIDCMD))
        {
            if (sizeof(g_ParamConfig[ii].pICDCMD) > dwNameLen)
            {
                return FALSE;   // 输入缓冲区大小不够
            }
            else
            {
                strcpy(pName, g_ParamConfig[ii].pICDCMD);
                dwNameLen = strlen(g_ParamConfig[ii].pICDCMD);
                return TRUE;
            }
        }
    }
	
    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
//  Func:       
//  Describe:   取得应答包的长度
//  Input:      
//              pBuf:       消息指针
//              dwLen:      消息长度
//  Output:
//              bEndReply:  第一个消息是否为应答结束包
//  Return:
//              0:          该缓冲区内没有一个完整的包
//              -1:         该缓冲区内的数据有误
//              >0:         该缓冲区内第一个消息包的长度
//  Note:       pBuf内可能有多个消息包, 但是本函数仅返回第一个消息包的长度
///////////////////////////////////////////////////////////////////////////////
int  __stdcall
DllGetMsgLen(char *pBuf, DWORD dwLen, BOOL &bEndReply)
{
    LPAPPLY_MSG pReply;
	
    InitDll();
	
    if (dwLen < sizeof(APPLY_MSG))
    {
        return 0;
    }
    
    pReply = (LPAPPLY_MSG)pBuf;
    bEndReply = pReply->byMorePkt == 0 ? TRUE : FALSE;

    if (dwLen < htonl(pReply->dwLen))
    {
        return 0;
    }
	else
	{
		return htonl(pReply->dwLen);
	}
}

///////////////////////////////////////////////////////////////////////////////
//  Func:       
//  Describe:   获取启动消息
//  Input:      
//              dwStartMsgType: 启动消息类型
//              dwLen:          pBuf缓冲区的大小
//  Output:
//              pBuf:       消息指针
//              dwLen:      消息长度
//  Return:
//              TRUE:       获取成功
//              FALSE:      获取失败
//  Note:       
///////////////////////////////////////////////////////////////////////////////
BOOL __stdcall 
DllGetStartMsg(char *pBuf, DWORD &dwLen, DWORD dwStartMsgType)
{
    InitDll();
    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
//  Func:       
//  Describe:   获取注册消息
//  Input:      
//              dwLoginMsgType: 注册消息类型
//              dwLen:          pBuf缓冲区的大小
//  Output:
//              pBuf:       消息指针
//              dwLen:      消息长度
//  Return:
//              TRUE:       获取成功
//              FALSE:      获取失败
//  Note:       
///////////////////////////////////////////////////////////////////////////////
BOOL __stdcall 
DllGetLoginMsg(char *pBuf, DWORD &dwLen, DWORD dwLoginMsgType)
{
    InitDll();
    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
//  Func:       
//  Describe:   是否启动成功
//  Input:      
//              pBuf:           启动应答消息指针
//              dwLen:          启动应答消息长度
//              dwStartMsgType: 启动消息类型
//  Output:
//  Return:
//              TRUE:       启动成功
//              FALSE:      启动失败
//  Note:       
///////////////////////////////////////////////////////////////////////////////
BOOL __stdcall 
DllIsStartSucc(char *pBuf, DWORD &dwLen, DWORD dwStartMsgType)
{
    InitDll();
    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
//  Func:       
//  Describe:   是否注册成功
//  Input:      
//              pBuf:           注册应答消息指针
//              dwLen:          注册应答消息长度
//              dwLoginMsgType: 注册消息类型
//  Output:
//  Return:
//              TRUE:       注册成功
//              FALSE:      注册失败
//  Note:       
///////////////////////////////////////////////////////////////////////////////
BOOL __stdcall 
DllIsLoginSucc(char *pBuf, DWORD &dwLen, DWORD dwStartMsgType)
{
    InitDll();
    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
//  Func:       
//  Describe:   根据原始请求消息将第三方前置机发来的应答消息转换成AppSvr类型的消息
//  Input:      
//              pBuf:       消息指针
//              dwLen:      消息长度
//              pApply:     原始请求消息指针
//              dwApplyLen: 原始请求消息长度
//              dwMsgLen:   pMsg缓冲区长度
//  Output:
//              pMsg:       转换后的消息指针
//              dwMsgLen:   转换后的消息长度
//              bEndReply:  应答结束标志
//  Return:
//              TRUE:       转换成功
//              FALSE:      转换失败
//  Note:       
///////////////////////////////////////////////////////////////////////////////
BOOL
TransToAppSvrMsg(char *pBuf, DWORD dwLen, char *pApply, DWORD dwApplyLen, char *pMsg, 
                 DWORD &dwMsgLen, BOOL &bEndReply)
{
    LPAPPLY_MSG	    pReply;
	LPAPPLY_MSG		pSrc;
	LPAPPLY_MSG		pDst;
	LPPARAM_CONFIG  pConfig;	
	
    InitDll();
	
	if (!pApply || !pBuf)				                //20030425 
	{
		return FALSE;
	}
	
	pReply = (LPAPPLY_MSG)&pBuf[sizeof(MSG_ADDRESS)];	// 计费前置机应答报文
	pSrc = (LPAPPLY_MSG)&pApply[sizeof(MSG_ADDRESS)];	// 原始请求报文
    pDst  = (LPAPPLY_MSG)pMsg;						    // 转换后的报文
	
	//-->tu delete 下面这一句以防止异常 20030617
	//pBuf[sizeof(MSG_ADDRESS) + htonl(pDst->dwLen)] = 0;	//20030425 LIUSHAOHUA 
    //<--
    if (dwLen - sizeof(MSG_ADDRESS) > dwMsgLen)
    {
        // 输出缓冲区不足
        return FALSE;
    }
	
    // 只需要更换请求ID
    memcpy(pDst, pReply, dwLen - sizeof(MSG_ADDRESS));
    pDst->dwRequestID = pSrc->dwRequestID;
    
	DWORD dwLength;
	dwLength = htonl(pDst->dwLen) - sizeof(APPLY_MSG);
	
	pConfig = GetParamConfigFromICDName(pSrc->pCMD);
	
	CString strResult;

    //modify by xuwenfu 20090327 提到分支外定义,避免重复申请变量
	CString strTitle;
	CString strWidth;
	CString strRet ;
	CString strNewData;
	//modify by xuwenfu 20090327 将获取Reply数据提到分支外面

	CString strData;
	strData = pReply->pData;
	//保证获取数据内容完整
	strData = strData.Left(htonl(pReply->dwLen) -sizeof(APPLY_MSG) + 1);

	//add by huguangwen 2009032
	LPBRIEF_CONFIG  pBriefConfig;
	pBriefConfig = GetBriefConfigFromMIDName(pSrc->pCMD);
	
	//写日志
	NewWriteLog(pDst->pData, "TransToAppSvrMsg", pReply->pCMD, pReply->dwRequestID);
    if (htonl(pReply->dwSeq) <= 1)
    {
		if (!_stricmp(pSrc->pCMD, "10022"))//2.5	10022用户基本信息查询
		{
			CString strResult;
			CString strCustId;//客户编号
			CString strCustName;//客户姓名
			CString strSegmentId;//品牌id
			CString strSegmentName;//品牌名称
			CString strAreaID;//地区代码
			CString strAreaName;//地区名称
			CString strCustGread;//客户级别
			CString strCustGread2;//政企客户服务等级 
			CString strProductStatusName;//用户状态
			CString strAddress;//宽带地址
			//////////////////////////////////////////////////////////////////////////
			CString strInnerXML;
			CMarkup xml;
			CString strANSIData;
			strANSIData = UTF8ToANSI(strData);

			//创建xml对象
			if (xml.SetDoc(strANSIData))
			{
				if (xml.FindElem_C(_T("/soap:Envelope/soap:Bodyns1:querycustinfo3Response/ns1:out")))
				{
					strInnerXML = xml.GetData();
					if (xml.SetDoc(strInnerXML))
					{
						if(xml.FindElem_C(_T("/custInfo/resultId")))
						{
							strResult = xml.GetData();
							if (!strResult.Compare(_T("0")))
							{
								//客户编号
								if(xml.FindElem_C(_T("/custInfo/custId")))
								{
									strCustId = xml.GetData();
								}
								else
								{
									strData = _T("111~取custId字段出错");
								}
								//客户名称
								if(xml.FindElem_C(_T("/custInfo/custName")))
								{
									strCustName = xml.GetData();
								}
								else
								{
									strData = _T("111~取custId字段出错");
								}
								//品牌id
								if(xml.FindElem_C(_T("/custInfo/segmentId")))
								{
									strSegmentId = xml.GetData();
								}
								else
								{
									strData = _T("111~取segmentId字段出错");
								}
								//品牌名称
								if(xml.FindElem_C(_T("/custInfo/segmentName")))
								{
									strSegmentName = xml.GetData();
								}
								else
								{
									strData = _T("111~取segmentName字段出错");
								}
								//地区代码
								if(xml.FindElem_C(_T("/custInfo/areaId")))
								{
									strAreaID = xml.GetData();
								}
								else
								{
									strData = _T("111~取areaId字段出错");
								}
								//地区名称
								if(xml.FindElem_C(_T("/custInfo/areaName")))
								{
									strAreaName = xml.GetData();
								}
								else
								{
									strData = _T("111~取areaName字段出错");
								}
								//客户级别
								if(xml.FindElem_C(_T("/custInfo/custGread")))
								{
									strCustGread = xml.GetData();
								}
								else
								{
									strData = _T("111~取custGread字段出错");
								}
								//政企客户服务等级
								if(xml.FindElem_C(_T("/custInfo/custGread2")))
								{
									strCustGread2 = xml.GetData();
								}
								else
								{
									strData = _T("111~取custGread2字段出错");
								}
								//用户状态
								if(xml.FindElem_C(_T("/custInfo/productStatusName")))
								{
									strProductStatusName = xml.GetData();
								}
								else
								{
									strData = _T("111~取productStatusName字段出错");
								}
								//宽带安装地址
								if(xml.FindElem_C(_T("/custInfo/address")))
								{
									strAddress = xml.GetData();
								}
								else
								{
									strData = _T("111~取address字段出错");
								}
								strData = _T("0~") + strCustId + _T("~") + strCustName + _T("~") + strSegmentId + _T("~")
									+ strSegmentName + _T("~") + strAreaID + _T("~") + strAreaName + _T("~") + strCustGread + _T("~") + 
									strCustGread2 + _T("~") + strProductStatusName + _T("~") + strAddress + _T(";");
							}
							else
							{
								strData = _T("111~resultID为非0");
							}
						}
						else
						{
							strData = _T("111~取resultID字段出错;");
						}
					}
					else
					{
						strData = _T("111~创建内部xml对象出错;");
					}
				}
				else
				{
					strData = _T("111~获取内部xml字符串出错;");
				}
			}
			else
			{
				strData = _T("111~创建xml对象出错;");
			}
			//////////////////////////////////////////////////////////////////////////
			strTitle = pConfig->strTitle;
			strWidth = pConfig->strWidth;		
			
			strRet = GetField(strData, g_strRSep, g_strFSep);
			
			if (!strRet.Compare("0"))
			{
				strNewData = strRet + g_strRSep;
				strNewData += strTitle + strWidth;
				//added 20091120 与ivr级别保持一致
				CString strUserClass;  //客户级别
				CString strTmp;

				
				for (int i = 0; i < 6; i++)
				{
					strTmp = GetField(strData, g_strRSep, g_strFSep);
					strNewData += strTmp + g_strFSep;
				}
				strUserClass = GetField(strData, g_strRSep, g_strFSep);
				if(!_stricmp(strUserClass,"1000"))
				{
					strUserClass = "60";
				}
				else if(!_stricmp(strUserClass,"1100"))
				{
					strUserClass = "50";
				}
				else if(!_stricmp(strUserClass,"1200"))
				{
					strUserClass = "40";
				}
				else if(!_stricmp(strUserClass,"1300") || !_stricmp(strUserClass,"0"))
				{
					strUserClass = "10";
				}	
				strNewData += strUserClass+g_strFSep;

	            CString strCUserclass; //政企级别，add by tongyufeng，20100513，CRM返回第9个字段代表政企级别

                strCUserclass = GetField(strData, g_strRSep, g_strFSep);
				if(!_stricmp(strCUserclass,"0"))
				{
                    strCUserclass="";
				}

				strNewData += strCUserclass+g_strFSep;

                //10022命令字增加了用户状态、宽带用户安装地址，增加此2项的返回
				CString cUsertype;
				CString Bnanzhuang;

                cUsertype = GetField(strData, g_strRSep, g_strFSep);
				strNewData += cUsertype+g_strFSep;

				Bnanzhuang = GetField(strData, g_strRSep, g_strFSep);
                strNewData += Bnanzhuang+g_strRSep;
                
			}
			else
			{
				strNewData = "111;查询结果;40;查询结束,没有相关记录;";
			}
		}
		else if (!_stricmp(pSrc->pCMD, "10033"))//2.8	 (CRM接口)+10033获取产品ID、帐户ID和客户ID
		{
			CString strUserFlag;
			CString strClientFlag;
			CString strAccountsFlag;
						
			strTitle = pConfig->strTitle;
			strWidth = pConfig->strWidth;
			
			strRet = GetField(strData, g_strRSep, g_strFSep);
			
			if (!strRet.Compare("0"))
			{
				strNewData = strRet + g_strRSep;
				strNewData += strTitle + strWidth;
				strNewData += strData;
			}
			else
			{
				strNewData = "111;查询结果;40;查询结束,没有相关记录;";
			}

		}
		else if (!_stricmp(pSrc->pCMD, "10035"))//2.10	(CRM接口)+10035用户产品资料查询
		{
			CString strDatatoApp;

			strTitle = pConfig->strTitle;
			strWidth = pConfig->strWidth;
			
			strRet = GetField(strData, g_strRSep, g_strFSep);
			
			if (!strRet.Compare("0"))
			{
				CString strUserInfo;
				CString strUserInfotemp;
				CString strAppendInfo;
				CString strUserInfoData;

				//BOSS返回数据格式(共22项，前15项只有一个记录，后7项可能多记录返回)

				//返回码；用户标识~客户标识~用户名称~主体产品标识~主体产品名称~业务接入号~用户状态~
				//用户状态名称~用户生效时间~用户失效时间~用户地址~催缴标识~业务密码~计费模式~用户群组标识；
				//服务标识~服务名称~生效日期~失效日期~附加属性~操作标识~状态修改时间；[若有多条，并列返回]

				

				strUserInfo = GetField(strData, g_strRSep, g_strRSep); //取用户信息

				for(int i=0 ; i<11 ; i++)   //前11个不用做符号和中文的转换
				{
					strUserInfoData += GetField(strUserInfo, g_strFSep, g_strRSep);
					strUserInfoData += "~";
				}

				//处理第12个参数 BEGIN
				strUserInfotemp = GetField(strUserInfo, g_strFSep, g_strRSep);

				for (int kk = 0; kk < pBriefConfig->iCount; kk++)
				{
					if (12 == atoi(pBriefConfig->pCodeConfig[kk].pIndex))
					{
						strUserInfotemp = GetBriefCode(pBriefConfig->pCodeConfig[kk].pContent, strUserInfotemp);
					}
					else
					{
						strUserInfotemp = strUserInfotemp;
					}
				}
				strUserInfoData += strUserInfotemp;
				strUserInfoData += "~";
				//处理第12个参数 END
				
				//第13个参数不用转换
				strUserInfoData += GetField(strUserInfo, g_strFSep, g_strRSep);
				strUserInfoData += "~";	
				
				//14 BEGIN
				strUserInfotemp = GetField(strUserInfo, g_strFSep, g_strRSep);
				for (int kk = 0; kk < pBriefConfig->iCount; kk++)
				{
					if (14 == atoi(pBriefConfig->pCodeConfig[kk].pIndex))
					{
						strUserInfotemp = GetBriefCode(pBriefConfig->pCodeConfig[kk].pContent, strUserInfotemp);
					}
					else
					{
						strUserInfotemp = strUserInfotemp;
					}
				}
				strUserInfoData += strUserInfotemp;
				strUserInfoData += "~";
				//14 END

				//第15个参数不用转换
				strUserInfoData += GetField(strUserInfo, g_strFSep, g_strRSep);
				strUserInfoData += "~";	
				
				//转换后的用户信息部分字符串加给返回数据部分
				strDatatoApp = strUserInfoData;
				
				//附加信息处理 BEGIN
				strAppendInfo = GetField(strData, g_strRSep, g_strRSep);//取第一条附加信息
				strAppendInfo += ";";
				strDatatoApp += strAppendInfo;

				strAppendInfo = GetField(strData, g_strRSep, g_strRSep);//取第二条附加信息
				
				while(strAppendInfo.GetLength() != 0)
				{
					strAppendInfo += ";";
					strDatatoApp +="~~~~~~~~~~~~~~~";//15个分隔符，填补用户信息部分
					strDatatoApp += strAppendInfo;
					strAppendInfo = GetField(strData, g_strRSep, g_strRSep);//取一条附加信息	
				}
				//附加信息处理 END
				
				strNewData = strRet + g_strRSep;
				strNewData += strTitle + strWidth;
				strNewData += strDatatoApp;
			}
			else
			{
				strNewData = "111;查询结果;40;查询结束,没有相关记录;";
			}
		}
    	// add by tongyufeng,20101228,增加程控自动业务办理接口
		else if (!_stricmp(pSrc->pCMD, "10041"))
		{			
			strTitle = "操作结果;";
			strWidth = "40;"; 
			
			strRet = GetField(strData, g_strRSep, g_strFSep);

			strNewData = strRet + g_strRSep;
			strNewData += strTitle + strWidth;
			strNewData += strRet;
			strNewData += ";";
			
            /*
			if (!strRet.Compare("0"))
			{
				strNewData = strRet + g_strRSep;
				strNewData += strTitle + strWidth;
                strNewData += strRet;
				strNewData += ";";
			}
			else
			{
				strNewData = "111;查询结果;40;操作失败;";
			}*/


		}

    	// add by tongyufeng,2011-02-21,所有话费一次返回，短信使用
		else if (!_stricmp(pSrc->pCMD, "10045"))
		{			
			strTitle = "操作结果;";
			strWidth = "40;"; 
			
			strRet = GetField(strData, g_strRSep, g_strFSep);

			strNewData = strRet + g_strRSep;
			strNewData += strTitle + strWidth;
			strNewData += strRet;
			strNewData += ";";
			

		}

		else if (!_stricmp(pSrc->pCMD, "10034"))//2.9	 (CRM接口)+10034 修改客户密码
		{			
			strTitle = "操作结果;";
			strWidth = "200;";  //15个
			
			strRet = GetField(strData, g_strRSep, g_strFSep);
			
			strNewData = strRet + g_strRSep;
			strNewData += strTitle + strWidth;
			strNewData += strRet;
			strNewData += ";";
		}
		else if (!_stricmp(pSrc->pCMD, "10015"))//2.4	(CRM接口)10015 修改用户密码
		{			
			strTitle = "操作结果;";
			strWidth = "200;";  
			
			strRet = GetField(strData, g_strRSep, g_strFSep);
			
			strNewData = strRet + g_strRSep;
			strNewData += strTitle + strWidth;
			strNewData += strRet;
			strNewData += ";";
		}
		else if (!_stricmp(pSrc->pCMD, "10036"))//2.11	 (CRM接口)+10036客户产品资料查询
		{
			CString strRecord;
			CString strDataToApp;
			CString strField;

			strTitle = pConfig->strTitle;
			strWidth = pConfig->strWidth;
			
			strRet = GetField(strData, g_strRSep, g_strFSep);
			
			if (!strRet.Compare("0"))
			{
				strRecord = GetField(strData, g_strRSep, g_strRSep);
				while(strRecord.GetLength() != 0)
				{					
					for( int i = 0 ; i < 5 ; i++ )
					{
						strDataToApp += GetField(strRecord, g_strFSep, g_strRSep);
						strDataToApp += "~";
					}

					//对第6个参数进行 标识-》中文的转换 BEGIN
					strField = GetField(strRecord, g_strFSep, g_strRSep);
					for (int kk = 0; kk < pBriefConfig->iCount; kk++)
					{
						if (6 == atoi(pBriefConfig->pCodeConfig[kk].pIndex))
						{
							strField = GetBriefCode(pBriefConfig->pCodeConfig[kk].pContent, strField);
						}
						else
						{
							strField = strField;
						}
					}
					strDataToApp += strField;
					strDataToApp += "~";
					//对第6个参数进行 标识-》中文的转换 END
					
					//第7个参数不用转
					strDataToApp += GetField(strRecord, g_strFSep, g_strRSep);
					strDataToApp += ";";
					
					//取下一条记录
					strRecord = GetField(strData, g_strRSep, g_strRSep);

				}

				strNewData = strRet + g_strRSep;
				strNewData += strTitle + strWidth;
				strNewData += strDataToApp;
			}
			else
			{
				strNewData = "111;查询结果;40;查询结束,没有相关记录;";
			}
		}
		
		else if (!_stricmp(pSrc->pCMD, "10037"))//2.12	 (CRM接口)+10037 帐户信息查询
		{
			CString strRecord;
			CString strDataToApp;
			CString strField;

			strTitle = pConfig->strTitle;
			strWidth = pConfig->strWidth;
			
			strRet = GetField(strData, g_strRSep, g_strFSep);
			
			if (!strRet.Compare("0"))
			{
				strRecord = GetField(strData, g_strRSep, g_strRSep);
				while(strRecord.GetLength() != 0)
				{					
					for( int i = 0 ; i < 6 ; i++ )
					{
						strDataToApp += GetField(strRecord, g_strFSep, g_strRSep);
						strDataToApp += "~";
					}

					//对第7个参数进行 标识-》中文的转换 BEGIN
					strField = GetField(strRecord, g_strFSep, g_strRSep);
					for (int kk = 0; kk < pBriefConfig->iCount; kk++)
					{
						if (7 == atoi(pBriefConfig->pCodeConfig[kk].pIndex))
						{
							strField = GetBriefCode(pBriefConfig->pCodeConfig[kk].pContent, strField);
						}
						else
						{
							strField = strField;
						}
					}
					strDataToApp += strField;
					strDataToApp += "~";

					//8到13不转换
					for(int i = 7 ; i < 13 ; i++ )
					{
						strDataToApp += GetField(strRecord, g_strFSep, g_strRSep);
						strDataToApp += "~";
					}

					//对第14个参数进行 标识-》中文的转换 BEGIN
					strField = GetField(strRecord, g_strFSep, g_strRSep);
					for (int kk = 0; kk < pBriefConfig->iCount; kk++)
					{
						if (14 == atoi(pBriefConfig->pCodeConfig[kk].pIndex))
						{
							strField = GetBriefCode(pBriefConfig->pCodeConfig[kk].pContent, strField);
						}
						else
						{
							strField = strField;
						}
					}
					strDataToApp += strField;
					strDataToApp += "~";
					//对第14个参数进行 标识-》中文的转换 END
					
					//第15个参数不用转
					strDataToApp += GetField(strRecord, g_strFSep, g_strRSep);
					strDataToApp += ";";
					
					//取下一条记录
					strRecord = GetField(strData, g_strRSep, g_strRSep);

				}

				strNewData = strRet + g_strRSep;
				strNewData += strTitle + strWidth;
				strNewData += strDataToApp;
			}
			else
			{
				strNewData = "111;查询结果;40;查询结束,没有相关记录;";
			}
		}
		
		else if (!_stricmp(pSrc->pCMD, "10038"))//2.13	 (CRM接口)+10038 套餐信息查询
		{			
			strTitle = pConfig->strTitle;
			strWidth = pConfig->strWidth;
			
			strRet = GetField(strData, g_strRSep, g_strFSep);
			
			if (!strRet.Compare("0"))
			{
				strNewData = strRet + g_strRSep;
				strNewData += strTitle + strWidth;
				strNewData += strData;
			}
			else
			{
				strNewData = "111;查询结果;40;查询结束,没有相关记录;";
			}
		}
		/************************************************************************/
		/* added by wangyue,2011-11-30,10046/10047                                                                     */
		/************************************************************************/
		else if (!_stricmp(pSrc->pCMD, "10046"))//2.18	10046查询同账户下的宽带号码
		{
			strTitle = _T("返回码~序号1~宽带号码1~序号2~宽带号码2~");
			strWidth = _T("10~10~20~10~20~");
			
			strRet = GetField(strData, g_strRSep, g_strFSep);
			CString strNo1,strBroadNo1,strNo2,strBroadNo2,strTemp;

			if (!strRet.Compare("0"))
			{
				strNewData += strTitle+strWidth;
				strNewData += _T("0");
				strTemp = GetField(strData, g_strRSep, g_strFSep);
				while(!strTemp.IsEmpty());
				{
					strNewData += g_strFSep + strTemp;
				}
			}
			else
			{
				strRet = _T("111");
			}
		}
		else if (!_stricmp(pSrc->pCMD, "10047"))//2.19	10047查询同身份证办理的宽带号码
		{
			strTitle = _T("返回码~序号1~宽带号码1~序号2~宽带号码2~");
			strWidth = _T("10~10~20~10~20~");
			
			strRet = GetField(strData, g_strRSep, g_strFSep);
			CString strNo1,strBroadNo1,strNo2,strBroadNo2,strTemp;
			
			if (!strRet.Compare("0"))
			{
				strNewData += strTitle+strWidth;
					strNewData += _T("0");
				strTemp = GetField(strData, g_strRSep, g_strFSep);
				while(!strTemp.IsEmpty());
				{
					strNewData += g_strFSep + strTemp;
				}
			}
			else
			{
				strRet = _T("111");
			}
		}
		// add by xuwenfu 20090327 必须有else
		else
		{
			strNewData = strData;
		}
        //add by xuwenfu 公共调用，提到分支函数外面
		strcpy(pDst->pData, strNewData);
		pDst->dwLen = htonl(strNewData.GetLength() + sizeof(APPLY_MSG));
    }
	// add by xuwenfu 20090327 增加多包处理
	else if (htonl(pReply->dwSeq) > 1)
	{
		GetField(strData, g_strRSep, g_strRSep); //去掉返回码
		strNewData = strData;
        strcpy(pDst->pData, strNewData);
        pDst->dwLen = htonl(strNewData.GetLength() + sizeof(APPLY_MSG));
	}
	
	dwMsgLen = htonl(pDst->dwLen);
	//add by xuwenfu 20090327 增加结束符 /0
	pMsg[dwMsgLen - 1] = 0;
	bEndReply = !pDst->byMorePkt;
	
    return TRUE;
}





