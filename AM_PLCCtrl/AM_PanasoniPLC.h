#pragma once
#include "AM_PLCBase.h"

class CAM_PanasoniPLC : public CAM_PLCBase
{
public:
	CAM_PanasoniPLC(mutex&, CISerialPort*);
	virtual ~CAM_PanasoniPLC();

	int ReadInPort(const int iPortNo, const char *pszPortHex, const int iStation = 1);
	int CheckOutPort(const int iPortNo, const char *pszPortHex, const int iStation = 1);
	int CheckRelay(const int iPortNo, const char *pszPortHex, const int iStation = 1);

	int OpenOutPort(const int iPortNo, const char *pszPortHex, const int iStation = 1);
	int CloseOutPort(const int iPortNo, const char *pszPortHex, const int iStation = 1);
	int SetRelay(const int iPortNo, const char *pszPortHex, const int iStation = 1);
	int ClearRelay(const int iPortNo, const char *pszPortHex, const int iStation = 1);

	int ReadDTString(char *pszData, const char *pszStartAdr, const char *pszEndAdr, const DWORD dwTimeout = 100, const int iStation = 1); //Read More DT
	int ReadDTVal(int &iDTVal, const char *pszdAdr, const DWORD dwTimeout = 100, const int iStation = 1);	//Read Single DT

	int WriteDTString(const char *pszData, const char *pszStartAdr, const char *pszEndAdr, const DWORD dwTimeout = 1000, const int iStation = 1);  //Write More DT
	int WriteDTVal(const long lDTVal, const char *pszdAdr, const DWORD dwTimeout = 100, const int iStation = 1);  //Write Single DT

	int CheckMulitRelay(char *pszData, const int iSTAddr, const int iEDAddr, const int iStation = 1);
	int CheckMulitInPort(char *pszData, const int iSTAddr, const int iEDAddr, const int iStation = 1);
	int CheckMulitOutPort(char *pszData, const int iSTAddr, const int iEDAddr, const int iStation = 1);

private:

	char* BCD(char *pszDst, const int iVal, const int iNoOfDigits);
	char* Hex(char *pszDst, const int iVal, const int iNoOfDigits);
	BYTE  CharTransByte(const char cData);

	char* GenPLCCmdStr(char *pszDst, const int iStationNo, const char *pszCommandCode, const char *pszData);

	BOOL isWriteOK(const char *pszResponsecode);

	int ReadSingleArea(const int iStation, const char *pszContactCode, const int iPortNo, const char *pszPortHex);
	BOOL SetSingleArea(const int iStation, const char *pszContactCode, const int iPortNo, const char *pszPortHex, const BOOL bFlag);	

	BOOL ParseResponse(char *pszDstData, char *pszDstStr, const char *pszesponseCode, const char *pszesponse);
	int ReadRCCData(char *pszDstData, const int iStation, const char *pszContactCode, const int iSTAddr, const int iEDAddr);

	char* ExchangeHL16(char *pszDst, const char *pszSrc);
	BOOL SendStr(const char *pszCmdStr);
	BOOL ReadStr(char *pszRecvStr, const DWORD dwTimeOut = 120);

};
