#pragma once

#ifdef AM_MODBUS_EXPORTS
#define AM_MODBUS_API __declspec(dllexport)
#else
#define AM_MODBUS_API __declspec(dllexport)
#endif

class CIModBus
{
public:
	CIModBus() {};
	virtual ~CIModBus() {};

	virtual BOOL Connect(const int iPortNo, const DWORD dwBaud, const int iDataBits, const int iPairty, const int iStopBits) = 0;

	virtual BOOL Connect(LPCTSTR lpstrIp, const int iPortNo) = 0;
	virtual void DisConnect() = 0;
	virtual BOOL IsConnect() = 0;

	virtual BOOL ReadCoilByBit(const int iAddr, int *piVal, const int iCount = 1) = 0;			 //读线圈 按位
	virtual BOOL ReadDispersedByBit(const int iAddr, int *piVal, const int iCount = 1) = 0;       //读离散 按位
	virtual BOOL ReadHoldRegisterByByte(const int iAddr, int *piVal, const int iCount = 1) = 0;	 //读保持寄存器 按字节
	virtual BOOL ReadInRegisterByByte(const int iAddr, int *piVal, const int iCount = 1) = 0;	 //读输入寄存器 按字节

	virtual BOOL WriteSingleCoilByByte(const int iAddr, const int iVal) = 0;	 //写单个线圈 按字节
	virtual BOOL WriteSingleRegisterByByte(const int iAddr, const int iVal) = 0; // 写单个寄存器 按字节
	virtual BOOL WriteMultiCoilByByte(const int iAddr, const int *piVal, const int iCount) = 0;	   //写多个线圈 按字节
	virtual BOOL WriteMultiRegisterByByte(const int iAddr, const int *piVal, const int iCount) = 0;  // 写多个寄存器 按字节
	virtual DWORD GetLastError() = 0;
	virtual void SetFollowID(const int iFollowID) = 0; //设置访问的知道 主从站 默认为0x01
};

extern "C"
{
	void AM_MODBUS_API AM_CreateModbus(CIModBus **ppIModbus, const int iType, std::mutex& m, CISerialPort* pISerialPort);
	void AM_MODBUS_API AM_ReleaseModbus(CIModBus *pIModbus);
};