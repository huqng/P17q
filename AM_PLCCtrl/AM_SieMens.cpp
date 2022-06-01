#include "stdafx.h"
#include "AM_SieMens.h"

CAM_Siemens::CAM_Siemens()
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

	AM_CreateModbus(&m_pIModbus, 1);
}

CAM_Siemens::~CAM_Siemens()
{
	DisConnect();

	if (m_pIModbus)
	{
		AM_ReleaseModbus(m_pIModbus);	
	}

	if (m_pbInitFlag)
	{
		delete[] m_pbInitFlag;
	}

	m_pbInitFlag = NULL;
}


int CAM_Siemens::ReadInPort(const int iPortNo, const char *pszPortHex, const int iStation /* = 1 */)
{
	int			iRet = -1;

	if (m_pIModbus && m_pIModbus->IsConnect())
	{
		m_csMyLock.Lock();
		int iAddr = 0;
		int iValue = 0;
		iAddr = atoi(pszPortHex) + iPortNo * 8;
		if (m_pIModbus->ReadDispersedByBit(iAddr, &iValue))
		{
			if (iValue)
			{
				iRet = 1;
			}
			else
			{
				iRet = 0;
			}
		}
		m_csMyLock.Unlock();

	}

	return iRet;
}

int CAM_Siemens::CheckOutPort(const int iPortNo, const char *pszPortHex, const int iStation /* = 1 */)
{
	int			iRet = -1;

	if (m_pIModbus && m_pIModbus->IsConnect())
	{
		m_csMyLock.Lock();
		int iAddr = 0;
		int iValue = 0;
		iAddr = atoi(pszPortHex) + iPortNo * 8;
		if (m_pIModbus->ReadCoilByBit(iAddr, &iValue))
		{
			if (iValue)
			{
				iRet = 1;
			}
			else
			{
				iRet = 0;
			}
		}
		m_csMyLock.Unlock();

	}

	return iRet;
}

int CAM_Siemens::CheckRelay(const int iPortNo, const char *pszPortHex, const int iStation /* = 1 */)
{
	int			iRet = -1;

	if (m_pIModbus && m_pIModbus->IsConnect())
	{
		m_csMyLock.Lock();
		int iValue = 0;
		int iAddr = 0;
		int iBit = 0;
		if (iPortNo % 2 == 0)
		{
			iAddr = iPortNo;
			iBit = atoi(pszPortHex) + 8;
		}
		else
		{
			iAddr = iPortNo - 1;
			iBit = atoi(pszPortHex);
		}
		if (m_pIModbus->ReadHoldRegisterByByte(iAddr / 2, &iValue))
		{
			if (iValue & (1 << iBit))
			{
				iRet = 1;
			}
			else
			{
				iRet = 0;
			}
		}
		m_csMyLock.Unlock();

	}

	return iRet;
}

int CAM_Siemens::OpenOutPort(const int iPortNo, const char *pszPortHex, const int iStation /* = 1 */)
{
	int			iRet = -1;

	if (m_pIModbus && m_pIModbus->IsConnect())
	{
		m_csMyLock.Lock();
		int iAddr = 0;
		iAddr = atoi(pszPortHex) + iPortNo * 8;
		if (m_pIModbus->WriteSingleCoilByByte(iAddr, 1))
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

int CAM_Siemens::CloseOutPort(const int iPortNo, const char *pszPortHex, const int iStation /* = 1 */)
{
	int			iRet = -1;

	if (m_pIModbus && m_pIModbus->IsConnect())
	{
		m_csMyLock.Lock();
		int iAddr = 0;
		iAddr = atoi(pszPortHex) + iPortNo * 8;
		if (m_pIModbus->WriteSingleCoilByByte(iAddr, 0))
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

int CAM_Siemens::SetRelay(const int iPortNo, const char *pszPortHex, const int iStation /* = 1 */)
{
	int			iRet = -1;

	if (m_pIModbus && m_pIModbus->IsConnect())
	{
		m_csMyLock.Lock();
		int iValue = 0;
		int iAddr = 0;
		int iBit = 0;
		if (iPortNo % 2 == 0)
		{
			iAddr = iPortNo;
			iBit = atoi(pszPortHex) + 8;
		}
		else
		{
			iAddr = iPortNo - 1;
			iBit = atoi(pszPortHex);
		}
		if (m_pIModbus->ReadHoldRegisterByByte(iAddr / 2, &iValue))
		{
			iValue = iValue | (1 << iBit);
			
			if (m_pIModbus->WriteSingleRegisterByByte(iAddr / 2, iValue))
			{
				iRet = 1;
			}
		}
		m_csMyLock.Unlock();

	}

	return iRet;
}

int CAM_Siemens::ClearRelay(const int iPortNo, const char *pszPortHex, const int iStation /* = 1 */)
{
	int			iRet = -1;

	if (m_pIModbus && m_pIModbus->IsConnect())
	{
		m_csMyLock.Lock();
		int iValue = 0;
		int iAddr = 0;
		int iBit = 0;
		if (iPortNo % 2 == 0)
		{
			iAddr = iPortNo;
			iBit = atoi(pszPortHex) + 8;
		}
		else
		{
			iAddr = iPortNo - 1;
			iBit = atoi(pszPortHex);
		}
		if (m_pIModbus->ReadHoldRegisterByByte(iAddr / 2, &iValue))
		{
			iValue = iValue & (~(1 << iBit));

			if (m_pIModbus->WriteSingleRegisterByByte(iAddr / 2, iValue))
			{
				iRet = 1;
			}
		}
		m_csMyLock.Unlock();

	}

	return iRet;
}

int CAM_Siemens::WriteDTString(const char *pszData, const char *pszStartAdr, const char *pszEndAdr, const DWORD dwTimeout /* = 1000 */, const int iStation /* = 1 */)
{
	return 0;
}

int CAM_Siemens::WriteDTVal(const long lDTVal, const char *pszdAdr, const DWORD dwTimeout /* = 100 */, const int iStation /* = 1 */)
{
	int iSTAddr = 0;
	int iRet = -1;
	int iValue = (int)lDTVal;
	int aiValue[2] = {0};
	aiValue[0] = (lDTVal >> 16) & 0x0000FFFF;
	aiValue[1] = lDTVal & 0x0000FFFF;
	iSTAddr = atoi(pszdAdr);

	if (m_pIModbus && m_pIModbus->IsConnect())
	{
		m_csMyLock.Lock();
//		if (m_pIModbus->WriteMultiRegisterByByte(iSTAddr, aiValue, 2))
		if (m_pIModbus->WriteSingleRegisterByByte(iSTAddr, iValue))
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

int CAM_Siemens::ReadDTString(char *pszData, const char *pszStartAdr, const char *pszEndAdr, const DWORD dwTimeout /* = 100 */, const int iStation /* = 1 */)
{
	return 0;
}

int CAM_Siemens::ReadDTVal(int &iDTVal, const char *pszdAdr, const DWORD dwTimeout /* = 100 */, const int iStation /* = 1 */)
{
	int iCount = 1;
	int iSTAddr = 0;
	int iRet = -1;
	int *piValue = NULL;
	iSTAddr = atoi(pszdAdr);
	piValue = new int[iCount];

	if (m_pIModbus && m_pIModbus->IsConnect())
	{
		m_csMyLock.Lock();

		if (m_pIModbus->ReadHoldRegisterByByte(iSTAddr, piValue, iCount))
		{
			iDTVal = piValue[0];
			iRet = 1;
		}
		else
		{
			iRet = 0;
		}
		m_csMyLock.Unlock();

	}

	if (piValue)
	{
		delete[] piValue;
	}

	piValue = NULL;

	return iRet;
}

int CAM_Siemens::CheckMulitInPort(char *pszData, const int iSTAddr, const int iEDAddr, const int iStation /* = 1 */)
{
	int iCount = 0;
	int iRet = -1;
	int *piValue = NULL;
	iCount = iEDAddr - iSTAddr + 1;
	piValue = new int[iCount];

	if (m_pIModbus && m_pIModbus->IsConnect())
	{
		m_csMyLock.Lock();
		
		if (m_pIModbus->ReadDispersedByBit(iSTAddr, piValue, iCount))
		{
			for (int i = 0; i < iCount; ++i)
			{
				if (piValue[i])
				{
					pszData[i / 8] |= (1 << (i % 8));
				}
			}

			iRet = 1;
		}
		else
		{
			iRet = 0;
		}
		m_csMyLock.Unlock();

	}

	if (piValue)
	{
		delete[] piValue;
	}

	piValue = NULL;

	return iRet;
}

int CAM_Siemens::CheckMulitOutPort(char *pszData, const int iSTAddr, const int iEDAddr, const int iStation /* = 1 */)
{
	int iCount = 0;
	int iRet = -1;
	int *piValue = NULL;
	iCount = iEDAddr - iSTAddr + 1;
	piValue = new int[iCount];

	if (m_pIModbus && m_pIModbus->IsConnect())
	{
		m_csMyLock.Lock();

		if (m_pIModbus->ReadCoilByBit(iSTAddr, piValue, iCount))
		{
			for (int i = 0; i < iCount; ++i)
			{
				if (piValue[i])
				{
					pszData[i / 8] |= (1 << (i % 8));
				}
			}

			iRet = 1;
		}
		else
		{
			iRet = 0;
		}
		m_csMyLock.Unlock();

	}

	if (piValue)
	{
		delete[] piValue;
	}

	piValue = NULL;

	return iRet;
}

int CAM_Siemens::CheckMulitRelay(char *pszData, const int iSTAddr, const int iEDAddr, const int iStation /* = 1 */)
{
	int iCount = 0;
	int iRet = -1;
	int *piValue = NULL;
	iCount = iEDAddr - iSTAddr + 1;
	piValue = new int[iCount];

	if (m_pIModbus && m_pIModbus->IsConnect())
	{
		m_csMyLock.Lock();

		if (m_pIModbus->ReadHoldRegisterByByte(iSTAddr, piValue, iCount))
		{
			for (int i = 0; i < iCount; ++i)
			{
				pszData[i * 2 + 1] = (piValue[i] & 0x00FF);
				pszData[i * 2] = ((piValue[i] >> 8) & 0x00FF);
			}

			iRet = 1;
		}
		else
		{
			iRet = 0;
		}
		m_csMyLock.Unlock();

	}

	if (piValue)
	{
		delete[] piValue;
	}

	piValue = NULL;

	return iRet;
}