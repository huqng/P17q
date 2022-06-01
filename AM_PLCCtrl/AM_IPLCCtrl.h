#pragma once

#ifdef AM_PLCCTRL_EXPORTS
#define AM_PLCCTRL_API __declspec(dllexport)
#else
#define AM_PLCCTRL_API __declspec(dllimport)
#endif


class CIPLCCtrl
{
public:
	CIPLCCtrl() {}
	virtual ~CIPLCCtrl() {}
	virtual BOOL Connect(LPCTSTR lpstrIp, const int iPort) = 0;
	virtual BOOL SetCom(const int iComNo, const DWORD dwBaud, const int iDataBits, const int iParity, const int iStopBits) = 0;
	virtual BOOL Connect() = 0;
	virtual void DisConnect() = 0;
	virtual BOOL IsConnect() = 0;

	virtual int ReadInPort(const int iPortNo, const char *pszPortHex, const int iStation = 1) = 0;
	virtual int CheckOutPort(const int iPortNo, const char *pszPortHex, const int iStation = 1) = 0;
	virtual int CheckRelay(const int iPortNo, const char *pszPortHex, const int iStation = 1) = 0;

	virtual int OpenOutPort(const int iPortNo, const char *pszPortHex, const int iStation = 1) = 0;
	virtual int CloseOutPort(const int iPortNo, const char *pszPortHex, const int iStation = 1) = 0;
	virtual int SetRelay(const int iPortNo, const char *pszPortHex, const int iStation = 1) = 0;
	virtual int ClearRelay(const int iPortNo, const char *pszPortHex, const int iStation = 1) = 0;

	virtual int CheckMulitRelay(char *pszData, const int iSTAddr, const int iEDAddr, const int iStation = 1) = 0;
	virtual int CheckMulitInPort(char *pszData, const int iSTAddr, const int iEDAddr, const int iStation = 1) = 0;
	virtual int CheckMulitOutPort(char *pszData, const int iSTAddr, const int iEDAddr, const int iStation = 1) = 0;

	virtual int ReadDTString(char *pszData, const char *pszStartAdr, const char *pszEndAdr, const DWORD dwTimeout=100, const int iStation = 1) = 0; //Read More DT
	virtual int ReadDTVal(int &iDTVal, const char *pszdAdr, const DWORD dwTimeout = 100, const int iStation = 1) = 0;	//Read Single DT

	virtual int WriteDTString(const char *pszData, const char *pszStartAdr, const char *pszEndAdr, const DWORD dwTimeout = 1000, const int iStation = 1) = 0;  //Write More DT
	virtual int WriteDTVal(const long lDTVal, const char *pszdAdr, const DWORD dwTimeout = 100, const int iStation = 1) = 0;  //Write Single DT
};

extern "C"
{
	void AM_PLCCTRL_API AM_CreatePLCCtrl(CIPLCCtrl **pPLC, const int iMode, std::mutex& m, CISerialPort* pISerialPort);
	void AM_PLCCTRL_API AM_ReleasePLCCtrl(CIPLCCtrl *pPLC);
};
