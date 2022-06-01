#pragma once

class CMyLock
{
public:
	CMyLock()
	{
		InitializeCriticalSection(&m_csLock);
	}

	~CMyLock()
	{
		DeleteCriticalSection(&m_csLock);
	}

	void Lock()
	{
		EnterCriticalSection(&m_csLock);
	}

	void Unlock()
	{
		LeaveCriticalSection(&m_csLock);
	}

private:
	CRITICAL_SECTION m_csLock;
};

