
#ifndef _ALERT_API_H_
#define _ALERT_API_H_

/*************************************************
  Copyright (C) 1988-1999, Huawei Tech. Co., Ltd.
  File name:      alertapi.h
  Author:       minguobin
  Version:      1.0  
  Date:	        1999-08-30
  Description: 本API主要提供给各个需要告警的设备（又称告警源）进行告警时使用。
               告警源在使用本API时，需要进行以下步骤：
               1.进行告警源端口的初始化，调用AlertInit函数；
			   2.在告警服务器上登记告警源，调用RegistAlertSrc函数；
               3.在告警源登记成功后，一定要处理与告警服务器的握手，因为告警服
			   务器将根据握手来判断告警源是否已经吊死，调用ShakeHand函数，握手
               定时由SHAKE_HAND_TIME定义；
               4.根据需要，向告警服务器发送告警信息，调用Alert函数；
			   5.停止告警，特别是告警时长为0的告警，调用StopAlert函数；
               6.根据需要，可以注销已经登记的告警源，调用UnRegistAlertSrc函数；
               7.根据需要，可以查询告警信息，调用QueryAlert函数,此时要使用RecvMsg函数
			   接收返回消息；
               8.可以根据需要对告警信息进行删除，调用ClearAll函数；
               9.告警源退出时，一定要调用AlertExit函数释放端口。
               
  Others:         // 其它内容的说明
  Function List:  // 主要函数列表，每条记录应包括函数名及功能简要说明
    1. AlertInit : 初始化告警源的端口;
    2. AlertExit : 释放告警源的端口;
    3. RegistAlertSrc : 在告警服务器上登记告警源信息;
    4. UnRegistAlertSrc :　在告警服务器上注销登记的告警源信息;
    5. ClearAll : 删除指定告警源的所有告警信息;
    6. Alert : 告警源向告警服务器发送告警信息;
    7. StopAlert : 告警源向告警服务器发送停止告警信息;
    8. QueryAlert : 根据条件查询告警信息;
    9. MaskAlt : 对指定的告警信息进行屏蔽;
    10. ShakeHand : 与告警服务器进行握手;
    11.RecvMsg : 接收由告警服务器发送的消息;
    12.SendMsg : 向告警服务器发送消息;
	13.SetTcpOutside : 设置Tcp初始化标志，以决定是由本API初始化还是由告警源自己初始化。

  History:        // 修改历史记录列表，每条修改记录应包括修改日期、修改
                  // 者及修改内容简述  
    1. Date: 1999-09-07
       Author: lihaitao
       Modification:增加StopAlert函数，修改了代码规范
    2. ...
*************************************************/


const DWORD MAX_WAITTIME = 60000;
const DWORD SHAKE_HAND_TIME = 1000;  /*握手定时*/

/*	定义了在告警箱可以使用的灯位，各个告警设备可以根据情况进行灯位登记 */
enum ALERT_DEVICE
{
    ALERT_LIGHT_1	=  30,		/*右上角右数第三列，上数第九灯位*/
    ALERT_LIGHT_2,		/*右上角右数第三列，上数第八灯位*/
    ALERT_LIGHT_3,		/*右上角右数第三列，上数第七灯位*/
    ALERT_LIGHT_4,		/*右上角右数第三列，上数第六灯位*/
    ALERT_LIGHT_5,		/*右上角右数第三列，上数第五灯位*/
    ALERT_LIGHT_6,		/*右上角右数第三列，上数第四灯位*/
    ALERT_LIGHT_7,		/*右上角右数第三列，上数第三灯位*/
    ALERT_LIGHT_8,		/*右上角右数第三列，上数第二灯位*/
    ALERT_LIGHT_9,		/*右上角右数第三列，上数第一灯位*/
    ALERT_LIGHT_10,		/*右上角右数第二列，上数第九灯位*/
    ALERT_LIGHT_11,		/*右上角右数第二列，上数第八灯位*/
    ALERT_LIGHT_12,		/*右上角右数第二列，上数第七灯位， 本灯位被UIDB占用*/
    ALERT_LIGHT_13,		/*右上角右数第二列，上数第六灯位， 本灯位被FP占用*/
    ALERT_LIGHT_14,		/*右上角右数第二列，上数第五灯位， 本灯位被VP占用*/
    ALERT_LIGHT_15,		/*右上角右数第二列，上数第四灯位， 本灯位被FEP占用*/
    ALERT_LIGHT_16,		/*右上角右数第二列，上数第三灯位， 本灯位被PROXY占用*/
    ALERT_LIGHT_17,		/*右上角右数第二列，上数第二灯位， 本灯位被IVR占用*/
    ALERT_LIGHT_18,		/*右上角右数第二列，上数第一灯位， 本灯位被CCS占用*/
    ALERT_LIGHT_29,		/*右上角右数第一列，上数第九灯位*/
    ALERT_LIGHT_20,		/*右上角右数第一列，上数第八灯位*/
    ALERT_LIGHT_21,		/*右上角右数第一列，上数第七灯位*/
    ALERT_LIGHT_22,		/*右上角右数第一列，上数第六灯位*/
    ALERT_LIGHT_23,		/*右上角右数第一列，上数第五灯位*/
    ALERT_LIGHT_24,		/*右上角右数第一列，上数第四灯位*/
    ALERT_LIGHT_25,		/*右上角右数第一列，上数第三灯位*/
    ALERT_LIGHT_26,		/*右上角右数第一列，上数第二灯位*/
    ALERT_LIGHT_27		/*右上角右数第一列，上数第一灯位*/
};

/*	定义了在告警箱的告警级别 */
enum ALERT_LEVEL
{
    EMERGENCE = 0,      /*紧急告警*/
    IMPORTANCE,	        /*重要告警*/
    WARNNING,           /*一般告警*/
    INFORMATION         /*提示信息*/
};

#ifdef __cplusplus
extern "C" {  // only need to export C interface if
              // used by C++ source code
#endif
/*************************************************
  Function:       AlertInit
  Description:    初始化告警源的端口
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input: 
	1. dwServerIP :   告警服务器的IP地址
	2. dwPort     :   告警源的端口号, 本端口号由ICD统一分配; 
  Output:         
  Return:         TRUE : 成功;  FALSE : 失败
  Others:         如果函数失败,请不要再执行其他的API
*************************************************/
__declspec(dllexport) BOOL AlertInit(DWORD dwServerIP, 
                                     DWORD dwPort);

/*************************************************
  Function:       AlertExit
  Description:    释放告警源的端口
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input: 
  Output:         
  Return:         
  Others:         如果已经调用AlertInit,一定调用本函数进行释放
*************************************************/
__declspec(dllexport) void AlertExit();

/*************************************************
  Function:       RegistAlertSrc
  Description:    在告警服务器上登记告警源信息
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input: 
    1.dwSrcIP         :   告警源的IP地址;
    2.dwSrcPort       :   告警源的端口号, 本端口号由ICD统一分配; 
    3.lpszServiceName :   告警源的名称;
    4.byLevel         :   告警级别;
    5.lpszAppName     :   告警源的应用程序名;
    6.byLightPos      :   告警源登记的灯位;
	7.byMod           :   告警源登记的模块号;
  Output:         
  Return:         TRUE : 成功;  FALSE : 失败
  Others:         
*************************************************/
__declspec(dllexport) BOOL RegistAlertSrc(DWORD dwSrcIP, 
                                          DWORD dwSrcPort, 
                                          LPSTR lpszServiceName, 
                                          BYTE byLevel, 
                                          LPSTR lpszAppName, 
                                          BYTE byLightPos, 
                                          BYTE byMod);

/*************************************************
  Function:       UnRegistAlertSrc
  Description:   在告警服务器上注销登记的告警源信息
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input: 
    1.dwSrcIP     :   告警源的IP地址;
    2.dwSrcPort   :   告警源的端口号, 本端口号由ICD统一分配; 
  Output:         
  Return:         TRUE : 成功;  FALSE : 失败
  Others:         
*************************************************/
__declspec(dllexport) BOOL UnRegistAlertSrc(DWORD dwSrcIP,
                                            DWORD dwSrcPort);

/*************************************************
  Function:       ClearAll
  Description:   删除指定告警源的所有告警信息
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input: 
    1.lpszServiceName    :   在告警服务器中登记的告警源的名称;
  Output:         
  Return:         TRUE : 成功;  FALSE : 失败
  Others:         
*************************************************/
__declspec(dllexport) BOOL ClearAll(LPSTR lpszServiceName);

/*************************************************
  Function:       Alert
  Description:   告警源向告警服务器发送告警信息
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input: 
    1.wCata           : 告警类别;
    2.byLevel         : 告警级别;
    3.lpszDescription : 告警描述;
    4.lpRawData       : 告警数据, 二进制数据, 如果告警源需要提示一些二进制错误信息, 写在这里
                        一般的告警不用填写;
    5.iRawLen         : 数据长度;
    6.dwTime          : 告警时长, 如果为零，表示无限告警而不停止, 单位：毫秒;
    7.dwEvent         : 告警事件, 表示告警信息, 请填写告警事件的序号（将由告警服务器统一考虑）;
  Output:         
  Return:         >0 : 成功, 返回告警的序号;  -1 : 失败
  Others:         
*************************************************/
__declspec(dllexport) LONG Alert(WORD wCata, 
                                 BYTE byLevel, 
                                 LPSTR lpszDescription, 
                                 void* lpRawData, 
                                 int iRawLen, 
                                 DWORD dwTime, 
                                 DWORD dwEvent);

/*************************************************
  Function:       StopAlert
  Description:   告警源向告警服务器发送停止告警信息
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input: 
    1.dwCata          : 告警信息序号;
  Output:         
  Return:         TRUE : 成功;  FALSE : 失败
  Others:         只有调用Alert函数，且告警时长为0时，才需用本函数停止告警。
*************************************************/
__declspec(dllexport) BOOL StopAlert(LONG lAlertFlow);

/*************************************************
  Function:       QueryAlert
  Description:    根据条件查询告警信息
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input: 
    1.dwTimeBegin : 查询起始时间;
    2.dwTImeEnd   : 查询结束时间;
    3.wCata       : 查询告警类别;
    4.byLevel     : 查询告警级别;
    5.dwCount     : 返回记录个数;
    6.byDirection  : 返回记录的顺序 1 - 顺序  0 - 逆序;
    7.pszSrcName    : 查询告警源名;
    8.pszQuery     : 存放返回数据的地址;
    9.dwBufSize   : 返回数据占用的空间大小;
  Output:         
  Return:         查询到的告警记录的个数。
  Others:         
*************************************************/
__declspec(dllexport) long  QueryAlert(DWORD dwTimeBegin, 
                                       DWORD dwTImeEnd, 
                                       WORD wCata, 
                                       BYTE byLevel, 
                                       BYTE byDirection, 
                                       char *cpSrcName,
                                       char * cpQuery, 
                                       DWORD dwBufSize);

/*************************************************
  Function:       MaskAlt
  Description:   对指定的告警信息进行屏蔽
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input: 
    1.dwSrcIP     :   告警源的IP地址;
    2.dwSrcPort   :   告警源的端口号, 本端口号由ICD统一分配; 
    3.bMaskFlg    :   屏蔽标志, TRUE - 屏蔽告警, FALSE - 取消屏蔽;
  Output:         
  Return:         TRUE : 成功;  FALSE : 失败
  Others:         
*************************************************/
__declspec(dllexport) BOOL MaskAlt(DWORD dwSrcIP, 
                                   DWORD dwSrcPort, 
                                   BOOL bMaskFlg);

/*************************************************
  Function:       RecvMsg
  Description:   接收由告警服务器发送的消息;
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input: 
    1.ulpSourceIP     :   如果收到消息, 存放消息来源的IP地址;
    2.ulpSourceprocID :   如果收到消息, 存放消息来源的端口号;
    3.cpData         :   如果收到消息, 存放收到的消息结构;
    4.ulDataLen      :   如果收到消息, 存放收到的消息结构的长度;
  Output:         
  Return:         消息的长度
  Others:         只有需要调用QueryAlert,才需要通过本函数收消息
*************************************************/
__declspec(dllexport) int  RecvMsg(unsigned long * ulpSourceIP, 
                                   unsigned long * ulpSourceprocID, 
                                   char * cpData, 
                                   unsigned long * ulpDataLen);

/*************************************************
  Function:       SendMsg
  Description:   向告警服务器发送消息;
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input: 
    1.ulTargetIP     :   发送消息的目的IP地址;
    2.ulTargetprocID :   发送消息的目的端口号;
    3.cpData         :   发送的消息结构;
    4.ulDataLen      :   发送的消息结构的长度;
    5.ulTimeOut      :   发送超时时长;
  Output:         
  Return:         消息的长度
  Others:         告警源一般不使用
*************************************************/
__declspec(dllexport) int  SendMsg(unsigned long ulTargetIP, 
                                   unsigned long ulTargetprocID, 
                                   char * cpData, 
                                   unsigned long ulDataLen, 
                                   unsigned long ulTimeOut);

/*************************************************
  Function:       ShakeHand
  Description:   向告警服务器发送消息
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input: 
  Output:         
  Return:         
  Others:        在使用时要定时执行本函数 
*************************************************/
__declspec(dllexport) void ShakeHand();

/*************************************************
  Function:       SetTcpOutside
  Description:    设置Tcp初始化标志，以决定是由本API初始化还是由告警源自己初始化
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input:
    1、bFlag    :  标明是否由本API进行底层通讯模块的初始化
                   bFlag = TRUE; 表明由告警源自己调用TcpInit，进行通讯底层初始化，
				   并调用TcpExit结束
                   bFlag = FALSE;表明由本API调用TcpInit，进行通讯底层初始化，
				   并在AlertExit中调用TcpExit结束
  Output:         
  Return:         
  Others:        在使用时要定时执行本函数 
*************************************************/
__declspec(dllexport) void SetTcpInitOutside(BOOL bFlag = TRUE);
#ifdef __cplusplus
}
#endif

#endif
