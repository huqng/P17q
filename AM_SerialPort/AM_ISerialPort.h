#pragma once

#ifdef AM_SERIALPORT_EXPORTS
#define AM_SERIALPORT_API __declspec(dllexport)
#else
#define AM_SERIALPORT_API __declspec(dllimport)
#endif

class CISerialPort
{
public:
	CISerialPort() {}
	virtual ~CISerialPort() {};

	virtual BOOL Init(const int iComNo, const DWORD dwBaud, const int iDataBits, const int iParity, const int iStopBits) = 0;   // Init Com Port Info
	virtual BOOL IsPortOpen() = 0;																								// Check Port State	
	virtual BOOL OpenPort() = 0;																								// Open Com Port
	virtual void ClosePort() = 0;																								// Close Com Port
	virtual void SetPortNo(const int iPort) = 0;																				// Set Com No
	virtual void SetBaud(const DWORD dwBaud) = 0;																				// Set Baud 
	virtual void SetPort(const int iDataBits, const int iParity, const int iStopBits) = 0;										// Set DataBits Parity StopBits
	virtual DWORD ReadCommByByte(UCHAR *pucRecv, DWORD dwLength) = 0;
	virtual DWORD ReadComm(char *pcRecv, DWORD dwLength) = 0; 
	virtual BOOL ReadComByNum(char *pcRecv, DWORD dwLength, DWORD *pdwLength, const DWORD dwTimeOut = 0) = 0;
	virtual BOOL ReadComByEndChar(char *pcRecv, const DWORD dwMaxLen, const char cEndChar, DWORD *pdwLength, const DWORD dwTimeOut = 100) = 0;
	virtual DWORD WriteComm(const char *pcRecv, DWORD dwLength) = 0;
	virtual BOOL ClearWritebuf() = 0;
	virtual BOOL ClearReadBuf() = 0;
};

extern "C"
{
	void AM_SERIALPORT_API AM_CreateSerialPort(CISerialPort **ppSP);
	void AM_SERIALPORT_API AM_ReleaseSerialPort(CISerialPort *pSP);
};