// NewLogFile.h: interface for the CNewLogFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NewLogFile_H__B54C8CB9_0B51_11D4_A50E_0000214FFB02__INCLUDED_)
#define AFX_NewLogFile_H__B54C8CB9_0B51_11D4_A50E_0000214FFB02__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CNewLogFile  
{
public:
	CNewLogFile(CString strNewLogFileName, CString strBakNewLogFileName, DWORD dwLogSize);
	virtual ~CNewLogFile();

    void WriteNewLogFile(CString strLog);

private:
    CRITICAL_SECTION m_csLock;

    CString     m_strNewLogFileName;
    CString     m_strBakNewLogFileName;
    CFile       m_NewLogFile;
    DWORD       m_dwLogSize;

    void OpenNewLogFile(void);
    void CloseNewLogFile(void);
};

#endif // !defined(AFX_NewLogFile_H__B54C8CB9_0B51_11D4_A50E_0000214FFB02__INCLUDED_)
