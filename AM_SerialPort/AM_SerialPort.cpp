// AM_SerialPort.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "AM_SerialPort.h"


CSerialPort::CSerialPort()
{
	m_hCom = NULL;
	m_iDataBits = 8;
	m_iParity = 0;
	m_iStopBits = 0;
	m_dwBaud = 9600;
	m_iComNo = 1;
	m_bConnected = FALSE;

	memset(m_szPort, NULL, sizeof(m_szPort));
	memset(&m_osRead, NULL, sizeof(OVERLAPPED));
	memset(&m_osWrite, NULL, sizeof(OVERLAPPED));
	_tcscpy(m_szPort, _T("COM1"));
}

CSerialPort::~CSerialPort()
{
	ClosePort();
}

BOOL CSerialPort::Init(const int iComNo, const DWORD dwBaud, const int iDataBits, const int iParity, const int iStopBits)
{
	m_bConnected = FALSE;
	m_dwBaud = dwBaud;
	m_iDataBits = iDataBits;
	m_iParity = iParity;
	m_iStopBits = iStopBits;
	m_iFlowCtrl = 0;

	if (iComNo <= 0)
	{
		m_iComNo = 1;
	}
	else
	{
		m_iComNo = iComNo;
	}

	if (m_iComNo < 10)
	{
		memset(m_szPort, NULL, sizeof(m_szPort));
		wsprintf(m_szPort, _T("COM%d"), m_iComNo);

	}
	else
	{
		memset(m_szPort, NULL, sizeof(m_szPort));
		wsprintf(m_szPort, _T("\\\\.\\COM%d"), m_iComNo);
	}


	if (OpenPort())
	{
		return TRUE;
	}

	return FALSE;
}

BOOL CSerialPort::OpenPort()
{
	COMMTIMEOUTS TimeOuts;

	if (m_bConnected)
	{
		return TRUE;
	}

	m_hCom = CreateFile(m_szPort, GENERIC_READ | GENERIC_WRITE, 0, NULL,  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);

	if (m_hCom == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	memset(&m_osRead,0,sizeof(OVERLAPPED));
	memset(&m_osWrite,0,sizeof(OVERLAPPED));

	SetupComm(m_hCom, MAXBLOCK, MAXBLOCK);
	SetCommMask(m_hCom, EV_RXCHAR);


	TimeOuts.ReadIntervalTimeout=MAXDWORD; 
	TimeOuts.ReadTotalTimeoutMultiplier=0; 
	TimeOuts.ReadTotalTimeoutConstant=0; 
	TimeOuts.WriteTotalTimeoutMultiplier=0; 
	TimeOuts.WriteTotalTimeoutConstant=1000;

	SetCommTimeouts(m_hCom, &TimeOuts);

	m_bConnected = TRUE;

	if (!ConfigConnection())
	{
		return FALSE;
	}

	return TRUE;
}

void CSerialPort::ClosePort()
{
	if (!m_bConnected)
	{
		return;
	}

	m_bConnected = FALSE;

	SetCommMask(m_hCom, 0); 

	CloseHandle(m_hCom);

	m_hCom = NULL;
}

void CSerialPort::SetPortNo(const int iPort)
{
	if (iPort <= 0)
	{
		return;
	}



	if (iPort == m_iComNo)
	{
		return;	
	}


	m_iComNo = iPort;


	if (m_bConnected)
	{
		ClosePort();

		if (m_iComNo < 10)
		{
			memset(m_szPort, NULL, sizeof(m_szPort));
			wsprintf(m_szPort, _T("COM%d"), m_iComNo);

		}
		else
		{
			memset(m_szPort, NULL, sizeof(m_szPort));
			wsprintf(m_szPort, _T("\\\\.\\COM%d"), m_iComNo);
		}

		OpenPort();
	}
	else
	{

		if (m_iComNo < 10)
		{
			memset(m_szPort, NULL, sizeof(m_szPort));
			wsprintf(m_szPort, _T("COM%d"), m_iComNo);

		}
		else
		{
			memset(m_szPort, NULL, sizeof(m_szPort));
			wsprintf(m_szPort, _T("\\\\.\\COM%d"), m_iComNo);
		}
	}
}

void CSerialPort::SetBaud(const DWORD dwBaud)
{
	m_dwBaud = dwBaud;
	ConfigConnection();
}

void CSerialPort::SetPort(const int iDataBits, const int iParity, const int iStopBits)
{
	m_iDataBits = iDataBits;
	m_iParity = iParity;
	m_iStopBits = iDataBits;
	ConfigConnection();
}

BOOL CSerialPort::ConfigConnection()
{
	if (!m_bConnected)
	{
		return FALSE;
	}

	DCB dcb;

	if (!GetCommState(m_hCom, &dcb))
	{
		return FALSE;
	}

	dcb.fBinary=TRUE;
	dcb.BaudRate = m_dwBaud; 
	dcb.ByteSize = m_iDataBits; 
	dcb.fParity = TRUE;

	switch(m_iParity) 
	{
	case 0: 
		{
			dcb.Parity=NOPARITY;
		}
		break;

	case 1: 
		{
			dcb.Parity=ODDPARITY;
		}
		break;

	case 2: 
		{
			dcb.Parity=EVENPARITY;
		}
		break;
	default:;
	}

	switch(m_iStopBits) 
	{
	case 0: 
		{
			dcb.StopBits=ONESTOPBIT;
		}
		break;

	case 1: 
		{
			dcb.StopBits=ONE5STOPBITS;
		}
		break;

	case 2: 
		{
			dcb.StopBits=TWOSTOPBITS;
		}
		break;

	default:
		{

		}
		break;
	}



	dcb.fOutxCtsFlow = (1 == m_iFlowCtrl);
	dcb.fRtsControl= (1 == m_iFlowCtrl ? RTS_CONTROL_HANDSHAKE : RTS_CONTROL_ENABLE);

	dcb.fInX=dcb.fOutX= (2 == m_iFlowCtrl);
	dcb.XonChar = XON;
	dcb.XoffChar = XOFF;
	dcb.XonLim = 50;
	dcb.XoffLim = 50;

	return SetCommState(m_hCom, &dcb);
}

BOOL CSerialPort::ClearReadBuf()
{
	if (!m_bConnected)
	{
		return FALSE;
	}

	return PurgeComm(m_hCom, PURGE_RXABORT|PURGE_RXCLEAR);
}

BOOL CSerialPort::ClearWritebuf()
{
	if (!m_bConnected)
	{
		return FALSE;
	}

	return PurgeComm(m_hCom, PURGE_TXABORT|PURGE_TXCLEAR);
}

DWORD CSerialPort::WriteComm(const char *pcRecv, DWORD dwLength)
{
	BOOL bState =  FALSE;
	DWORD dwLengthTemp = dwLength;
	COMSTAT ComStat;
	DWORD dwErrorFlags = 0;

	ClearCommError(m_hCom, &dwErrorFlags, &ComStat);

	memset(&m_osWrite,0,sizeof(OVERLAPPED));
	bState = WriteFile(m_hCom, pcRecv, dwLengthTemp, &dwLengthTemp, &m_osWrite); 

	if (!bState)
	{
		if (GetLastError()==ERROR_IO_PENDING)
		{
			GetOverlappedResult(m_hCom, &m_osWrite, &dwLengthTemp,TRUE);			
		}
	}

	return dwLengthTemp;
}

DWORD CSerialPort::ReadCommByByte(UCHAR *pucRecv, DWORD dwLength)
{
	DWORD dwLengthTemp = 0;
	COMSTAT ComStat;
	DWORD dwErrorFlags = 0;

	ClearCommError(m_hCom, &dwErrorFlags, &ComStat);

	dwLengthTemp = min(dwLength, ComStat.cbInQue);
	if(!dwLengthTemp) 
	{
		return dwLengthTemp;
	}

	ReadFile(m_hCom, pucRecv, dwLengthTemp, &dwLengthTemp, &m_osRead);

	return dwLengthTemp;
}

DWORD CSerialPort::ReadComm(char *pcRecv, DWORD dwLength)
{
	DWORD dwLengthTemp = 0;
	COMSTAT ComStat;
	DWORD dwErrorFlags = 0;
	ClearCommError(m_hCom, &dwErrorFlags, &ComStat);

	dwLengthTemp = min(dwLength, ComStat.cbInQue);

	if (0 == dwLengthTemp)
	{
		return dwLengthTemp;
	}

	memset(&m_osRead,0,sizeof(OVERLAPPED));

	if (!ReadFile(m_hCom, pcRecv, dwLengthTemp, &dwLengthTemp, &m_osRead))
	{
		if(GetLastError() == ERROR_IO_PENDING)
		{
			GetOverlappedResult(m_hCom, &m_osWrite, &dwLengthTemp, FALSE); 
		}
		else
		{
			dwLengthTemp = 0;
		}
	}

	return dwLengthTemp;
}

BOOL CSerialPort::ReadComByNum(char *pcRecv, DWORD dwLength, DWORD *pdwLength, const DWORD dwTimeOut /* = 0 */)
{
	DWORD st = GetTickCount();

	DWORD LengthReturn = 0;	
	*pdwLength = 0;			

	*pdwLength = *pdwLength + LengthReturn;

	do
	{
		LengthReturn = ReadComm(pcRecv + (*pdwLength), dwLength - *pdwLength); 

		*pdwLength = *pdwLength + LengthReturn; 

		if(dwTimeOut == 0) 
		{
			break;
		}

		if(dwLength == *pdwLength)
		{
			break;
		}
	}while(GetTickCount() - st < dwTimeOut);

	return ((dwLength == *pdwLength) ? TRUE : FALSE);
}

BOOL CSerialPort::ReadComByEndChar(char *pcRecv, const DWORD dwMaxLen, const char cEndChar, DWORD *pdwLength, const DWORD dwTimeOut /* = 100 */)
{
	DWORD st = GetTickCount();

	DWORD LengthReturn = 0;	
	*pdwLength = 0;			

	*pdwLength = *pdwLength + LengthReturn;

	do
	{
		if(dwMaxLen == *pdwLength)
		{
			break;
		}

		LengthReturn = ReadComm(pcRecv + (*pdwLength),1);

		if(LengthReturn>0)
		{
			*pdwLength = *pdwLength + LengthReturn;	

			if(pcRecv[*pdwLength - 1] == cEndChar)
			{
				return TRUE;
			}

			if(dwTimeOut == 0)
			{
				continue;
			}

			st = GetTickCount();	
		}

	}while(GetTickCount() - st < dwTimeOut);

	return FALSE;
}

BOOL CSerialPort::IsPortOpen()
{
	return m_bConnected;
}

void AM_CreateSerialPort(CISerialPort **ppSP)
{
	if (*ppSP)
	{
		delete *ppSP;
		*ppSP = NULL;
	}
	CSerialPort  *pSP = new CSerialPort();
	*ppSP = pSP;
}

void AM_ReleaseSerialPort(CISerialPort *pSP)
{
	if (pSP)
	{
		delete pSP;
	}

	pSP = NULL;
}