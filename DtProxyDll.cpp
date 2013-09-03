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
CString g_strVPCodePositive;    // ����Ϊ����ʱ��VP�������뼯
CString g_strVPCodeNegative;    // ����Ϊ����ʱ��VP�������뼯
CString g_strAuto1860Code;      // 1860������ӳ�伯
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
//  Describe:   ��IVR���͵���Ϣת��Ϊ�ⲿ�ӿ�Э��
//  Input:      
//              pBuf:       ��Ϣָ��
//              dwLen:      ��Ϣ����
//              dwTransLen: pTrans�������Ĵ�С
//              dwIndexLen: pSeqIndex�������Ĵ�С
//  Output:
//              pTrans:     ת�������Ϣָ��
//              dwTransLen: ת�������Ϣ����
//              pSeqIndex:  ���ɵ���ˮ��ָ��
//              dwIndexLen: ���ɵ���ˮ�ŵĳ���
//  Return:
//              TRUE:       ת���ɹ�
//              FALSE:      ת��ʧ��
//  Note:       dwTransLen, dwIndexLen�����������������. 
//              ��������Ĭ��Ϊ�����������һ��������ʵ��,����ָ����������
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
	
    // ��ȡ����������˵��
    pConfig = GetParamConfigFromICDName(pSPCall->pProcName);    
	if (!pConfig)
    {						
        return FALSE;   // δ֪������
    }
    strMIDCMD = pConfig->pMIDCMD;
	
    // ������ˮ��
    if (!MakeSequence(pSeqIndex, dwIndexLen))
    {
        return FALSE;   // ��ˮ������ʧ��
    }
    dwSeq = *(DWORD *)pSeqIndex;
	
    // ����ת��(ͨ��ת����ʽ)
    pParam = &pSPCall->SPPara;
    pSrcData = (char *)pParam + sizeof(SP_PARAM) * pSPCall->byParaNum;
    for (int ii = 0; ii < pSPCall->byParaNum; ii++)
    {
        // ��ת���������, ����Ĭ�ϸ�ʽת��
        if (pParam->byParaType != OUTPUT_PARAM)
        {
            strData += TransParamToString(pParam, pSrcData);                
            strData += g_strFSep;
        }
		
        pSrcData += pParam->wDataLen;
        pParam++;
    }
	
	//д��־
	NewWriteLog(strData, "DllTransIVRMsg", pConfig->pICDCMD, dwSeq);
	
	if(!_stricmp(pConfig->pICDCMD, "P_SCEGetUserPaytype")) //��ȡ�û���������
	{
		CString strServiceNo; //�������
		CString strCallerNo;  //���к���  ��ʱ����
		CString strCalleeNo;  //���к���  ��ʱ����
		strServiceNo = GetField(strData, g_strFSep, g_strRSep);
		GetField(strData, g_strFSep, g_strRSep);
		GetField(strData, g_strFSep, g_strRSep);
		strData = strServiceNo + g_strFSep;
	}

	else if (!_stricmp(pConfig->pICDCMD, "P_SCEHandsetNoTypeJudg")) //�ֻ�������֤
	{
		CString strHandsetNo;
		CString strUserType;   //Ԥ��
		strHandsetNo = GetField(strData, g_strFSep, g_strRSep);
		strUserType = GetField(strData, g_strFSep, g_strRSep);

		strData = strHandsetNo + g_strFSep;
	}

	//modified by huguangwen 20090309  (����BOSS�Ĳ����м����û�����)
	//modified by wangyue 20130318,��I_SCE_Login/P_SCELogin�����ӿںϲ�
	//else if (!_stricmp(pConfig->pICDCMD, "P_SCELogin"))  //������֤
	//{
	//	CString strHandsetNo;
	//	CString strPassword;
	//	CString strUserType;  //�û����ͣ�Ԥ����
	//	
	//	
	//	strHandsetNo = GetField(strData, g_strFSep, g_strRSep);
	//	strPassword  = GetField(strData, g_strFSep, g_strRSep);
	//	strUserType  = GetField(strData, g_strFSep, g_strRSep);

	//	bool bMobileNum = false;
	//	bMobileNum = IsMobileNumber(strHandsetNo);
	//	if (true == bMobileNum)   //c��
	//	{
	//		strUserType = "4";
	//	}
	//	else					  //����
	//	{
	//		strUserType = "2";
	//	}

	//	strData = strHandsetNo + g_strFSep + strUserType + g_strFSep + strPassword + g_strFSep;
	//}

	// add by tongyufeng,20101228,����SP�Զ�ҵ�����ӿ�
	//modified by wangyue,20130506,��Ϊ�ӿڸ��������޸���Σ����ӵ��к������������
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_AutoService"))  //�̿�ҵ�����
	{
		CString strCityCode;//���д���
		CString strChannelID;//����ID��1002��10000��ϵͳ
		CString strHandsetNo; //�û�����
		CString strServiceType;  //ҵ�����ͣ�0-�߲�����  1-������ʾ 2-���б��� 3-���еȴ�
		CString strOptType;  //�������ͣ�0��ͨ��1�˶�
		CString strUserType; //�û����ͣ�1-�ֻ���10-�̻�
		
		strCityCode = GetField(strData, g_strFSep, g_strRSep);
		strChannelID = GetField(strData, g_strFSep, g_strRSep);
		strHandsetNo = GetField(strData, g_strFSep, g_strRSep);
		strUserType = GetField(strData, g_strFSep, g_strRSep);
		strServiceType = GetField(strData, g_strFSep, g_strRSep);
		strOptType = GetField(strData, g_strFSep, g_strRSep);
		
		strData = strCityCode + g_strFSep + strChannelID + g_strFSep + strHandsetNo + g_strFSep + strUserType + g_strFSep + strServiceType + g_strFSep 
			+ strOptType + g_strFSep;
	}

	// add by tongyufeng,2011-02-21,���ӻ��Ѷ���������
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_AllFeeSms"))
	{
		CString strHandsetNo; //�û�����
		CString strUserType; //�û����ͣ�1-�ֻ���10-�̻�
		CString strCitycode; //�û�����
		
		strHandsetNo = GetField(strData, g_strFSep, g_strRSep);
		strUserType = GetField(strData, g_strFSep, g_strRSep);
        //strCitycode = GetField(strData, g_strFSep, g_strRSep);
		
		strData = strHandsetNo + g_strFSep + _T("1~")/*querytype*/ + strUserType + g_strFSep;
	}
	
	else if (!_stricmp(pConfig->pICDCMD, "P_SCEChangePassword")
		|| !_stricmp(pConfig->pICDCMD, "I_SCE_PasswdRest"))  //�޸�����\��������
	{
		CString strHandsetNo;
		CString strOldPWD;  //������
		CString strNewPWd;  //������
		CString strUserType; //nOption��1��ͬ��2�̶���3�ƶ���4���պ��룩
		CString strCityCode;//���д���

		strHandsetNo = GetField(strData, g_strFSep, g_strRSep);
		strOldPWD = GetField(strData, g_strFSep, g_strRSep);
		strNewPWd = GetField(strData, g_strFSep, g_strRSep);
		strUserType = GetField(strData, g_strFSep, g_strRSep);
		strCityCode = GetField(strData, g_strFSep, g_strRSep);

		bool bMobileNum = false;
		bMobileNum = IsMobileNumber(strHandsetNo);
		if (true == bMobileNum)   //c��
		{
			strUserType = "4";
		}
		else					  //����
		{
			strUserType = "2";
		}
		if(!_stricmp(pConfig->pICDCMD, "P_SCEChangePassword"))//�޸�����
		{
			strData = strUserType + g_strFSep + strHandsetNo + g_strFSep + strCityCode + g_strFSep +_T("")/*��Ϣ��ˮ��*/
				+g_strFSep + _T("1002")/*ϵͳ��ʶ 1002:10000��*/ +g_strFSep + _T("")/*�Ӵ�ID*/ + g_strFSep + strOldPWD + g_strFSep + strNewPWd + g_strFSep + _T("1")/*1:�����޸ģ�2����������*/
				+ g_strFSep + _T("2")/*1:��ѯ���룬2:��������,��������CRM2.0�Ժ�ֻ��ҵ�����룬�����ֲ�ѯ����*/+g_strFSep;
		}
		else if(!_stricmp(pConfig->pICDCMD, "I_SCE_PasswdReset"))//��������
		{
			strData = strUserType + g_strFSep + strHandsetNo + g_strFSep + strCityCode + g_strFSep +_T("")/*��Ϣ��ˮ��*/
				+g_strFSep + _T("1002")/*ϵͳ��ʶ 1002:10000��*/ +g_strFSep + _T("")/*�Ӵ�ID*/ + g_strFSep + strOldPWD + g_strFSep + strNewPWd + g_strFSep + _T("2")/*1:�����޸ģ�2����������*/
				+ g_strFSep + _T("2")/*1:��ѯ���룬2:��������*/+g_strFSep;
		}
	}

	else if (!_stricmp(pConfig->pICDCMD, "P_SCEResetPassword"))  //��������
	{
		CString strHandsetNo;
		CString strPassWord;
		CString strUserType;
		strHandsetNo = GetField(strData, g_strFSep, g_strRSep);
		strPassWord = GetField(strData, g_strFSep, g_strRSep);
		strUserType = GetField(strData, g_strFSep, g_strRSep);

		strData = strHandsetNo + g_strFSep + strPassWord + g_strFSep;
	}

	//modified by huguangwen 20090309  (����BOSS�Ĳ����м����û�����,�ͱ���������������)
	else if (!_stricmp(pConfig->pICDCMD, "P_SCEGetTotalFee"))  //�½Ự�Ѳ�ѯ(�󸶷�)֧�֣�����
	{
		CString strHandsetNo;
		CString striType;    //��������(������)
		CString strUserType;
		CString strStartTime; //��ѯ�·�
		CString strEndtime; //��ʱ����
		
		strHandsetNo = GetField(strData, g_strFSep, g_strRSep);
		GetField(strData, g_strFSep, g_strRSep);
		strUserType = GetField(strData, g_strFSep, g_strRSep);
		strStartTime = GetField(strData, g_strFSep, g_strRSep);
		GetField(strData, g_strFSep, g_strRSep);

		bool bMobileNum = false;
		bMobileNum = IsMobileNumber(strHandsetNo);
		if (true == bMobileNum)   //c��
		{
			strUserType = "1";
		}
		else					  //����
		{
			strUserType = "10";
		}

		strData = strHandsetNo + g_strFSep + strUserType + g_strFSep
			+ strStartTime + g_strFSep + "0" + g_strFSep;
	}

	else if (!_stricmp(pConfig->pICDCMD, "P_SCEPhoneIDCheck"))  //֤��������֤
	{
		CString strHandsetNo;  //�û��ֻ�����
		CString strIDNo;       //֤������
		CString strUserType;
		strHandsetNo = GetField(strData, g_strFSep, g_strRSep);
		strIDNo = GetField(strData, g_strFSep, g_strRSep);
		strUserType = GetField(strData, g_strFSep, g_strRSep);

		strData = strHandsetNo + g_strFSep + strIDNo + g_strFSep;
	}
	else if (!_stricmp(pConfig->pICDCMD, "P_SCEAppealOpenClose")) //ͣ��������
	{
		CString strHandsetNo;
		CString strAppealType;//��������
		CString strUserType;
		strHandsetNo = GetField(strData, g_strFSep, g_strRSep);
		strAppealType = GetField(strData, g_strFSep, g_strRSep);
		strUserType = GetField(strData, g_strFSep, g_strRSep);

		strData = strHandsetNo + g_strFSep + strAppealType + g_strFSep;
	}

	//modified by huguangwen 20090309  (����BOSS�Ĳ����м����û�����,�ͱ���������������)
	else if (!_stricmp(pConfig->pICDCMD, "P_SCEGetRealTimeFee"))  //ʵʱ����(�󸶷�)
	{
		CString strHandsetNo;
		CString strUserType;
		strHandsetNo = GetField(strData, g_strFSep, g_strRSep);
		strUserType = GetField(strData, g_strFSep, g_strRSep);
		//20130402��wangyue�����̴���dtproxy��ѯ��ʼʱ��ͽ���ʱ�䣬�����µĽӿڲ���Ҫ����������
		GetField(strData, g_strFSep, g_strRSep);
		GetField(strData, g_strFSep, g_strRSep);

		bool bMobileNum = false;
		bMobileNum = IsMobileNumber(strHandsetNo);
		if (true == bMobileNum)   //c��
		{
			strUserType = "1";
		}
		else					  //����
		{
			strUserType = "10";
		}

		strData = strHandsetNo + g_strFSep + "1"/*�ӿ�Э���querytype�ֶΣ�κ������������Ϊ�գ�����Ĭ��Ϊ1*/ + g_strFSep + strUserType  + g_strFSep;
	}


	else if (!_stricmp(pConfig->pICDCMD, "P_SceQueryScore"))  //���ֲ�ѯ
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

	else if (!_stricmp(pConfig->pICDCMD, "P_SCEVAGetTotalFee"))  //����ѯ(�󸶷�)
	{
		CString strHandsetNo;
		CString strUserType;
		strHandsetNo = GetField(strData, g_strFSep, g_strRSep);
		strUserType = GetField(strData, g_strFSep, g_strRSep);
		//20130402��wangyue�����̴���dtproxy��ѯ��ʼʱ��ͽ���ʱ�䣬�����µĽӿڲ���Ҫ����������
		GetField(strData, g_strFSep, g_strRSep);
		GetField(strData, g_strFSep, g_strRSep);

		bool bMobileNum = false;
		bMobileNum = IsMobileNumber(strHandsetNo);
		if (true == bMobileNum)   //c��
		{
			strUserType = "1";
		}
		else					  //����
		{
			strUserType = "10";
		}

		strData = strHandsetNo + g_strFSep + "1"/*�ӿ�Э���querytype�ֶΣ�κ������������Ϊ�գ�����Ĭ��Ϊ1*/ + g_strFSep + strUserType  + g_strFSep;
	}

	//ԭ��������

	
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_GetUserinfor"))  //1.1.1  �û�����I_SCE_GetUserinfor
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
		if (true == bMobileNum)   //c��
		{
			strUserType = "1";
		}
		else					  //����
		{
			strUserType = "10";
		}

		strData = strServiceNo + g_strFSep + strUserType + g_strFSep;
	}
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_ChangePassword"))  //1.1.2  �޸�����I_SCE_ChangePassword
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
		if (true == bMobileNum)   //c��
		{
			strUserType = "1";
		}
		else					  //����
		{
			strUserType = "10";
		}

		strData = strServiceNo + g_strFSep + strUserType + g_strFSep + strOldPassWord + g_strFSep + strPassWord + g_strFSep;		
	}

	//modify by tongyufeng 20111124���޸�Ƿ�Ѳ�ѯ�ӿڣ�IVR���Ϊ4��������ͬʱ��ѯ�˻�����ʹ��IVR����,ȥ���ӿ��ж�
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_GetDebtFee"))   //1.1.3  ��ѯǷ��I_SCE_GetDebtFee
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
		if (true == bMobileNum)   //c��
		{
			strUserType = "1";
		}
		else					  //����
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
	  I_SCE_QueryFreeResource  (�����������ר�ýӿ�)                                                                   */
	/************************************************************************/
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_QueryBrandNoByIDCard"))  //10047 ��ѯͬ���֤����Ŀ������
	{
		CString strIDCard;   //����
		CString strcitycode;   //��������
		
		strIDCard = GetField(strData,g_strFSep,g_strRSep);
		strIDCard.Replace(_T("*"),_T("X"));
		strcitycode = GetField(strData,g_strFSep,g_strRSep);
		
		strData = strIDCard + g_strFSep;
	}
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_QueryFreeResource"))  //10048 ��ѯ�û������Դ
	{
		CString strMobileNum;	//�ֻ�����
		CString strUserType;	//�û�����
		
		strMobileNum = GetField(strData,g_strFSep,g_strRSep);
		strUserType = GetField(strData,g_strFSep,g_strRSep);
		
		strData = strMobileNum + g_strFSep + strUserType + g_strFSep;
	}
	
	/************************************************************************/
	/*end: added by tongyufeng,wangyue,2011-11-24                                                                     */
	/************************************************************************/
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_GetMonthFee"))  //1.1.5  ĳ���ʵ���ѯI_SCE_GetMonthFee
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
		if (true == bMobileNum)   //c��
		{
			strUserType = "1";
		}
		else					  //����
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
	
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_GetBalance"))   //1.1.6  ����ѯI_SCE_GetBalance
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
		if (true == bMobileNum)   //c��
		{
			strUserType = "1";
		}
		else					  //����
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
	
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_HandsetNoTypeJudg"))  //1.1.8  ������Ч��У��I_SCE_HandsetNoTypeJudg
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
		if (true == bMobileNum)   //c��
		{
			strUserType = "1";
		}
		else					  //����
		{
			strUserType = "10";
		}
		
		if (!strNumFlag.Compare("1"))
		{
			strData = strHandsetNo + g_strFSep + strUserType + g_strFSep ; 	
		}		
	}
	
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_CHKNumMatch"))  //1.1.9  �ʺźͺ���ƥ��У�飺I_SCE_CHKNumMatch
	{
		CString strHandsetNo;
		CString strCityCode;
		CString strContractNo;

		strHandsetNo = GetField(strData, g_strFSep, g_strRSep);
		strCityCode = GetField(strData, g_strFSep, g_strRSep);
		strContractNo = GetField(strData, g_strFSep, g_strRSep);
		
		CString strUserType;
		bool bMobileNum = false;//��bMobileNum�ĳ�ʼ��
		bMobileNum = IsMobileNumber(strHandsetNo);
		if (true == bMobileNum)   //c��
		{
			strUserType = "1";
		}
		else					  //����
		{
			strUserType = "10";
		}

		strData = strHandsetNo + g_strFSep + strUserType + g_strFSep + strContractNo + g_strFSep;		
	}

	
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_RealTimeFee"))  //1.1.10  ʵʱ���ò�ѯI_SCE_RealTimeFee
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
		if (true == bMobileNum)   //c��
		{
			strUserType = "1";
		}
		else					  //����
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
		|| !_stricmp(pConfig->pICDCMD, "P_SCELogin"))  //1.1.11  �û�������֤I_SCE_Login
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
		if (true == bMobileNum)   //c��
		{
			strUserType = "4";
		}
		else					  //����
		{
			strUserType = "2";
			//����̻����볤��<=8�����������
			if (strHandsetNo.GetLength()<=8)
			{
				strHandsetNo = strCityCode + strHandsetNo;
			}
		}

		strData = strHandsetNo + g_strFSep + strUserType + g_strFSep + strPassword + g_strFSep + strCityCode + g_strFSep;
	}
	//10033��10046�ӿںϲ�
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
		if (true == bMobileNum)   //c��
		{
			strUserType = "1";
		}
		else					  //����
		{
			strUserType = "10";
		}

		strData = strServiceNoo + g_strFSep + strUserType + g_strFSep;		
	}

	//add by lilong 20090430 begin
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_GetUserinfor1") || !_stricmp(pConfig->pICDCMD, "I_SCE_GetUserinfor2"))  //1.1.1  �û�����I_SCE_GetUserinfor
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
		
		if (true == bMobileNum)   //c��
		{
			strUserType = "1";
		}
		else					  //����
		{
			strUserType = "10";
		}
		
		strData = strServiceNo + g_strFSep + strUserType + g_strFSep;
 
	}
    //add by lilong 20090430 end


    else
    {
        // Ĭ��ΪIVR�����Ĵ洢���̵ĵ��ò��������Լ���𶼺�MID�������һ��
        CString strOldData;
        int     ii = 0;
		
        strOldData = strData;
        strData.Empty();
        while (strOldData.GetLength())
        {
            CString strTmp;
			CString strField;
			
			// ȡ��ÿһ���ֶΰ���ָ���ĸ�ʽ��ʽ��
            strField = GetField(strOldData, g_strFSep, g_strRSep);
			strTmp.Format(pConfig->pFormat[ii], strField);
            strData += strTmp;
            strData += g_strFSep;
            ii++;
        }
    }
	
    // ��ĩβ���ֶηָ���滻��Ϊ��¼�ָ��
    int nCount = strData.GetLength() - g_strFSep.GetLength();
    strData.Delete(nCount, g_strFSep.GetLength());
    strData += g_strRSep;
	
    // ��д���Ľṹ
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
//  Describe:   ��AppSvr���͵���Ϣת��Ϊ�ⲿ�ӿ�Э��
//  Input:      
//              pBuf:       ��Ϣָ��
//              dwLen:      ��Ϣ����
//              dwTransLen: pTrans�������Ĵ�С
//              dwIndexLen: pSeqIndex�������Ĵ�С
//  Output:
//              pTrans:     ת�������Ϣָ��
//              dwTransLen: ת�������Ϣ����
//              pSeqIndex:  ���ɵ���ˮ��ָ��
//              dwIndexLen: ���ɵ���ˮ�ŵĳ���
//  Return:
//              TRUE:       ת���ɹ�
//              FALSE:      ת��ʧ��
//  Note:       dwTransLen, dwIndexLen�����������������. 
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
        return FALSE;   // ���������̫С
    }
	
    // ��ȡ����������˵��
    pConfig = GetParamConfigFromICDName(pAppSvr->pCMD);
	if (!pConfig)
    {						
        return FALSE;   // δ֪������
    }
	
    // ������ˮ��
    if (!MakeSequence(pSeqIndex, dwIndexLen))
    {
        return FALSE;   // ��ˮ������ʧ��
    }
    dwSeq = *(DWORD *)pSeqIndex;
	
    // ���ӷ��ظ�ʽ
    CString strData;
    CString strISDN;
    CString strWorkNo;
	CString strSrcData;
	CString strDstData;
	char    pData[24*1024];
	DWORD	dwDataLen;
    
    // ȡ�����ݶ�
	dwDataLen = htonl(pAppSvr->dwLen) - sizeof(APPLY_MSG) + 1;
	if (dwDataLen >= 24*1024)
	{
		return FALSE;
	}
	
	
	memcpy(pData, pAppSvr->pData, dwDataLen);
	pData[dwDataLen] = 0;
    strSrcData = pData;
	
	//д��־
	NewWriteLog(strSrcData, "DllTransAppSvrMsg", pAppSvr->pCMD, dwSeq);
	


	//ԭ��������

	if (!_stricmp(pAppSvr->pCMD, "10022"))//10022 �û�������Ϣ��ѯ
	{
		CString strHandsetNo;//�û��ֻ�����
		strHandsetNo = GetField(strSrcData, g_strFSep, g_strRSep);

		CString strUserType;
		bool bMobileNum = false;
		bMobileNum = IsMobileNumber(strHandsetNo);
		if (true == bMobileNum)   //c��
		{
			strUserType = "1";
		}
		else					  //����
		{
			strUserType = "10";
		}
		if (strlen(strHandsetNo)<9)
		{
			strHandsetNo="0371"+strHandsetNo;			
		}

		//modify by tongyufeng 20100827,Ϊ�˿��������Բ�ѯ��ȥ����usertype��CRM�Ѿ�֧�֣�
		strDstData = strHandsetNo + g_strFSep + strUserType + g_strRSep;
		//strDstData = strHandsetNo + g_strFSep;

	}

	else if (!_stricmp(pAppSvr->pCMD, "10033"))//(CRM�ӿ�)+10033��ȡ��ƷID���ʻ�ID�Ϳͻ�ID
	{
		CString strHandsetNo;//�û��ֻ�����
		strHandsetNo = GetField(strSrcData, g_strFSep, g_strRSep);

		CString strUserType;
		bool bMobileNum = false;
		bMobileNum = IsMobileNumber(strHandsetNo);
		if (true == bMobileNum)   //c��
		{
			strUserType = "1";
		}
		else					  //����
		{
			strUserType = "10";
		}

		strDstData = strHandsetNo + g_strFSep + strUserType + g_strRSep;
	}

	else if (!_stricmp(pAppSvr->pCMD, "10035"))//(CRM�ӿ�)+10035�û���Ʒ���ϲ�ѯ
	{
		CString strUserFlag;//�û���ʶ
		strUserFlag = GetField(strSrcData, g_strFSep, g_strRSep);
		
		strDstData = strUserFlag + g_strRSep;
	}

	// add by tongyufeng,20101228,����SP�Զ�ҵ�����ӿ�
	else if (!_stricmp(pAppSvr->pCMD, "10041"))//�Զ�ҵ�������
	{

		CString strHandsetNo; //�û�����
		CString strServiceType;  //ҵ�����ͣ�0-�߲�����  1-������ʾ 2-���б��� 3-���еȴ�
		CString strOptType;  //�������ͣ�1-�Ǽ�   3-ȡ��
		CString strUserType; //�û����ͣ�1-�ֻ���10-�̻�
		
		strHandsetNo = GetField(strSrcData, g_strFSep, g_strRSep);
        strUserType = GetField(strSrcData, g_strFSep, g_strRSep);
		strServiceType = GetField(strSrcData, g_strFSep, g_strRSep);
		strOptType = GetField(strSrcData, g_strFSep, g_strRSep);
		
		strDstData = strHandsetNo + g_strFSep + strUserType + g_strFSep + strServiceType + g_strFSep 
			+ strOptType + g_strRSep;
	}
    //add by tongyufeng,2011-02-21
	else if (!_stricmp(pAppSvr->pCMD, "10045"))//���л���һ�η��أ�����ʹ�� 
	{
		
		CString strHandsetNo; //�û�����
		CString strServiceType;  //ҵ������
		
		strHandsetNo = GetField(strSrcData, g_strFSep, g_strRSep);
		strServiceType = GetField(strSrcData, g_strFSep, g_strRSep);
		
		strDstData = strHandsetNo + g_strFSep + strServiceType + g_strRSep;
	}

	else if (!_stricmp(pAppSvr->pCMD, "10034"))//2.9	 (CRM�ӿ�)+10034 �޸Ŀͻ�����
	{
		CString strClientFlag;//�ͻ���ʶ
		CString strOldPassword;//������
		CString strNewPassword;//������

		strClientFlag = GetField(strSrcData, g_strFSep, g_strRSep);
		strOldPassword = GetField(strSrcData, g_strFSep, g_strRSep);
		strNewPassword = GetField(strSrcData, g_strFSep, g_strRSep);
		
		strDstData = strClientFlag + g_strFSep + strOldPassword + g_strFSep + strNewPassword + g_strRSep;
	}

	else if (!_stricmp(pAppSvr->pCMD, "10015"))//2.4	(CRM�ӿ�)10015 �޸��û�����
	{
		CString strNumber;//�绰����
		CString strOldPassword;//������
		CString strNewPassword;//������

		strNumber = GetField(strSrcData, g_strFSep, g_strRSep);
		strOldPassword = GetField(strSrcData, g_strFSep, g_strRSep);
		strNewPassword = GetField(strSrcData, g_strFSep, g_strRSep);

		CString strUserType;//�û�����
		bool bMobileNum = false;
		bMobileNum = IsMobileNumber(strNumber);
		if (true == bMobileNum)   //c��
		{
			strUserType = "1";
		}
		else					  //����
		{
			strUserType = "10";
		}
		
		strDstData = strNumber + g_strFSep 
				   + strUserType + g_strFSep 
				   + strOldPassword + g_strFSep
				   + strNewPassword + g_strRSep;
	}

	else if (!_stricmp(pAppSvr->pCMD, "10036"))//(CRM�ӿ�)+10036�ͻ���Ʒ���ϲ�ѯ
	{
		CString strUserFlag;//�û���ʶ
		
		strUserFlag = GetField(strSrcData, g_strFSep, g_strRSep);

		strDstData = strUserFlag + g_strRSep;
	}

	else if (!_stricmp(pAppSvr->pCMD, "10037"))//2.12	 (CRM�ӿ�)+10037 �ʻ���Ϣ��ѯ
	{
		CString strUserFlag;//�û���ʶ
		
		strUserFlag = GetField(strSrcData, g_strFSep, g_strRSep);
		
		strDstData = strUserFlag + g_strRSep;
	}

	else if (!_stricmp(pAppSvr->pCMD, "10038"))//2.13	 (CRM�ӿ�)+10038 �ײ���Ϣ��ѯ
	{
		CString strUserFlag;//�û���ʶ
		
		strUserFlag = GetField(strSrcData, g_strFSep, g_strRSep);
		
		strDstData = strUserFlag + g_strRSep;
	}
	/************************************************************************/
	/* added by wangyue,2011-11-30 10046��10047���˹��ӿ�                                                                     */
	/************************************************************************/
	else if (!_stricmp(pAppSvr->pCMD, "10046"))//2.18	10046��ѯͬ�˻��µĿ������
	{
		CString strAccountID;
		strAccountID = GetField(strSrcData,g_strFSep,g_strRSep);

		strDstData = strAccountID + g_strRSep;
	}
	else if (!_stricmp(pAppSvr->pCMD, "10047"))//2.19	10047��ѯͬ���֤����Ŀ������
	{
		CString strIDCard;
		strIDCard = GetField(strSrcData,g_strFSep,g_strRSep);
		
		strDstData = strIDCard + g_strRSep;
	}
	else                          
    {
        strDstData = strSrcData;
    }
	
    // �滻��ˮ��, �������ݶ�, �������¼���
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
//  Describe:   �ж�������Ӧ����Ϣ����������Ӧ����Ϣ
//  Input:      
//              pBuf:       ��Ϣָ��
//              dwLen:      ��Ϣ����
//  Output:
//  Return:
//              TRUE:       ����Ӧ����Ϣ
//              FALSE:      ������Ӧ����Ϣ
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
//  Describe:   ����ԭʼ������Ϣ��������ǰ�û�������Ӧ����Ϣת����IVR���ͻ���
//              AppSvr���͵���Ϣ
//  Input:      
//              pBuf:       ��Ϣָ��
//              dwLen:      ��Ϣ����
//              pApply:     ԭʼ������Ϣָ��
//              dwApplyLen: ԭʼ������Ϣ����
//              dwMsgLen:   pMsg����������
//  Output:
//              pMsg:       ת�������Ϣָ��
//              dwMsgLen:   ת�������Ϣ����
//              bEndReply:  Ӧ�������־
//  Return:
//              TRUE:       ת���ɹ�
//              FALSE:      ת��ʧ��
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
	
		  // ���Ʋ����������Լ�������
	//=============�ߴ���20081201Ϊ�����������=========
	//	LPSP_CALL pSPCall2;
    LPSP_PARAM pParam2;
	LPSP_CALL pSrcSPCall2 = pSrcSPCall;
    char *pSrcData2;
    CString         strData2;
	pParam2 = &pSrcSPCall2->SPPara;
    pSrcData2 = (char *)pParam2 + sizeof(SP_PARAM) * pSrcSPCall2->byParaNum;
    for (int ii = 0; ii < pSrcSPCall2->byParaNum; ii++)
    {
        // ��ת���������, ����Ĭ�ϸ�ʽת��
        if (pParam2->byParaType != OUTPUT_PARAM)
        {
            strData2 += TransParamToString(pParam2, pSrcData2);                
            strData2 += g_strFSep;
        }
		
        pSrcData2 += pParam2->wDataLen;
        pParam2++;
    }
	//=============
	
	memcpy(pSPCallAck, pSrcSPCall, sizeof(IVR_MSG_HEAD));	//������Ϣ����ͷ�ṹ
	
    pSPCallAck->IVRHead.bySendNode = pSrcSPCall->IVRHead.byRecvNode;
    pSPCallAck->IVRHead.bySendPort = pSrcSPCall->IVRHead.byRecvPort;
	pSPCallAck->IVRHead.byRecvNode = pSrcSPCall->IVRHead.bySendNode;
	pSPCallAck->IVRHead.byRecvPort = pSrcSPCall->IVRHead.bySendPort;
	
	pSPCallAck->IVRHead.wMsgType   = MSG_SPCALL_ACK;	//���м�����ص���Ϣ	
	
	pSPCallAck->wStatus = 0;	//ִ�гɹ�
	
	memcpy(pSPCallAck->pDataSource, pSrcSPCall->pDataSource, sizeof(pSrcSPCall->pDataSource));
	memcpy(pSPCallAck->pUserID, pSrcSPCall->pUserID, sizeof(pSrcSPCall->pUserID));
	memcpy(pSPCallAck->pProcName, pSrcSPCall->pProcName, sizeof(pSrcSPCall->pProcName));
	
	pSPCallAck->byParaNum = pSrcSPCall->byParaNum;
	
	
	
	SP_PARAM* pPara = (SP_PARAM *)&pSrcSPCall->SPPara;
	
   	int iParaLen = pSrcSPCall->byParaNum * sizeof(SP_PARAM);	//�������峤��
	for (int i = 0; i < pSrcSPCall->byParaNum; i++ )	        
	{
		iParaLen += pPara->wDataLen;                            //ȡ�����в����ĳ���
		pPara++;
	}	
	
	memcpy((LPSTR)&pSPCallAck->SPPara, (LPSTR)&pSrcSPCall->SPPara, iParaLen);
	
    // ��ȡ���ص����ݶ�����
    DWORD dwDataLen = htonl(pReply->dwLen) - sizeof(APPLY_MSG) + 1;

	pConfig = GetParamConfigFromICDName(pSrcSPCall->pProcName);

    memcpy(pSrcReplyData, pReply->pData, dwDataLen);
    pSrcReplyData[dwDataLen] = 0;
    strSrcReplyData = pSrcReplyData;
	
	////////////////////////////////////
	if (!_stricmp(pConfig->pICDCMD, "P_SCEGetUserPaytype")) //��ȡ�û���������
	{
		CString strRet;
		strRet = GetField(strSrcReplyData, g_strRSep, g_strFSep);

		strTran = strRet;
	}
	else if (!_stricmp(pConfig->pICDCMD, "P_SCEHandsetNoTypeJudg"))// �ֻ�������֤
	{
		CString strRet;
		strRet = GetField(strSrcReplyData, g_strRSep, g_strFSep);
		
		strTran = strRet;		
	}
	//modified by wangyue 20130318,��I_SCE_Login/P_SCELogin�����ӿںϲ�
	//else if (!_stricmp(pConfig->pICDCMD, "P_SCELogin")) //������֤
	//{
	//	CString strRet;
	//	strRet = GetField(strSrcReplyData, g_strRSep, g_strFSep);

	//	strTran = strRet;
	//}

	// add by tongyufeng,20101228,����SP�Զ�ҵ�����ӿ� 
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_AutoService")) //SP�̿�ҵ�����
	{
		CString strInnerXML;
		CString strResult;
		CMarkup xml;
		CString strANSIData;
		strANSIData = UTF8ToANSI(strSrcReplyData);

		//����xml����
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

	// add by tongyufeng,2011-02-21,�����Զ����������ֽӿ�
	/************************************************************************/
	/* date:    20130328
	   author:	wangyue
	   purpose: modified,convert xml messages to former format*/
	/************************************************************************/
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_AllFeeSms")) //SP�̿�ҵ�����
	{
		/*CString strInnerXML;
		CString strResult;
		CMarkup xml;
		CString strANSIData;
		strANSIData = UTF8ToANSI(strSrcReplyData);

		//����xml����
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
		CString strCurrentFee;//��ǰ����
		CString strDiscount;//�Żݽ��
		CString strRealFee;//ʵ�ջ���
		CString strBalance;//���
		CString strOwed;//Ƿ��
		//////////////////////////////////////////////////////////////////////////
		//ȥ��03d4..
		strSrcReplyData = strSrcReplyData.Right(strSrcReplyData.GetLength()-6);
		//ȡresult
		strResult = GetElementValue(strSrcReplyData,_T("result"));
		if (!strResult.Compare(_T("0")))
		{
			//ȡcurrentFee
			strCurrentFee= GetElementValue(strSrcReplyData,_T("currentFee"));
			//ȡdiscount
			strDiscount = GetElementValue(strSrcReplyData,_T("discount"));
			//ȡrealFee
			strRealFee = GetElementValue(strSrcReplyData,_T("realFee"));
			//ȡbalance
			strBalance = GetElementValue(strSrcReplyData,_T("balance"));
			//ȡowedfee
			strOwed = GetElementValue(strSrcReplyData,_T("owedfee"));
			//��ϳ�ԭ���ı��ĸ�ʽ
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

			CString strMoney;    //������Ϣ

			for (int i=0;i<5;i++)
			{
				strMoney = GetField(strSrcReplyData, g_strRSep, g_strFSep);
				strMoney.TrimLeft(' ');
				strMoney.TrimRight(' ');

				double dYuan = 0.00;
				dYuan = atof(strMoney)/100.00;  //�ֱ��Ԫ
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
		|| !_stricmp(pConfig->pICDCMD, "I_SCE_PasswdReset")) //�޸�����
	{
		CString strInnerXML;
		CString strResult;
		CMarkup xml;
		CString strANSIData;
		strANSIData = UTF8ToANSI(strSrcReplyData);

		//����xml����
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
	else if (!_stricmp(pConfig->pICDCMD, "P_SCEResetPassword")) //��������
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
	else if (!_stricmp(pConfig->pICDCMD, "P_SCEGetTotalFee"))  //�½Ự�Ѳ�ѯ(�󸶷�)
	{
		//ҵ�����
		CString strNo;
		//��������
		CString strBillingCycle;
		//�������
		CString strCode;
		//������Ϣ
		CString strMsg;
		//��ѯ���
		CString strResult;
		//�ܷ���
		CString strTotalFee;
		BILLITEM ArrItem[78]={
			BILLITEM(_T("�»�����"),_T("251001"),_T("0")),BILLITEM(_T("����ͨ����"),_T("251002"),_T("0")),
				BILLITEM(_T("�ۺ���Ϣ�����"),_T("251003"),_T("0")),BILLITEM(_T("���շ�"),_T("251004"),_T("0")),
				BILLITEM(_T("���շ�"),_T("251005"),_T("0")),BILLITEM(_T("���˴���"),_T("251006"),_T("0")),
				BILLITEM(_T("ΥԼ��"),_T("251007"),_T("0")),
				BILLITEM(_T("����"),_T("251008"),_T("0")),BILLITEM(_T("��_11808����"),_T("252001"),_T("0")),
				BILLITEM(_T("��_12361����"),_T("252002"),_T("0")),BILLITEM(_T("��_4008��;��"),_T("252003"),_T("0")),
				BILLITEM(_T("��_4008�л���"),_T("252004"),_T("0")),BILLITEM(_T("��_800��;��"),_T("252005"),_T("0")),
				BILLITEM(_T("��_800�л���"),_T("252006"),_T("0")),BILLITEM(_T("��_IP�۰�̨��;"),_T("252007"),_T("0")),
				BILLITEM(_T("��_IP���ʳ�;"),_T("252008"),_T("0")),BILLITEM(_T("��_IP���ڳ�;"),_T("252009"),_T("0")),
				BILLITEM(_T("��_VOD�㲥��"),_T("252010"),_T("0")),BILLITEM(_T("��_�������ط�"),_T("252011"),_T("0")),
				BILLITEM(_T("��_����һ��ͨ�ͻ���"),_T("252012"),_T("0")),BILLITEM(_T("��_�����շ��ն�����"),_T("252013"),_T("0")),
				BILLITEM(_T("��_�۰�̨��;��"),_T("252014"),_T("0")),BILLITEM(_T("��_���ʳ�;��"),_T("252015"),_T("0")),
				BILLITEM(_T("��_���ڳ�;��"),_T("252016"),_T("0")),BILLITEM(_T("��_����豸���������"),_T("252017"),_T("0")),
				BILLITEM(_T("��_���ͨ�ŷ�"),_T("252018"),_T("0")),BILLITEM(_T("��_��������"),_T("252019"),_T("0")),
				BILLITEM(_T("��_�߲�������Ϣ��"),_T("252020"),_T("0")),BILLITEM(_T("��_����ͨ����"),_T("252021"),_T("0")),
				BILLITEM(_T("��_����ͨ����"),_T("252022"),_T("0")),BILLITEM(_T("��_����ͨ�ŷ�"),_T("252023"),_T("0")),
				BILLITEM(_T("��_����ʹ�÷�"),_T("252024"),_T("0")),BILLITEM(_T("��_��ҵ�񿪻���"),_T("252026"),_T("0")),
				BILLITEM(_T("��_��Ϣ��"),_T("252027"),_T("0")),BILLITEM(_T("��_Ԥ���"),_T("252028"),_T("0")),
				BILLITEM(_T("��_Ԥ���Ѳ����������"),_T("252029"),_T("0")),BILLITEM(_T("��_Ԥ�����ͻ���"),_T("252030"),_T("0")),
				BILLITEM(_T("��_�����"),_T("252031"),_T("0")),BILLITEM(_T("��_���ɽ�"),_T("252032"),_T("0")),
				BILLITEM(_T("��_�ն��豸��"),_T("252033"),_T("0")),BILLITEM(_T("��_װ�ƻ���"),_T("252034"),_T("0")),
				BILLITEM(_T("��_�Żݷ���"),_T("252035"),_T("0")),BILLITEM(_T("�����캽ҵ���"),_T("252036"),_T("0")),
				BILLITEM(_T("��;�����ܷ�"),_T("252037"),_T("0")),BILLITEM(_T("VPNҵ��ʹ�÷�"),_T("252038"),_T("0")),
				BILLITEM(_T("������ʾ��"),_T("252039"),_T("0")),BILLITEM(_T("�������Ʒ�"),_T("252040"),_T("0")),
				BILLITEM(_T("����ͨ����"),_T("252041"),_T("0")),BILLITEM(_T("���ӷ����"),_T("252042"),_T("0")),
				BILLITEM(_T("���Ѱ��칦�ܷ�"),_T("252043"),_T("0")),BILLITEM(_T("�߲�������"),_T("252044"),_T("0")),
				BILLITEM(_T("��������ܷ�"),_T("252045"),_T("0")),BILLITEM(_T("��װ�����ܷ�"),_T("252046"),_T("0")),
				BILLITEM(_T("Эͬͨ�ŷ�"),_T("252047"),_T("0")),BILLITEM(_T("�������������ܷ�"),_T("252048"),_T("0")),
				BILLITEM(_T("GCҵ���ܷ�"),_T("252049"),_T("0")),BILLITEM(_T("���������"),_T("252050"),_T("0")),
				BILLITEM(_T("��������ʹ�÷�"),_T("252051"),_T("0")),BILLITEM(_T("�������ѹ��ܷ�"),_T("252052"),_T("0")),
				BILLITEM(_T("�Ҽ��ֹ��ܷ�"),_T("252053"),_T("0")),BILLITEM(_T("������ʾ��"),_T("252054"),_T("0")),
				BILLITEM(_T("���ΰ�ʹ�÷�"),_T("252055"),_T("0")),BILLITEM(_T("����ʹ�÷�"),_T("252056"),_T("0")),
				BILLITEM(_T("ҹ���İɹ��ܷ�"),_T("252057"),_T("0")),BILLITEM(_T("���Ű��¹��ܷ�"),_T("252058"),_T("0")),
				BILLITEM(_T("�����ײͷ�"),_T("252059"),_T("0")),BILLITEM(_T("���۹ܼ���ʹ�÷�"),_T("252060"),_T("0")),
				BILLITEM(_T("����Խ����ܷ�"),_T("252061"),_T("0")),BILLITEM(_T("���ΰ����ܷ�"),_T("252062"),_T("0")),
				BILLITEM(_T("WLAN���¹��ܷ�"),_T("252063"),_T("0")),BILLITEM(_T("�������ܷ�"),_T("252064"),_T("0")),
				BILLITEM(_T("VPNҵ��ʹ�÷�"),_T("252065"),_T("0")),BILLITEM(_T("���Ű��¹��ܷ�"),_T("252066"),_T("0")),
				BILLITEM(_T("���ΰ����ܷ�"),_T("252067"),_T("0")),BILLITEM(_T("����eͨ���ܷ�"),_T("252068"),_T("0")),
				BILLITEM(_T("˾��eͨ���ܷ�"),_T("252069"),_T("0")),BILLITEM(_T("��������ܷ�"),_T("252070"),_T("0")),
				BILLITEM(_T("�Żݷ���"),_T("260004"),_T("0"))
			
		};
		//ȥ��03d4..
		strSrcReplyData = strSrcReplyData.Right(strSrcReplyData.GetLength()-6);
		CMarkup xml;
		if (xml.SetDoc(strSrcReplyData))
		{
			//ȡresult�ֶ�
			if(xml.FindElem_C(_T("/soapenv:Envelope/soapenv:Body/multiRef[@id='id0']/result")))
			{
				strResult = xml.GetData();
				if (!strResult.Compare(_T("0")))
				{
					//ȡaccNbr�ֶ�
					if(xml.FindElem_C(_T("/soapenv:Envelope/soapenv:Body/multiRef[@id='id0']/accNbr")))
					{
						strNo = xml.GetData();
					}
					//ȡbillingCycle�ֶ�
					if(xml.FindElem_C(_T("/soapenv:Envelope/soapenv:Body/multiRef[@id='id0']/billingCycle")))
					{
						strBillingCycle = xml.GetData();
					}
					//ȡtotalAmount�ֶ�
					if(xml.FindElem_C(_T("/soapenv:Envelope/soapenv:Body/multiRef[@id='id0']/totalAmount")))
					{
						strTotalFee = xml.GetData();
						strTotalFee = FenToYuan(strTotalFee);
					}
					//ȡuserBillInfoArray�ֶ�
					if(xml.FindElem_C(_T("/soapenv:Envelope/soapenv:Body/multiRef[@id='id0']/userBillInfoArray")))
					{
						int nAmountOfItem = 0;
						CString strAttr = xml.GetAttrib(_T("soapenc:arrayType"));
						strAttr = strAttr.Mid(strAttr.Find(_T("["))+1,strAttr.GetLength()-strAttr.Find(_T("["))-2);
						nAmountOfItem = atoi(strAttr);
						//ѭ��ȡnAmountOfItem���˵������BILLITEM ArrItem[76]�����У�����Ӧ���˵������
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

							//��ȡ�����˵���������˵�ID������BILLITEM ArrItem[78]������
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
						//�������������˵�����ϳ�1~0~2��ʽ��������Ϊ0����д0;
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
	
		//������Ҫ�õ���	
		CString strAllMsg;
		CString strHandsetNo;
		CString strStartTime;
		strHandsetNo = GetField(strData2, g_strRSep, g_strFSep);
		GetField(strData2, g_strRSep, g_strFSep);
		GetField(strData2, g_strRSep, g_strFSep);
		
		//������Ҫ�õ���
		strStartTime = GetField(strData2, g_strRSep, g_strFSep);
		strAllMsg = "�𾴵��й�����" + strHandsetNo + "�ͻ���";
		strAllMsg += "��" + strStartTime.Left(4) + "��";
		strAllMsg += strStartTime.Mid(4,2) + "�µ�";
		
		//������Ҫ�õ���
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
                				
				//������Ҫ�õ���
				strCurrentTitle = GetField(strTitle, g_strRSep, g_strFSep);
				
				
				if( 0 == strMoney.GetLength() )
				{
					strTran += "0000000" + g_strFSep;
				}
				else
				{
					//��Ӷ�������
					if (_stricmp (pConfig->pVPCode[iGuangwen],"0000000"))
					{
						strAllMsg += strCurrentTitle + "Ϊ" + strMoney + "Ԫ��";
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
			
			//�����ŵ����������ġ������ű�ɡ�������
			strAllMsg = strAllMsg.Left(strAllMsg.GetLength() - 2);
			strAllMsg += "��";

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
	else if (!_stricmp(pConfig->pICDCMD, "P_SCEPhoneIDCheck"))// ֤��������֤
	{
		CString strRet;
		strRet = GetField(strSrcReplyData, g_strRSep, g_strFSep);

		strTran = strRet;
	}
	else if (!_stricmp(pConfig->pICDCMD, "P_SCEAppealOpenClose"))  // ͣ��������
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
	else if (!_stricmp(pConfig->pICDCMD, "P_SCEGetRealTimeFee")) //ʵʱ����(�󸶷�)
	{
		CString strResult;
		CString strRealFee;
		CString strBalance;
		CString strANSIData;
		strANSIData = UTF8ToANSI(strSrcReplyData);
		//////////////////////////////////////////////////////////////////////////
		//����xml����
		CMarkup xml;
		if (xml.SetDoc(strANSIData))
		{
			//ȡresult
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
							strSrcReplyData = _T("111~ȡrealFee�ֶ�ʧ��;");
						}
					}
					else
					{
						strSrcReplyData = _T("111~ȡbalance�ֶ�ʧ��;");
					}
				}
				else
				{
					strSrcReplyData = _T("111~result�ֶη�0;");
				}
			}
			else
			{
				strSrcReplyData = _T("111~ȡresult�ֶ�ʧ��;");
			}
		}
		else
		{
			strSrcReplyData = _T("111~xml���󴴽�ʧ��;");
		}
		//////////////////////////////////////////////////////////////////////////
		CString strRet;
		strRet = GetField(strSrcReplyData, g_strRSep, g_strFSep);
		if(!_stricmp(strRet,"0"))
		{
			//���ϱ�����ԪΪ��λ
			strTran = "0~";
			CString strCurrentFee;  //ʵʱ����
			CString strBalanceFee;  //���	
			strCurrentFee = GetField(strSrcReplyData, g_strRSep, g_strFSep);
			//strBalanceFee = GetField(strSrcReplyData, g_strRSep, g_strFSep);

			strCurrentFee.TrimLeft(' ');
			strCurrentFee.TrimRight(' ');

			double dYuan = 0.00;
			dYuan = atof(strCurrentFee)/100.00;  //�ֱ��Ԫ
			CString strCurrentFeeYuan;
			strCurrentFeeYuan.Format("%.2f",dYuan);
			
			//�������� BEGIN
			CString strMsg;
			CString strHandsetNo;
			strHandsetNo = GetField(strData2, g_strRSep, g_strFSep);
			strMsg = "�𾴵��й�����" + strHandsetNo + "�ͻ�������";
			strMsg += "ʵʱ����Ϊ" + strCurrentFeeYuan + "Ԫ��~";
			//�������� END
			
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
			//=====���Ӷ��Ŵ���begin 20081202

			strTran += strMsg;
			
			for (int ii = 1; ii < 5; ii++)
			{
				strTran += "";
			}
			//=====���Ӷ��Ŵ���End 20081202
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
	else if (!_stricmp(pConfig->pICDCMD, "P_SCEVAGetTotalFee"))  //����ѯ(�󸶷�)
	{
		CString strResult;
		CString strRealFee;
		CString strBalance;
		//////////////////////////////////////////////////////////////////////////
		//ȥ��03d4..
		strSrcReplyData = strSrcReplyData.Right(strSrcReplyData.GetLength()-6);
		//ȡresult
		strResult = GetElementValue(strSrcReplyData,_T("result"));
		if (!strResult.Compare(_T("0")))
		{
			//ȡbalance
			strBalance= GetElementValue(strSrcReplyData,_T("balance"));
			//ȡRealFee
			strRealFee = GetElementValue(strSrcReplyData,_T("realFee"));
			//��ϳ�ԭ���ı��ĸ�ʽ
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
			//CString strstrCurrentFee;  //ʵʱ����
			CString strBalanceFee;  //���1

			GetField(strSrcReplyData, g_strRSep, g_strFSep);
			strBalanceFee = GetField(strSrcReplyData, g_strRSep, g_strFSep);

			strBalanceFee.TrimLeft(' ');
			strBalanceFee.TrimRight(' ');

			double dYuan = 0.00;
			dYuan = atof(strBalanceFee)/100.00;  //�ֱ��Ԫ
			CString strBalanceFeeYuan;
			strBalanceFeeYuan.Format("%.2f",dYuan);
			
			//�������� BEGIN
			CString strMsg;
			CString strHandsetNo;
			strHandsetNo = GetField(strData2, g_strRSep, g_strFSep);
			strMsg = "�𾴵��й�����" + strHandsetNo + "�ͻ�������";
			strMsg += "���Ϊ" + strBalanceFeeYuan + "Ԫ��~";
			//�������� END

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
			
			//=====���Ӷ��Ŵ���begin 20081202
			strTran += strMsg;
			
			for (int n = 1; n < 5; n++)
			{
				strTran += "~";
			}
			//=====���Ӷ��Ŵ���end 20081202
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
	else if (!_stricmp(pConfig->pICDCMD, "P_SceQueryScore"))  //���ֲ�ѯ
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
			
			//CString strTotalPoint;  //�û������ۻ�����
			CString strCurrentPoint;  //�û���ǰʣ�����
  
			//strTotalPoint = GetField(strSrcReplyData, g_strRSep, g_strFSep);
			strCurrentPoint = GetField(strSrcReplyData, g_strRSep, g_strFSep);
			
			//�������� BEGIN
			CString strMsg;
			CString strHandsetNo;
			strHandsetNo = GetField(strData2, g_strRSep, g_strFSep);
			//modify by tongyufeng,20100121,Ӧ�ͻ�Ҫ��ȥ���������ۻ�����
			strMsg = "�𾴵��й�����" + strHandsetNo + "�ͻ������ĵ�ǰʣ�����Ϊ"+strCurrentPoint + "��~";
			//strMsg += "�����ۻ�����Ϊ" + strTotalPoint + "��" + "��ǰʣ�����Ϊ" + strCurrentPoint + "��~" ;
			//�������� END

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
			
			//=====���Ӷ��Ŵ���begin 20081202
			strTran += strMsg;
			
			for (int m = 1; m < 5; m++)
			{
				strTran += "";
			}
			//=====���Ӷ��Ŵ���End 20081202
		}
		else 
		{
			strTran = strRet;
		}
		
	}
	
	//ԭ���������� BEGIN
	
	//BOSSδ����������������ֵ����ʱ�޷�ʵ��
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_GetUserinfor"))//1.1.1  �û�����I_SCE_GetUserinfor
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
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_ChangePassword"))//1.1.2  �޸�����I_SCE_ChangePassword
	{
		CString strRet;
		strRet = GetField(strSrcReplyData, g_strRSep, g_strFSep);
		
		strTran = strRet;		
	}
	
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_GetDebtFee"))//1.1.3  ��ѯǷ��I_SCE_GetDebtFee
	{
		CString strRet;
		strRet = GetField(strSrcReplyData, g_strRSep, g_strFSep);

		//CString strstrCurrentFee;  //ʵʱ����
		CString strBalanceFee;  //���
		
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
				dYuan = atof(strBalanceFee)/100.00;  //�ֱ��Ԫ
				CString strBalanceFeeYuan;
				strBalanceFeeYuan.Format("%.2f",dYuan);
				
				strBalanceFee.Remove('-');
				strTran += pConfig->pVPCode[0] + strBalanceFeeYuan + g_strFSep;	
				
				for ( int ii = 1; ii < 20; ii++)
				{
					strTran += "0000000~";
				}
				
				//�������� BEGIN
				CString strMsg;
				CString strHandsetNo;
				strHandsetNo = GetField(strData2, g_strRSep, g_strFSep);
				strMsg = "�𾴵��й�����" + strHandsetNo + "�ͻ�������";
				strMsg += "Ƿ���ܶ�Ϊ" + strBalanceFeeYuan + "Ԫ��~";
				//�������� END
				//=====���Ӷ��Ŵ���begin 20081202
				
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

		//����xml����
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

		//����xml����
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
		//�Զ˷������ݸ�ʽ��������;���~�������;
		CString strRet=_T("");		//�Զ˷�����
		CString strSN1=_T("");		//���
		CString strSN2=_T("");		//���
		CString strBroadNo=_T("");	//�������
		
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
				strTran = "2~";  //��������˵�2�����룬˵�����˶������IVR������
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

		//����xml����
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
		//�Զ˷������ݸ�ʽ��������;���~�������;
		CString strRet=_T("");		//�Զ˷�����
		CString strSN1=_T("");		//���
		CString strSN2=_T("");		//���
		CString strBroadNo=_T("");	//�������
		
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
				strTran = "2~";   //��������˵�2�����룬˵�����˶������IVR������
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
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_QueryFreeResource"))  //10048 ��ѯ�û������Դ
	{
		CString strResult;
		
		//////////////////////////////////////////////////////////////////////////
		//ȥ��03d4..
		strSrcReplyData = strSrcReplyData.Right(strSrcReplyData.GetLength()-6);
		CMarkup xml;
		if (xml.SetDoc(strSrcReplyData))
		{
			//ȡresult�ֶ�
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
						//��Դ���ƣ�id����λ���ܹ���ʹ������ʣ��������ʹ����
						CString strName,strType,strUnit,strFree,strUsable,strUsed;
						//ѭ��ȡÿ�������Դ������
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
							//ƴstrSrcReplyData
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
		//�Զ˷������ݸ�ʽ��������;���~��Դ����~�ܹ���ʹ����Դ����~�����Դ��ʹ����~ʣ�������Դ��~��Դ��λ;
		//˵��������û����ڶ��������Դ�����ض��"
		//���磺0��0~�����ԴA~300~40~260~�֣�1~�����ԴB~100~10~90~�֣�
		/************************************************************************/
		/* �𾴵Ŀͻ�,�����µģ���Դ���ƣ���ʹ�ã��ܹ���ʹ����Դ����������Դ��λ�������ã������Դ��ʹ����������Դ��λ��,
		ʣ�ࣨʣ�������Դ��������Դ��λ������; 
		����Դ���ƣ���ʹ�ã��ܹ���ʹ����Դ����������Դ��λ�������ã������Դ��ʹ����������Դ��λ��,ʣ�ࣨʣ�������Դ����
		����Դ��λ�����á���л��ʹ���й�����*/
		/************************************************************************/
		CString strRet;	//�Զ˷�����
		CString strSN;	//���
		CString strResourceName;//��Դ����
		CString strTotalResource;//�ܹ���ʹ����Դ����
		CString strFreeResource;//�����Դ��ʹ����
		CString strSurplusFreeResource;//ʣ�������Դ��
		CString strMeasurement;	//��Դ��λ
		
		strRet = GetField(strSrcReplyData,g_strFSep,g_strRSep);
		strRet.TrimLeft();
		strRet.TrimRight();

		CString strSM = _T("0~�𾴵Ŀͻ�,�����µ�");

		if (!_stricmp(strRet,"0"))//���سɹ�
		{
			//ȡ��һ����¼
			strRet = GetField(strSrcReplyData,g_strRSep,g_strRSep);
			while (!strRet.IsEmpty())
			{
				//ȡrecord(��¼)�е�ÿһ��field(�ֶ�)
				strSN = GetField(strRet,g_strFSep,g_strFSep);
				strResourceName = GetField(strRet,g_strFSep,g_strFSep);
				strTotalResource = GetField(strRet,g_strFSep,g_strFSep);
				strFreeResource = GetField(strRet,g_strFSep,g_strFSep);
				strSurplusFreeResource = GetField(strRet,g_strFSep,g_strFSep);
				strMeasurement = GetField(strRet,g_strFSep,g_strFSep);
				//date��2013-08-21 ���ݵ���������Ҫ����������е�kת����M
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

				//��װ��������
				strSM += strResourceName;
				strSM += _T("�ܿ�ʹ��");
				strSM += strTotalResource;
				strSM += strMeasurement;
				strSM += _T("��");

				strSM += _T("����");
				strSM += strFreeResource;
				strSM += strMeasurement;
				strSM += _T("��");

				strSM += _T("ʣ��");
				strSM += strSurplusFreeResource;
				strSM += strMeasurement;
				strSM += _T("��");

				strRet = GetField(strSrcReplyData,g_strRSep,g_strRSep);//ȡ��һ��record(��¼)
			}

			strSM = strSM.Left(strSM.GetLength()-2);
			strSM += _T("��");
			strSM += _T("��л��ʹ���й�����");
//			TRACE("dtproxy SM=%s",strSM);

			//���ڿ��ܶ��Ź�������Ҫ�ָ�
			CString strSmsSub[5]; 
			CString	strSms;
			MyTrimSm2(140, strSM, strSmsSub, 5);
			for(int j =0; j < 5; j++)                     //5������
			{
				strSms += strSmsSub[j] + g_strFSep;
			}
			strTran += strSms + g_strRSep ;
	//		TRACE("dtproxy SM=%s",strTran);
		}
		else if (!_stricmp(strRet,"101"))//��ѯ���Ϊ��
		{
			strTran = "101~";
		}
		else//��ѯʧ��
		{
			strTran = "111~";
		}
	}

/************************************************************************/
/*end: added by tongyufeng,wangyue,2011-11-24                                                                     */
/************************************************************************/
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_GetMonthFee"))//1.1.5  ĳ���ʵ���ѯI_SCE_GetMonthFee
	{
		CString strRet;
		CString strMoney;
		CString strMoneyYuan;
		
		//������Ҫ�õ���	
		CString strAllMsg;
		CString strHandsetNo;
		CString strStartTime;
		strHandsetNo = GetField(strData2, g_strRSep, g_strFSep);
		GetField(strData2, g_strRSep, g_strFSep);
		GetField(strData2, g_strRSep, g_strFSep);
		
		//������Ҫ�õ���
		strStartTime = GetField(strData2, g_strRSep, g_strFSep);
		strAllMsg = "�𾴵��й�����" + strHandsetNo + "�ͻ���";
		strAllMsg += "��" + strStartTime.Left(4) + "��";
		strAllMsg += strStartTime.Mid(4,2) + "�µ�";
		
		//������Ҫ�õ���
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
				
				//������Ҫ�õ���
				strCurrentTitle = GetField(strTitle, g_strRSep, g_strFSep);
				
				
				if( 0 == strMoney.GetLength() )
				{
					strTran += "0000000" + g_strFSep;
				}
				else
				{
					//��Ӷ�������
					if (_stricmp (pConfig->pVPCode[iGuangwen],"0000000"))
					{
						strAllMsg += strCurrentTitle + "Ϊ" + strMoney + "Ԫ��";
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
			
			//�����ŵ����������ġ������ű�ɡ�������
			strAllMsg = strAllMsg.Left(strAllMsg.GetLength() - 2);
			strAllMsg += "��";


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
		else if (!_stricmp(strRet, "111"))// ʧ��
		{
			strTran = "111~";
		}	
	}

	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_GetBalance"))//1.1.6  ����ѯI_SCE_GetBalance
	{
		CString strRet;
		strRet = GetField(strSrcReplyData, g_strRSep, g_strFSep);
		if(!_stricmp(strRet,"0"))
		{
			strTran = "0~";
			//CString strstrCurrentFee;  //ʵʱ����
			CString strBalanceFee;  //���
			
			GetField(strSrcReplyData, g_strRSep, g_strFSep);
			strBalanceFee = GetField(strSrcReplyData, g_strRSep, g_strFSep);
			
			strBalanceFee.TrimLeft(' ');
			strBalanceFee.TrimRight(' ');

			double dYuan;
			dYuan = 0.00;
			dYuan = atof(strBalanceFee)/100.00;  //���ղ���
			CString strBalanceFeeYuan;
			strBalanceFeeYuan.Format("%.2f",dYuan);
			
			//���� BEGIN
			CString strMsg;
			CString strHandsetNo;
			strHandsetNo = GetField(strData2, g_strRSep, g_strFSep);
			strMsg = "�𾴵��й�����" + strHandsetNo + "�ͻ�������";
			strMsg += "���Ϊ" + strBalanceFeeYuan + "Ԫ��~";
			//���� END
			

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
			
			//=====���Ӷ��Ŵ���begin 20081202

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
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_HandsetNoTypeJudg"))//1.1.8  ������Ч��У��I_SCE_HandsetNoTypeJudg
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
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_CHKNumMatch"))//1.1.9  �ʺźͺ���ƥ��У�飺I_SCE_CHKNumMatch
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
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_RealTimeFe"))//1.1.10  ʵʱ���ò�ѯI_SCE_RealTimeFee
	{
		CString strRet;
		strRet = GetField(strSrcReplyData, g_strRSep, g_strFSep);
		if(!_stricmp(strRet,"0"))
		{
			strTran = "0~";
			CString strstrCurrentFee;  //ʵʱ����
			CString strBalanceFee;  //���
			
			strstrCurrentFee = GetField(strSrcReplyData, g_strRSep, g_strFSep);
			//strBalanceFee = GetField(strSrcReplyData, g_strRSep, g_strFSep);
			
			strstrCurrentFee.TrimLeft(' ');
			strstrCurrentFee.TrimRight(' ');
			

			double dYuan;
			dYuan = atof(strstrCurrentFee)/100.00;  //���ղ���
			CString strstrCurrentFeeYuan;
			strstrCurrentFeeYuan.Format("%.2f",dYuan);
			
			//���� BEGIN
			CString strMsg;
			CString strHandsetNo;
			strHandsetNo = GetField(strData2, g_strRSep, g_strFSep);
			strMsg = "�𾴵��й�����" + strHandsetNo + "�ͻ�������";
			strMsg += "ʵʱ����Ϊ" + strstrCurrentFeeYuan + "Ԫ��~";
			//���� END
			
			
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
			
			//=====���Ӷ��Ŵ���begin 20081202

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
		|| !_stricmp(pConfig->pICDCMD, "P_SCELogin"))//1.1.11  �û�������֤I_SCE_Login
	{
		CString strInnerXML;
		CString strResult;
		CMarkup xml;
		CString strANSIData;
		strANSIData = UTF8ToANSI(strSrcReplyData);

		//����xml����
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
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_GetUserinfor1"))//1.1.1  �û�����I_SCE_GetUserinfor
	{
		CString strResult;
		CString strCustId;//�ͻ����
		CString strCustName;//�ͻ�����
		CString strSegmentId;//Ʒ��id
		CString strSegmentName;//Ʒ������
		CString strAreaID;//��������
		CString strAreaName;//��������
		CString strCustGread;//�ͻ�����
		CString strCustGread2;//����ͻ�����ȼ� 
		CString strProductStatusName;//�û�״̬
		CString strAddress;//�����ַ
		//////////////////////////////////////////////////////////////////////////
		CString strInnerXML;
		CMarkup xml;
		CString strANSIData;
		strANSIData = UTF8ToANSI(strSrcReplyData);

		//����xml����
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
								strSrcReplyData = _T("111~ȡcustGread�ֶγ���");
							}
						}
						else
						{
							strSrcReplyData = _T("111~resultIDΪ��0");
						}
					}
					else
					{
						strSrcReplyData = _T("111~ȡresultID�ֶγ���;");
					}
				}
				else
				{
					strSrcReplyData = _T("111~�����ڲ�xml�������;");
				}
			}
			else
			{
				strSrcReplyData = _T("111~��ȡ�ڲ�xml�ַ�������;");
			}
		}
		else
		{
			strSrcReplyData = _T("111~����xml�������;");
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
			���ظ�IVR��
				10����ͨ�û� 
				40�������û�
				50�����û�
				60����ʯ���û�
			��Э�飺1 ��ʯ�� 2 �� 3 ���� 4 ��ͨ�� 5 ����� 0 ��ͨ�û�
			��Э�飺1000 ��ʯ�� 1100 �� 1200 ����  1300��ͨ
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
				//Ĭ�ϴ���
			}
			strTran += strUserClass + "~0~" + strVIPFLag + "~";
			
		
		}		
		else
		{
			strTran = "111~";
		}		
	}
	else if (!_stricmp(pConfig->pICDCMD, "I_SCE_GetUserinfor2"))//ҹ������ʹ�ã�20130507��wangyue
	{
		CString strResult;
		CString strCustId;//�ͻ����
		CString strCustName;//�ͻ�����
		CString strSegmentId;//Ʒ��id
		CString strSegmentName;//Ʒ������
		CString strAreaID;//��������
		CString strAreaName;//��������
		CString strCustGread;//�ͻ�����
		CString strCustGread2;//����ͻ�����ȼ� 
		CString strProductStatusName;//�û�״̬
		CString strAddress;//�����ַ
		//////////////////////////////////////////////////////////////////////////
		CString strInnerXML;
		CMarkup xml;
		CString strANSIData;
		strANSIData = UTF8ToANSI(strSrcReplyData);

		//����xml����
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
								strSrcReplyData = _T("111~ȡcustGread�ֶγ���");
							}
						}
						else
						{
							strSrcReplyData = _T("111~resultIDΪ��0");
						}
					}
					else
					{
						strSrcReplyData = _T("111~ȡresultID�ֶγ���;");
					}
				}
				else
				{
					strSrcReplyData = _T("111~�����ڲ�xml�������;");
				}
			}
			else
			{
				strSrcReplyData = _T("111~��ȡ�ڲ�xml�ַ�������;");
			}
		}
		else
		{
			strSrcReplyData = _T("111~����xml�������;");
		}
		//////////////////////////////////////////////////////////////////////////
		CString strRet;
		CString strUserClass;
		CString strVIPFLag;
		CString strGovFlag=_T("0");//�����־
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
			���ظ�IVR��
				10����ͨ�û� 
				40�������û�
				50�����û�
				60����ʯ���û�
			��Э�飺1 ��ʯ�� 2 �� 3 ���� 4 ��ͨ�� 5 ����� 0 ��ͨ�û�
			��Э�飺1000 ��ʯ�� 1100 �� 1200 ����  1300��ͨ
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
				//Ĭ�ϴ���
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
	
	//д��־
	NewWriteLog(strTran, "DllTransReplyMsg", pConfig->pICDCMD, pReply->dwRequestID);
	
	
	for (int i = 0; i < pSPCallAck->byParaNum; i++ )
	{
		//������������������������
		if ((pPara->byParaType == OUTPUT_PARAM) || (pPara->byParaType == INOUT_PARAM)) 
		{
            CString pChar;
			
			pChar = GetField(strTran, g_strFSep, g_strRSep);
			
			// ȥ��ǰ��Ŀո�
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
//  Describe:   ��������ǰ�û�������������Ӧ����Ϣת����IVR���ͻ�AppSvr���͵���Ϣ
//  Input:      
//              pBuf:       ��Ϣָ��
//              dwLen:      ��Ϣ����
//              dwMsgLen:   pMsg����������
//  Output:
//              pMsg:       ת�������Ϣָ��
//              dwMsgLen:   ת�������Ϣ����
//  Return:
//              TRUE:       ת���ɹ�
//              FALSE:      ת��ʧ��
//  Note:       ����û��ԭʼ����, ֻ�ܸ������ý���ת��
///////////////////////////////////////////////////////////////////////////////
BOOL __stdcall
DllTransNoApplyReplyMsg(char *pBuf, DWORD dwLen, char *pMsg, DWORD &dwMsgLen)
{
    LPAPPLY_MSG     pSrcReply;
	
    InitDll();
	
    pSrcReply = (LPAPPLY_MSG)&pBuf[sizeof(MSG_ADDRESS)];
	
    if (dwLen - sizeof(MSG_ADDRESS) > dwMsgLen)
    {
        // �������������
        return FALSE;
    }
	
	dwMsgLen = htonl(pSrcReply->dwLen);
    memcpy(pMsg, pSrcReply, dwMsgLen);
	
    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//  Func:       
//  Describe:   ȡ��Ӧ�������ˮ��
//  Input:      
//              pBuf:       ��Ϣָ��
//              dwLen:      ��Ϣ����
//              dwIndexLen: ��ˮ�Ż������ĳ���
//  Output:
//              pSeqIndex:  ��ˮ��ָ��
//              dwIndexLen: ��ˮ�ŵĳ���
//  Return:
//              TRUE:       ȡ����ˮ��
//              FALSE:      �޷�ȡ����ˮ��
//  Note:       ĳ��Э�鷽ʽ����û����ˮ�ŵĸ���
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
//  Describe:   ȡ��Ӧ�������Ϣ����(ICD���������)
//  Input:      
//              pBuf:       ��Ϣָ��
//              dwLen:      ��Ϣ����
//              dwNameLen:  pName�������ĳ���
//  Output:
//              pName:      ��Ϣ����ָ��
//              dwNameLen:  ��Ϣ���͵ĳ���
//  Return:
//              TRUE:       ȡ��������
//              FALSE:      �޷�ȡ��������
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
    strMIDCMD = pMIDCMD;             // MID��������
	
    for (int ii = 0; ii < CONFIG_NUM; ii++)
    {
        if (!g_ParamConfig[ii].bUsed)
        {
            break;
        }
		
        // MID��������ƥ��
        if (!strMIDCMD.CompareNoCase(g_ParamConfig[ii].pMIDCMD))
        {
            if (sizeof(g_ParamConfig[ii].pICDCMD) > dwNameLen)
            {
                return FALSE;   // ���뻺������С����
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
//  Describe:   ȡ��Ӧ����ĳ���
//  Input:      
//              pBuf:       ��Ϣָ��
//              dwLen:      ��Ϣ����
//  Output:
//              bEndReply:  ��һ����Ϣ�Ƿ�ΪӦ�������
//  Return:
//              0:          �û�������û��һ�������İ�
//              -1:         �û������ڵ���������
//              >0:         �û������ڵ�һ����Ϣ���ĳ���
//  Note:       pBuf�ڿ����ж����Ϣ��, ���Ǳ����������ص�һ����Ϣ���ĳ���
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
//  Describe:   ��ȡ������Ϣ
//  Input:      
//              dwStartMsgType: ������Ϣ����
//              dwLen:          pBuf�������Ĵ�С
//  Output:
//              pBuf:       ��Ϣָ��
//              dwLen:      ��Ϣ����
//  Return:
//              TRUE:       ��ȡ�ɹ�
//              FALSE:      ��ȡʧ��
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
//  Describe:   ��ȡע����Ϣ
//  Input:      
//              dwLoginMsgType: ע����Ϣ����
//              dwLen:          pBuf�������Ĵ�С
//  Output:
//              pBuf:       ��Ϣָ��
//              dwLen:      ��Ϣ����
//  Return:
//              TRUE:       ��ȡ�ɹ�
//              FALSE:      ��ȡʧ��
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
//  Describe:   �Ƿ������ɹ�
//  Input:      
//              pBuf:           ����Ӧ����Ϣָ��
//              dwLen:          ����Ӧ����Ϣ����
//              dwStartMsgType: ������Ϣ����
//  Output:
//  Return:
//              TRUE:       �����ɹ�
//              FALSE:      ����ʧ��
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
//  Describe:   �Ƿ�ע��ɹ�
//  Input:      
//              pBuf:           ע��Ӧ����Ϣָ��
//              dwLen:          ע��Ӧ����Ϣ����
//              dwLoginMsgType: ע����Ϣ����
//  Output:
//  Return:
//              TRUE:       ע��ɹ�
//              FALSE:      ע��ʧ��
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
//  Describe:   ����ԭʼ������Ϣ��������ǰ�û�������Ӧ����Ϣת����AppSvr���͵���Ϣ
//  Input:      
//              pBuf:       ��Ϣָ��
//              dwLen:      ��Ϣ����
//              pApply:     ԭʼ������Ϣָ��
//              dwApplyLen: ԭʼ������Ϣ����
//              dwMsgLen:   pMsg����������
//  Output:
//              pMsg:       ת�������Ϣָ��
//              dwMsgLen:   ת�������Ϣ����
//              bEndReply:  Ӧ�������־
//  Return:
//              TRUE:       ת���ɹ�
//              FALSE:      ת��ʧ��
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
	
	pReply = (LPAPPLY_MSG)&pBuf[sizeof(MSG_ADDRESS)];	// �Ʒ�ǰ�û�Ӧ����
	pSrc = (LPAPPLY_MSG)&pApply[sizeof(MSG_ADDRESS)];	// ԭʼ������
    pDst  = (LPAPPLY_MSG)pMsg;						    // ת����ı���
	
	//-->tu delete ������һ���Է�ֹ�쳣 20030617
	//pBuf[sizeof(MSG_ADDRESS) + htonl(pDst->dwLen)] = 0;	//20030425 LIUSHAOHUA 
    //<--
    if (dwLen - sizeof(MSG_ADDRESS) > dwMsgLen)
    {
        // �������������
        return FALSE;
    }
	
    // ֻ��Ҫ��������ID
    memcpy(pDst, pReply, dwLen - sizeof(MSG_ADDRESS));
    pDst->dwRequestID = pSrc->dwRequestID;
    
	DWORD dwLength;
	dwLength = htonl(pDst->dwLen) - sizeof(APPLY_MSG);
	
	pConfig = GetParamConfigFromICDName(pSrc->pCMD);
	
	CString strResult;

    //modify by xuwenfu 20090327 �ᵽ��֧�ⶨ��,�����ظ��������
	CString strTitle;
	CString strWidth;
	CString strRet ;
	CString strNewData;
	//modify by xuwenfu 20090327 ����ȡReply�����ᵽ��֧����

	CString strData;
	strData = pReply->pData;
	//��֤��ȡ������������
	strData = strData.Left(htonl(pReply->dwLen) -sizeof(APPLY_MSG) + 1);

	//add by huguangwen 2009032
	LPBRIEF_CONFIG  pBriefConfig;
	pBriefConfig = GetBriefConfigFromMIDName(pSrc->pCMD);
	
	//д��־
	NewWriteLog(pDst->pData, "TransToAppSvrMsg", pReply->pCMD, pReply->dwRequestID);
    if (htonl(pReply->dwSeq) <= 1)
    {
		if (!_stricmp(pSrc->pCMD, "10022"))//2.5	10022�û�������Ϣ��ѯ
		{
			CString strResult;
			CString strCustId;//�ͻ����
			CString strCustName;//�ͻ�����
			CString strSegmentId;//Ʒ��id
			CString strSegmentName;//Ʒ������
			CString strAreaID;//��������
			CString strAreaName;//��������
			CString strCustGread;//�ͻ�����
			CString strCustGread2;//����ͻ�����ȼ� 
			CString strProductStatusName;//�û�״̬
			CString strAddress;//�����ַ
			//////////////////////////////////////////////////////////////////////////
			CString strInnerXML;
			CMarkup xml;
			CString strANSIData;
			strANSIData = UTF8ToANSI(strData);

			//����xml����
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
								//�ͻ����
								if(xml.FindElem_C(_T("/custInfo/custId")))
								{
									strCustId = xml.GetData();
								}
								else
								{
									strData = _T("111~ȡcustId�ֶγ���");
								}
								//�ͻ�����
								if(xml.FindElem_C(_T("/custInfo/custName")))
								{
									strCustName = xml.GetData();
								}
								else
								{
									strData = _T("111~ȡcustId�ֶγ���");
								}
								//Ʒ��id
								if(xml.FindElem_C(_T("/custInfo/segmentId")))
								{
									strSegmentId = xml.GetData();
								}
								else
								{
									strData = _T("111~ȡsegmentId�ֶγ���");
								}
								//Ʒ������
								if(xml.FindElem_C(_T("/custInfo/segmentName")))
								{
									strSegmentName = xml.GetData();
								}
								else
								{
									strData = _T("111~ȡsegmentName�ֶγ���");
								}
								//��������
								if(xml.FindElem_C(_T("/custInfo/areaId")))
								{
									strAreaID = xml.GetData();
								}
								else
								{
									strData = _T("111~ȡareaId�ֶγ���");
								}
								//��������
								if(xml.FindElem_C(_T("/custInfo/areaName")))
								{
									strAreaName = xml.GetData();
								}
								else
								{
									strData = _T("111~ȡareaName�ֶγ���");
								}
								//�ͻ�����
								if(xml.FindElem_C(_T("/custInfo/custGread")))
								{
									strCustGread = xml.GetData();
								}
								else
								{
									strData = _T("111~ȡcustGread�ֶγ���");
								}
								//����ͻ�����ȼ�
								if(xml.FindElem_C(_T("/custInfo/custGread2")))
								{
									strCustGread2 = xml.GetData();
								}
								else
								{
									strData = _T("111~ȡcustGread2�ֶγ���");
								}
								//�û�״̬
								if(xml.FindElem_C(_T("/custInfo/productStatusName")))
								{
									strProductStatusName = xml.GetData();
								}
								else
								{
									strData = _T("111~ȡproductStatusName�ֶγ���");
								}
								//�����װ��ַ
								if(xml.FindElem_C(_T("/custInfo/address")))
								{
									strAddress = xml.GetData();
								}
								else
								{
									strData = _T("111~ȡaddress�ֶγ���");
								}
								strData = _T("0~") + strCustId + _T("~") + strCustName + _T("~") + strSegmentId + _T("~")
									+ strSegmentName + _T("~") + strAreaID + _T("~") + strAreaName + _T("~") + strCustGread + _T("~") + 
									strCustGread2 + _T("~") + strProductStatusName + _T("~") + strAddress + _T(";");
							}
							else
							{
								strData = _T("111~resultIDΪ��0");
							}
						}
						else
						{
							strData = _T("111~ȡresultID�ֶγ���;");
						}
					}
					else
					{
						strData = _T("111~�����ڲ�xml�������;");
					}
				}
				else
				{
					strData = _T("111~��ȡ�ڲ�xml�ַ�������;");
				}
			}
			else
			{
				strData = _T("111~����xml�������;");
			}
			//////////////////////////////////////////////////////////////////////////
			strTitle = pConfig->strTitle;
			strWidth = pConfig->strWidth;		
			
			strRet = GetField(strData, g_strRSep, g_strFSep);
			
			if (!strRet.Compare("0"))
			{
				strNewData = strRet + g_strRSep;
				strNewData += strTitle + strWidth;
				//added 20091120 ��ivr���𱣳�һ��
				CString strUserClass;  //�ͻ�����
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

	            CString strCUserclass; //���󼶱�add by tongyufeng��20100513��CRM���ص�9���ֶδ������󼶱�

                strCUserclass = GetField(strData, g_strRSep, g_strFSep);
				if(!_stricmp(strCUserclass,"0"))
				{
                    strCUserclass="";
				}

				strNewData += strCUserclass+g_strFSep;

                //10022�������������û�״̬������û���װ��ַ�����Ӵ�2��ķ���
				CString cUsertype;
				CString Bnanzhuang;

                cUsertype = GetField(strData, g_strRSep, g_strFSep);
				strNewData += cUsertype+g_strFSep;

				Bnanzhuang = GetField(strData, g_strRSep, g_strFSep);
                strNewData += Bnanzhuang+g_strRSep;
                
			}
			else
			{
				strNewData = "111;��ѯ���;40;��ѯ����,û����ؼ�¼;";
			}
		}
		else if (!_stricmp(pSrc->pCMD, "10033"))//2.8	 (CRM�ӿ�)+10033��ȡ��ƷID���ʻ�ID�Ϳͻ�ID
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
				strNewData = "111;��ѯ���;40;��ѯ����,û����ؼ�¼;";
			}

		}
		else if (!_stricmp(pSrc->pCMD, "10035"))//2.10	(CRM�ӿ�)+10035�û���Ʒ���ϲ�ѯ
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

				//BOSS�������ݸ�ʽ(��22�ǰ15��ֻ��һ����¼����7����ܶ��¼����)

				//�����룻�û���ʶ~�ͻ���ʶ~�û�����~�����Ʒ��ʶ~�����Ʒ����~ҵ������~�û�״̬~
				//�û�״̬����~�û���Чʱ��~�û�ʧЧʱ��~�û���ַ~�߽ɱ�ʶ~ҵ������~�Ʒ�ģʽ~�û�Ⱥ���ʶ��
				//�����ʶ~��������~��Ч����~ʧЧ����~��������~������ʶ~״̬�޸�ʱ�䣻[���ж��������з���]

				

				strUserInfo = GetField(strData, g_strRSep, g_strRSep); //ȡ�û���Ϣ

				for(int i=0 ; i<11 ; i++)   //ǰ11�����������ź����ĵ�ת��
				{
					strUserInfoData += GetField(strUserInfo, g_strFSep, g_strRSep);
					strUserInfoData += "~";
				}

				//�����12������ BEGIN
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
				//�����12������ END
				
				//��13����������ת��
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

				//��15����������ת��
				strUserInfoData += GetField(strUserInfo, g_strFSep, g_strRSep);
				strUserInfoData += "~";	
				
				//ת������û���Ϣ�����ַ����Ӹ��������ݲ���
				strDatatoApp = strUserInfoData;
				
				//������Ϣ���� BEGIN
				strAppendInfo = GetField(strData, g_strRSep, g_strRSep);//ȡ��һ��������Ϣ
				strAppendInfo += ";";
				strDatatoApp += strAppendInfo;

				strAppendInfo = GetField(strData, g_strRSep, g_strRSep);//ȡ�ڶ���������Ϣ
				
				while(strAppendInfo.GetLength() != 0)
				{
					strAppendInfo += ";";
					strDatatoApp +="~~~~~~~~~~~~~~~";//15���ָ�������û���Ϣ����
					strDatatoApp += strAppendInfo;
					strAppendInfo = GetField(strData, g_strRSep, g_strRSep);//ȡһ��������Ϣ	
				}
				//������Ϣ���� END
				
				strNewData = strRet + g_strRSep;
				strNewData += strTitle + strWidth;
				strNewData += strDatatoApp;
			}
			else
			{
				strNewData = "111;��ѯ���;40;��ѯ����,û����ؼ�¼;";
			}
		}
    	// add by tongyufeng,20101228,���ӳ̿��Զ�ҵ�����ӿ�
		else if (!_stricmp(pSrc->pCMD, "10041"))
		{			
			strTitle = "�������;";
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
				strNewData = "111;��ѯ���;40;����ʧ��;";
			}*/


		}

    	// add by tongyufeng,2011-02-21,���л���һ�η��أ�����ʹ��
		else if (!_stricmp(pSrc->pCMD, "10045"))
		{			
			strTitle = "�������;";
			strWidth = "40;"; 
			
			strRet = GetField(strData, g_strRSep, g_strFSep);

			strNewData = strRet + g_strRSep;
			strNewData += strTitle + strWidth;
			strNewData += strRet;
			strNewData += ";";
			

		}

		else if (!_stricmp(pSrc->pCMD, "10034"))//2.9	 (CRM�ӿ�)+10034 �޸Ŀͻ�����
		{			
			strTitle = "�������;";
			strWidth = "200;";  //15��
			
			strRet = GetField(strData, g_strRSep, g_strFSep);
			
			strNewData = strRet + g_strRSep;
			strNewData += strTitle + strWidth;
			strNewData += strRet;
			strNewData += ";";
		}
		else if (!_stricmp(pSrc->pCMD, "10015"))//2.4	(CRM�ӿ�)10015 �޸��û�����
		{			
			strTitle = "�������;";
			strWidth = "200;";  
			
			strRet = GetField(strData, g_strRSep, g_strFSep);
			
			strNewData = strRet + g_strRSep;
			strNewData += strTitle + strWidth;
			strNewData += strRet;
			strNewData += ";";
		}
		else if (!_stricmp(pSrc->pCMD, "10036"))//2.11	 (CRM�ӿ�)+10036�ͻ���Ʒ���ϲ�ѯ
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

					//�Ե�6���������� ��ʶ-�����ĵ�ת�� BEGIN
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
					//�Ե�6���������� ��ʶ-�����ĵ�ת�� END
					
					//��7����������ת
					strDataToApp += GetField(strRecord, g_strFSep, g_strRSep);
					strDataToApp += ";";
					
					//ȡ��һ����¼
					strRecord = GetField(strData, g_strRSep, g_strRSep);

				}

				strNewData = strRet + g_strRSep;
				strNewData += strTitle + strWidth;
				strNewData += strDataToApp;
			}
			else
			{
				strNewData = "111;��ѯ���;40;��ѯ����,û����ؼ�¼;";
			}
		}
		
		else if (!_stricmp(pSrc->pCMD, "10037"))//2.12	 (CRM�ӿ�)+10037 �ʻ���Ϣ��ѯ
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

					//�Ե�7���������� ��ʶ-�����ĵ�ת�� BEGIN
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

					//8��13��ת��
					for(int i = 7 ; i < 13 ; i++ )
					{
						strDataToApp += GetField(strRecord, g_strFSep, g_strRSep);
						strDataToApp += "~";
					}

					//�Ե�14���������� ��ʶ-�����ĵ�ת�� BEGIN
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
					//�Ե�14���������� ��ʶ-�����ĵ�ת�� END
					
					//��15����������ת
					strDataToApp += GetField(strRecord, g_strFSep, g_strRSep);
					strDataToApp += ";";
					
					//ȡ��һ����¼
					strRecord = GetField(strData, g_strRSep, g_strRSep);

				}

				strNewData = strRet + g_strRSep;
				strNewData += strTitle + strWidth;
				strNewData += strDataToApp;
			}
			else
			{
				strNewData = "111;��ѯ���;40;��ѯ����,û����ؼ�¼;";
			}
		}
		
		else if (!_stricmp(pSrc->pCMD, "10038"))//2.13	 (CRM�ӿ�)+10038 �ײ���Ϣ��ѯ
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
				strNewData = "111;��ѯ���;40;��ѯ����,û����ؼ�¼;";
			}
		}
		/************************************************************************/
		/* added by wangyue,2011-11-30,10046/10047                                                                     */
		/************************************************************************/
		else if (!_stricmp(pSrc->pCMD, "10046"))//2.18	10046��ѯͬ�˻��µĿ������
		{
			strTitle = _T("������~���1~�������1~���2~�������2~");
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
		else if (!_stricmp(pSrc->pCMD, "10047"))//2.19	10047��ѯͬ���֤����Ŀ������
		{
			strTitle = _T("������~���1~�������1~���2~�������2~");
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
		// add by xuwenfu 20090327 ������else
		else
		{
			strNewData = strData;
		}
        //add by xuwenfu �������ã��ᵽ��֧��������
		strcpy(pDst->pData, strNewData);
		pDst->dwLen = htonl(strNewData.GetLength() + sizeof(APPLY_MSG));
    }
	// add by xuwenfu 20090327 ���Ӷ������
	else if (htonl(pReply->dwSeq) > 1)
	{
		GetField(strData, g_strRSep, g_strRSep); //ȥ��������
		strNewData = strData;
        strcpy(pDst->pData, strNewData);
        pDst->dwLen = htonl(strNewData.GetLength() + sizeof(APPLY_MSG));
	}
	
	dwMsgLen = htonl(pDst->dwLen);
	//add by xuwenfu 20090327 ���ӽ����� /0
	pMsg[dwMsgLen - 1] = 0;
	bEndReply = !pDst->byMorePkt;
	
    return TRUE;
}





