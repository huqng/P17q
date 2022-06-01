// Fyd_ModBusTcp.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "ModBusTcp.h"
#define MODBUSBEGIN 0x001000

CModBusTcp::CModBusTcp()
{
	m_dwErrCode = 0;
	m_iFollowID = 0x01;
	m_pClient = NULL;

	AM_CreateClient(&m_pClient);
}

CModBusTcp::~CModBusTcp()
{
	if (m_pClient)
	{
		m_pClient->DisConnect();

		AM_ReleaseClient(m_pClient);
	}
}

BOOL CModBusTcp::Connect(const int iPortNo, const DWORD dwBaud, const int iDataBits, const int iPairty, const int iStopBits)
{
	return FALSE;
}

BOOL CModBusTcp::Connect(LPCTSTR lpstrIp, const int iPortNo)
{
	BOOL bRet = FALSE;
	if (m_pClient)
	{
		if (!m_pClient->ConnectServer(lpstrIp, iPortNo))
		{
			m_dwErrCode = m_pClient->GetLastError();
		}
		else
		{
			bRet = TRUE;
		}
	}
	else
	{
		m_dwErrCode = 1;
	}

	return bRet;
}

void CModBusTcp::DisConnect()
{
	if (m_pClient)
	{
		m_pClient->DisConnect();
	}
}

BOOL CModBusTcp::IsConnect()
{
	BOOL bRet = FALSE;
	
	if (m_pClient)
	{
		bRet = m_pClient->IsConnect();
	}
	else
	{
		m_dwErrCode = 1;
	}

	return bRet;
}

BOOL CModBusTcp::ReadCoilByBit(const int iAddr, int *piVal, const int iCount /* = 1 */)
{
	BOOL bRet = FALSE;
	m_csMyLock.Lock();
	if (m_pClient)
	{
		BYTE pbySend[1024] = { 0 };
		m_pClient->RecvData(pbySend, 1024);
		memset(pbySend, NULL, 1024);
		int iSendLength = 0;
		DWORD dwSTTime = 0;
		iSendLength = CreateReadSendData(0x01, iAddr, pbySend, iCount);

		if (m_pClient->SendData(pbySend, iSendLength))
		{
			memset(pbySend, NULL, 1024);
			dwSTTime = ::GetTickCount();
			while (::GetTickCount() - dwSTTime < 500)
			{
				if (m_pClient->RecvData(pbySend, 1024) > 0)
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
	m_csMyLock.Unlock();

	return bRet;
}

BOOL CModBusTcp::ReadDispersedByBit(const int iAddr, int *piVal, const int iCount /* = 1 */)
{
	BOOL bRet = FALSE;
	m_csMyLock.Lock();
	if (m_pClient)
	{
		BYTE pbySend[1024] = { 0 };
		m_pClient->RecvData(pbySend, 1024);
		memset(pbySend, NULL, 1024);
		int iSendLength = 0;
		DWORD dwSTTime = 0;
		iSendLength = CreateReadSendData(0x02, iAddr, pbySend, iCount);

		if (m_pClient->SendData(pbySend, iSendLength))
		{
			dwSTTime = ::GetTickCount();
			while (::GetTickCount() - dwSTTime < 500)
			{
				memset(pbySend, 0, 1024);

				if (m_pClient->RecvData(pbySend, 1024) > 0)
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
	m_csMyLock.Unlock();

	return bRet;
}

BOOL CModBusTcp::ReadHoldRegisterByByte(const int iAddr, int *piVal, const int iCount /* = 1 */)
{
	BOOL bRet = FALSE;
	m_csMyLock.Lock();
	if (m_pClient)
	{
		BYTE pbySend[1024] = { 0 };
		m_pClient->RecvData(pbySend, 1024);
		memset(pbySend, NULL, 1024);
		int iSendLength = 0;
		DWORD dwSTTime = 0;
		iSendLength = CreateReadSendData(0x03, iAddr, pbySend, iCount);

		if (m_pClient->SendData(pbySend, iSendLength))
		{
			memset(pbySend, NULL, 1024);
			dwSTTime = ::GetTickCount();
			while (::GetTickCount() - dwSTTime < 500)
			{
				ASleep(10);
				if (m_pClient->RecvData(pbySend, 1024) > 0)
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
	m_csMyLock.Unlock();

	return bRet;
}

BOOL CModBusTcp::ReadInRegisterByByte(const int iAddr, int *piVal, const int iCount /* = 1 */)
{
	BOOL bRet = FALSE;
	m_csMyLock.Lock();
	if (m_pClient)
	{
		BYTE pbySend[1024] = { 0 };
		m_pClient->RecvData(pbySend, 1024);
		memset(pbySend, NULL, 1024);
		int iSendLength = 0;
		DWORD dwSTTime = 0;
		iSendLength = CreateReadSendData(0x04, iAddr, pbySend, iCount);

		if (m_pClient->SendData(pbySend, iSendLength))
		{
			memset(pbySend, NULL, 1024);
			dwSTTime = ::GetTickCount();
			while (::GetTickCount() - dwSTTime < 500)
			{				
				if (m_pClient->RecvData(pbySend, 1024) > 0)
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
	m_csMyLock.Unlock();

	return bRet;
}

BOOL CModBusTcp::WriteSingleCoilByByte(const int iAddr, const int iVal)
{
	BOOL bRet = FALSE;
	m_csMyLock.Lock();
	if (m_pClient)
	{
		BYTE pbySend[1024] = { 0 };
		m_pClient->RecvData(pbySend, 1024);
		memset(pbySend, NULL, 1024);
		int iSendLength = 0;
		DWORD dwSTTime = 0;
		iSendLength = CreateWriteSendData(0x05, iAddr, pbySend, &iVal, 1);

		if (m_pClient->SendData(pbySend, iSendLength))
		{
			memset(pbySend, NULL, 1024);
			dwSTTime = ::GetTickCount();
			while (::GetTickCount() - dwSTTime < 500)
			{
				if (m_pClient->RecvData(pbySend, 1024) > 0)
				{
		
					bRet = TRUE;

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
	m_csMyLock.Unlock();

	return bRet;
}

BOOL CModBusTcp::WriteSingleRegisterByByte(const int iAddr, const int iVal)
{
	BOOL bRet = FALSE;
	m_csMyLock.Lock();
	if (m_pClient)
	{
		BYTE pbySend[1024] = { 0 };
		m_pClient->RecvData(pbySend, 1024);
		memset(pbySend, NULL, 1024);
		int iSendLength = 0;
		int iRecvLength = 0;
		DWORD dwSTTime = 0;
		iSendLength = CreateWriteSendData(0x06, iAddr, pbySend, &iVal, 1);

		if (m_pClient->SendData(pbySend, iSendLength))
		{
			memset(pbySend, NULL, 1024);
			dwSTTime = ::GetTickCount();
			while (::GetTickCount() - dwSTTime < 500)
			{
				int iLength = 0;
				iLength = m_pClient->RecvData(pbySend, 1024);

				if (iLength > 0)
				{
					iRecvLength += iLength;
					if (iRecvLength == iSendLength)
					{
						bRet = TRUE;

						break;
					}
					
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
	m_csMyLock.Unlock();

	return bRet;
}

BOOL CModBusTcp::WriteMultiCoilByByte(const int iAddr, const int *piVal, const int iCount)
{
	BOOL bRet = FALSE;
	m_csMyLock.Lock();
	if (m_pClient)
	{
		BYTE pbySend[1024] = { 0 };
		m_pClient->RecvData(pbySend, 1024);
		memset(pbySend, NULL, 1024);
		int iSendLength = 0;
		DWORD dwSTTime = 0;
		iSendLength = CreateWriteSendData(0x0F, iAddr, pbySend, piVal, iCount);

		if (m_pClient->SendData(pbySend, iSendLength))
		{
			memset(pbySend, NULL, 1024);
			dwSTTime = ::GetTickCount();
			while (::GetTickCount() - dwSTTime < 500)
			{
				if (m_pClient->RecvData(pbySend, 1024) > 0)
				{
					bRet = TRUE;
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
	m_csMyLock.Unlock();

	return bRet;
}

BOOL CModBusTcp::WriteMultiRegisterByByte(const int iAddr, const int *piVal, const int iCount)
{
	BOOL bRet = FALSE;
	m_csMyLock.Lock();
	if (m_pClient)
	{
		BYTE pbySend[1024] = { 0 };
		m_pClient->RecvData(pbySend, 1024);
		memset(pbySend, NULL, 1024);
		int iSendLength = 0;
		DWORD dwSTTime = 0;
		iSendLength = CreateWriteSendData(0x10, iAddr, pbySend, piVal, iCount);

		if (m_pClient->SendData(pbySend, iSendLength))
		{
			memset(pbySend, NULL, 1024);
			dwSTTime = ::GetTickCount();
			while (::GetTickCount() - dwSTTime < 500)
			{
				if (m_pClient->RecvData(pbySend, 1024) > 0)
				{
					bRet = TRUE;
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
	m_csMyLock.Unlock();

	return bRet;
}

DWORD CModBusTcp::GetLastError()
{
	return (MODBUSBEGIN + m_dwErrCode);
}

void CModBusTcp::SetFollowID(const int iFollowID)
{
	m_iFollowID = iFollowID;
}

int CModBusTcp::CreateReadSendData(const int iType, const int iAddr, BYTE *pbyRecvData, const int iCount)
{
	pbyRecvData[0] = 0x00;
	pbyRecvData[1] = 0x01;
	pbyRecvData[2] = 0x00;
	pbyRecvData[3] = 0x00;
	pbyRecvData[4] = 0x00;
	pbyRecvData[5] = 0x06;
	pbyRecvData[6] = m_iFollowID;
	pbyRecvData[7] = iType;
	pbyRecvData[8] = (iAddr >> 8) & 0xFF;
	pbyRecvData[9] = iAddr & 0xFF;
	pbyRecvData[10] = (iCount >> 8) & 0xFF;
	pbyRecvData[11] = iCount & 0xFF;
	return 12;
}

int CModBusTcp::CreateWriteSendData(const int iType, const int iAddr, BYTE *pbyRecvData, const int *piData, const int iCount)
{
	int		iRet = 0;
	pbyRecvData[0] = 0x00;
	pbyRecvData[1] = 0x01;
	pbyRecvData[2] = 0x00;
	pbyRecvData[3] = 0x00;
	pbyRecvData[6] = m_iFollowID;
	pbyRecvData[7] = iType;
	pbyRecvData[8] = (iAddr >> 8) & 0xFF;
	pbyRecvData[9] = iAddr & 0xFF;

	switch (iType)
	{
	case 0x05:
		{
			pbyRecvData[4] = 0x00;
			pbyRecvData[5] = 0x06;
			if (piData[0])
			{
				pbyRecvData[10] = 0xFF;
			}
			else
			{
				pbyRecvData[10] = 0x00;
			}
			pbyRecvData[11] = 0x00;
			iRet = 12;
		}
		break;

	case 0x06:
		{
			pbyRecvData[4] = 0x00;
			pbyRecvData[5] = 0x06;
			pbyRecvData[10] = (piData[0] >> 8) & 0xFF;
			pbyRecvData[11] = piData[0] & 0xFF;
			iRet = 12;
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

			pbyRecvData[4] = ((0x07 + iTemp) >> 8) & 0xFF;
			pbyRecvData[5] = (0x07 + iTemp) & 0xFF;
			pbyRecvData[10] = (iCount >> 8) & 0xFF;
			pbyRecvData[11] = iCount;
			pbyRecvData[12] = iTemp;
			memset((void *)&pbyRecvData[13], 0, iTemp);

			for (int iIndex = 0; iIndex < iCount; ++iIndex)
			{
				if (piData[iIndex] != 0)
				{
					pbyRecvData[13 + iIndex / 8] |= (1 << (iIndex % 8));
				}
			}

			iRet = 13 + iTemp;
		}
		break;

	case 0x10:
		{
			pbyRecvData[4] = ((0x07 + iCount * 2) >> 8) & 0xFF;
			pbyRecvData[5] = (0x07 + iCount * 2) & 0xFF;
			pbyRecvData[10] = (iCount >> 8) & 0xFF;
			pbyRecvData[11] = iCount;
			pbyRecvData[12] = iCount * 2;
			for (int iIndex = 0; iIndex < iCount; ++iIndex)
			{
				pbyRecvData[13 + iIndex * 2 + 0] = (piData[iIndex] >> 8) & 0xFF;
				pbyRecvData[13 + iIndex * 2 + 1] = piData[iIndex] & 0xFF;
			}

			iRet = 13 + iCount * 2;
		}
		break;
	}

	return iRet;
}

BOOL CModBusTcp::ParseRecvData(const int iType, const BYTE *pbyRecvData, int *piData, const int iCount /* = 1 */)
{
	BOOL bRet = FALSE;
	switch (iType)
	{
	case 0x01:
	case 0x02:
		{
			if (0x01 == pbyRecvData[7] || 0x02 == pbyRecvData[7])
			{
				for (int iIndex = 0; iIndex < iCount; ++iIndex)
				{
					piData[iIndex] = (pbyRecvData[9 + iIndex / 8] >> (iIndex % 8)) & 0x01;
				}

				bRet = TRUE;
			}
		}
		break;

	case 0x03:
	case 0x04:
		{
			if (0x03 == pbyRecvData[7] || 0x04 == pbyRecvData[7])
			{
				for (int iIndex = 0; iIndex < iCount; ++iIndex)
				{
					piData[iIndex] = (pbyRecvData[9 + iIndex * 2 + 0] << 8) + pbyRecvData[9 + iIndex * 2 + 1];
				}

				bRet = TRUE;
			 }
		}
		break;

	default:
		{
			if (0x05 == pbyRecvData[7] || 0x06 == pbyRecvData[7] ||
				0x0F == pbyRecvData[7] || 0x10 == pbyRecvData[7])
			{
				bRet = TRUE;
			}
		}
	}

	return bRet;
}

void CModBusTcp::ASleep(const int iSleepTime)
{
	timeBeginPeriod(1);
	Sleep(iSleepTime);
	timeEndPeriod(1);
}
