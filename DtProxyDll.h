// DtProxyDll.h : main header file for the DTPROXYDLL DLL
//

#if !defined(AFX_DTPROXYDLL_H__D9A30E8E_4807_11D4_A53E_0000214FFB02__INCLUDED_)
#define AFX_DTPROXYDLL_H__D9A30E8E_4807_11D4_A53E_0000214FFB02__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
CString g_strDebug;		//g_strDebug为true时，DllTransReplyMsg中输出IvrCall.Log，方便调试
/////////////////////////////////////////////////////////////////////////////
// CDtProxyDllApp
// See DtProxyDll.cpp for the implementation of this class
//

class CDtProxyDllApp : public CWinApp
{
public:
	CDtProxyDllApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDtProxyDllApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CDtProxyDllApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
/*
BOOL __stdcall 
DllTransSPCallMsg(char *pBuf, DWORD dwLen, char *pTrans, DWORD &dwTransLen, 
                       char *pSeqIndex, DWORD &dwIndexLen);

extern "C" EXPORT 
BOOL DllTransApplyMsg(char *pBuf, DWORD dwLen, char *pTrans, DWORD &dwTransLen, 
                      char *pSeqIndex, DWORD &dwIndexLen);

extern "C" EXPORT 
BOOL DllIsReplyMsg(char *pBuf, DWORD dwLen);

extern "C" EXPORT 
BOOL DllTransReplyMsg(char *pBuf, DWORD dwLen, char *pTask, char *pMsg, 
                      DWORD &dwMsgLen, BOOL &bEndReply);

extern "C" EXPORT 
BOOL DllTransNoApplyReplyMsg(char *pBuf, DWORD dwLen, char *pMsg, DWORD &dwMsgLen);

extern "C" EXPORT 
BOOL DllGetApplyIndex(char *pSeqIndex, DWORD &dwIndexLen);

extern "C" EXPORT 
BOOL DllGetReplySPName(CString &strName);

extern "C" EXPORT 
int  DllGetMsgLen(char *pBuf, DWORD dwLen);
*/


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DTPROXYDLL_H__D9A30E8E_4807_11D4_A53E_0000214FFB02__INCLUDED_)

