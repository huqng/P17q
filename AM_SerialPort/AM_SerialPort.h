#include "AM_ISerialPort.h"

#define MAXBLOCK	8192
#define XON			0x11
#define XOFF		0x13


class CSerialPort : public CISerialPort
{
public:
	CSerialPort();
	~CSerialPort();

	BOOL Init(const int iComNo, const DWORD dwBaud, const int iDataBits, const int iParity, const int iStopBits);   // Init Com Port Info
	BOOL IsPortOpen();																								// Check Port State	
	BOOL OpenPort();																								// Open Com Port
	void ClosePort();																								// Close Com Port
	void SetPortNo(const int iPort);																				// Set Com No
	void SetBaud(const DWORD dwBaud);																				// Set Baud 
	void SetPort(const int iDataBits, const int iParity, const int iStopBits);										// Set DataBits Parity StopBits
	DWORD ReadCommByByte(UCHAR *pucRecv, DWORD dwLength);
	DWORD ReadComm(char *pcRecv, DWORD dwLength); 
	BOOL ReadComByNum(char *pcRecv, DWORD dwLength, DWORD *pdwLength, const DWORD dwTimeOut = 0);
	BOOL ReadComByEndChar(char *pcRecv, const DWORD dwMaxLen, const char cEndChar, DWORD *pdwLength, const DWORD dwTimeOut = 100);
	DWORD WriteComm(const char *pcRecv, DWORD dwLength);
	BOOL ClearWritebuf();
	BOOL ClearReadBuf();

private:
	BOOL					ConfigConnection();	


private:
	volatile HANDLE			m_hCom;
	volatile BOOL			m_bConnected;
	OVERLAPPED				m_osRead;
	OVERLAPPED				m_osWrite;
	DWORD					m_dwBaud;
	TCHAR					m_szPort[128];
	int						m_iStopBits;
	int						m_iParity;
	int						m_iDataBits;
	int						m_iComNo;
	int						m_iFlowCtrl;
};