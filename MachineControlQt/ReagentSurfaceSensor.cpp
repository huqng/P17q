#include "stdafx.h"
#include "config.h"
#include "ReagentSurfaceSensor.h"

ReagentSurfaceSensor::ReagentSurfaceSensor() :
	m_iReadValue(0),
	m_iBase(0)
{
	this->Reconnect();
}

void ReagentSurfaceSensor::Reconnect()
{
	VCI_CloseDevice(3, 0);
	int iRet = VCI_OpenDevice(3, 0, 0);
	vic.AccCode = 0x00000000;
	vic.AccMask = 0xFFFFFFFF;
	vic.Filter = 1;
	vic.Timing0 = 0x03;
	vic.Timing1 = 0x1C;
	vic.Mode = 0;
	iRet = VCI_InitCAN(3, 0, 0, &vic);
	iRet = VCI_StartCAN(3, 0, 0);
	iRet = VCI_ClearBuffer(3, 0, 0);

}

int ReagentSurfaceSensor::Calibrate()
{
	//lock_guard<mutex> lk(m_mtx);
	//vcoCalib.ID = 0x605;
	//vcoCalib.DataLen = 8;
	//vcoCalib.Data[0] = 0x2B;
	//vcoCalib.Data[1] = 0x01;
	//vcoCalib.Data[2] = 0x60;
	//vcoCalib.Data[3] = 0x00;
	//vcoCalib.Data[4] = 0x01;
	//vcoCalib.Data[5] = 0x00;
	//vcoCalib.Data[6] = 0x00;
	//vcoCalib.Data[7] = 0x00;
	//vcoCalib.SendType = 0;
	//vcoCalib.ExternFlag = 0;
	//vcoCalib.RemoteFlag = 0;
	//int iRet = VCI_Transmit(3, 0, 0, &vcoCalib, 8);

	constexpr int BASE_SAMPLE_CNT = 100;
	constexpr int BASE_SAMPLE_INTERVAL = 20;

	int iSum = 0;
	int iCnt = 0;
	for (int i = 0; i < BASE_SAMPLE_CNT; i++)
	{
		if (ReadVal())
		{
			iSum += m_iReadValue;
			Sleep(BASE_SAMPLE_INTERVAL);
			iCnt++;
		}
	}


	if (iCnt == 0)
		return -1;
	else
	{
		m_iBase = iSum / iCnt;
		m_iReadValue = m_iBase;
		return 0;
	}
}

bool ReagentSurfaceSensor::SurfaceTouched()
{
	if (ReadVal())
	{
		return m_iReadValue - m_iBase > stoi(iniCfg[u8"加液"][u8"液面感应电容阈值"]);
	}
	else
	{
		return false;
	}
}

bool ReagentSurfaceSensor::ReadVal()
{
	lock_guard<mutex> lk(m_mtx);
	memset(&vcoRecv, 0, sizeof(vcoRecv));
	int iRet = VCI_ClearBuffer(3, 0, 0);
	Sleep(20);
	iRet = VCI_Receive(3, 0, 0, &vcoRecv, 1, 0);
	if (iRet == 1 && vcoRecv.ID == 0x185)
	{
		m_iReadValue = *(int*)&vcoRecv.Data[2];
		return true;
	}
	else
	{
		return false;
	}

}


int ReagentSurfaceSensor::GetLastReadValue()const
{
	return m_iReadValue - m_iBase;
}

int ReagentSurfaceSensor::GetBase()const
{
	return m_iBase;
}
