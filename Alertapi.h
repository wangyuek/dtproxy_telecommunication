
#ifndef _ALERT_API_H_
#define _ALERT_API_H_

/*************************************************
  Copyright (C) 1988-1999, Huawei Tech. Co., Ltd.
  File name:      alertapi.h
  Author:       minguobin
  Version:      1.0  
  Date:	        1999-08-30
  Description: ��API��Ҫ�ṩ��������Ҫ�澯���豸���ֳƸ澯Դ�����и澯ʱʹ�á�
               �澯Դ��ʹ�ñ�APIʱ����Ҫ�������²��裺
               1.���и澯Դ�˿ڵĳ�ʼ��������AlertInit������
			   2.�ڸ澯�������ϵǼǸ澯Դ������RegistAlertSrc������
               3.�ڸ澯Դ�Ǽǳɹ���һ��Ҫ������澯�����������֣���Ϊ�澯��
			   �����������������жϸ澯Դ�Ƿ��Ѿ�����������ShakeHand����������
               ��ʱ��SHAKE_HAND_TIME���壻
               4.������Ҫ����澯���������͸澯��Ϣ������Alert������
			   5.ֹͣ�澯���ر��Ǹ澯ʱ��Ϊ0�ĸ澯������StopAlert������
               6.������Ҫ������ע���Ѿ��Ǽǵĸ澯Դ������UnRegistAlertSrc������
               7.������Ҫ�����Բ�ѯ�澯��Ϣ������QueryAlert����,��ʱҪʹ��RecvMsg����
			   ���շ�����Ϣ��
               8.���Ը�����Ҫ�Ը澯��Ϣ����ɾ��������ClearAll������
               9.�澯Դ�˳�ʱ��һ��Ҫ����AlertExit�����ͷŶ˿ڡ�
               
  Others:         // �������ݵ�˵��
  Function List:  // ��Ҫ�����б�ÿ����¼Ӧ���������������ܼ�Ҫ˵��
    1. AlertInit : ��ʼ���澯Դ�Ķ˿�;
    2. AlertExit : �ͷŸ澯Դ�Ķ˿�;
    3. RegistAlertSrc : �ڸ澯�������ϵǼǸ澯Դ��Ϣ;
    4. UnRegistAlertSrc :���ڸ澯��������ע���Ǽǵĸ澯Դ��Ϣ;
    5. ClearAll : ɾ��ָ���澯Դ�����и澯��Ϣ;
    6. Alert : �澯Դ��澯���������͸澯��Ϣ;
    7. StopAlert : �澯Դ��澯����������ֹͣ�澯��Ϣ;
    8. QueryAlert : ����������ѯ�澯��Ϣ;
    9. MaskAlt : ��ָ���ĸ澯��Ϣ��������;
    10. ShakeHand : ��澯��������������;
    11.RecvMsg : �����ɸ澯���������͵���Ϣ;
    12.SendMsg : ��澯������������Ϣ;
	13.SetTcpOutside : ����Tcp��ʼ����־���Ծ������ɱ�API��ʼ�������ɸ澯Դ�Լ���ʼ����

  History:        // �޸���ʷ��¼�б�ÿ���޸ļ�¼Ӧ�����޸����ڡ��޸�
                  // �߼��޸����ݼ���  
    1. Date: 1999-09-07
       Author: lihaitao
       Modification:����StopAlert�������޸��˴���淶
    2. ...
*************************************************/


const DWORD MAX_WAITTIME = 60000;
const DWORD SHAKE_HAND_TIME = 1000;  /*���ֶ�ʱ*/

/*	�������ڸ澯�����ʹ�õĵ�λ�������澯�豸���Ը���������е�λ�Ǽ� */
enum ALERT_DEVICE
{
    ALERT_LIGHT_1	=  30,		/*���Ͻ����������У������ھŵ�λ*/
    ALERT_LIGHT_2,		/*���Ͻ����������У������ڰ˵�λ*/
    ALERT_LIGHT_3,		/*���Ͻ����������У��������ߵ�λ*/
    ALERT_LIGHT_4,		/*���Ͻ����������У�����������λ*/
    ALERT_LIGHT_5,		/*���Ͻ����������У����������λ*/
    ALERT_LIGHT_6,		/*���Ͻ����������У��������ĵ�λ*/
    ALERT_LIGHT_7,		/*���Ͻ����������У�����������λ*/
    ALERT_LIGHT_8,		/*���Ͻ����������У������ڶ���λ*/
    ALERT_LIGHT_9,		/*���Ͻ����������У�������һ��λ*/
    ALERT_LIGHT_10,		/*���Ͻ������ڶ��У������ھŵ�λ*/
    ALERT_LIGHT_11,		/*���Ͻ������ڶ��У������ڰ˵�λ*/
    ALERT_LIGHT_12,		/*���Ͻ������ڶ��У��������ߵ�λ�� ����λ��UIDBռ��*/
    ALERT_LIGHT_13,		/*���Ͻ������ڶ��У�����������λ�� ����λ��FPռ��*/
    ALERT_LIGHT_14,		/*���Ͻ������ڶ��У����������λ�� ����λ��VPռ��*/
    ALERT_LIGHT_15,		/*���Ͻ������ڶ��У��������ĵ�λ�� ����λ��FEPռ��*/
    ALERT_LIGHT_16,		/*���Ͻ������ڶ��У�����������λ�� ����λ��PROXYռ��*/
    ALERT_LIGHT_17,		/*���Ͻ������ڶ��У������ڶ���λ�� ����λ��IVRռ��*/
    ALERT_LIGHT_18,		/*���Ͻ������ڶ��У�������һ��λ�� ����λ��CCSռ��*/
    ALERT_LIGHT_29,		/*���Ͻ�������һ�У������ھŵ�λ*/
    ALERT_LIGHT_20,		/*���Ͻ�������һ�У������ڰ˵�λ*/
    ALERT_LIGHT_21,		/*���Ͻ�������һ�У��������ߵ�λ*/
    ALERT_LIGHT_22,		/*���Ͻ�������һ�У�����������λ*/
    ALERT_LIGHT_23,		/*���Ͻ�������һ�У����������λ*/
    ALERT_LIGHT_24,		/*���Ͻ�������һ�У��������ĵ�λ*/
    ALERT_LIGHT_25,		/*���Ͻ�������һ�У�����������λ*/
    ALERT_LIGHT_26,		/*���Ͻ�������һ�У������ڶ���λ*/
    ALERT_LIGHT_27		/*���Ͻ�������һ�У�������һ��λ*/
};

/*	�������ڸ澯��ĸ澯���� */
enum ALERT_LEVEL
{
    EMERGENCE = 0,      /*�����澯*/
    IMPORTANCE,	        /*��Ҫ�澯*/
    WARNNING,           /*һ��澯*/
    INFORMATION         /*��ʾ��Ϣ*/
};

#ifdef __cplusplus
extern "C" {  // only need to export C interface if
              // used by C++ source code
#endif
/*************************************************
  Function:       AlertInit
  Description:    ��ʼ���澯Դ�Ķ˿�
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input: 
	1. dwServerIP :   �澯��������IP��ַ
	2. dwPort     :   �澯Դ�Ķ˿ں�, ���˿ں���ICDͳһ����; 
  Output:         
  Return:         TRUE : �ɹ�;  FALSE : ʧ��
  Others:         �������ʧ��,�벻Ҫ��ִ��������API
*************************************************/
__declspec(dllexport) BOOL AlertInit(DWORD dwServerIP, 
                                     DWORD dwPort);

/*************************************************
  Function:       AlertExit
  Description:    �ͷŸ澯Դ�Ķ˿�
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input: 
  Output:         
  Return:         
  Others:         ����Ѿ�����AlertInit,һ�����ñ����������ͷ�
*************************************************/
__declspec(dllexport) void AlertExit();

/*************************************************
  Function:       RegistAlertSrc
  Description:    �ڸ澯�������ϵǼǸ澯Դ��Ϣ
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input: 
    1.dwSrcIP         :   �澯Դ��IP��ַ;
    2.dwSrcPort       :   �澯Դ�Ķ˿ں�, ���˿ں���ICDͳһ����; 
    3.lpszServiceName :   �澯Դ������;
    4.byLevel         :   �澯����;
    5.lpszAppName     :   �澯Դ��Ӧ�ó�����;
    6.byLightPos      :   �澯Դ�Ǽǵĵ�λ;
	7.byMod           :   �澯Դ�Ǽǵ�ģ���;
  Output:         
  Return:         TRUE : �ɹ�;  FALSE : ʧ��
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
  Description:   �ڸ澯��������ע���Ǽǵĸ澯Դ��Ϣ
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input: 
    1.dwSrcIP     :   �澯Դ��IP��ַ;
    2.dwSrcPort   :   �澯Դ�Ķ˿ں�, ���˿ں���ICDͳһ����; 
  Output:         
  Return:         TRUE : �ɹ�;  FALSE : ʧ��
  Others:         
*************************************************/
__declspec(dllexport) BOOL UnRegistAlertSrc(DWORD dwSrcIP,
                                            DWORD dwSrcPort);

/*************************************************
  Function:       ClearAll
  Description:   ɾ��ָ���澯Դ�����и澯��Ϣ
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input: 
    1.lpszServiceName    :   �ڸ澯�������еǼǵĸ澯Դ������;
  Output:         
  Return:         TRUE : �ɹ�;  FALSE : ʧ��
  Others:         
*************************************************/
__declspec(dllexport) BOOL ClearAll(LPSTR lpszServiceName);

/*************************************************
  Function:       Alert
  Description:   �澯Դ��澯���������͸澯��Ϣ
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input: 
    1.wCata           : �澯���;
    2.byLevel         : �澯����;
    3.lpszDescription : �澯����;
    4.lpRawData       : �澯����, ����������, ����澯Դ��Ҫ��ʾһЩ�����ƴ�����Ϣ, д������
                        һ��ĸ澯������д;
    5.iRawLen         : ���ݳ���;
    6.dwTime          : �澯ʱ��, ���Ϊ�㣬��ʾ���޸澯����ֹͣ, ��λ������;
    7.dwEvent         : �澯�¼�, ��ʾ�澯��Ϣ, ����д�澯�¼�����ţ����ɸ澯������ͳһ���ǣ�;
  Output:         
  Return:         >0 : �ɹ�, ���ظ澯�����;  -1 : ʧ��
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
  Description:   �澯Դ��澯����������ֹͣ�澯��Ϣ
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input: 
    1.dwCata          : �澯��Ϣ���;
  Output:         
  Return:         TRUE : �ɹ�;  FALSE : ʧ��
  Others:         ֻ�е���Alert�������Ҹ澯ʱ��Ϊ0ʱ�������ñ�����ֹͣ�澯��
*************************************************/
__declspec(dllexport) BOOL StopAlert(LONG lAlertFlow);

/*************************************************
  Function:       QueryAlert
  Description:    ����������ѯ�澯��Ϣ
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input: 
    1.dwTimeBegin : ��ѯ��ʼʱ��;
    2.dwTImeEnd   : ��ѯ����ʱ��;
    3.wCata       : ��ѯ�澯���;
    4.byLevel     : ��ѯ�澯����;
    5.dwCount     : ���ؼ�¼����;
    6.byDirection  : ���ؼ�¼��˳�� 1 - ˳��  0 - ����;
    7.pszSrcName    : ��ѯ�澯Դ��;
    8.pszQuery     : ��ŷ������ݵĵ�ַ;
    9.dwBufSize   : ��������ռ�õĿռ��С;
  Output:         
  Return:         ��ѯ���ĸ澯��¼�ĸ�����
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
  Description:   ��ָ���ĸ澯��Ϣ��������
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input: 
    1.dwSrcIP     :   �澯Դ��IP��ַ;
    2.dwSrcPort   :   �澯Դ�Ķ˿ں�, ���˿ں���ICDͳһ����; 
    3.bMaskFlg    :   ���α�־, TRUE - ���θ澯, FALSE - ȡ������;
  Output:         
  Return:         TRUE : �ɹ�;  FALSE : ʧ��
  Others:         
*************************************************/
__declspec(dllexport) BOOL MaskAlt(DWORD dwSrcIP, 
                                   DWORD dwSrcPort, 
                                   BOOL bMaskFlg);

/*************************************************
  Function:       RecvMsg
  Description:   �����ɸ澯���������͵���Ϣ;
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input: 
    1.ulpSourceIP     :   ����յ���Ϣ, �����Ϣ��Դ��IP��ַ;
    2.ulpSourceprocID :   ����յ���Ϣ, �����Ϣ��Դ�Ķ˿ں�;
    3.cpData         :   ����յ���Ϣ, ����յ�����Ϣ�ṹ;
    4.ulDataLen      :   ����յ���Ϣ, ����յ�����Ϣ�ṹ�ĳ���;
  Output:         
  Return:         ��Ϣ�ĳ���
  Others:         ֻ����Ҫ����QueryAlert,����Ҫͨ������������Ϣ
*************************************************/
__declspec(dllexport) int  RecvMsg(unsigned long * ulpSourceIP, 
                                   unsigned long * ulpSourceprocID, 
                                   char * cpData, 
                                   unsigned long * ulpDataLen);

/*************************************************
  Function:       SendMsg
  Description:   ��澯������������Ϣ;
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input: 
    1.ulTargetIP     :   ������Ϣ��Ŀ��IP��ַ;
    2.ulTargetprocID :   ������Ϣ��Ŀ�Ķ˿ں�;
    3.cpData         :   ���͵���Ϣ�ṹ;
    4.ulDataLen      :   ���͵���Ϣ�ṹ�ĳ���;
    5.ulTimeOut      :   ���ͳ�ʱʱ��;
  Output:         
  Return:         ��Ϣ�ĳ���
  Others:         �澯Դһ�㲻ʹ��
*************************************************/
__declspec(dllexport) int  SendMsg(unsigned long ulTargetIP, 
                                   unsigned long ulTargetprocID, 
                                   char * cpData, 
                                   unsigned long ulDataLen, 
                                   unsigned long ulTimeOut);

/*************************************************
  Function:       ShakeHand
  Description:   ��澯������������Ϣ
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input: 
  Output:         
  Return:         
  Others:        ��ʹ��ʱҪ��ʱִ�б����� 
*************************************************/
__declspec(dllexport) void ShakeHand();

/*************************************************
  Function:       SetTcpOutside
  Description:    ����Tcp��ʼ����־���Ծ������ɱ�API��ʼ�������ɸ澯Դ�Լ���ʼ��
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input:
    1��bFlag    :  �����Ƿ��ɱ�API���еײ�ͨѶģ��ĳ�ʼ��
                   bFlag = TRUE; �����ɸ澯Դ�Լ�����TcpInit������ͨѶ�ײ��ʼ����
				   ������TcpExit����
                   bFlag = FALSE;�����ɱ�API����TcpInit������ͨѶ�ײ��ʼ����
				   ����AlertExit�е���TcpExit����
  Output:         
  Return:         
  Others:        ��ʹ��ʱҪ��ʱִ�б����� 
*************************************************/
__declspec(dllexport) void SetTcpInitOutside(BOOL bFlag = TRUE);
#ifdef __cplusplus
}
#endif

#endif
