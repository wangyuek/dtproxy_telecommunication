// NewLogFile.cpp: implementation of the CNewLogFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewLogFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern DWORD	g_dwNewLogFileSize;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNewLogFile::CNewLogFile(CString strNewLogFileName, CString strBakNewLogFileName, DWORD dwLogSize)
{
    m_strNewLogFileName    = strNewLogFileName; 
    m_strBakNewLogFileName = strBakNewLogFileName; 
    m_dwLogSize         = dwLogSize;
    InitializeCriticalSection(&m_csLock);
}

CNewLogFile::~CNewLogFile()
{
    DeleteCriticalSection(&m_csLock);
}

/******************************************************************************
����:   OpenNewLogFile()
˵��:   ���¼���־�ļ�
����:   NONE
���:   NONE
����:   NONE        
˵��:   
******************************************************************************/
void CNewLogFile::OpenNewLogFile()
{
    try
    {

        m_NewLogFile.Open(m_strNewLogFileName, 
                       CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite);

        // �������־�ļ�����
        if (m_NewLogFile.GetLength() > m_dwLogSize)
        {
            // ���������־�ļ�����, ��ɾ��������־�ļ�, Ȼ����־�ļ�����Ϊ
            // ������־�ļ�
            m_NewLogFile.Close();
            DeleteFile(m_strBakNewLogFileName);
            CFile::Rename(m_strNewLogFileName, m_strBakNewLogFileName);

            // ���´���־�ļ�
            m_NewLogFile.Open(m_strNewLogFileName, 
                           CFile::modeCreate | CFile::modeWrite);    
        }
        
        m_NewLogFile.SeekToEnd();
    }
    catch(...)
    {
    }
}

/******************************************************************************
����:   CloseNewLogFile()
˵��:   �ر��¼���־�ļ�
����:   NONE
���:   NONE
����:   NONE        
˵��:   
******************************************************************************/
void CNewLogFile::CloseNewLogFile()
{
    try
    {
        m_NewLogFile.Close();    
    }
    catch(...)
    {
    }
}

/******************************************************************************
����:   WriteNewLogFile(CString strWarningLog)
˵��:   дһ���¼���־���ļ�
����:   NONE
���:   NONE
����:   NONE        
˵��:   
******************************************************************************/
void CNewLogFile::WriteNewLogFile(CString strLog)
{
    EnterCriticalSection(&m_csLock);
    
    OpenNewLogFile();
    try
    {
        m_NewLogFile.Write(strLog, strLog.GetLength());
    }
    catch(...)
    {
    }
    CloseNewLogFile();

    LeaveCriticalSection(&m_csLock);
}
