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
函数:   OpenNewLogFile()
说明:   打开事件日志文件
输入:   NONE
输出:   NONE
返回:   NONE        
说明:   
******************************************************************************/
void CNewLogFile::OpenNewLogFile()
{
    try
    {

        m_NewLogFile.Open(m_strNewLogFileName, 
                       CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite);

        // 如果该日志文件超长
        if (m_NewLogFile.GetLength() > m_dwLogSize)
        {
            // 如果备份日志文件存在, 先删除备份日志文件, 然后将日志文件更名为
            // 备份日志文件
            m_NewLogFile.Close();
            DeleteFile(m_strBakNewLogFileName);
            CFile::Rename(m_strNewLogFileName, m_strBakNewLogFileName);

            // 重新打开日志文件
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
函数:   CloseNewLogFile()
说明:   关闭事件日志文件
输入:   NONE
输出:   NONE
返回:   NONE        
说明:   
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
函数:   WriteNewLogFile(CString strWarningLog)
说明:   写一条事件日志到文件
输入:   NONE
输出:   NONE
返回:   NONE        
说明:   
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
