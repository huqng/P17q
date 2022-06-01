#pragma once

const int MAX_STATION = 10;
const int RESPONSE_TIMEOUT = 500;
#include "../AM_SerialPort/AM_ISerialPort.h"
#include "../AM_Client/AM_IClient.h"
#include "../AM_Modbus/AM_IModbus.h"
//#ifdef _DEBUG
//#pragma comment(lib, "../../../Bin/Debug/AM_SerialPort.lib")
//#pragma comment(lib, "../../../Bin/Debug/AM_Client.lib")
//#pragma comment(lib, "../../../Bin/Debug/AM_Modbus.lib")
//#else
//#pragma comment(lib, "../../../Bin/Release/AM_SerialPort.lib")
//#pragma comment(lib, "../../../Bin/Release/AM_Client.lib")
//#pragma comment(lib, "../../../Bin/Release/AM_Modbus.lib")
//#endif
#include "AM_IPLCCtrl.h"


class CMyLock
{
public:
	CMyLock()
	{
		::InitializeCriticalSection(&m_csLock);
	}

	~CMyLock()
	{
		::DeleteCriticalSection(&m_csLock);
	}

	void Lock()
	{
		::EnterCriticalSection(&m_csLock);
	}

	void Unlock()
	{
		::LeaveCriticalSection(&m_csLock);
	}

private:
	
	CRITICAL_SECTION  m_csLock;
};

class CAM_PLCBase : public CIPLCCtrl
{
public:
	CAM_PLCBase(mutex& m, CISerialPort* pISerialPort) :m_mtx(m), m_pISerialPort(pISerialPort) {};
	virtual ~CAM_PLCBase() {};

	virtual BOOL SetCom(const int iComNo, const DWORD dwBaud, const int iDataBits, const int iParity, const int iStopBits);
	virtual BOOL Connect();
	virtual void DisConnect();
	virtual BOOL IsConnect();
	virtual BOOL Connect(LPCTSTR lpstrIp, const int iPort);
protected:

	BOOL		*m_pbInitFlag;
	mutex&		m_mtx;
	CISerialPort	*m_pISerialPort;
	CIClient *m_piClient;
	CIModBus *m_pIModbus;
};

