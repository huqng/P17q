#pragma once
#include "AM_IClient.h"

unsigned int __stdcall Thread_C_RecvMsg(void *pParam);			//客戶端接收消息線程

class CLock
{
public:
	CLock()
	{
		::InitializeCriticalSection(&m_csLock);
	};

	~CLock()
	{
		::DeleteCriticalSection(&m_csLock);
	}

	inline void Lock()
	{
		::EnterCriticalSection(&m_csLock);
	};

	inline void Unlock()
	{
		::LeaveCriticalSection(&m_csLock);
	};

private:
	CRITICAL_SECTION m_csLock;
};

class CAM_Client : public CIClient
{
public:
	CAM_Client();
	~CAM_Client();

	BOOL ConnectServer(LPCTSTR lpstrIP, const int iPort = 6666); 
	BOOL SetSendTimeOut(const int iTimeOut);
	void SetReceiveProc(AM_OnReceiveData fun, void *pParam);
	void Stop(); 

	LPCTSTR GetServerIP();
	BOOL SendData(BYTE* pbyData, DWORD dwSize);
	DWORD GetLastError();

	void OnReceiveData(DWORD dwEventType, BYTE* pReceiveData, DWORD dwReceiveSize);
	int RecvData();
	int RecvData(BYTE *pbyData, DWORD dwSize);

	BOOL IsConnect();
	void DisConnect();
private:
	AM_OnReceiveData m_pFun;
	void			 *m_pParam;
	int				 m_iSocket;
	int				 m_iPortNum;
	DWORD			 m_dwErrorCode;
	BOOL			 m_bConnect;
	HANDLE			 m_hClient;
	tstring			 m_strIp;
	CLock			 m_csClient;
	WSAEVENT		 m_wsaEvent;
	BOOL ConnectServer();
};