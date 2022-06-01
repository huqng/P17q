#include "stdafx.h"
#include "AM_PLCBase.h"

BOOL CAM_PLCBase::SetCom(const int iComNo, const DWORD dwBaud, const int iDataBits, const int iParity, const int iStopBits)
{
	BOOL	bRet = FALSE;

	if (m_pISerialPort)
	{
		m_pISerialPort->SetPortNo(iComNo);
		m_pISerialPort->SetBaud(dwBaud);
		m_pISerialPort->SetPort(iDataBits, iParity, iStopBits);
		bRet = TRUE;
	}

	return bRet;
}

BOOL CAM_PLCBase::Connect()
{
	BOOL		bRet = FALSE;

	if (m_pISerialPort)
	{
		bRet = m_pISerialPort->OpenPort();
	}

	return bRet;
}

BOOL CAM_PLCBase::Connect(LPCTSTR lpstrIp, const int iPort)
{
	BOOL bRet = FALSE;

	if (m_piClient)
	{
		bRet = m_piClient->ConnectServer(lpstrIp, iPort);
	}
	else if (m_pIModbus)
	{
		bRet = m_pIModbus->Connect(lpstrIp, iPort);
	}

	return bRet;
}

void CAM_PLCBase::DisConnect()
{
	if (m_pISerialPort)
	{
		m_pISerialPort->ClosePort();
	}

	if (m_piClient)
	{
		m_piClient->DisConnect();
	}

	if (m_pIModbus)
	{
		m_pIModbus->DisConnect();
	}
}

BOOL CAM_PLCBase::IsConnect()
{
	BOOL bRet = FALSE;

	if (m_pISerialPort)
	{
		bRet = m_pISerialPort->IsPortOpen();
	}
	else if (m_piClient)
	{
		bRet = m_piClient->IsConnect();
	}
	else if (m_pIModbus)
	{
		bRet = m_pIModbus->IsConnect();
	}

	return bRet;
}