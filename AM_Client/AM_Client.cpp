// AM_Client.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "CAM_Client.h"


CAM_Client::CAM_Client()
{
	m_pParam = NULL;
	m_pFun = NULL;
	m_bConnect = FALSE;
	m_dwErrorCode = 0;
	m_hClient = NULL;
	m_iPortNum = 6666;
	m_wsaEvent = WSA_INVALID_EVENT;
}

CAM_Client::~CAM_Client()
{
	m_pFun = NULL;
	Stop();
	m_pParam = NULL;
}

BOOL CAM_Client::ConnectServer(LPCTSTR lpstrIP, const int iPort/* = 6666*/)
{
// 	m_csClient.Lock();
// 	if (!m_bConnect)
// 	{
// 		m_iPortNum = iPort;
// 		BOOL bOpt = TRUE;
// 		int iOptLen = sizeof(BOOL);
// 		m_strIp = lpstrIP;
// 		m_iSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
// 
// 		if (INVALID_SOCKET == m_iSocket)
// 		{
// 			m_csClient.Unlock();
// 			return m_bConnect;
// 		}
// 
// 		sockaddr_in name;
// 		memset(&name,0,sizeof(sockaddr_in));
// 		name.sin_family = AF_INET;
// 		name.sin_port = htons((unsigned short)m_iPortNum);
// 		if (lpstrIP)
// 		{
// 			USES_CONVERSION;
// 			name.sin_addr.S_un.S_addr = ::inet_addr(T2A(lpstrIP));
// 		}
// 		else
// 		{
// 			name.sin_addr.S_un.S_addr = ::inet_addr("127.0.0.1");
// 		}
// 
// 		int iRet = connect((SOCKET)m_iSocket,(sockaddr*)&name,sizeof(sockaddr_in));
// 
// 
// 		if(iRet == SOCKET_ERROR)
// 		{
// 			m_csClient.Unlock();
// 			return m_bConnect;
// 		}
// 
// 		unsigned long optval2 = 1;
// 		ioctlsocket((SOCKET)m_iSocket,FIONBIO,&optval2);
// 
// 		tcp_keepalive	tcplive;
// 		tcp_keepalive	tcpliveout;
// 		DWORD			dwVal = 0;
// 		int				iSendLength = 0;		
// 		tcplive.onoff = TRUE;
// 		tcplive.keepalivetime = 3000;
// 		tcplive.keepaliveinterval = 500;
// 
// 		iRet = ::setsockopt(m_iSocket, IPPROTO_TCP, TCP_NODELAY, (char *)&bOpt, sizeof(BOOL));
// 		iRet = ::setsockopt(m_iSocket, SOL_SOCKET, SO_KEEPALIVE, (char *)&bOpt, sizeof(BOOL));
// 		iRet = ::WSAIoctl(m_iSocket, SIO_KEEPALIVE_VALS, &tcplive, sizeof(tcplive), &tcpliveout, sizeof(tcpliveout), &dwVal, NULL, NULL);
// 		iRet = ::setsockopt(m_iSocket, SOL_SOCKET, SO_SNDBUF, (char *)&iSendLength, sizeof(int));
// 
// 		iSendLength = 1000;
// 		iRet = ::setsockopt(m_iSocket, SOL_SOCKET, SO_SNDTIMEO, (char *)&iSendLength, sizeof(int));
// 
// 		if (m_pFun)
// 		{
// 			unsigned	 tid;
// 			m_hClient = (HANDLE)_beginthreadex(NULL, 0, Thread_C_RecvMsg, this, 0, &tid);
// 			if (m_hClient)
// 			{
// 				m_bConnect = TRUE;
// 				m_csClient.Unlock();
// 				OnReceiveData(AM_TCP_EVENT_ONLINE, NULL, 0);
// 				return m_bConnect;
// 			}
// 			else
// 			{
// 				::shutdown(m_iSocket, SD_BOTH);
// 				::closesocket(m_iSocket);
// 				m_csClient.Unlock();
// 				return m_bConnect;
// 			}
// 		}
// 		else
// 		{
// 			m_bConnect = TRUE;
// 			m_csClient.Unlock();
// 			return m_bConnect;
// 		}
// 	}
// 	m_csClient.Unlock();
// 	return m_bConnect;


	bool bSuccess = false;
	m_csClient.Lock();

	if (!m_bConnect)
	{
		struct sockaddr_in saddr;
		bSuccess = true;
		if (InetPton(AF_INET, lpstrIP, &saddr.sin_addr) != 1)
		{
			bSuccess = false;
		}

		saddr.sin_port = htons(iPort);
		saddr.sin_family = AF_INET;

		if (bSuccess)
		{
			if ((m_iSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
				bSuccess = false;
		}

		if (bSuccess)
		{
			// Put socket into non-blocking mode
			unsigned long mode = 1;
			if (ioctlsocket(m_iSocket, FIONBIO, &mode) != NO_ERROR)
				bSuccess = false;
		}

		if (bSuccess)
		{
			BOOL bTrue = TRUE;

			// Disable Nagle algorithm
			if (setsockopt(m_iSocket, IPPROTO_TCP, TCP_NODELAY, (const char *)&bTrue, sizeof(BOOL)) != NO_ERROR)
				bSuccess = false;
		}

		if (bSuccess)
		{
			if (::connect(m_iSocket, (struct sockaddr *)&saddr, sizeof(saddr)) == SOCKET_ERROR)
			{
				if (WSAGetLastError() == WSAEWOULDBLOCK)
				{
					fd_set writeFd, errFd;
					timeval tv;

					tv.tv_sec = 3;
					tv.tv_usec = 0;

					FD_ZERO(&writeFd);
					FD_ZERO(&errFd);
					FD_SET(m_iSocket, &writeFd);
					FD_SET(m_iSocket, &errFd);

					// Check if a connection gets established within specified timeout
					select(0, NULL, &writeFd, &errFd, &tv);

					if (!FD_ISSET(m_iSocket, &writeFd))
					{
						// Socket not writable means connection failed
						bSuccess = false;
					}
				}
				else
				{
					bSuccess = false;
				}
			}
		}

		if (bSuccess)
		{
			// Put socket back into blocking mode
			unsigned long mode = 0;
			if (ioctlsocket(m_iSocket, FIONBIO, &mode) != NO_ERROR)
				bSuccess = false;
		}

		if (bSuccess)
		{
			// Create read event
			m_wsaEvent = WSACreateEvent();

			if (m_wsaEvent != WSA_INVALID_EVENT)
			{
				if (WSAEventSelect(m_iSocket, m_wsaEvent, FD_READ | FD_CLOSE) == SOCKET_ERROR)
					bSuccess = false;
			}
			else
			{
				bSuccess = false;
			}
		}

		if (bSuccess)
		{
			// Connection established
			m_bConnect = true;
		}
		else
		{
			DisConnect();
		}
	}
	m_csClient.Unlock();
	return m_bConnect;
}

BOOL CAM_Client::SetSendTimeOut(const int iTimeOut)
{
	int		iRet = 0;
	iRet = ::setsockopt(m_iSocket, SOL_SOCKET, SO_SNDTIMEO, (char *)&iTimeOut, sizeof(int));
	if (SOCKET_ERROR == iRet)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

BOOL CAM_Client::ConnectServer()
{
	m_csClient.Lock();


	m_iSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (INVALID_SOCKET == m_iSocket)
	{
		m_csClient.Unlock();
		return FALSE;
	}

	sockaddr_in name;
	memset(&name,0,sizeof(sockaddr_in));
	name.sin_family = AF_INET;
	name.sin_port = htons((unsigned short)m_iPortNum);

	USES_CONVERSION;
	name.sin_addr.S_un.S_addr = ::inet_addr(T2A(const_cast<LPTSTR>(m_strIp.c_str())));

	int iRet = ::connect((SOCKET)m_iSocket,(sockaddr*)&name,sizeof(sockaddr_in));

	if(iRet == SOCKET_ERROR)
	{
		::shutdown(m_iSocket, SD_BOTH);
		::closesocket(m_iSocket);
		m_csClient.Unlock();
		return FALSE;
	}

	unsigned long optval2 = 1;
	ioctlsocket((SOCKET)m_iSocket, FIONBIO,&optval2);

	BOOL bOpt = TRUE;
	int iOptLen = sizeof(BOOL);
	tcp_keepalive	tcplive;
	tcp_keepalive	tcpliveout;
	DWORD			dwVal = 0;
	int				iSendLength = 0;
	tcplive.onoff = TRUE;
	tcplive.keepalivetime = 3000;
	tcplive.keepaliveinterval = 500;
	iRet = ::setsockopt(m_iSocket, IPPROTO_TCP, TCP_NODELAY, (char *)&bOpt, sizeof(BOOL));
	iRet = ::setsockopt(m_iSocket, SOL_SOCKET, SO_KEEPALIVE, (char *)&bOpt, sizeof(BOOL));
	iRet = ::WSAIoctl(m_iSocket, SIO_KEEPALIVE_VALS, &tcplive, sizeof(tcplive), &tcpliveout, sizeof(tcpliveout), &dwVal, NULL, NULL);
	iRet = ::setsockopt(m_iSocket, SOL_SOCKET, SO_SNDBUF, (char *)&iSendLength, sizeof(int));
	iSendLength = 1000;
	iRet = ::setsockopt(m_iSocket, SOL_SOCKET, SO_SNDTIMEO, (char *)&iSendLength, sizeof(int));

	OnReceiveData(AM_TCP_EVENT_ONLINE, NULL, 0);
	m_csClient.Unlock();
	return TRUE;
}

void CAM_Client::SetReceiveProc(AM_OnReceiveData fun, void *pParam)
{
	m_pFun = fun;
	m_pParam = pParam;
}

void CAM_Client::Stop()
{
	if (m_bConnect)
	{
		m_bConnect = FALSE;
		::shutdown(m_iSocket, SD_BOTH);
		::closesocket(m_iSocket);
		::WaitForSingleObject(m_hClient, INFINITE);		
	}

	if (m_hClient)
	{
		::CloseHandle(m_hClient);
		m_hClient = NULL;
	}
}

BOOL CAM_Client::IsConnect()
{
	return m_bConnect;
}

void CAM_Client::DisConnect()
{

	if (m_wsaEvent != WSA_INVALID_EVENT)
	{
		WSACloseEvent(m_wsaEvent);
		m_wsaEvent = WSA_INVALID_EVENT;
	}

	if (m_iSocket != INVALID_SOCKET)
	{
		if (m_bConnect)
		{
			shutdown(m_iSocket, SD_BOTH);
		}

		closesocket(m_iSocket);
		m_iSocket = INVALID_SOCKET;
	}

	m_bConnect = false;

	::WaitForSingleObject(m_hClient, INFINITE);	

	if (m_hClient)
	{
		::CloseHandle(m_hClient);
		m_hClient = NULL;
	}
}

DWORD CAM_Client::GetLastError()
{
	return m_dwErrorCode;
}

LPCTSTR CAM_Client::GetServerIP()
{
	return m_strIp.c_str();
}

BOOL CAM_Client::SendData(BYTE* pbyData, DWORD dwSize)
{
	int			iIndex = 0;
	BOOL		bRet = FALSE;
	int			iSocket = 0;

	if (m_bConnect)
	{
		int iOffset = 0;
		int iWrite;
		DWORD dwLeft = dwSize;
		int iLoop = 0;
		int iTotal = 0;
		int iNewTimeout = 10 * 1000;

		while((iLoop <= iNewTimeout) && (dwLeft > 0))
		{
			iWrite = ::send((SOCKET)m_iSocket, (char *)(pbyData + iOffset), dwLeft, 0);
			if(iWrite == 0)
			{
				bRet = FALSE;
				OnReceiveData(AM_TCP_EVENT_OFFLINE, pbyData, dwSize);
				DisConnect();
				break;
			}

			if(iWrite == SOCKET_ERROR)
			{
				if(::WSAGetLastError() != WSAEWOULDBLOCK)
				{
					bRet = FALSE;
					OnReceiveData(AM_TCP_EVENT_OFFLINE, pbyData, dwSize);
					DisConnect();
					break;
				}
			}

			if((iWrite < 0) || (iWrite == (int)dwLeft))
			{
				bRet = TRUE;
				break;				
			}

			iOffset += iWrite;
			dwLeft -= iWrite;
			iTotal += iWrite;
			if(dwLeft > 0)
			{
				Sleep(100);
			}

			iLoop++;
		}
	}
	

	return bRet;
}


// bool BasicOpClient::send(const uint8_t * const pBuf, size_t len)
// {
// 	assert(pBuf);
// 
// 	if (m_isConnected)
// 	{
// 		for (size_t totalSent = 0; totalSent < len;)
// 		{
// 			int res = ::send(m_socket, (const char *)(&pBuf[totalSent]), len - totalSent, 0);
// 
// 			if (res < 0)
// 			{
// 				disconnect();
// 				return false;
// 			}
// 
// 			totalSent += res;
// 		}
// 
// 		return true;
// 	}
// 
// 	return false;
// }

void CAM_Client::OnReceiveData(DWORD dwEventType, BYTE* pReceiveData, DWORD dwReceiveSize)
{
	if (m_pFun)
	{
		m_pFun(m_pParam, dwEventType, m_strIp.c_str(), pReceiveData, dwReceiveSize);
	}
}

int CAM_Client::RecvData()
{
	int			iLength = 0;
	int			iRet = -1;
	char		szMsg[512] = {0};
	iLength = ::recv(m_iSocket, szMsg, 512, 0);

	if (0 == iLength)
	{
		iRet = -1;
		OnReceiveData(AM_TCP_EVENT_OFFLINE, (BYTE *)szMsg, iLength);
		DisConnect();
	}
	else if (iLength < 0)
	{
		int iLocalError = WSAGetLastError();
		if(iLocalError == WSAEWOULDBLOCK)
		{
			iRet = 0;
		}
		else
		{
			iRet = -1;
			OnReceiveData(AM_TCP_EVENT_OFFLINE, (BYTE *)szMsg, iLength);
			DisConnect();
		}
	}
	else
	{
		OnReceiveData(AM_TCP_EVENT_DATA, (BYTE *)szMsg, iLength);
		iRet = 1;
	}

	return iRet;
}

int CAM_Client::RecvData(BYTE *pbyData, DWORD dwSize)
{
	if (m_pFun)
	{
		return 0;
	}

// 	int		iRet = 0;
// 
// 	iRet = ::recv(m_iSocket, (char *)pbyData, dwSize, 0);
// 
// 	if (iRet == 0)
// 	{
// 		return -1;
// 	}
// 	else if (iRet == -1)
// 	{
// 		return 0;
// 	}
// 	else
// 	{
// 		return iRet;
// 	}

	assert(pbyData);

	WSANETWORKEVENTS networkEvents;
	DWORD nRes = WSAWaitForMultipleEvents(1, &m_wsaEvent, FALSE, 0, FALSE);
	int result = 0;

	if (nRes == WSA_WAIT_EVENT_0)
	{
		if (WSAEnumNetworkEvents(m_iSocket, m_wsaEvent, &networkEvents) == 0)
		{
			if ((networkEvents.lNetworkEvents & FD_READ) != 0)
			{
				int received = ::recv(m_iSocket, (char *)pbyData, dwSize, 0);

				if (received > 0)
				{
					return received;
				}
				// Error / connection lost
				DisConnect();
				return -1;
			}

			if ((networkEvents.lNetworkEvents & FD_CLOSE) != 0)
			{
				DisConnect();
				return -1;
			}
		}
		else
		{
			DisConnect();
			return -1;
		}
	}

	// No data before timeout
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//						Thead		Begin								//
//////////////////////////////////////////////////////////////////////////

unsigned int __stdcall Thread_C_RecvMsg(void *pParam)
{
	CAM_Client *pClient = (CAM_Client *)pParam;
	int		iRet = 0;

	while (TRUE)
	{
		iRet = pClient->RecvData();

		if (-1 == iRet)
		{
			break;
		}

		Sleep(100);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////
//						Thead		Begin								//
//////////////////////////////////////////////////////////////////////////

void AM_CreateClient(CIClient** ppClient)
{
	if (*ppClient)
	{
		delete *ppClient;
	}

	*ppClient = NULL;

	*ppClient = new CAM_Client();
}

void AM_ReleaseClient(CIClient* pClient)
{
	if (pClient != NULL)
	{
		delete pClient;
	}
}
