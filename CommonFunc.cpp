// 本文件定义了一些非输出函数, 供内部使用

#include "stdafx.h"
#include "CommonFunc.h"
#include "NewLogFile.h"


CNewLogFile g_IvrLog("IvrCall.Log", "IvrCallBak.Log", 10*1024*1024);

extern char g_pLoginUserName[];
extern char g_pLoginPWD[];

extern CString g_strSrcAddr;
extern CString g_strDstAddr;
extern CString g_strRetType;

extern AUTO1860_CODE   g_Auto1860Code[];
extern PARAM_CONFIG    g_ParamConfig[];
extern HLR_MSG			g_HLR_ZMIO;
extern HLR_MSG			g_HLR_ZMSO;
extern HLR_MSG			g_HLR_ZMSO_Repalce;

extern CString g_strFSep;
extern CString g_strRSep;
extern CString g_strFeeSep;
extern CString g_strVPCodePositive;
extern CString g_strVPCodeNegative;
extern CString g_strAuto1860Code;
extern CString g_strPathOfFaxFile;

extern CString g_strDebug;

///////////////////////////////////////////////////////////////////////////////
void NewWriteLog(CString strMsg, CString strTip, CString strCallName, DWORD dwRequestId)//add by yaoleibin
{
	if(!g_strDebug.CompareNoCase("true"))
	{
		CString strCurrentTime;
        SYSTEMTIME mTime;
        GetLocalTime(&mTime);
        strCurrentTime.Format("[%04d%02d%02d %02d:%02d:%02d:%03d]", 
            mTime.wYear, mTime.wMonth, mTime.wDay, 
            mTime.wHour, mTime.wMinute, mTime.wSecond, mTime.wMilliseconds);

		dwRequestId = ntohl(dwRequestId);
		int d1 = dwRequestId/16777216;
		dwRequestId %= 16777216;
		int d2 = dwRequestId/65536;
		dwRequestId %= 65536;
		int d3 = dwRequestId/256;
		int d4 = dwRequestId%256;

		CString strRequestId;
		strRequestId.Format(" [%02x %02x %02x %02x]", d1, d2, d3, d4);

		CString strMessage;
		strMessage = strCurrentTime + "\t" + strTip + " = " + strCallName + strRequestId
			+ "\r\n" + strMsg + "\r\n\r\n";
		g_IvrLog.WriteNewLogFile(strMessage);
	}
}

void NewWriteLog(CString strMsg, CString strTip, CString strCallName, char* pRequestId)//add by yaoleibin
{
	if(!g_strDebug.CompareNoCase("true"))
	{
		CString strCurrentTime;
        SYSTEMTIME mTime;
        GetLocalTime(&mTime);
        strCurrentTime.Format("[%04d%02d%02d %02d:%02d:%02d:%03d]", 
            mTime.wYear, mTime.wMonth, mTime.wDay, 
            mTime.wHour, mTime.wMinute, mTime.wSecond, mTime.wMilliseconds);

		char szTemp[21] = "";
		memcpy(szTemp, pRequestId, 20);
		szTemp[21] = '\0';
		CString strRequestId;
		strRequestId.Format(" [%s]", szTemp);

		CString strMessage;
		strMessage = strCurrentTime + "\t" + strTip + " = " + strCallName + strRequestId
			+ "\r\n" + strMsg + "\r\n\r\n";
		g_IvrLog.WriteNewLogFile(strMessage);
	}
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//  Func:       
//  Describe:   从指定字串中取出第一个字串串, 并从主串中将该子串删除
//  Input:      
//              strRecord:      主串
//              strRecSep:      字段分割符
//              strFieldSep:    记录分割符
//  Output:     
//              strRecord:      剔除子串后的主串
//  Return:     子串,可能是空串
//  Note:       
///////////////////////////////////////////////////////////////////////////////

CString GetField(CString &strRecord, CString strFieldSep, CString strRecSep)  
{
	int iFieldSepPos;		// 第一个字段分割符的位置索引
	int iRecSepPos;			// 第一个记录分割符的位置索引
	int iStartPos;			// 两个位置索引中最靠前的一个位置索引
	CString strSep;			// 分割符
	CString strField;

//  ASSERT(strRecSep.GetLength());
//	ASSERT(strFieldSep.GetLength());

	iFieldSepPos = strRecord.Find(strFieldSep);
	iRecSepPos   = strRecord.Find(strRecSep);
	
	// 没有找到指定的字段分割符和记录分割符, 该字串是无效的, 需要清空, 
	// 并返回空串
	if (iFieldSepPos < 0 && iRecSepPos < 0)
	{
//		strField = "";
   		strField = strRecord;
		strRecord.Empty();
		return strField;
	}

	//  取两个位置索引中最靠前的一个
	if (iFieldSepPos >= 0)
	{
		if (iFieldSepPos < iRecSepPos || iRecSepPos < 0 )
		{
			iStartPos = iFieldSepPos;
			strSep = strFieldSep;
		}
		else
		{
			iStartPos = iRecSepPos;
			strSep = strRecSep;
		}
	}
	else
	{
		iStartPos = iRecSepPos;
		strSep = strRecSep;
	}

	// 取出第一个字段, 但是不包含分割符, 然后将该字段以及分割符从
	// 字符串中去掉
	strField = strRecord.Left(iStartPos);
	iStartPos = iStartPos + strSep.GetLength();
	strRecord.Delete(0, iStartPos);

	return strField;
}

///////////////////////////////////////////////////////////////////////////////
//  Func:       
//  Describe:   根据1860自动流程传递过来的操作类型对应MID所定义新业务代码
//  Input:      
//              dwAuto1860Type: 1860自动流程传递过来的操作类型
//              bDigital:       是否为数字手机
//  Output:
//  Return:     
//              "":             未找到对应的新业务代码
//              其它:           新业务代码字串
//  Note:       
///////////////////////////////////////////////////////////////////////////////
CString GetMIDCodeFromConfig(DWORD dwAuto1860Type, BOOL bDigital)
{
    CString strNewCode;
    
    if (dwAuto1860Type < AUTO1860_TYPE_NUM)
    {
        // 手机类别匹配
        if (bDigital)
        {
            strNewCode = g_Auto1860Code[dwAuto1860Type].pDigitialCode;
        }
        else
        {
            strNewCode = g_Auto1860Code[dwAuto1860Type].pAnalogCode;
        }
    }
    
    return strNewCode;
}

///////////////////////////////////////////////////////////////////////////////
//  Func:       
//  Describe:   根据ICD侧的命令字获取该命令的配置
//  Input:      
//              strICDCMD:      ICD侧的命令字
//  Output:
//  Return:     
//              NULL:           该命令字未配置
//              其它:           该命令字配置数据结构指针
//  Note:       
///////////////////////////////////////////////////////////////////////////////
LPPARAM_CONFIG GetParamConfigFromICDName(CString strICDCMD)
{
    for (int ii = 0; ii < CONFIG_NUM; ii++)
    {
        if (!g_ParamConfig[ii].bUsed)
        {
            break;
        }

        // 命令字匹配
        if (!strICDCMD.CompareNoCase(g_ParamConfig[ii].pICDCMD))
        {
            return &g_ParamConfig[ii];
        }
    }

    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
//  Func:       
//  Describe:   生成一个流水号
//  Input:      
//              dwLen:      pSeq缓冲区的大小
//  Output:
//              pSeq:       生成的流水号指针
//              dwLen:      生成的流水号的长度
//  Return:  
//  Note:       dwLen属于输入输出参数. 由于不同的协议对流水号的方式要求不一样,
//              所以需要根据具体的协议生成流水号
///////////////////////////////////////////////////////////////////////////////
BOOL MakeSequence(char *pSeq, DWORD &dwLen)
{
    static DWORD dwSeq = 1;

    ASSERT(dwLen >= sizeof(dwSeq));
    *(DWORD *)pSeq = htonl(dwSeq);
    dwLen = sizeof(dwSeq);
    dwSeq++;

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//  Func:       
//  Describe:   读取配置文件
//  Input:      
//  Output:
//  Return:     
//              TRUE:           配置文件读取成功
//              FALSE:          配置文件不存在
//  Note:       
///////////////////////////////////////////////////////////////////////////////
int ReadCfgFile()
{

	CString		strSection;
	CString		strICDCmd;
	CString		strMIDCmd;
	CString		strFormat;
	CString		strVP;
	CString		strField;
	CString		strDNA;
	CString		strTitle;			//20030407 LIUSHAOHUA
	CString		strWidth;			//20030407 LIUSHAOHUA
	int			ii, jj;
	
	if (_access(DTPROXYDLL_INI, 0))
	{
		AfxMessageBox("配置文件找不到, 请退出");
		return FALSE;
	}
	
	CString strFileName = DTPROXYDLL_INI;

	/////////////////////
	//写日志
	g_strDebug.GetBuffer(64);
	GetPrivateProfileString("COMMON", "Debug", "true", (char *)(LPCSTR)g_strDebug, 64, strFileName);
	g_strDebug.ReleaseBuffer();
	g_strDebug.TrimRight();
	/////////////////////

    // 语音编码串
	g_strVPCodePositive.GetBuffer(8192);
	GetPrivateProfileString("COMMON", "VPPositive", "", (char *)(LPCSTR)g_strVPCodePositive, 
			8192, strFileName);
    g_strVPCodePositive.ReleaseBuffer();
	g_strVPCodePositive.TrimRight();

    // 语音编码串
	g_strVPCodeNegative.GetBuffer(8192);
	GetPrivateProfileString("COMMON", "VPNegative", "", (char *)(LPCSTR)g_strVPCodeNegative, 
			8192, strFileName);
    g_strVPCodeNegative.ReleaseBuffer();
	g_strVPCodeNegative.TrimRight();

    // Auto1860操作码映射串
	g_strAuto1860Code.GetBuffer(8192);
	GetPrivateProfileString("COMMON", "Auto1860", "", (char *)(LPCSTR)g_strAuto1860Code, 
			8192, strFileName);
    g_strAuto1860Code.ReleaseBuffer();
	g_strAuto1860Code.TrimRight();

	// 传真文件路径设置
	g_strPathOfFaxFile.GetBuffer(128);
	GetPrivateProfileString("COMMON", "PathOfFaxFile", "", (char *)(LPCSTR)g_strPathOfFaxFile, 
			8192, strFileName);
    g_strPathOfFaxFile.ReleaseBuffer();
	g_strPathOfFaxFile.TrimRight();
	

    // 分割符
	g_strFSep.GetBuffer(NAMELEN + 1);
	GetPrivateProfileString("Seperator", "FieldSep", "~", (char *)(LPCSTR)g_strFSep, 
			NAMELEN + 1, strFileName);
    g_strFSep.ReleaseBuffer();
	g_strFSep.TrimRight();
	
	g_strRSep.GetBuffer(NAMELEN + 1);
	GetPrivateProfileString("Seperator", "RecSep", ";", (char *)(LPCSTR)g_strRSep, 
			NAMELEN + 1, strFileName);
    g_strRSep.ReleaseBuffer();
	g_strRSep.TrimRight();
	
	g_strFeeSep.GetBuffer(NAMELEN + 1);
	GetPrivateProfileString("Seperator", "FeeSep", ".", (char *)(LPCSTR)g_strFeeSep, 
			NAMELEN + 1, strFileName);
    g_strFeeSep.ReleaseBuffer();
	g_strFeeSep.TrimRight();

    //	命令字配置
    int nn = 0;
	for (ii = 0; ii < CONFIG_NUM; ii++)
	{
		strSection.Format("COMMAND%d", ii + 1);

		// ICD侧报文的命令字
		strICDCmd.GetBuffer(NAMELEN + 1);
		GetPrivateProfileString((LPCSTR)strSection, "ICDCMD", "", (char *)(LPCSTR)strICDCmd, 
								 NAMELEN + 1, strFileName);
        strICDCmd.ReleaseBuffer();
		strICDCmd.TrimRight();

		// MID侧报文的命令字
		strMIDCmd.GetBuffer(CMD_SIZE + 1);
		GetPrivateProfileString((LPCSTR)strSection, "MIDCMD", "", (char *)(LPCSTR)strMIDCmd, 
								 CMD_SIZE + 1, strFileName);
        strMIDCmd.ReleaseBuffer();
		strMIDCmd.TrimRight();

		if (strICDCmd == "" || strMIDCmd == "")
		{
			continue;
		}
		
        g_ParamConfig[nn].bUsed = TRUE;
		strcpy((char *)&g_ParamConfig[nn].pICDCMD, strICDCmd);
		strcpy((char *)&g_ParamConfig[nn].pMIDCMD, strMIDCmd);

		// 取得命令字格式
		strFormat.GetBuffer(4096);
		//strSection 数值
		GetPrivateProfileString((LPCSTR)strSection, "Format", "", (char *)(LPCSTR)strFormat, 
								 4096, strFileName);
		strFormat.ReleaseBuffer();
		strFormat.TrimRight();

		// 标题段 -->  20030407 LIUSHAOHUA 添加 
		strTitle.GetBuffer(2048);
		GetPrivateProfileString((LPCSTR)strSection, "Title", "", 
			(char *)(LPCSTR)strTitle, 2048, strFileName);
        strTitle.ReleaseBuffer();
		strTitle.TrimRight();
		strTitle.TrimLeft();

		// 列宽段
		strWidth.GetBuffer(2048);
		GetPrivateProfileString((LPCSTR)strSection, "Width", "", 
			(char *)(LPCSTR)strWidth, 2048, strFileName);
        strWidth.ReleaseBuffer();
		strWidth.TrimRight();
		strWidth.TrimLeft();

		strcpy((char *)&g_ParamConfig[nn].strTitle, strTitle);
		strcpy((char *)&g_ParamConfig[nn].strWidth, strWidth);
		//<--  20030407 LIUSHAOHUA 添加

		for (jj = 0; ; jj++)
		{
			strField = GetField(strFormat, g_strFSep, g_strFSep);
			
			if (!strField.GetLength())
			{
				break;
			}

			strcpy((char *)&g_ParamConfig[nn].pFormat[jj], strField);
		}

		//取得语音代码
		strVP.GetBuffer(4096);
		GetPrivateProfileString((LPCSTR)strSection, "VPPositive", "", (char *)(LPCSTR)strVP, 
								 4096, strFileName);
		strVP.ReleaseBuffer();
		strVP.TrimRight();

		for (jj = 0; ; jj++)
		{
			strField = GetField(strVP, g_strFSep, g_strFSep);
			
			if (!strField.GetLength()) 
			{
				break;
			}
			
			strField.TrimLeft();
			strcpy((char *)&g_ParamConfig[nn].pVPCode[jj], strField);
		}
		
		g_ParamConfig[nn].nVPCount = jj;

		//取得负语音代码
		strVP.GetBuffer(4096);
		GetPrivateProfileString((LPCSTR)strSection, "VPNegative", "", (char *)(LPCSTR)strVP, 
								 4096, strFileName);
		strVP.ReleaseBuffer();
		strVP.TrimRight();

		for (jj = 0; ; jj++)
		{
			strField = GetField(strVP, g_strFSep, g_strFSep);
			
			if (!strField.GetLength()) 
			{
				break;
			}
			
			strField.TrimLeft();
			strcpy((char *)&g_ParamConfig[nn].pVPCode2[jj], strField);
		}
		
        nn++;
    }

//	ReadBriefCfgFile();

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//  Func:       
//  Describe:   读取配置文件
//  Input:      
//  Output:
//  Return:     
//              TRUE:           配置文件读取成功
//              FALSE:          配置文件不存在
//  Note:       
///////////////////////////////////////////////////////////////////////////////
int ReadBriefCfgFile()
{
	CString		strSection;
	CString		strMIDCmd;
	CString		strItem;
	CString		strIndex;
	CString		strField;
	int			ii;
	int			jj;
	
	if (_access(".\\Brief.ini", 0))
	{
		AfxMessageBox("配置文件Brief.ini找不到, 请退出");
		return FALSE;
	}
	
	CString strFileName = ".\\Brief.ini";
	
	//g_BriefConfig
	
	int nn = 0;
	
	for (ii = 0; ii < CONFIG_NUM; ii++)
	{
		strSection.Format("COMMAND%d", ii + 1);
		
		// MID侧报文的命令字
		strMIDCmd.GetBuffer(20);
		GetPrivateProfileString((LPCSTR)strSection, "MIDCMD", "", 
			(char *)(LPCSTR)strMIDCmd, 20, strFileName);
        strMIDCmd.ReleaseBuffer();
		strMIDCmd.TrimRight();
		
		if (strMIDCmd == "")
		{
			continue;
		}
		
        g_BriefConfig[nn].bUsed = TRUE;
		strcpy((char *)&g_BriefConfig[nn].pMIDCMD, strMIDCmd);
		
		for (jj = 0; ; jj++)
		{
			
			if (jj >= 16)
			{
				break;
			}
			
			strField.Format("Index%d", jj + 1);
			strIndex.GetBuffer(4096);
			GetPrivateProfileString((LPCSTR)strSection, strField, "", 
				(char *)(LPCSTR)strIndex, 4096, strFileName);
			strIndex.ReleaseBuffer();
			strIndex.TrimRight();
			
			strField.Format("Item%d", jj + 1);
			strItem.GetBuffer(4096);
			GetPrivateProfileString((LPCSTR)strSection, strField, "", 
				(char *)(LPCSTR)strItem, 4096, strFileName);
			strItem.ReleaseBuffer();
			strItem.TrimRight();
			
			if (strIndex == "")
			{
				break;
			}
			
			strcpy((char *)&g_BriefConfig[nn].pCodeConfig[jj].pIndex, strIndex);
			strcpy((char *)&g_BriefConfig[nn].pCodeConfig[jj].pContent, strItem);
		}
		
		g_BriefConfig[nn].iCount = jj;
        nn++;
    }
	
	return TRUE;
}

LPBRIEF_CONFIG GetBriefConfigFromMIDName(CString strMIDCMD)
{
    for (int ii = 0; ii < CONFIG_NUM; ii++)
    {
        if (!g_BriefConfig[ii].bUsed)
        {
            break;
        }
		
        // 命令字匹配
        if (!strMIDCMD.CompareNoCase(g_BriefConfig[ii].pMIDCMD))
		{
			return &g_BriefConfig[ii];
		}
		
		
	}
	
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
//  Func:       
//  Describe:   
//  Input:      strPriCode, 发来的操作码
//  Output:
//  Return:     对应发给计费系统的操作码
//  Note:       
///////////////////////////////////////////////////////////////////////////////
CString GetBriefCode(CString strSource, CString strPriCode)
{
   	int     nPos;
    CString strNewCode;
    CString strTmp;
	CString strSour;
	
	strSour = strSource;
    strNewCode = strPriCode;
	
	if (strNewCode == "")
	{
		return strNewCode;
	}
	
	strPriCode += ".";
    nPos = strSour.Find(strPriCode);
    if (nPos >= 0)
    {
        CString strCodeConfig;
		
        strTmp = strSour.Mid(nPos);
        strCodeConfig = GetField(strTmp, "~", "~");
        GetField(strCodeConfig, ".", ".");
        return strCodeConfig;
    }
    else
    {
        return strNewCode;  // 默认值
    }
}

///////////////////////////////////////////////////////////////////////////////
//  Func:       
//  Describe:   读取配置文件
//  Input:      
//  Output:
//  Return:     
//              TRUE:           配置文件读取成功
//              FALSE:          配置文件不存在
//  Note:       
///////////////////////////////////////////////////////////////////////////////
int ReadHLRCfgFile()
{
	CString		strSection;
	CString		strField;
	CString		strShili;
	CString		strHuawei;
	int			jj;
	
	if (_access(".\\HLRConfig.ini", 0))
	{
		AfxMessageBox("配置文件HLRConfig.ini找不到, 请退出");
		return FALSE;
	}
	
	CString strFileName = ".\\HLRConfig.ini";

	int nn = 0;
	
	strSection = "ZMIO";
	for (jj = 0; jj < 32; jj++)
	{
		strField.Format("Shili%d", jj + 1);
		strShili.GetBuffer(128);
		GetPrivateProfileString((LPCSTR)strSection, strField, "", 
					(char *)(LPCSTR)strShili, 128, strFileName);
		strShili.ReleaseBuffer();
		strShili.TrimRight();
		
		if (strShili == "")
			break;

		strField.Format("Huawei%d", jj + 1);
		strHuawei.GetBuffer(128);
		GetPrivateProfileString((LPCSTR)strSection, strField, "", 
					(char *)(LPCSTR)strHuawei, 128, strFileName);
		strHuawei.ReleaseBuffer();
		strHuawei.TrimRight();

		strcpy((char *)&g_HLR_ZMIO.pHLRConfig[jj].pShiLi, strShili);
		strcpy((char *)&g_HLR_ZMIO.pHLRConfig[jj].pHuaWei, strHuawei);
    }
	g_HLR_ZMIO.iCount = jj;

	strSection = "ZMSO";
	for (jj = 0; jj < 32; jj++)
	{
		strField.Format("Shili%d", jj + 1);
		strShili.GetBuffer(128);
		GetPrivateProfileString((LPCSTR)strSection, strField, "", 
					(char *)(LPCSTR)strShili, 128, strFileName);
		strShili.ReleaseBuffer();
		strShili.TrimRight();
		
		if (strShili == "")
			break;

		strField.Format("Huawei%d", jj + 1);
		strHuawei.GetBuffer(128);
		GetPrivateProfileString((LPCSTR)strSection, strField, "", 
					(char *)(LPCSTR)strHuawei, 128, strFileName);
		strHuawei.ReleaseBuffer();
		strHuawei.TrimRight();

		strcpy((char *)&g_HLR_ZMSO.pHLRConfig[jj].pShiLi, strShili);
		strcpy((char *)&g_HLR_ZMSO.pHLRConfig[jj].pHuaWei, strHuawei);
    }
	g_HLR_ZMSO.iCount = jj;

	strSection = "ZMSO_Replace";
	for (jj = 0; jj < 32; jj++)
	{
		strField.Format("Shili%d", jj + 1);
		strShili.GetBuffer(128);
		GetPrivateProfileString((LPCSTR)strSection, strField, "", 
					(char *)(LPCSTR)strShili, 128, strFileName);
		strShili.ReleaseBuffer();
		strShili.TrimRight();
		
		if (strShili == "")
			break;

		strField.Format("Huawei%d", jj + 1);
		strHuawei.GetBuffer(128);
		GetPrivateProfileString((LPCSTR)strSection, strField, "", 
					(char *)(LPCSTR)strHuawei, 128, strFileName);
		strHuawei.ReleaseBuffer();
		strHuawei.TrimRight();

		strcpy((char *)&g_HLR_ZMSO_Repalce.pHLRConfig[jj].pShiLi, strShili);
		strcpy((char *)&g_HLR_ZMSO_Repalce.pHLRConfig[jj].pHuaWei, strHuawei);
    }
	g_HLR_ZMSO_Repalce.iCount = jj;

	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
//  Func:       
//  Describe:   根据存储过程的调用参数类型得到转换后的参数字串
//  Input:      
//              pParam:         待转换参数的类型说明
//              pData:          待转换的参数内存指针, 长度及类型由pParam指定
//  Output:
//  Return:     转换后的字串              
//  Note:       
///////////////////////////////////////////////////////////////////////////////
CString TransParamToString(LPSP_PARAM pParam, char *pData)
{
	CString			strPara;
	LPDATE_TYPE		pDate; 
	LPTIME_TYPE		pTime;
	LPDATETIME_TYPE	pDTime;

	pDate  = (LPDATE_TYPE)pData;
	pTime  = (LPTIME_TYPE)pData;
	pDTime = (LPDATETIME_TYPE)pData;
	
	switch (pParam->byDataType)
	{
	case D_DOUBLE:
		strPara.Format("%f", *(double *)pData);
		break;
	
	case D_INT_1:
		strPara.Format("%d", *(BYTE *)pData);
		break;
	
	case D_INT_2:
		strPara.Format("%d", *(short *)pData);
		break;
		
	case D_INT_3:
        strPara = "";       // 不存在这种数据类型
        break;

	case D_INT_4:
		strPara.Format("%d", *(long *)pData);
		break;
		
	case D_BCD:
	case D_BCD_END:
		//整形
		strPara.Format("%d", *(long *)pData);
		break;
		
	case D_DATE:
		//日期
		strPara.Format("%04d%02d%02d", pDate->wYear, pDate->wMonth, pDate->wDay);
		break;
	
	case D_TIME:
		//时间
		strPara.Format("%02d%02d%02d", pTime->wHour, pTime->wMinute, pTime->wSecond);
		break;
		
	case D_DATETIME:
		//日期时间
		strPara.Format("%04d%02d%02d%02d%02d%02d", 
                       pDTime->wYear, pDTime->wMonth, pDTime->wDay, 
					   pDTime->wHour, pDTime->wMinute, pDTime->wSecond, 
                       pDTime->dwFraction);
		break;
		
	case D_NULL:
        strPara = "";
        break;

	case D_CHAR:
	case D_CHAR_END:
	default:
		//如果是字符串
        char pBuf[255];

        ASSERT(pParam->wDataLen <= D_CHAR_END);
        memset(pBuf, 0 , sizeof(pBuf));
        memcpy(pBuf, pData, pParam->wDataLen);        
		strPara = pBuf;
		break;
	}

	return strPara;
}

///////////////////////////////////////////////////////////////////////////////
//  Func:       
//  Describe:   将中间件返回的结果集的某个字段转换成指定类型的数据
//  Input:      
//              pParam:         指定的转换类型格式说明
//              dwDataLen:      pData缓冲区的长度
//              strField:       需要转换的某个字段  
//  Output:
//              pData:          转换后的参数所存放的内存地址
//              dwDataLen:      转换后的参数长度
//  Return:                   
//              TRUE:           转换成功
//              FALSE:          转换失败
//  Note:       
///////////////////////////////////////////////////////////////////////////////
BOOL TransStringToParam(LPSP_PARAM pParam, char *pData, DWORD &dwDataLen, CString strField)
{
	DWORD			dwLen;
	LPDATE_TYPE		pDate; 
	LPTIME_TYPE		pTime;
	LPDATETIME_TYPE	pDTime;
	int				iInt;

	pDate  = (LPDATE_TYPE)pData;
	pTime  = (LPTIME_TYPE)pData;
	pDTime = (LPDATETIME_TYPE)pData;

	dwLen = strField.GetLength();
	strField.GetBuffer(dwLen);

    ASSERT(pParam->wDataLen <= D_CHAR_END); 
	if (pParam->wDataLen > dwDataLen)
	{
        dwDataLen = 0;      // pData缓冲区不足
		return FALSE;
	}

	memset(pData, 0, pParam->wDataLen);
	switch (pParam->byDataType)
	{
	case D_DOUBLE:
		*(double *)pData = atof(strField);
		break;
	
	case D_INT_1:
	case D_INT_2:
	case D_INT_3:
	case D_INT_4:
		iInt = atoi(strField);
		memcpy(pData, &iInt, pParam->wDataLen);
		break;
		
	case D_BCD:
	case D_BCD_END:
		//整形
		*(long *)pData = atoi(strField);
		break;
		
	case D_DATE:
		//日期
		sscanf(strField, "%04%02d%02d", &pDate->wYear, &pDate->wMonth, &pDate->wDay);  
		break;
	
	case D_TIME:
		//时间
		sscanf(strField, "%02d%02d%02d", &pTime->wHour, &pTime->wMinute, &pTime->wSecond);  
		break;
		
	case D_DATETIME:
		//日期时间
		sscanf(strField, "%04%02d%02d%02d%02d%02d",
			   &pDTime->wYear, &pDTime->wMonth, &pDTime->wDay, &pDTime->wHour, 
			   &pDTime->wMinute, &pDTime->wSecond, &pDTime->dwFraction);
		break;
		
	case D_NULL:
	case D_CHAR:
	case D_CHAR_END:
	default:
		//如果是字符串
		memcpy(pData, strField, dwLen);
		break;
	}
	
	strField.ReleaseBuffer();
    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//  Func:       
//  Describe:   动态库初始化函数
//  Input:      
//  Output:
//  Return:                   
//  Note:       
///////////////////////////////////////////////////////////////////////////////
void InitDll()
{
    static BOOL bInit = FALSE;
    
    if (!bInit)
    {
        memset(&g_Auto1860Code[0], 0, sizeof(AUTO1860_CODE) * AUTO1860_CODE_NUM);
        memset(&g_ParamConfig[0], 0, sizeof(PARAM_CONFIG) * CONFIG_NUM);

        bInit = ReadCfgFile();        
    }
}

CString GetFee(CString strFee)
{
    CString strIndex;
    CString strDescribe;
    CString strAmount;

    strIndex = GetField(strFee, g_strFeeSep, g_strFeeSep);
    strDescribe = GetField(strFee, g_strFeeSep, g_strFeeSep);
    strAmount = GetField(strFee, g_strFeeSep, g_strFeeSep);


    if (strAmount.GetLength())
    {
        strAmount.Replace(".", "");     // 元变为分
        strAmount.Replace("-", "");     // 负数变正数
    }
    else
    {
        strAmount = "0";
    }

    return strAmount;
}

CString GetVPCode(CString strFee)
{
    CString strIndex;
    CString strDescribe;
    CString strAmount;
    CString strTmp;
    int     nPos;

    strIndex = GetField(strFee, g_strFeeSep, g_strFeeSep);
    strDescribe = GetField(strFee, g_strFeeSep, g_strFeeSep);
    strAmount = GetField(strFee, g_strFeeSep, g_strFeeSep);
    
    if (!strIndex.GetLength())
    {
        return "0000";
    }

    // 根据费用的正负取相应的语音
    if (atoi(strAmount) < 0)
    {
        strTmp = g_strVPCodeNegative;
    }
    else
    {
        strTmp = g_strVPCodePositive;
    }

    // 从VP语音配置串中根据索引查找相应的配置项
    // 格式为: "索引1.语音编码1~索引2.语音编码2~...索引N.语音编码N~"
    strIndex += ".";
    nPos = strTmp.Find(strIndex);
    if (nPos >= 0)
    {
        CString strVPConfig;
        CString strVPCode;

        // 取出该配置串"索引.语音编码"
        strTmp = strTmp.Mid(nPos);
        strVPConfig = GetField(strTmp, "~", "~");
        
        GetField(strVPConfig, ".", "."); 
        strVPCode = GetField(strVPConfig, ".", ".");
        
        return strVPCode;
    }
    else
    {
        return "0000";
    }
}

CString GetFeeVP(CString strFee)
{
    CString strAmount;
    CString strVPCode;
    CString strFeeVP;

    strAmount = GetFee(strFee);
    strVPCode = GetVPCode(strFee);
    strFeeVP.Format("%05s03%s", strVPCode, strAmount);
    return strFeeVP;
}

// 根据IVR发来的Auto1860操作码获得对应的发给计费系统的操作码
CString GetAuto1860Code(CString strPriCode)
{
    int     nPos;
    CString strNewCode;
    CString strTmp;

    strNewCode = strPriCode;
    strPriCode += ".";
    nPos = g_strAuto1860Code.Find(strPriCode);
    if (nPos >= 0)
    {
        CString strCodeConfig;

        strTmp = g_strAuto1860Code.Mid(nPos);
        strCodeConfig = GetField(strTmp, "~", "~");
        GetField(strCodeConfig, ".", ".");
        return strCodeConfig;
    }
    else
    {
        return strNewCode;  // 默认值
    }
}

bool IsMobileNumber(CString strISDN)    
{
	CString str4Compare;
	bool bMobileNum = false;
	str4Compare = strISDN.Left(3);
	
	if ( !str4Compare.Compare("133") || !str4Compare.Compare("153")
		|| !str4Compare.Compare("189")|| !str4Compare.Compare("180") || !str4Compare.Compare("181"))
	{
		bMobileNum = true;
	}
    
	strISDN.TrimLeft(' ');
	strISDN.TrimRight(' ');
	
    if (strISDN.GetLength() == 11 && bMobileNum == true)
	{
		return true;
	}
	else
	{
		return false;
	}
}


int JudgeHandsetNo(CString strISDN)    
{
	CString str4Compare;
	CString str1Compare;

	
	strISDN.TrimLeft(' ');
	strISDN.TrimRight(' ');

	str4Compare = strISDN.Left(3);
	str1Compare = strISDN.Left(1);
	
	if ( !str4Compare.Compare("133") || !str4Compare.Compare("153")
		|| !str4Compare.Compare("189")|| !str4Compare.Compare("180"))
	{
		return 1;
	}	
    else if (!str1Compare.Compare("0"))
	{
		return 2;
	}
	else
	{
		return 3;
	}
}

// 短信切割功能  xuzhenquan 20100624
void MyTrimSm2(int nLenMax, CString strSmsTotal, CString strSmsSub[], int nCount)
{
	int nLenTotal = 0;
	int nLenCurrent = 0;
	unsigned char* pChar = 0;
	CString strResult;
	
	int j = 0 ;
	for(int i=0; i<nCount; i++)
	{
		nLenTotal = strSmsTotal.GetLength();
		if(nLenTotal > nLenMax)
		{
			nLenCurrent = nLenMax;
			pChar = (unsigned char*)(LPCSTR)strSmsTotal;
			for(j =nLenMax-1; j>=0; j--)
			{
				if(pChar[j] <= 127)
					break;
			}
			if((nLenMax-1-j)%2 != 0)
			{
				nLenCurrent--;
			}
			strResult = strSmsTotal.Left(nLenCurrent);
			strSmsTotal = strSmsTotal.Mid(nLenCurrent);
			strSmsSub[i] = strResult;
		}
		else
		{
			strResult = strSmsTotal;
			strSmsTotal = "";
			strSmsSub[i] = strResult;
			break;
		}
	}
	strResult = "";	
}
//added by wangyue 20130326
CString YuanToFen(CString strYuan)
{
	double dMoney = 0.0;
	dMoney = atof(strYuan);
	dMoney *= 100;
	strYuan.Format("%.0f",dMoney);
	
	return strYuan;
}

CString FenToYuan(CString strFen)
{
	double dMoney = 0.0;
	dMoney = atof(strFen);
	dMoney /= 100;
	strFen.Format("%.2f",dMoney);
	
	return strFen;
}

int GetSubStringNoCase(CString &strParentStr,CString &strSubStr)
{
	int nPos = 0;
	int nCount=0;
	while(-1 != (nPos = strParentStr.Find(strSubStr,nPos)))
	{
		nPos++;
		nCount++;
	}
	
	return nCount;
}

CString UTF8ToUnicode(char* UTF8)
{
	DWORD dwUnicodeLen;        //转换后Unicode的长度
	wchar_t *pwText;            //保存Unicode的指针
	CString strUnicode;        //返回值
	
	//获得转换后的长度，并分配内存
	dwUnicodeLen = MultiByteToWideChar(CP_UTF8,0,UTF8,-1,NULL,0);
	pwText = new wchar_t[dwUnicodeLen];
	if (!pwText)
	{		
		return strUnicode;
	}
	
	//转为Unicode
	MultiByteToWideChar(CP_UTF8,0,UTF8,-1,pwText,dwUnicodeLen);
	//转为CString
	strUnicode.Format(_T("%s"),pwText);
	//清除内存
	delete []pwText;
	//返回转换好的Unicode字串
	return strUnicode;
}

CString UnicodeToUTF8(wchar_t *pwszUnicode)
{
	char*     pElementText;
	int    iTextLen;
	// wide char to multi char
	iTextLen = WideCharToMultiByte(CP_UTF8,
		0,
		pwszUnicode,
		-1,
		NULL,
		0,
		NULL,
		NULL );
	pElementText = new char[iTextLen + 1];
	memset( ( void* )pElementText, 0, sizeof( char ) * ( iTextLen + 1 ) );
	::WideCharToMultiByte(CP_UTF8,
		0,
		pwszUnicode,
		-1,
		pElementText,
		iTextLen,
		NULL,
		NULL );
	CString strText;
	strText = pElementText;
	delete[] pElementText;
	return strText;
}

CString GetElementValue(CString &strXML,CString strTag)
{
	CString strValue;
	CString strStartTag = _T("<")+strTag;
	CString strEndTag = _T("</")+strTag+_T(">");
	int nStartPos = strXML.Find(strStartTag);
	nStartPos = strXML.Find(_T(">"),nStartPos);
	int nEndPos = strXML.Find(strEndTag,nStartPos);
	if (nStartPos != -1 && nEndPos !=-1 && nStartPos<nEndPos)
	{
		int nLen = nEndPos-nStartPos-1;
		strValue = strXML.Mid(nStartPos+1,nLen);
	}
	return strValue;
}

CString GetElement(CString &strXML,CString strTag)
{
	CString strElement;
	CString strStartTag = _T("<")+strTag;
	CString strEndTag_1 = _T("</")+strTag+_T(">");
	CString strEndTag_2 = _T("/>");
	int nEndPos = -1;
	int nStartPos=-1;
	//找到“<Tag”的位置
	nStartPos = strXML.Find(strStartTag);
	//找到</Tag>的位置,</Tag>的位置在离nStartPos最近的位置
	int nEndPos_1 = strXML.Find(strEndTag_1,nStartPos);
	//找到"/>"的位置，"/>"的位置为离nStartPos最近的位置
	int nEndPos_2 = strXML.Find(strEndTag_2,nStartPos);
	

	//比较nEndPos_1和nEndPos_2两个哪个离nStartPos最近
	if (-1 == nEndPos_1)
	{
		nEndPos = nEndPos_2;
	}
	else if (-1 == nEndPos_2)
	{
		nEndPos = nEndPos_1;
	}
	else
	{
		nEndPos = (nEndPos_1<nEndPos_2)?nEndPos_1:nEndPos_2;
	}

	if (nStartPos != -1 && nEndPos !=-1 && nStartPos<nEndPos)
	{
		int nLen = nEndPos-nStartPos+strEndTag_1.GetLength();
		strElement = strXML.Mid(nStartPos,nLen);
	}

	return strElement;
}

CString GetElementAttr(CString &strXML,CString strTag,CString strAttrName)
{
	CString strAttrValue;
	CString strElement = GetElement(strXML,strTag);
	int nStartPos = -1;
	int nEndPos=-1;
	nStartPos = strElement.Find(strAttrName);
	if (-1 != nStartPos)
	{
		nStartPos = strElement.Find(_T("\""),nStartPos);
		if (-1 != nStartPos)
		{
			nEndPos = strElement.Find(_T("\""),nStartPos);
		}
	}

	if (-1 != nStartPos && -1 != nEndPos &&(nStartPos<nEndPos))
	{
		strAttrValue.Mid(nStartPos+1,nEndPos-nStartPos-1);
	}

	return strAttrValue;
}

CString UTF8ToANSI(CString strUTF8)
{
	//去掉第一个“<”左边的乱码字符
	strUTF8 = strUTF8.Right(strUTF8.GetLength()-strUTF8.Find('<',0));
//	strUTF8 = strUTF8.Left(strUTF8.ReverseFind('>'));
	
	//将报文由utf-8转为UNICODE
	// If the function succeeds and cchWideChar is 0, the return value is the required size, 
	//in characters, for the buffer indicated by lpWideCharStr
	CString strANSIData;
	//因为返回的是需要的字符数
	int nBytesNeed_1 = MultiByteToWideChar(
		CP_UTF8,0,strUTF8.GetBuffer(strUTF8.GetLength()),
		strUTF8.GetLength(),NULL,0);
	strUTF8.ReleaseBuffer();
	if (nBytesNeed_1)
	{
		wchar_t *pwszUnicode = new wchar_t[nBytesNeed_1+1];
		memset(pwszUnicode,0,nBytesNeed_1+1);
		MultiByteToWideChar(CP_UTF8,0,
			strUTF8.GetBuffer(strUTF8.GetLength()),
			strUTF8.GetLength(),pwszUnicode,nBytesNeed_1);
		//由UNICODE转为local ANSI
		int nBytesNeed_2 = WideCharToMultiByte(CP_ACP,0,pwszUnicode,nBytesNeed_1,NULL,0,0,0);
		char *pszANSI = new char[nBytesNeed_2+1];
		memset(pszANSI,0,nBytesNeed_2+1);
		if (pszANSI)
		{
			WideCharToMultiByte(CP_ACP,0,pwszUnicode,nBytesNeed_1,pszANSI,nBytesNeed_2,0,0);
			char szEndSurplus[] = {0x3b,0x00};
			strANSIData = pszANSI;
			if (strANSIData.Find(szEndSurplus,0))
			{
				strANSIData = strANSIData.Left(strANSIData.GetLength()-1);
			}
		}
		//清理
		delete []pszANSI;
		delete []pwszUnicode;
	}
	return strANSIData;
}