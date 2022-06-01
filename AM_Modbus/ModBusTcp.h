#pragma once
#include "AM_IModBus.h"


class CModBusTcp : public CIModBus
{
public:
	CModBusTcp();
	virtual ~CModBusTcp();
	virtual BOOL Connect(const int iPortNo, const DWORD dwBaud, const int iDataBits, const int iPairty, const int iStopBits);
	virtual BOOL Connect(LPCTSTR lpstrIp, const int iPortNo);
	virtual void DisConnect();
	virtual BOOL IsConnect();

	virtual BOOL ReadCoilByBit(const int iAddr, int *piVal, const int iCount = 1);			 //读线圈 按位
	virtual BOOL ReadDispersedByBit(const int iAddr, int *piVal, const int iCount = 1);       //读离散 按位
	virtual BOOL ReadHoldRegisterByByte(const int iAddr, int *piVal, const int iCount = 1);	 //读保持寄存器 按字节
	virtual BOOL ReadInRegisterByByte(const int iAddr, int *piVal, const int iCount = 1);	 //读输入寄存器 按字节

	virtual BOOL WriteSingleCoilByByte(const int iAddr, const int iVal);	 //写单个线圈 按字节
	virtual BOOL WriteSingleRegisterByByte(const int iAddr, const int iVal); // 写单个寄存器 按字节
	virtual BOOL WriteMultiCoilByByte(const int iAddr, const int *piVal, const int iCount);	   //写多个线圈 按字节
	virtual BOOL WriteMultiRegisterByByte(const int iAddr, const int *piVal, const int iCount);  // 写多个寄存器 按字节
	virtual DWORD GetLastError();
	virtual void SetFollowID(const int iFollowID); //设置访问的知道 主从站 默认为0x01

private:
	int m_iFollowID;
	CIClient *m_pClient;
	DWORD m_dwErrCode;
	CMyLock m_csMyLock;
	void ASleep(const int iSleepTime);
	int CreateReadSendData(const int iType, const int iAddr, BYTE *pbyRecvData, const int iCount);
	int CreateWriteSendData(const int iType, const int iAddr, BYTE *pbyRecvData, const int *piData, const int iCount);
	BOOL ParseRecvData(const int iType, const BYTE *pbyRecvData, int *piData, const int iCount = 1);
};