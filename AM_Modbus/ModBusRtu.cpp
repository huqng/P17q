#include "stdafx.h"
#include "ModbusRtu.h"
#define SLEEPTIME 50

CModbusRtu::CModbusRtu(mutex& m, CISerialPort* pISerialPort):
	m_mtx(m)
{
	m_iFollowID = 1;
	m_pISerialPort = pISerialPort;

	//AM_CreateSerialPort(&m_pISerialPort);
}

CModbusRtu::~CModbusRtu()
{
	//if (m_pISerialPort)
	//{
	//	m_pISerialPort->ClosePort();
	//	AM_ReleaseSerialPort(m_pISerialPort);
	//}
}

void CModbusRtu::SetFollowID(const int iFollowID)
{
	if (iFollowID > 0 && iFollowID < 256)
	{
		m_iFollowID = iFollowID;
	}
	else
	{
		m_iFollowID = 0x01;
	}
}

BOOL CModbusRtu::Connect(LPCTSTR lpstrIp, const int iPortNo)
{
	return FALSE;
}

BOOL CModbusRtu::Connect(const int iPortNo, const DWORD dwBaud, const int iDataBits, const int iPairty, const int iStopBits)
{
	BOOL bRet = FALSE;

	if (m_pISerialPort && !m_pISerialPort->IsPortOpen())
	{
		m_pISerialPort->Init(iPortNo, dwBaud, iDataBits, iPairty, iStopBits);
		bRet = m_pISerialPort->OpenPort();
	}
	else
	{
		bRet = m_pISerialPort->IsPortOpen();
	}

	return bRet;
}

BOOL CModbusRtu::IsConnect()
{
	BOOL bRet = FALSE;

	if (m_pISerialPort)
	{
		bRet = m_pISerialPort->IsPortOpen();
	}

	return bRet;
}

void CModbusRtu::DisConnect()
{
	m_pISerialPort->ClosePort();
}

BOOL CModbusRtu::ReadCoilByBit(const int iAddr, int *piVal, const int iCount /* = 1 */)
{
	BOOL bRet = FALSE;
	m_mtx.lock();
	if (m_pISerialPort)
	{
		BYTE pbySend[1024] = { 0 };
		m_pISerialPort->ClearReadBuf();
		m_pISerialPort->ClearWritebuf();
		memset(pbySend, NULL, 1024);
		int iSendLength = 0;
		DWORD dwRecvLength = 0;
		DWORD dwSTTime = 0;
		iSendLength = CreateReadSendData(0x01, iAddr, pbySend, iCount);

		if (m_pISerialPort->WriteComm((char *)pbySend, iSendLength))
		{
			memset(pbySend, NULL, 1024);
			dwSTTime = ::GetTickCount();
			dwRecvLength = 4 + (iCount / 8 + 1);
			while (::GetTickCount() - dwSTTime < 500)
			{
				Sleep(SLEEPTIME);
				if (m_pISerialPort->ReadComByNum((char *)pbySend, dwRecvLength,  &dwRecvLength, 100))
				{
					if (ParseRecvData(0x01, pbySend, piVal, iCount))
					{
						bRet = TRUE;
					}

					break;
				}

				ASleep(1);
			}

			if (!bRet)
			{
				m_dwErrCode = 2;//接收数据超时 或者数据返回有误
			}
		}
	}
	else
	{
		m_dwErrCode = 1; //为创建客户端对象
	}
	m_mtx.unlock();

	return bRet;
}

BOOL CModbusRtu::ReadDispersedByBit(const int iAddr, int *piVal, const int iCount /* = 1 */)
{
	BOOL bRet = FALSE;
	m_mtx.lock();
	if (m_pISerialPort)
	{
		BYTE pbySend[1024] = { 0 };
		m_pISerialPort->ClearReadBuf();
		m_pISerialPort->ClearWritebuf();
		memset(pbySend, NULL, 1024);
		int iSendLength = 0;
		DWORD dwRecvLength = 0;
		DWORD dwSTTime = 0;
		iSendLength = CreateReadSendData(0x02, iAddr, pbySend, iCount);

		if (m_pISerialPort->WriteComm((char *)pbySend, iSendLength))
		{
			memset(pbySend, NULL, 1024);
			dwSTTime = ::GetTickCount();
			dwRecvLength = 4 + (iCount / 8 + 1);
			while (::GetTickCount() - dwSTTime < 500)
			{
				Sleep(SLEEPTIME);
				if (m_pISerialPort->ReadComByNum((char *)pbySend, dwRecvLength,  &dwRecvLength, 100))
				{
					if (ParseRecvData(0x02, pbySend, piVal, iCount))
					{
						bRet = TRUE;
					}

					break;
				}

				ASleep(1);
			}

			if (!bRet)
			{
				m_dwErrCode = 2;//接收数据超时 或者数据返回有误
			}
		}
	}
	else
	{
		m_dwErrCode = 1; //为创建客户端对象
	}
	m_mtx.unlock();

	return bRet;
}

BOOL CModbusRtu::ReadHoldRegisterByByte(const int iAddr, int *piVal, const int iCount /* = 1 */)
{
	BOOL bRet = FALSE;
	m_mtx.lock();
	if (m_pISerialPort)
	{
		BYTE pbySend[1024] = { 0 };
		m_pISerialPort->ClearReadBuf();
		m_pISerialPort->ClearWritebuf();
		memset(pbySend, NULL, 1024);
		int iSendLength = 0;
		DWORD dwRecvLength = 0;
		DWORD dwSTTime = 0;
		iSendLength = CreateReadSendData(0x03, iAddr, pbySend, iCount);

		if (m_pISerialPort->WriteComm((char *)pbySend, iSendLength))
		{
			memset(pbySend, NULL, 1024);
			dwSTTime = ::GetTickCount();
			dwRecvLength = 4 + iCount * 2;
			while (::GetTickCount() - dwSTTime < 500)
			{
				Sleep(SLEEPTIME);
				if (m_pISerialPort->ReadComByNum((char *)pbySend, dwRecvLength,  &dwRecvLength, 100))
				{
					if (ParseRecvData(0x03, pbySend, piVal, iCount))
					{
						bRet = TRUE;
					}

					break;
				}

				ASleep(1);
			}

			if (!bRet)
			{
				m_dwErrCode = 2;//接收数据超时 或者数据返回有误
			}
		}
	}
	else
	{
		m_dwErrCode = 1; //为创建客户端对象
	}
	m_mtx.unlock();

	return bRet;
}

BOOL CModbusRtu::ReadInRegisterByByte(const int iAddr, int *piVal, const int iCount /* = 1 */)
{
	BOOL bRet = FALSE;
	m_mtx.lock();
	if (m_pISerialPort)
	{
		BYTE pbySend[1024] = { 0 };
		m_pISerialPort->ClearReadBuf();
		m_pISerialPort->ClearWritebuf();
		memset(pbySend, NULL, 1024);
		int iSendLength = 0;
		DWORD dwRecvLength = 0;
		DWORD dwSTTime = 0;
		iSendLength = CreateReadSendData(0x04, iAddr, pbySend, iCount);

		if (m_pISerialPort->WriteComm((char *)pbySend, iSendLength))
		{
			memset(pbySend, NULL, 1024);
			dwSTTime = ::GetTickCount();
			dwRecvLength = 4 + iCount * 2;
			while (::GetTickCount() - dwSTTime < 500)
			{
				Sleep(SLEEPTIME);
				if (m_pISerialPort->ReadComByNum((char *)pbySend, dwRecvLength,  &dwRecvLength, 100))
				{
					if (ParseRecvData(0x04, pbySend, piVal, iCount))
					{
						bRet = TRUE;
					}

					break;
				}

				ASleep(1);
			}

			if (!bRet)
			{
				m_dwErrCode = 2;//接收数据超时 或者数据返回有误
			}
		}
	}
	else
	{
		m_dwErrCode = 1; //为创建客户端对象
	}
	m_mtx.unlock();

	return bRet;
}

BOOL CModbusRtu::WriteSingleCoilByByte(const int iAddr, const int iVal)
{
	BOOL bRet = FALSE;
	m_mtx.lock();
	if (m_pISerialPort)
	{
		BYTE pbySend[1024] = { 0 };
		m_pISerialPort->ClearReadBuf();
		m_pISerialPort->ClearWritebuf();
		memset(pbySend, NULL, 1024);
		int iSendLength = 0;
		DWORD dwSTTime = 0;
		DWORD dwRecvLength = 0;
		iSendLength = CreateWriteSendData(0x05, iAddr, pbySend, &iVal, 1);

		if (m_pISerialPort->WriteComm((char *)pbySend, iSendLength))
		{
			memset(pbySend, NULL, 1024);
			dwSTTime = ::GetTickCount();
			dwRecvLength = 6;
			while (::GetTickCount() - dwSTTime < 500)
			{
				Sleep(SLEEPTIME);
				if (m_pISerialPort->ReadComByNum((char *)pbySend, dwRecvLength, &dwRecvLength, 100) > 0)
				{
					if (ParseRecvData(0x05, pbySend, NULL))
					{
						bRet = TRUE;
					}

					break;
				}

				ASleep(1);
			}

			if (!bRet)
			{
				m_dwErrCode = 2;//接收数据超时 或者数据返回有误
			}
		}
	}
	else
	{
		m_dwErrCode = 1; //为创建客户端对象
	}
	m_mtx.unlock();

	return bRet;
}

BOOL CModbusRtu::WriteSingleRegisterByByte(const int iAddr, const int iVal)
{
	BOOL bRet = FALSE;
	m_mtx.lock();
	if (m_pISerialPort)
	{
		BYTE pbySend[1024] = { 0 };
		m_pISerialPort->ClearReadBuf();
		m_pISerialPort->ClearWritebuf();
		memset(pbySend, NULL, 1024);
		int iSendLength = 0;
		DWORD dwSTTime = 0;
		DWORD dwRecvLength = 0;
		iSendLength = CreateWriteSendData(0x06, iAddr, pbySend, &iVal, 1);

		if (m_pISerialPort->WriteComm((char *)pbySend, iSendLength))
		{
			memset(pbySend, NULL, 1024);
			dwSTTime = ::GetTickCount();
			dwRecvLength = 6;
			while (::GetTickCount() - dwSTTime < 500)
			{
				Sleep(SLEEPTIME);
				if (m_pISerialPort->ReadComByNum((char *)pbySend, dwRecvLength, &dwRecvLength, 100) > 0)
				{

					if (ParseRecvData(0x06, pbySend, NULL))
					{
						bRet = TRUE;
					}

					break;
				}

				ASleep(1);
			}

			if (!bRet)
			{
				m_dwErrCode = 2;//接收数据超时 或者数据返回有误
			}
		}
	}
	else
	{
		m_dwErrCode = 1; //为创建客户端对象
	}
	m_mtx.unlock();

	return bRet;
}

BOOL CModbusRtu::WriteMultiCoilByByte(const int iAddr, const int *piVal, const int iCount)
{
	BOOL bRet = FALSE;
	m_mtx.lock();
	if (m_pISerialPort)
	{
		BYTE pbySend[1024] = { 0 };
		m_pISerialPort->ClearReadBuf();
		m_pISerialPort->ClearWritebuf();
		memset(pbySend, NULL, 1024);
		int iSendLength = 0;
		DWORD dwSTTime = 0;
		DWORD dwRecvLength = 0;
		iSendLength = CreateWriteSendData(0x0F, iAddr, pbySend, piVal, iCount);

		if (m_pISerialPort->WriteComm((char *)pbySend, iSendLength))
		{
			memset(pbySend, NULL, 1024);
			dwSTTime = ::GetTickCount();
			dwRecvLength = 6;
			while (::GetTickCount() - dwSTTime < 500)
			{
				Sleep(SLEEPTIME);
				if (m_pISerialPort->ReadComByNum((char *)pbySend, dwRecvLength, &dwRecvLength, 100) > 0)
				{

					if (ParseRecvData(0x0F, pbySend, NULL))
					{
						bRet = TRUE;
					}

					break;
				}

				ASleep(1);
			}

			if (!bRet)
			{
				m_dwErrCode = 2;//接收数据超时 或者数据返回有误
			}
		}
	}
	else
	{
		m_dwErrCode = 1; //为创建客户端对象
	}
	m_mtx.unlock();

	return bRet;
}

BOOL CModbusRtu::WriteMultiRegisterByByte(const int iAddr, const int *piVal, const int iCount)
{
	BOOL bRet = FALSE;
	m_mtx.lock();
	if (m_pISerialPort)
	{
		BYTE pbySend[1024] = { 0 };
		m_pISerialPort->ClearReadBuf();
		m_pISerialPort->ClearWritebuf();
		memset(pbySend, NULL, 1024);
		int iSendLength = 0;
		DWORD dwSTTime = 0;
		DWORD dwRecvLength = 0;
		iSendLength = CreateWriteSendData(0x10, iAddr, pbySend, piVal, iCount);

		if (m_pISerialPort->WriteComm((char *)pbySend, iSendLength))
		{
			memset(pbySend, NULL, 1024);
			dwSTTime = ::GetTickCount();
			dwRecvLength = 6;
			while (::GetTickCount() - dwSTTime < 500)
			{
				Sleep(SLEEPTIME);
				if (m_pISerialPort->ReadComByNum((char *)pbySend, dwRecvLength, &dwRecvLength, 100) > 0)
				{

					if (ParseRecvData(0x10, pbySend, NULL))
					{
						bRet = TRUE;
					}
	
					break;
				}

				ASleep(1);
			}

			if (!bRet)
			{
				m_dwErrCode = 2;//接收数据超时 或者数据返回有误
			}
		}
	}
	else
	{
		m_dwErrCode = 1; //为创建客户端对象
	}
	m_mtx.unlock();

	return bRet;
}

int CModbusRtu::CreateReadSendData(const int iType, const int iAddr, BYTE *pbyRecvData, const int iCount)
{
	pbyRecvData[0] = m_iFollowID;
	pbyRecvData[1] = iType;
	pbyRecvData[2] = (iAddr >> 8) & 0xFF;
	pbyRecvData[3] = iAddr & 0xFF;
	pbyRecvData[4] = (iCount >> 8) & 0xFF;
	pbyRecvData[5] = iCount & 0xFF;
	USHORT usCrc = CalcRtuCrc(pbyRecvData, 6);
	pbyRecvData[6] = usCrc & 0x00FF;
	pbyRecvData[7] = (usCrc & 0xFF00) >> 8;
	return 8;
}

int CModbusRtu::CreateWriteSendData(const int iType, const int iAddr, BYTE *pbyRecvData, const int *piData, const int iCount)
{
	int		iRet = 0;
	pbyRecvData[0] = m_iFollowID;
	pbyRecvData[1] = iType;
	pbyRecvData[2] = (iAddr >> 8) & 0xFF;
	pbyRecvData[3] = iAddr & 0xFF;

	switch (iType)
	{
	case 0x05:
		{
			if (piData[0])
			{
				pbyRecvData[4] = 0xFF;
			}
			else
			{
				pbyRecvData[4] = 0x00;
			}
			pbyRecvData[5] = 0x00;
			USHORT usCrc = CalcRtuCrc(pbyRecvData, 6);
			pbyRecvData[6] = usCrc & 0x00FF;
			pbyRecvData[7] = (usCrc & 0xFF00) >> 8;
			iRet = 8;
		}
		break;

	case 0x06:
		{
			pbyRecvData[4] = (piData[0] >> 8) & 0xFF;
			pbyRecvData[5] = piData[0] & 0xFF;
			USHORT usCrc = CalcRtuCrc(pbyRecvData, 6);
			pbyRecvData[6] = usCrc & 0x00FF;
			pbyRecvData[7] = (usCrc & 0xFF00) >> 8;
			iRet = 8;
		}
		break;

	case 0x0F:
		{
			int iTemp = 0;
			iTemp = iCount / 8;

			if (iCount % 8 > 0)   //如果长度不为8的整数倍 需要增加一个字节
			{
				iTemp += 1;
			}
		
			pbyRecvData[4] = (iCount >> 8) & 0xFF;
			pbyRecvData[5] = iCount;
			pbyRecvData[6] = iTemp;

			memset((void *)&pbyRecvData[7], 0, iTemp);

			for (int iIndex = 0; iIndex < iCount; ++iIndex)
			{
				if (piData[iIndex] != 0)
				{
					pbyRecvData[7 + iIndex / 8] |= (1 << (iIndex % 8));
				}
			}

			iRet = 7 + iTemp;
			USHORT usCrc = CalcRtuCrc(pbyRecvData, iRet);
			pbyRecvData[iRet] = usCrc & 0x00FF;
			pbyRecvData[iRet + 1] = (usCrc & 0xFF00) >> 8;
			iRet += 2;
		}
		break;

	case 0x10:
		{
			pbyRecvData[4] = (iCount >> 8) & 0xFF;
			pbyRecvData[5] = iCount;
			pbyRecvData[6] = iCount * 2;
			for (int iIndex = 0; iIndex < iCount; ++iIndex)
			{
				pbyRecvData[7 + iIndex * 2 + 0] = (piData[iIndex] >> 8) & 0xFF;
				pbyRecvData[7 + iIndex * 2 + 1] = piData[iIndex] & 0xFF;
			}

			iRet = 7 + iCount * 2;
			USHORT usCrc = CalcRtuCrc(pbyRecvData, iRet);
			pbyRecvData[iRet] = usCrc & 0x00FF;
			pbyRecvData[iRet + 1] = (usCrc & 0xFF00) >> 8;
			iRet += 2;
		}
		break;
	}

	return iRet;
}

BOOL CModbusRtu::ParseRecvData(const int iType, const BYTE *pbyRecvData, int *piData, const int iCount /* = 1 */)
{
	BOOL bRet = FALSE;
	switch (iType)
	{
	case 0x01:
	case 0x02:
		{
			if (0x01 == pbyRecvData[1] || 0x02 == pbyRecvData[1])
			{
				for (int iIndex = 0; iIndex < iCount; ++iIndex)
				{
					piData[iIndex] = (pbyRecvData[3 + iIndex / 8] >> (iIndex % 8)) & 0x01;
				}

				bRet = TRUE;
			}
		}
		break;

	case 0x03:
	case 0x04:
		{
			if (0x03 == pbyRecvData[1] || 0x04 == pbyRecvData[1])
			{
				for (int iIndex = 0; iIndex < iCount; ++iIndex)
				{
					piData[iIndex] = (pbyRecvData[3 + iIndex * 2 + 0] << 8) + pbyRecvData[3 + iIndex * 2 + 1];
				}

				bRet = TRUE;
			}
		}
		break;

	default:
		{
			if (0x05 == pbyRecvData[1] || 0x06 == pbyRecvData[1] ||
				0x0F == pbyRecvData[1] || 0x10 == pbyRecvData[1])
			{
				bRet = TRUE;
			}
		}
	}

	return bRet;
}

USHORT CModbusRtu::CalcRtuCrc(const BYTE *pbyData, const int iLength)
{
	USHORT usCrcValue = 0xFFFF;
	int i = 0;
	int j = 0;

	if (iLength > 78)
	{
		return usCrcValue;
	}

	while (j < iLength)
	{
		usCrcValue ^= pbyData[j];

		for (i = 0; i < 8; ++i)
		{
			if (usCrcValue & 0x0001)
			{
				usCrcValue >>= 1;
				usCrcValue ^= 0xA001;
			}
			else
			{
				usCrcValue >>= 1;
			}
		}

		++j;
	}

	return usCrcValue;
}

DWORD CModbusRtu::GetLastError()
{
	return m_dwErrCode;
}

void CModbusRtu::ASleep(const int iSleepTime)
{
	timeBeginPeriod(1);
	Sleep(iSleepTime);
	timeEndPeriod(1);
}
