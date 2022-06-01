#pragma once

#ifdef AM_CLIENT_EXPORTS
#define AM_CLINET_API __declspec(dllexport)
#else
#define AM_CLINET_API __declspec(dllimport)
#endif

#define AM_TCP_EVENT_ONLINE    0
#define AM_TCP_EVENT_OFFLINE   1
#define AM_TCP_EVENT_DATA      2

typedef void (*AM_OnReceiveData)(LPCVOID pParam, DWORD dwEventType, LPCTSTR lpstrIP, BYTE* pReceiveData, DWORD dwReceiveSize);

class CIClient
{
public:
	CIClient(){};
	virtual ~CIClient(){};

	virtual BOOL ConnectServer(LPCTSTR lpstrIP, const int iPort = 6666)=0; 
	virtual void SetReceiveProc(AM_OnReceiveData fun, void *pParam)=0;
	virtual void Stop()=0; 
	virtual BOOL IsConnect() = 0;
	virtual void DisConnect() = 0;

	virtual LPCTSTR GetServerIP()=0;
	virtual BOOL SendData(BYTE* pbyData, DWORD dwSize)=0;
	virtual DWORD GetLastError()=0;
	virtual	BOOL SetSendTimeOut(const int iTimeOut) = 0;
	virtual int	RecvData(BYTE *pbyData, DWORD dwSize) = 0;
};

extern "C"
{
	void AM_CLINET_API AM_CreateClient(CIClient** ppClient);
	void AM_CLINET_API AM_ReleaseClient(CIClient* pClient);
};
