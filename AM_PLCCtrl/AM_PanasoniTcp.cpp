#include "stdafx.h"
#include "AM_PanasoniTcp.h"

CAM_PanasoniTcp::CAM_PanasoniTcp()
{
	m_pbInitFlag = NULL;
	m_pISerialPort = NULL;
	m_piClient = NULL;
	m_pIModbus = NULL;
	m_pbInitFlag = new BOOL[MAX_STATION];
	if (m_pbInitFlag)
	{
		memset(m_pbInitFlag, FALSE, sizeof(BOOL) * MAX_STATION);
	}

	AM_CreateClient(&m_piClient);
}

CAM_PanasoniTcp::~CAM_PanasoniTcp()
{
	DisConnect();

	if (m_piClient)
	{
		AM_ReleaseClient(m_piClient);	
	}

	if (m_pbInitFlag)
	{
		delete[] m_pbInitFlag;
	}

	m_pbInitFlag = NULL;
}


int CAM_PanasoniTcp::ReadInPort(const int iPortNo, const char *pszPortHex, const int iStation /* = 1 */)
{
	int			iRet = -1;

	if (m_piClient && m_piClient->IsConnect())
	{
		m_csMyLock.Lock();
		iRet = ReadSingleArea(iStation, "X", iPortNo, pszPortHex);
		m_csMyLock.Unlock();

	}

	return iRet;
}

int CAM_PanasoniTcp::CheckOutPort(const int iPortNo, const char *pszPortHex, const int iStation /* = 1 */)
{
	int			iRet = -1;

	if (m_piClient && m_piClient->IsConnect())
	{
		m_csMyLock.Lock();
		iRet = ReadSingleArea(iStation, "Y", iPortNo, pszPortHex);
		m_csMyLock.Unlock();

	}

	return iRet;
}

int CAM_PanasoniTcp::CheckRelay(const int iPortNo, const char *pszPortHex, const int iStation /* = 1 */)
{
	int			iRet = -1;

	if (m_piClient && m_piClient->IsConnect())
	{
		m_csMyLock.Lock();
		iRet = ReadSingleArea(iStation, "R", iPortNo, pszPortHex);
		m_csMyLock.Unlock();

	}

	return iRet;
}

int CAM_PanasoniTcp::OpenOutPort(const int iPortNo, const char *pszPortHex, const int iStation /* = 1 */)
{
	int			iRet = -1;

	if (m_piClient && m_piClient->IsConnect())
	{
		m_csMyLock.Lock();
		if (SetSingleArea(iStation, "Y", iPortNo, pszPortHex, TRUE))
		{
			iRet = 1;
		}
		else
		{
			iRet = 0;
		}
		m_csMyLock.Unlock();

	}

	return iRet;
}

int CAM_PanasoniTcp::CloseOutPort(const int iPortNo, const char *pszPortHex, const int iStation /* = 1 */)
{
	int			iRet = -1;

	if (m_piClient && m_piClient->IsConnect())
	{
		m_csMyLock.Lock();
		if (SetSingleArea(iStation, "Y", iPortNo, pszPortHex, FALSE))
		{
			iRet = 1;
		}
		else
		{
			iRet = 0;
		}
		m_csMyLock.Unlock();

	}

	return iRet;
}

int CAM_PanasoniTcp::SetRelay(const int iPortNo, const char *pszPortHex, const int iStation /* = 1 */)
{
	int			iRet = -1;

	if (m_piClient && m_piClient->IsConnect())
	{
		m_csMyLock.Lock();
		if (SetSingleArea(iStation, "R", iPortNo, pszPortHex, TRUE))
		{
			iRet = 1;
		}
		else
		{
			iRet = 0;
		}
		m_csMyLock.Unlock();

	}

	return iRet;
}

int CAM_PanasoniTcp::ClearRelay(const int iPortNo, const char *pszPortHex, const int iStation /* = 1 */)
{
	int			iRet = -1;

	if (m_piClient && m_piClient->IsConnect())
	{
		m_csMyLock.Lock();
		if (SetSingleArea(iStation, "R", iPortNo, pszPortHex, FALSE))
		{
			iRet = 1;
		}
		else
		{
			iRet = 0;
		}
		m_csMyLock.Unlock();

	}

	return iRet;
}

char* CAM_PanasoniTcp::BCD(char *pszDst, const int iVal, const int iNoOfDigits)
{

	char	szTemp[20] = {0};

	int iValue = 0;
	int iValueTemp = 0;
	int iRes = 0;
	iValueTemp = iVal;

	for(int n = 1; n <= iNoOfDigits; n++) 
	{
		iValue = iValueTemp / 10;
		iRes = iValueTemp - iValue * 10;
		sprintf(pszDst, "%d", iRes);	
		strcat(pszDst, szTemp);
		strcpy(szTemp, pszDst);
		iValueTemp /= 10; // 0, 
	}
	strcpy(pszDst, szTemp);
	return pszDst;
}

char* CAM_PanasoniTcp::Hex(char *pszDst, const int iVal, const int iNoOfDigits)
{

	sprintf(pszDst, "%04X%s", iVal, "0000");

	return pszDst;
}

char* CAM_PanasoniTcp::GenPLCCmdStr(char *pszDst, const int iStationNo, const char *pszCommandCode, const char *pszData)
{
	char szTemp[10] = {0};
	BCD(szTemp, iStationNo, 2);
	sprintf(pszDst, "%c%s#%s%s**\r", '%', szTemp, pszCommandCode, pszData);
	return pszDst;
}

BOOL CAM_PanasoniTcp::ParseResponse(char *pszDstData, char *pszDstStr, const char *pszesponseCode, const char *pszesponse)
{
	int iLen = strlen(pszesponse);

	if((iLen < 9) || (pszesponse[0] !='%') || (pszesponse[iLen - 1] !='\r'))
	{
		return FALSE;
	}

	if('!' == pszesponse[3])
	{
		return FALSE;
	}
	else if('$' == pszesponse[3])
	{
		if(strncmp(&pszesponse[4], pszesponseCode, 2) != 0) 
		{
			return FALSE;
		}

		strncpy(pszDstStr, &pszesponse[1], 2);
	}

	if(iLen > 9)
	{
		strncpy(pszDstData, &pszesponse[6], iLen - 9);
	}
	else
	{
		pszDstData[0] = '\0';
	}

	return TRUE;
}

BOOL CAM_PanasoniTcp::isWriteOK(const char *pszResponsecode)
{
	char		szRecv[512] = {0};

	if(!ReadStr(szRecv, RESPONSE_TIMEOUT))
	{
		return FALSE;
	}

	char	szData[128] ={0};
	char	szStr[128] = {0};

	if(!ParseResponse(szData, szStr, pszResponsecode, szRecv))
	{
		return FALSE;
	}
	else
	{
		if('\0' == szData[0])
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
}

BOOL CAM_PanasoniTcp::SetSingleArea(const int iStation, const char *pszContactCode, const int iPortNo, const char *pszPortHex, const BOOL bFlag)
{
	char	szTemp[512] = {0};
	char	szInput[512] = {0};
	char	szVal[10] ={0};

	BCD(szTemp, iPortNo, 3);
	strcat(szTemp, pszPortHex);
	sprintf(szInput, "WCS%s%s", pszContactCode, szTemp);

	if (bFlag)
	{
		szVal[0] = '1';
	}
	else
	{
		szVal[0] = '0';
	}

	GenPLCCmdStr(szTemp, iStation, szInput, szVal);

	m_piClient->RecvData((BYTE *)szTemp, 512);


	if(!SendStr(szTemp))	
	{
		return FALSE;
	}

	return isWriteOK("WC");
}

int CAM_PanasoniTcp::ReadSingleArea(const int iStation, const char *pszContactCode, const int iPortNo, const char *pszPortHex)
{
	char	szTemp[512] = {0};
	char	szInput[512] = {0};
	char	szVal[10] ={0};
	char	szRecv[512] = {0};

	BCD(szVal, iPortNo, 3);
	strcat(szVal, pszPortHex);
	sprintf(szInput, "RCS%s", pszContactCode);


	GenPLCCmdStr(szTemp, iStation, szInput, szVal);

	m_piClient->RecvData((BYTE *)szTemp, 512);

	if(!SendStr(szTemp))	
	{
		return -1;
	}

	if (!ReadStr(szRecv, RESPONSE_TIMEOUT))
	{
		return -1;
	}

	char szData[128] = {0};
	char szStr[128] = {0};

	if(!ParseResponse(szData, szStr, "RC", szRecv))
	{
		return -1;
	}
	else
	{
		if(atoi(szData) == 0)
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
}

BOOL CAM_PanasoniTcp::SendStr(const char *pszCmdStr)
{
	int		nLength = 0;
	nLength = strlen(pszCmdStr);
	nLength = m_piClient->SendData((BYTE *)pszCmdStr, strlen(pszCmdStr));

	if(nLength == 1)
	{
		return TRUE;
	}
	else
	{
 		return FALSE;
	}
}

BOOL CAM_PanasoniTcp::ReadStr(char *pszRecvStr, const DWORD dwTimeOut /* = 120 */)
{
	DWORD dwSTTime = GetTickCount();
	int	  nLength = 0;
	do
	{
		char cTemp = 0;
		if(m_piClient->RecvData((BYTE *)pszRecvStr, 512) > 0)
		{
			break;
		}
		else
		{
			Sleep(1);
		}

		if(GetTickCount() - dwSTTime > dwTimeOut)
		{
			return FALSE;
		}

	}while(1);

	return TRUE;
}

int CAM_PanasoniTcp::ReadDTString(char *pszData, const char *pszStartAdr, const char *pszEndAdr, DWORD dwTimeout /* = 100 */, const int iStation /* = 1 */)
{
	int			iRet = -1;

	if (m_piClient && m_piClient->IsConnect())
	{
		m_csMyLock.Lock();
		iRet = 0;
		char		szCmdStr[512] = {0};
		char		szDestinationStr[10] = {0};	
		BCD(szDestinationStr, iStation, 2);
		sprintf(szCmdStr, "%c%s#RDD%s%s**\r", '%', szDestinationStr, pszStartAdr, pszEndAdr);

		m_piClient->RecvData((BYTE *)szCmdStr, 512);

		if(SendStr(szCmdStr))
		{
			if ( ReadStr(pszData, dwTimeout))
			{
				iRet = 1;
			}
		}
		m_csMyLock.Unlock();
	}

	return iRet;
}

int CAM_PanasoniTcp::ReadDTVal(int &iDTVal, const char *pszdAdr, DWORD dwTimeout /* = 100 */, const int iStation /* = 1 */)
{
	int			iRet = -1;

	if (m_piClient && m_piClient->IsConnect())
	{
		char	szData[128] = {0};

		if (ReadDTString(szData, pszdAdr, pszdAdr, dwTimeout, iStation))
		{
			char		szDataStr[128] = {0};
			char		szTemp[128] = {0};
			if (ParseResponse(szDataStr, szTemp, "RD", szData))
			{
				ExchangeHL16(szData, szDataStr);
				char* cStop;
				short sNum = (short)strtol(szData, &cStop, 16);	
				iDTVal = sNum;
				iRet = 1;
			}
			else
			{
				iRet = 0;
			}


		}	
		else
		{
			iRet = 0;
		}
	}

	return iRet;
}

char* CAM_PanasoniTcp::ExchangeHL16(char *pszDst, const char *pszSrc)
{
	pszDst[0] = pszSrc[2];
	pszDst[1] = pszSrc[3];
	pszDst[2] = pszSrc[0];
	pszDst[3] = pszSrc[1];
	return pszDst;
}

int CAM_PanasoniTcp::WriteDTString(const char *pszData, const char *pszStartAdr, const char *pszEndAdr, const DWORD dwTimeout /* = 1000 */, const int iStation /* = 1 */)
{
	int			iRet = -1;

	if (m_piClient && m_piClient->IsConnect())
	{
		m_csMyLock.Lock();
		iRet = 0;
		char		szCmdStr[512] = {0};
		char		szDestinationStr[10] = {0};	
		BCD(szDestinationStr, iStation, 2);
		sprintf(szCmdStr, "%c%s#WDD%s%s%s**\r", '%', szDestinationStr, pszStartAdr, pszEndAdr, pszData);

		m_piClient->RecvData((BYTE *)szCmdStr, 512);

		if(SendStr(szCmdStr))
		{
			if (isWriteOK("WD"))
			{
				iRet = 1;
			}
		}

		m_csMyLock.Unlock();
	}

	return iRet;
}

int CAM_PanasoniTcp::WriteDTVal(const long lDTVal, const char *pszdAdr, const DWORD dwTimeout /* = 100 */, const int iStation /* = 1 */)
{
	int			iRet = -1;

	if (m_piClient && m_piClient->IsConnect())
	{
		//		char	szTemp[10] = {0};
		char	szData[10] = {0};
		//		Hex(szTemp, lDTVal, 8);
		// 		szData[0] = szTemp[2];
		// 		szData[1] = szTemp[3];
		// 		szData[2] = szTemp[0];
		// 		szData[3] = szTemp[1];
		// 		strncpy(&szData[4], &szTemp[4], 4);
		sprintf(szData, "%02X%02X", lDTVal & 0x000000FF, (lDTVal & 0x0000FF00) >> 8/*, (lDTVal & 0x00FF0000) >> 16, (lDTVal & 0xFF000000) >> 24*/);
		iRet = WriteDTString(szData, pszdAdr, pszdAdr, dwTimeout, iStation);
	}

	return iRet;
}

int CAM_PanasoniTcp::CheckMulitInPort(char *pszData, const int iSTAddr, const int iEDAddr, const int iStation /* = 1 */)
{
	int			iRet = -1;

	if (m_piClient && m_piClient->IsConnect())
	{
		m_csMyLock.Lock();
		iRet = 0;
		char		szCmdStr[512] = {0};
		char		szDataTemp[512] = {0};

		if (ReadRCCData(szDataTemp, iStation, "X", iSTAddr, iEDAddr))
		{
			if (('$' == szDataTemp[3]) && ('R' == szDataTemp[4]) && ('C' == szDataTemp[5]))
			{
				iRet = 1;
				for (int i = 0; i < iEDAddr - iSTAddr + 1; ++i)
				{
					pszData[i * 2 + 0] = 0;
					BYTE byLow = CharTransByte(szDataTemp[i * 4 + 1 + 6]);
					BYTE byHigh = CharTransByte(szDataTemp[i * 4 + 0 + 6]);
					pszData[i * 2 + 0] = ((byHigh << 4) & 0xF0) + (byLow & 0x0F);
					pszData[i * 2 + 1] = 0;
					byLow = CharTransByte(szDataTemp[i * 4 + 3 + 6]);
					byHigh = CharTransByte(szDataTemp[i * 4 + 2 + 6]);
					pszData[i * 2 + 1] = ((byHigh << 4) & 0xF0) + (byLow & 0x0F);
				}
			}
		}

		m_csMyLock.Unlock();
	}

	return iRet;
}

int CAM_PanasoniTcp::CheckMulitOutPort(char *pszData, const int iSTAddr, const int iEDAddr, const int iStation /* = 1 */)
{
	int			iRet = -1;

	if (m_piClient && m_piClient->IsConnect())
	{
		m_csMyLock.Lock();
		iRet = 0;
		char		szCmdStr[512] = {0};
		char		szDataTemp[512] = {0};

		if (ReadRCCData(szDataTemp, iStation, "Y", iSTAddr, iEDAddr))
		{
			if (('$' == szDataTemp[3]) && ('R' == szDataTemp[4]) && ('C' == szDataTemp[5]))
			{
				iRet = 1;
				for (int i = 0; i < iEDAddr - iSTAddr + 1; ++i)
				{
					pszData[i * 2 + 0] = 0;
					BYTE byLow = CharTransByte(szDataTemp[i * 4 + 1 + 6]);
					BYTE byHigh = CharTransByte(szDataTemp[i * 4 + 0 + 6]);
					pszData[i * 2 + 0] = ((byHigh << 4) & 0xF0) + (byLow & 0x0F);
					pszData[i * 2 + 1] = 0;
					byLow = CharTransByte(szDataTemp[i * 4 + 3 + 6]);
					byHigh = CharTransByte(szDataTemp[i * 4 + 2 + 6]);
					pszData[i * 2 + 1] = ((byHigh << 4) & 0xF0) + (byLow & 0x0F);			
				}
			}
		}

		m_csMyLock.Unlock();
	}

	return iRet;
}

int CAM_PanasoniTcp::CheckMulitRelay(char *pszData, const int iSTAddr, const int iEDAddr, const int iStation /* = 1 */)
{
	int			iRet = -1;

	if (m_piClient && m_piClient->IsConnect())
	{
		m_csMyLock.Lock();
		iRet = 0;
		char		szCmdStr[512] = {0};
		char		szDataTemp[512] = {0};

		m_pISerialPort->ClearReadBuf();

		if (ReadRCCData(szDataTemp, iStation, "R", iSTAddr, iEDAddr))
		{
			if (('$' == szDataTemp[3]) && ('R' == szDataTemp[4]) && ('C' == szDataTemp[5]))
			{
				iRet = 1;
				for (int i = 0; i < iEDAddr - iSTAddr + 1; ++i)
				{
					pszData[i * 2 + 0] = 0;
					BYTE byLow = CharTransByte(szDataTemp[i * 4 + 1 + 6]);
					BYTE byHigh = CharTransByte(szDataTemp[i * 4 + 0 + 6]);
					pszData[i * 2 + 0] = ((byHigh << 4) & 0xF0) + (byLow & 0x0F);
					pszData[i * 2 + 1] = 0;
					byLow = CharTransByte(szDataTemp[i * 4 + 3 + 6]);
					byHigh = CharTransByte(szDataTemp[i * 4 + 2 + 6]);
					pszData[i * 2 + 1] = ((byHigh << 4) & 0xF0) + (byLow & 0x0F);				
				}
			}
		}

		m_csMyLock.Unlock();
	}

	return iRet;
}

int CAM_PanasoniTcp::ReadRCCData(char *pszDstData, const int iStation, const char *pszContactCode, const int iSTAddr, const int iEDAddr)
{
	int		iRet = -1;
	char	szTemp[512] = {0};
	char	szInput[512] = {0};
	char	szVal[10] ={0};

	sprintf(szInput, "RCC%s%04d%04d", pszContactCode, iSTAddr, iEDAddr);


	GenPLCCmdStr(szTemp, iStation, szInput, szVal);

	if(SendStr(szTemp))
	{
		if ( ReadStr(pszDstData, 100))
		{
			iRet = 1;
		}
	}

	return iRet;
}

BYTE CAM_PanasoniTcp::CharTransByte(const char cData)
{
	BYTE byRet = 0;

	if ((cData >= '0') && ('9' >= cData))
	{
		byRet = cData - '0';
	}
	else if ((cData >= 'A') && ('F' >= cData))
	{
		byRet = cData - 'A' + 10;
	}

	return byRet;
}