#include "stdafx.h"

#include "config.h"
#include "FormulaMgr.h"
#include "Modules.h"

bool PLCReadAndClearRelay(CIPLCCtrl* pPLC, const string& strSlave, const string& strSection, const string& strKey)
{
	const int iSlaveID = stoi(iniIO["Station"][strSlave]);
	string strAddr = iniIO[strSection][strKey];
	const int iWordAddr = strAddr.size() > 1 ? stoi(strAddr.substr(0, strAddr.size() - 1)) : 0;
	string strBitPosHex = strAddr.substr(strAddr.size() - 1, 1);

	int iRet1 = pPLC->CheckRelay(iWordAddr, strBitPosHex.c_str(), iSlaveID);
	if (iRet1 == 1)
	{
		int iRet2 = pPLC->ClearRelay(iWordAddr, strBitPosHex.c_str(), iSlaveID);
		return iRet2 == 1;
	}
	else
	{
		return false;
	}
}

bool PLCReadRelay(CIPLCCtrl* pPLC, const string& strSlave, const string& strSection, const string& strKey)
{
	const int iSlaveID = stoi(iniIO["Station"][strSlave]);
	const string& strAddr = iniIO[strSection][strKey];
	const int iWordAddr = strAddr.size() > 1 ? stoi(strAddr.substr(0, strAddr.size() - 1)) : 0;
	string strBitPosHex = strAddr.substr(strAddr.size() - 1, 1);

	int iRet = pPLC->CheckRelay(iWordAddr, strBitPosHex.c_str(), iSlaveID);
	if (iRet == 1)
		return true;
	else
		return false;
}

int PLCClearRelay(CIPLCCtrl* pPLC, const string& strSlave, const string& strSection, const string& strKey)
{
	const int iSlaveID = stoi(iniIO["Station"][strSlave]);
	const string& strAddr = iniIO[strSection][strKey];
	const int iWordAddr = strAddr.size() > 1 ? stoi(strAddr.substr(0, strAddr.size() - 1)) : 0;
	string strBitPosHex = strAddr.substr(strAddr.size() - 1, 1);

	int iRet = pPLC->ClearRelay(iWordAddr, strBitPosHex.c_str(), iSlaveID);
	if (iRet == 1)
		return 0;
	else
		return -1;
}

int PLCWriteRelay(CIPLCCtrl* pPLC, const string& strSlave, const string& strSection, const string& strKey)
{
	const int iSlaveID = stoi(iniIO["Station"][strSlave]);
	const string& strAddr = iniIO[strSection][strKey];
	const int iWordAddr = strAddr.size() > 1 ? stoi(strAddr.substr(0, strAddr.size() - 1)) : 0;
	string strBitPosHex = strAddr.substr(strAddr.size() - 1, 1);

	int iRet = pPLC->SetRelay(iWordAddr, strBitPosHex.c_str(), iSlaveID);
	if (iRet == 1)
		return 0;
	else
		return -1;
}

int PLCWriteDT(CIPLCCtrl* pPLC, int iValue, const string& strSlave, const string& strSection, const string& strKey)
{
	const int iSlaveID = stoi(iniIO["Station"][strSlave]);
	string strAddr = iniIO[strSection][strKey];

	int iRet = pPLC->WriteDTVal(iValue, strAddr.c_str(), 100, iSlaveID);
	return (iRet == 1) ? 0 : -1;
}

int PLCWrite2DTs(CIPLCCtrl* pPLC, int iValue, const string& strSlave, const string& strSection, const string& strKey)
{
	const int iSlaveID = stoi(iniIO["Station"][strSlave]);
	string strAddr1 = iniIO[strSection][strKey];
	string strAddr2 = to_string(stoi(iniIO[strSection][strKey]) + 1);
	if (strAddr2.size() < 5)
	{
		strAddr2.insert(strAddr2.begin(), 5 - strAddr2.size(), '0');
	}

	int iRet1 = pPLC->WriteDTVal(iValue & 0xFFFF, strAddr1.c_str(), 1000, iSlaveID);
	int iRet2 = pPLC->WriteDTVal((iValue >> 16) & 0xFFFF, strAddr2.c_str(), 1000, iSlaveID);
	return (iRet1 == 1 && iRet1 == iRet2) ? 0 : -1;
}

int PLCReadDT(CIPLCCtrl* pPLC, const string& strSlave, const string& strSection, const string& strKey)
{
	const int iSlaveID = stoi(iniIO["Station"][strSlave]);
	const string& strAddr = iniIO[strSection][strKey];

	int iRet = -1;
	int iVal = -12345678;
	iRet = pPLC->ReadDTVal(iVal, strAddr.c_str(), 100, iSlaveID);
	if (iRet == 1)
	{
		return iVal;
	}
	else
	{
		return 0;
	}
}

int PLCRead2DTs(CIPLCCtrl* pPLC, const string& strSlave, const string& strSection, const string& strKey)
{
	const int iSlaveID = stoi(iniIO["Station"][strSlave]);
	const string& strAddr1 = iniIO[strSection][strKey];
	string strAddr2 = to_string(stoi(iniIO[strSection][strKey]) + 1);
	if (strAddr2.size() < 5)
	{
		strAddr2.insert(strAddr2.begin(), 5 - strAddr2.size(), '0');
	}

	int iRet1 = -1;
	int iRet2 = -1;
	int iValL16 = -12345678;
	int iValH16 = -12345678;
	iRet1 = pPLC->ReadDTVal(iValL16, strAddr1.c_str(), 100, iSlaveID);
	iRet2 = pPLC->ReadDTVal(iValH16, strAddr2.c_str(), 100, iSlaveID);

	if (iRet1 == 1 && iRet1 == iRet2)
	{
		return (iValH16 << 16) | (iValL16 & 0xFFFF);
	}
	else
	{
		return 0;
	}
}



/* slide shelf */

SlideShelf::SlideShelf() :
	m_iCurrentStep(0)
{

}

bool SlideShelf::HasSlideAt(int iIndex)const
{
	return m_mapSlides.find(iIndex) != m_mapSlides.end();
}

void SlideShelf::AddSlide(const string& strID, const string& strHospital, const map<int, string>&& mapVReagent)
{
	m_mapSlides[1 + m_mapSlides.size()] = { strID, strHospital, move(mapVReagent) };
}

int SlideShelf::TotalStep()const
{
	return formulaInfo.GetFormulaStepCnt(this->m_strFormula);
}

/* reaction module */

ReactionModule::ReactionModule(int iIndex, mutex* pMtx, CISerialPort* pSerial) :
	m_iThisModIndex(iIndex),
	m_pShelf(nullptr),
	m_pSerial(pSerial),
	m_pModbus(nullptr),
	m_pPLC(nullptr),
	m_pMtxSerial(pMtx)
{
	AM_CreateModbus(&m_pModbus, 0, *m_pMtxSerial, m_pSerial);
	AM_CreatePLCCtrl(&m_pPLC, 0, *m_pMtxSerial, m_pSerial);

	this->InitFuncTable();
}

ReactionModule::~ReactionModule()
{
	if (m_pShelf != nullptr)
	{
		delete m_pShelf;
	}
	if (m_pModbus != nullptr)
	{
		AM_ReleaseModbus(m_pModbus);
	}
	if (m_pPLC != nullptr)
	{
		AM_ReleasePLCCtrl(m_pPLC);
	}
	//if (m_pSerial!= nullptr)
	//{
	//	m_pSerial->ClosePort();
	//	AM_ReleaseSerialPort(m_pSerial);
	//}
	//if (m_pMtxSerial != nullptr)
	//{
	//	delete m_pMtxSerial;
	//}
}

SlideShelf* ReactionModule::TakeShelf()
{
	SlideShelf* p = m_pShelf;
	m_pShelf = nullptr;
	return p;
}

void ReactionModule::PutShelf(SlideShelf* p)
{
	m_pShelf = p;
}

bool ReactionModule::Empty()const
{
	return m_pShelf == nullptr;
}

bool ReactionModule::Reacting()const
{
	if (m_pShelf != nullptr)
		return this->CurrentStep() < this->TotalStep();
	else
		return false;
}

bool ReactionModule::HasSlideAt(int iIndex)const
{
	if (m_pShelf != nullptr)
		return m_pShelf->HasSlideAt(iIndex);
	else
		return false;
}

int ReactionModule::TotalStep()const
{
	if (m_pShelf != nullptr)
		return m_pShelf->TotalStep();
	else
		return 0;
}

int ReactionModule::CurrentStep()const
{
	if (m_pShelf != nullptr)
		return m_pShelf->m_iCurrentStep;
	else
		return 0;
}

string ReactionModule::FormulaName()const
{
	if (m_pShelf != nullptr)
		return m_pShelf->m_strFormula;
	else
		return "";
}

string ReactionModule::GetVReagent(int iSlideIndex, int iStep)
{
	return m_pShelf->m_mapSlides[iSlideIndex].mapVReagent[iStep];
}

void ReactionModule::IncStep()
{
	if (m_pShelf != nullptr)
		m_pShelf->m_iCurrentStep++;
}

void ReactionModule::DecStep()
{
	if (m_pShelf != nullptr)
		m_pShelf->m_iCurrentStep--;
}

void ReactionModule::Clear()
{
	if (m_pShelf != nullptr)
	{
		delete m_pShelf;
		m_pShelf = nullptr;
	}
}


#define REACT_MOD_STATION stoi(iniIO["Station"][string(u8"反应模块").append(to_string(m_iThisModIndex))])

void ReactionModule::WriteDT(const char* pName, int iVal)
{
	this->m_mapWriteDTFunc[pName](iVal);
}

int ReactionModule::ReadDT(const char* pName)
{
	return m_mapReadDTFunc[pName]();
}

void ReactionModule::WriteRelay(const char* pAddr, bool bVal)
{
	string strAddr = string(pAddr);
	const int iWordAddr = strAddr.size() > 1 ? stoi(strAddr.substr(0, strAddr.size() - 1)) : 0;
	string strBitPosHex = strAddr.substr(strAddr.size() - 1, 1);

	if (bVal)
	{
		m_pPLC->SetRelay(iWordAddr, strBitPosHex.c_str(), REACT_MOD_STATION);
	}
	else
	{
		m_pPLC->ClearRelay(iWordAddr, strBitPosHex.c_str(), REACT_MOD_STATION);
	}
}

bool ReactionModule::ReadRelay(const char* pAddr)
{
	string strAddr = string(pAddr);
	const int iWordAddr = strAddr.size() > 1 ? stoi(strAddr.substr(0, strAddr.size() - 1)) : 0;
	string strBitPosHex = strAddr.substr(strAddr.size() - 1, 1);
	return m_pPLC->CheckRelay(iWordAddr, strBitPosHex.c_str(), REACT_MOD_STATION);
}

int ReactionModule::WriteRelayStartTiming(int iSlideIndex, bool bVal)
{
	return PLCWriteRelay(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-Relay", string(u8"开始计时").append(to_string(iSlideIndex)));
}

int ReactionModule::WriteDTReactionTime(int iSlideIndex, int iTime)
{
	return PLCWriteDT(m_pPLC, iTime, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-DT", string(u8"反应时间").append(to_string(iSlideIndex)));
}

bool ReactionModule::RACRelayReactionCompletion(int iSlideIndex)
{
	return PLCReadAndClearRelay(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-Relay", string(u8"反应完成").append(to_string(iSlideIndex)));
}

int ReactionModule::WriteRelayHeat(bool bVal)
{
	return PLCWriteRelay(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-Relay", string(u8"加热"));
}

int ReactionModule::WriteRelayWashStart(bool bVal)
{
	return PLCWriteRelay(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-Relay", string(u8"清洗玻片"));
}

int ReactionModule::WriteDTWashInterval(int iTime)
{
	return PLCWriteDT(m_pPLC, iTime, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-DT", string(u8"清洗时间"));
}

int ReactionModule::WriteDTWashCount(int iTime)
{
	return PLCWriteDT(m_pPLC, iTime, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-DT", string(u8"清洗次数"));
}

int ReactionModule::WriteDTAddPBS(int iVal)
{
	return PLCWriteDT(m_pPLC, iVal, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-DT", string(u8"加PBS"));
}

int ReactionModule::WriteDTPBSDelay(int iVal)
{
	return PLCWriteDT(m_pPLC, iVal, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-DT", string(u8"PBS延时"));
}

bool ReactionModule::RACRelayWashCompletion()
{
	return PLCReadAndClearRelay(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-Relay", string(u8"清洗完成"));
}

int ReactionModule::WriteRelayMoisturize(bool bVal)
{
	return PLCWriteRelay(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-Relay", string(u8"玻片保湿"));
}

int ReactionModule::WriteDTMoisturizationInterval(int iInterval)
{
	return PLCWriteDT(m_pPLC, iInterval, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-DT", string(u8"玻片保湿时间间隔"));
}

int ReactionModule::WriteRelayPBSStart(bool bVal)
{
	return PLCWriteRelay(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-Relay", string(u8"PBS流程"));
}

int ReactionModule::WriteRelayPBS(int iSlideIndex)
{
	return PLCWriteRelay(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-Relay", string(u8"PBS").append(to_string(iSlideIndex)));
}

int ReactionModule::WriteDTPBSReactionTime(int iTime)
{
	return PLCWriteDT(m_pPLC, iTime, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-DT", string(u8"PBS反应时间"));
}

int ReactionModule::WriteRelayPutShelfRequest(bool bVal)
{
	return PLCWriteRelay(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-Relay", string(u8"请求放架"));
}

bool ReactionModule::RACRelayPutShelfPermission()
{
	return PLCReadAndClearRelay(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-Relay", string(u8"允许放架"));
}

int ReactionModule::WriteRelayDelayStart(bool bVal)
{
	return PLCWriteRelay(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-Relay", string(u8"延时"));
}

int ReactionModule::WriteDTDelayTime(int iTime)
{
	return PLCWriteDT(m_pPLC, iTime, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-DT", string(u8"延时时间"));
}

bool ReactionModule::RACRelayLidOpening()
{
	return PLCReadRelay(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-Relay", string(u8"盖子打开"));
}

int ReactionModule::WriteRelayPutShelfCompletion(bool bVal)
{
	return PLCWriteRelay(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-Relay", string(u8"放架完成"));
}

int ReactionModule::WriteRelayPickShelfRequest(bool bVal)
{
	return PLCWriteRelay(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-Relay", string(u8"请求取架"));
}

bool ReactionModule::RACRelayPickShelfPermission()
{
	return PLCReadAndClearRelay(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-Relay", string(u8"允许取架"));
}

int ReactionModule::WriteRelayPickShelfCompletion(bool bVal)
{
	return PLCWriteRelay(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-Relay", string(u8"取架完成"));
}

int ReactionModule::WriteRelayPipetRequest(bool bVal)
{
	return PLCWriteRelay(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-Relay", string(u8"请求移液"));
}

bool ReactionModule::RACRelayPipetPermission()
{
	return PLCReadAndClearRelay(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-Relay", string(u8"允许移液"));
}

int ReactionModule::WriteRelayPipetting(bool bVal)
{
	return PLCWriteRelay(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-Relay", string(u8"移液中"));
}

int ReactionModule::WriteRelayPipetCompletion(bool bVal)
{
	
	if (PLCClearRelay(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-Relay", string(u8"移液中")) == 0)
	{
		return PLCWriteRelay(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-Relay", string(u8"移液完成"));
	}
	else
	{
		return -1;
	}
}

int ReactionModule::WriteRelayReset(bool bVal)
{
	return PLCWriteRelay(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-Relay", string(u8"复位"));
}

bool ReactionModule::RACRelayResetCompletion()
{
	return PLCReadAndClearRelay(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-Relay", string(u8"复位完成"));
}

int ReactionModule::WriteDTLidMotorSpeed(int iSpeed)
{
	return PLCWrite2DTs(m_pPLC, iSpeed, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-DT", string(u8"翻盖电机运行速度"));
}

int ReactionModule::WriteDTShelfMotorSpeed(int iSpeed)
{
	return PLCWrite2DTs(m_pPLC, iSpeed, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-DT", string(u8"玻片架电机运行速度"));
}

int ReactionModule::WriteDTPBSMotorSpeed(int iSpeed)
{
	return PLCWrite2DTs(m_pPLC, iSpeed, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-DT", string(u8"加液电机运行速度"));
}

int ReactionModule::WriteDTHorPos(int iPos)
{
	return PLCWrite2DTs(m_pPLC, iPos, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-DT", string(u8"玻片架加液位置"));
}

int ReactionModule::WriteDTVerPos(int iPos)
{
	return PLCWrite2DTs(m_pPLC, iPos, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-DT", string(u8"玻片架取放位置"));
}

int ReactionModule::ReadDTCurrentTemp()
{
	return PLCRead2DTs(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-DT", string(u8"当前温度"));
}

int ReactionModule::ReadDTLowerTemp()
{
	return PLCRead2DTs(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-DT", string(u8"加热低温"));
}

int ReactionModule::ReadDTHigherTemp()
{
	return PLCRead2DTs(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-DT", string(u8"加热高温"));
}

int ReactionModule::ReadDTWashInterval()
{
	return PLCReadDT(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-DT", string(u8"清洗时间"));
}

int ReactionModule::ReadDTWashCount()
{
	return PLCReadDT(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-DT", string(u8"清洗次数"));
}

int ReactionModule::ReadDTAddPBS()
{
	return PLCReadDT(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-DT", string(u8"加PBS"));
}

int ReactionModule::ReadDTPBSDelay()
{
	return PLCReadDT(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-DT", string(u8"PBS延时"));
}

int ReactionModule::ReadDTPBSReactionTime()
{
	return PLCReadDT(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-DT", string(u8"PBS反应时间"));
}

int ReactionModule::ReadDTMoisturizationInterval()
{
	return PLCReadDT(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-DT", string(u8"玻片保湿时间间隔"));
}

int ReactionModule::ReadDTDelayTime()
{
	return PLCReadDT(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-DT", string(u8"延时时间"));
}

int ReactionModule::ReadDTReactionTime(int iSlideIndex)
{
	return PLCReadDT(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-DT", string(u8"反应时间").append(to_string(iSlideIndex)));
}

int ReactionModule::ReadDTLidMotorSpeed()
{
	return PLCRead2DTs(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-DT", string(u8"翻盖电机运行速度"));
}

int ReactionModule::ReadDTShelfMotorSpeed()
{
	return PLCRead2DTs(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-DT", string(u8"玻片架电机运行速度"));
}

int ReactionModule::ReadDTPBSMotorSpeed()
{
	return PLCRead2DTs(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-DT", string(u8"加液电机运行速度"));
}

int ReactionModule::ReadDTHorPos()
{
	return PLCRead2DTs(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-DT", string(u8"玻片架加液位置"));
}

int ReactionModule::ReadDTVerPos()
{
	return PLCRead2DTs(m_pPLC, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-DT", string(u8"玻片架取放位置"));
}

double ReactionModule::GetCurrentTemp()
{
	int iTempSensorSlaveID = stoi(iniIO["Station"][string(u8"反应模块").append(to_string(m_iThisModIndex)).append(u8"温度计")]);
	int iTemp = -1000;
	m_pModbus->SetFollowID(iTempSensorSlaveID);
	if (m_pModbus->ReadHoldRegisterByByte(0, &iTemp))
	{
		return static_cast<double>(iTemp);
	}
	else
	{
		return -1000;

	}
}

int ReactionModule::WriteDTCurrentTemp(int iTemp)
{
	return PLCWrite2DTs(m_pPLC, iTemp, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-DT", string(u8"当前温度"));
}

int ReactionModule::WriteDTLowerTemp(int iTemp)
{
	return PLCWrite2DTs(m_pPLC, iTemp, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-DT", string(u8"加热低温"));
}

int ReactionModule::WriteDTHigherTemp(int iTemp)
{
	return PLCWrite2DTs(m_pPLC, iTemp, string(u8"反应模块").append(to_string(m_iThisModIndex)), u8"反应模块-PLC-DT", string(u8"加热高温"));
}

void ReactionModule::InitFuncTable()
{
	m_mapReadDTFunc[u8"当前温度"] = bind(&ReactionModule::ReadDTCurrentTemp, this);
	m_mapReadDTFunc[u8"加热低温"] = bind(&ReactionModule::ReadDTLowerTemp, this);
	m_mapReadDTFunc[u8"加热高温"] = bind(&ReactionModule::ReadDTHigherTemp, this);
	m_mapReadDTFunc[u8"清洗时间"] = bind(&ReactionModule::ReadDTWashInterval, this);
	m_mapReadDTFunc[u8"清洗次数"] = bind(&ReactionModule::ReadDTWashCount, this);
	m_mapReadDTFunc[u8"加PBS"] = bind(&ReactionModule::ReadDTAddPBS, this);
	m_mapReadDTFunc[u8"PBS延时"] = bind(&ReactionModule::ReadDTPBSDelay, this);
	m_mapReadDTFunc[u8"PBS反应时间"] = bind(&ReactionModule::ReadDTPBSReactionTime, this);
	m_mapReadDTFunc[u8"玻片保湿时间间隔"] = bind(&ReactionModule::ReadDTMoisturizationInterval, this);
	m_mapReadDTFunc[u8"延时时间"] = bind(&ReactionModule::ReadDTDelayTime, this);
	m_mapReadDTFunc[u8"反应时间1"] = bind(&ReactionModule::ReadDTReactionTime, this, 1);
	m_mapReadDTFunc[u8"反应时间2"] = bind(&ReactionModule::ReadDTReactionTime, this, 2);
	m_mapReadDTFunc[u8"反应时间3"] = bind(&ReactionModule::ReadDTReactionTime, this, 3);
	m_mapReadDTFunc[u8"反应时间4"] = bind(&ReactionModule::ReadDTReactionTime, this, 4);
	m_mapReadDTFunc[u8"反应时间5"] = bind(&ReactionModule::ReadDTReactionTime, this, 5);
	m_mapReadDTFunc[u8"反应时间6"] = bind(&ReactionModule::ReadDTReactionTime, this, 6);
	m_mapReadDTFunc[u8"反应时间7"] = bind(&ReactionModule::ReadDTReactionTime, this, 7);
	m_mapReadDTFunc[u8"反应时间8"] = bind(&ReactionModule::ReadDTReactionTime, this, 8);
	m_mapReadDTFunc[u8"反应时间9"] = bind(&ReactionModule::ReadDTReactionTime, this, 9);
	m_mapReadDTFunc[u8"反应时间10"] = bind(&ReactionModule::ReadDTReactionTime, this, 10);
	m_mapReadDTFunc[u8"翻盖电机运行速度"] = bind(&ReactionModule::ReadDTLidMotorSpeed, this);
	m_mapReadDTFunc[u8"玻片架电机运行速度"] = bind(&ReactionModule::ReadDTShelfMotorSpeed, this);
	m_mapReadDTFunc[u8"加液电机运行速度"] = bind(&ReactionModule::ReadDTPBSMotorSpeed, this);
	m_mapReadDTFunc[u8"玻片架加液位置"] = bind(&ReactionModule::ReadDTHorPos, this);
	m_mapReadDTFunc[u8"玻片架取放位置"] = bind(&ReactionModule::ReadDTVerPos, this);


	m_mapWriteDTFunc[u8"当前温度"] = bind(&ReactionModule::WriteDTCurrentTemp, this, std::placeholders::_1);
	m_mapWriteDTFunc[u8"加热低温"] = bind(&ReactionModule::WriteDTLowerTemp, this, std::placeholders::_1);
	m_mapWriteDTFunc[u8"加热高温"] = bind(&ReactionModule::WriteDTHigherTemp, this, std::placeholders::_1);
	m_mapWriteDTFunc[u8"清洗时间"] = bind(&ReactionModule::WriteDTWashInterval, this, std::placeholders::_1);
	m_mapWriteDTFunc[u8"清洗次数"] = bind(&ReactionModule::WriteDTWashCount, this, std::placeholders::_1);
	m_mapWriteDTFunc[u8"加PBS"] = bind(&ReactionModule::WriteDTAddPBS, this, std::placeholders::_1);
	m_mapWriteDTFunc[u8"PBS延时"] = bind(&ReactionModule::WriteDTPBSDelay, this, std::placeholders::_1);
	m_mapWriteDTFunc[u8"PBS反应时间"] = bind(&ReactionModule::WriteDTPBSReactionTime, this, std::placeholders::_1);
	m_mapWriteDTFunc[u8"玻片保湿时间间隔"] = bind(&ReactionModule::WriteDTMoisturizationInterval, this, std::placeholders::_1);
	m_mapWriteDTFunc[u8"延时时间"] = bind(&ReactionModule::WriteDTDelayTime, this, std::placeholders::_1);
	m_mapWriteDTFunc[u8"反应时间1"] = bind(&ReactionModule::WriteDTReactionTime, this, 1, std::placeholders::_1);
	m_mapWriteDTFunc[u8"反应时间2"] = bind(&ReactionModule::WriteDTReactionTime, this, 2, std::placeholders::_1);
	m_mapWriteDTFunc[u8"反应时间3"] = bind(&ReactionModule::WriteDTReactionTime, this, 3, std::placeholders::_1);
	m_mapWriteDTFunc[u8"反应时间4"] = bind(&ReactionModule::WriteDTReactionTime, this, 4, std::placeholders::_1);
	m_mapWriteDTFunc[u8"反应时间5"] = bind(&ReactionModule::WriteDTReactionTime, this, 5, std::placeholders::_1);
	m_mapWriteDTFunc[u8"反应时间6"] = bind(&ReactionModule::WriteDTReactionTime, this, 6, std::placeholders::_1);
	m_mapWriteDTFunc[u8"反应时间7"] = bind(&ReactionModule::WriteDTReactionTime, this, 7, std::placeholders::_1);
	m_mapWriteDTFunc[u8"反应时间8"] = bind(&ReactionModule::WriteDTReactionTime, this, 8, std::placeholders::_1);
	m_mapWriteDTFunc[u8"反应时间9"] = bind(&ReactionModule::WriteDTReactionTime, this, 9, std::placeholders::_1);
	m_mapWriteDTFunc[u8"反应时间10"] = bind(&ReactionModule::WriteDTReactionTime, this, 10, std::placeholders::_1);
	m_mapWriteDTFunc[u8"翻盖电机运行速度"] = bind(&ReactionModule::WriteDTLidMotorSpeed, this, std::placeholders::_1);
	m_mapWriteDTFunc[u8"玻片架电机运行速度"] = bind(&ReactionModule::WriteDTShelfMotorSpeed, this, std::placeholders::_1);
	m_mapWriteDTFunc[u8"加液电机运行速度"] = bind(&ReactionModule::WriteDTPBSMotorSpeed, this, std::placeholders::_1);
	m_mapWriteDTFunc[u8"玻片架加液位置"] = bind(&ReactionModule::WriteDTHorPos, this, std::placeholders::_1);
	m_mapWriteDTFunc[u8"玻片架取放位置"] = bind(&ReactionModule::WriteDTVerPos, this, std::placeholders::_1);
}





/* repair module */

RepairModule::RepairModule(mutex* pMtx, CISerialPort* pSerial) :
	m_mapShelf{ {1,nullptr}, {2,nullptr} },
	m_mapState{ {1, RepairModState::Empty},{2, RepairModState::Empty} },
	m_pSerial(pSerial),
	m_pModbus(nullptr),
	m_pPLC(nullptr),
	m_pMtxSerial(pMtx)
{
	//AM_CreateSerialPort(&m_pSerial);
	//m_pSerial->SetBaud(19200);
	//m_pSerial->SetPortNo(iPortNo);
	//m_pSerial->SetPortW(8, 1, 0);
	//m_pSerial->OpenPort();

	AM_CreateModbus(&m_pModbus, 0, *m_pMtxSerial, m_pSerial);
	AM_CreatePLCCtrl(&m_pPLC, 0, *m_pMtxSerial, m_pSerial);

	int iRet;

	// 2: in1
	// 3: out1
	// 4: in2
	// 5: out4

	//iRet = m_pPLC->OpenOutPort(0, "5", 1);
	//iRet = m_pPLC->CloseOutPort(0, "5", 1);


	//iRet = m_pPLC->OpenOutPort(0, "3", 1);
	//iRet = m_pPLC->OpenOutPort(0, "5", 1);
	 
	//iRet = m_pPLC->OpenOutPort(0, "3", 1);
	//iRet = m_pPLC->OpenOutPort(0, "5", 1);
	//iRet = m_pPLC->CloseOutPort(0, "3", 1);
	//iRet = m_pPLC->CloseOutPort(0, "5", 1);
	 
	//iRet = m_pPLC->CloseOutPort(0, "3", 1);
	//iRet = m_pPLC->CloseOutPort(0, "5", 1);

	PLCWriteRelay(m_pPLC, u8"修复模块", u8"修复模块-PLC-Relay", u8"流程启动");
	InitFuncTable();
}

RepairModule::~RepairModule()
{
	for (auto p : m_mapShelf)
	{
		if (p.second != nullptr)
		{
			delete p.second;
		}
	}
	if (m_pModbus != nullptr)
	{
		AM_ReleaseModbus(m_pModbus);
	}
	if (m_pPLC != nullptr)
	{
		AM_ReleasePLCCtrl(m_pPLC);
	}
	//if (m_pSerial != nullptr)
	//{
	//	m_pSerial->ClosePort();
	//	AM_ReleaseSerialPort(m_pSerial);
	//}
	//if (m_pMtxSerial != nullptr)
	//{
	//	delete m_pMtxSerial;
	//}
}

int RepairModule::WriteRelayContinueAddingWhenAddingTimeout(int iIndex)
{
	return PLCWriteRelay(m_pPLC, u8"修复模块", u8"修复模块-PLC-Relay", string(u8"加液超时继续加液").append(to_string(iIndex)));
}

int RepairModule::WriteRelayStopAddingWhenAddingTimeout(int iIndex)
{
	return PLCWriteRelay(m_pPLC, u8"修复模块", u8"修复模块-PLC-Relay", string(u8"加液超时继续运行").append(to_string(iIndex)));
}

int RepairModule::WriteRelayContinueAddingWhenAddingRefillingTimeout(int iIndex)
{
	return PLCWriteRelay(m_pPLC, u8"修复模块", u8"修复模块-PLC-Relay", string(u8"修复补液超时继续加液").append(to_string(iIndex)));
}

int RepairModule::WriteRelayStopAddingWhenAddingRefillingTimeout(int iIndex)
{
	return PLCWriteRelay(m_pPLC, u8"修复模块", u8"修复模块-PLC-Relay", string(u8"修复补液超时继续运行").append(to_string(iIndex)));
}

int RepairModule::WriteRelayContinueAddingWhenCoolingAddingTimeout(int iIndex)
{
	return PLCWriteRelay(m_pPLC, u8"修复模块", u8"修复模块-PLC-Relay", string(u8"降温加液超时继续加液").append(to_string(iIndex)));
}

int RepairModule::WriteRelayStopAddingWhenCoolingAddingTimeout(int iIndex)
{
	return PLCWriteRelay(m_pPLC, u8"修复模块", u8"修复模块-PLC-Relay", string(u8"降温加液超时继续运行").append(to_string(iIndex)));
}

bool RepairModule::RACRelayAlarm(int iAlarmIndex)
{
	return PLCReadAndClearRelay(m_pPLC, u8"修复模块", u8"修复模块-PLC-Relay", string(u8"报警").append(to_string(iAlarmIndex)));
}

bool RepairModule::ReadRelayAlarm(int iAlarmIndex)
{
	return PLCReadRelay(m_pPLC, u8"修复模块", u8"修复模块-PLC-Relay", string(u8"报警").append(to_string(iAlarmIndex)));
}

int RepairModule::ClearRelayAlarm(int iAlarmIndex)
{
	return PLCClearRelay(m_pPLC, u8"修复模块", u8"修复模块-PLC-Relay", string(u8"报警").append(to_string(iAlarmIndex)));
}

int RepairModule::WriteDTAddingTimeout(int iIndex, int iTime)
{
	return PLCWriteDT(m_pPLC, iTime, u8"修复模块", u8"修复模块-PLC-DT", string(u8"加液超时时间").append(to_string(iIndex)));
}

int RepairModule::WriteDTRefillingTimeout(int iIndex, int iTime)
{
	return PLCWriteDT(m_pPLC, iTime, u8"修复模块", u8"修复模块-PLC-DT", string(u8"修复补液超时时间").append(to_string(iIndex)));
}
int RepairModule::WriteDTRefillingInterval(int iIndex, int iTime)
{
	return PLCWriteDT(m_pPLC, iTime, u8"修复模块", u8"修复模块-PLC-DT", string(u8"修复补液间隔时间").append(to_string(iIndex)));
}

int RepairModule::WriteDTCoolingAddingTimeout(int iIndex, int iTime)
{
	return PLCWriteDT(m_pPLC, iTime, u8"修复模块", u8"修复模块-PLC-DT", string(u8"降温加液超时时间").append(to_string(iIndex)));
}

int RepairModule::ReadDTAddingTimeout(int iIndex)
{
	return PLCReadDT(m_pPLC, u8"修复模块", u8"修复模块-PLC-DT", string(u8"加液超时时间").append(to_string(iIndex)));
}

int RepairModule::ReadDTRefillingTimeout(int iIndex)
{
	return PLCReadDT(m_pPLC, u8"修复模块", u8"修复模块-PLC-DT", string(u8"修复补液超时时间").append(to_string(iIndex)));
}

int RepairModule::ReadDTRefillingInterval(int iIndex)
{
	return PLCReadDT(m_pPLC, u8"修复模块", u8"修复模块-PLC-DT", string(u8"修复补液间隔时间").append(to_string(iIndex)));
}

int RepairModule::ReadDTCoolingAddingTimeout(int iIndex)
{
	return PLCReadDT(m_pPLC, u8"修复模块", u8"修复模块-PLC-DT", string(u8"降温加液超时时间").append(to_string(iIndex)));
}

int RepairModule::WriteRelayReset()
{
	return PLCWriteRelay(m_pPLC, u8"修复模块", u8"修复模块-PLC-Relay", u8"复位");
}

SlideShelf* RepairModule::TakeShelf(int iIndex)
{
	SlideShelf* p = m_mapShelf[iIndex];
	m_mapShelf[iIndex] = nullptr;
	m_mapState[iIndex] = RepairModState::Empty;
	return p;
}

SlideShelf* RepairModule::GetShelf(int iIndex)
{
	return m_mapShelf[iIndex];
}

void RepairModule::PutShelf(int iIndex, SlideShelf* p)
{
	m_mapShelf[iIndex] = p;
}

bool RepairModule::Empty(int iIndex)const
{
	return m_mapShelf.at(iIndex) == nullptr;
}

bool RepairModule::IsRepairing(int iIndex)const
{
	return m_mapState.at(iIndex) == RepairModState::Repairing;
}

bool RepairModule::IsCompleted(int iIndex)const
{
	return m_mapState.at(iIndex) == RepairModState::Completed;
}


void RepairModule::SetStateRepairing(int iIndex)
{
	m_mapState[iIndex] = RepairModState::Repairing;
}

void RepairModule::SetStateCompleted(int iIndex)
{
	m_mapState[iIndex] = RepairModState::Completed;
}

#define REPAIR_MOD_STATION stoi(iniIO["Station"][string(u8"修复模块")])

void RepairModule::WriteDT(const char* pAddr, int iVal)
{
	m_mapWriteDTFunc[pAddr](iVal);
}

int RepairModule::ReadDT(const char* pAddr)
{
	return m_mapReadDTFunc[pAddr]();
}

void RepairModule::WriteRelay(const char* pAddr, bool bVal)
{
	string strAddr = string(pAddr);
	const int iWordAddr = strAddr.size() > 1 ? stoi(strAddr.substr(0, strAddr.size() - 1)) : 0;
	string strBitPosHex = strAddr.substr(strAddr.size() - 1, 1);

	if (bVal)
	{
		m_pPLC->SetRelay(iWordAddr, strBitPosHex.c_str(), REPAIR_MOD_STATION);
	}
	else
	{
		m_pPLC->ClearRelay(iWordAddr, strBitPosHex.c_str(), REPAIR_MOD_STATION);
	}
}

void RepairModule::WriteRelayByName(const char* pName, bool bVal)
{
	if (bVal)
	{
		PLCWriteRelay(m_pPLC, u8"修复模块", u8"修复模块-PLC-Relay", string(pName));
	}
	else
	{
		PLCClearRelay(m_pPLC, u8"修复模块", u8"修复模块-PLC-Relay", string(pName));
	}
}

bool RepairModule::ReadRelay(const char* pAddr)
{
	string strAddr = string(pAddr);
	const int iWordAddr = strAddr.size() > 1 ? stoi(strAddr.substr(0, strAddr.size() - 1)) : 0;
	string strBitPosHex = strAddr.substr(strAddr.size() - 1, 1);
	return m_pPLC->CheckRelay(iWordAddr, strBitPosHex.c_str(), REPAIR_MOD_STATION);
}

bool RepairModule::HasSlideAt(int iIndex, int iSlideIndex)const
{
	if (m_mapShelf.find(iIndex) != m_mapShelf.end() && m_mapShelf.at(iIndex) != nullptr)
	{
		return m_mapShelf.at(iIndex)->HasSlideAt(iSlideIndex);
	}
	else
	{
		return false;
	}
}

int RepairModule::WriteRepairParam(int iIndex)
{
	if (m_mapShelf[iIndex] == nullptr)
		return -1;

	int iTemp = m_mapShelf[iIndex]->m_iRepairTemp;
	int iTime = m_mapShelf[iIndex]->m_iRepairTime;

	int iRet = 0;
	iRet |= this->WriteDTTempLower(iIndex, iTemp);
	iRet |= this->WriteDTTempKeep(iIndex, iTemp + 1);
	iRet |= this->WriteDTTempUpper(iIndex, iTemp + 2);
	iRet |= this->WriteDTRepairTime(iIndex, iTime);

	return iRet;
}

void RepairModule::Clear(int iIndex)
{
	if (m_mapShelf[iIndex] != nullptr)
	{
		delete m_mapShelf[iIndex];
		m_mapShelf[iIndex] = nullptr;
	}

}

int RepairModule::WriteRelayFlowStart()
{
	return PLCWriteRelay(m_pPLC, u8"修复模块", u8"修复模块-PLC-Relay", u8"流程启动");
}

bool RepairModule::RACRelayPutShelfPermission(int iIndex)
{
	//return true;
	return PLCReadAndClearRelay(m_pPLC, u8"修复模块", u8"修复模块-PLC-Relay", string(u8"允许放架").append(to_string(iIndex)));
}

int RepairModule::WriteRelayPutShelfCompletion(int iIndex)
{
	return PLCWriteRelay(m_pPLC, u8"修复模块", u8"修复模块-PLC-Relay", string(u8"放架完成").append(to_string(iIndex)));
}

int RepairModule::WriteDTRepairTime(int iIndex, int iTime)
{
	return PLCWrite2DTs(m_pPLC, iTime, u8"修复模块", u8"修复模块-PLC-DT", string(u8"修复时间").append(to_string(iIndex)));
}

bool RepairModule::RACRelayPickShelfPermission(int iIndex)
{
	return PLCReadAndClearRelay(m_pPLC, u8"修复模块", u8"修复模块-PLC-Relay", string(u8"允许取架").append(to_string(iIndex)));
}

int RepairModule::WriteRelayPickShelfCompletion(int iIndex)
{
	return PLCWriteRelay(m_pPLC, u8"修复模块", u8"修复模块-PLC-Relay", string(u8"取架完成").append(to_string(iIndex)));
}

int RepairModule::WriteDTTempPickShelf(int iIndex, int iTemp)
{
	return PLCWrite2DTs(m_pPLC, iTemp, u8"修复模块", u8"修复模块-PLC-DT", string(u8"取架温度").append(to_string(iIndex)));
}

int RepairModule::WriteDTTempKeep(int iIndex, int iTemp)
{
	return PLCWrite2DTs(m_pPLC, iTemp, u8"修复模块", u8"修复模块-PLC-DT", string(u8"保持温度").append(to_string(iIndex)));
}

int RepairModule::WriteDTTempLower(int iIndex, int iTemp)
{
	return PLCWrite2DTs(m_pPLC, iTemp, u8"修复模块", u8"修复模块-PLC-DT", string(u8"最低温度").append(to_string(iIndex)));
}

int RepairModule::WriteDTTempUpper(int iIndex, int iTemp)
{
	return PLCWrite2DTs(m_pPLC, iTemp, u8"修复模块", u8"修复模块-PLC-DT", string(u8"最高温度").append(to_string(iIndex)));
}

int RepairModule::WriteDTCurrentTemp(int iIndex, int iTemp)
{
	return PLCWrite2DTs(m_pPLC, iTemp, u8"修复模块", u8"修复模块-PLC-DT", string(u8"当前温度").append(to_string(iIndex)));
}

int RepairModule::ReadDTRepairTime(int iIndex)
{
	return PLCRead2DTs(m_pPLC, "修复模块", u8"修复模块-PLC-DT", string(u8"修复时间").append(to_string(iIndex)));
}

int RepairModule::ReadDTTempPickShelf(int iIndex)
{
	return PLCRead2DTs(m_pPLC, u8"修复模块", u8"修复模块-PLC-DT", string(u8"取架温度").append(to_string(iIndex)));
}

int RepairModule::ReadDTTempKeep(int iIndex)
{
	return PLCRead2DTs(m_pPLC, u8"修复模块", u8"修复模块-PLC-DT", string(u8"保持温度").append(to_string(iIndex)));
}

int RepairModule::ReadDTTempLower(int iIndex)
{
	return PLCRead2DTs(m_pPLC, u8"修复模块", u8"修复模块-PLC-DT", string(u8"最低温度").append(to_string(iIndex)));
}

int RepairModule::ReadDTTempUpper(int iIndex)
{
	return PLCRead2DTs(m_pPLC, u8"修复模块", u8"修复模块-PLC-DT", string(u8"最高温度").append(to_string(iIndex)));
}

int RepairModule::ReadDTCurrentTemp(int iIndex)
{
	return PLCRead2DTs(m_pPLC, u8"修复模块", u8"修复模块-PLC-DT", string(u8"当前温度").append(to_string(iIndex)));
}

double RepairModule::GetCurrentTemp(int iIndex)
{
	int iTempSensorSlaveID = stoi(iniIO["Station"][string(u8"修复模块温度计").append(to_string(iIndex))]);
	int iTemp = -1000;
	m_pModbus->SetFollowID(iTempSensorSlaveID);
	if (m_pModbus->ReadHoldRegisterByByte(0, &iTemp))
	{
		return static_cast<double>(iTemp);
	}
	else
	{
		return -1000;

	}
}

void RepairModule::InitFuncTable()
{
	m_mapReadDTFunc[u8"修复时间1"] = bind(&RepairModule::ReadDTRepairTime, this, 1);
	m_mapReadDTFunc[u8"修复时间2"] = bind(&RepairModule::ReadDTRepairTime, this, 2);
	m_mapReadDTFunc[u8"取架温度1"] = bind(&RepairModule::ReadDTTempPickShelf, this, 1);
	m_mapReadDTFunc[u8"取架温度2"] = bind(&RepairModule::ReadDTTempPickShelf, this, 2);
	m_mapReadDTFunc[u8"保持温度1"] = bind(&RepairModule::ReadDTTempKeep, this, 1);
	m_mapReadDTFunc[u8"保持温度2"] = bind(&RepairModule::ReadDTTempKeep, this, 2);
	m_mapReadDTFunc[u8"最低温度1"] = bind(&RepairModule::ReadDTTempLower, this, 1);
	m_mapReadDTFunc[u8"最低温度2"] = bind(&RepairModule::ReadDTTempLower, this, 2);
	m_mapReadDTFunc[u8"最高温度1"] = bind(&RepairModule::ReadDTTempUpper, this, 1);
	m_mapReadDTFunc[u8"最高温度2"] = bind(&RepairModule::ReadDTTempUpper, this, 2);
	m_mapReadDTFunc[u8"当前温度1"] = bind(&RepairModule::ReadDTCurrentTemp, this, 1);
	m_mapReadDTFunc[u8"当前温度2"] = bind(&RepairModule::ReadDTCurrentTemp, this, 2);
	m_mapReadDTFunc[u8"加液超时时间1"] = bind(&RepairModule::ReadDTAddingTimeout, this, 1);
	m_mapReadDTFunc[u8"修复补液超时时间1"] = bind(&RepairModule::ReadDTRefillingTimeout, this, 1);
	m_mapReadDTFunc[u8"修复补液间隔时间1"] = bind(&RepairModule::ReadDTRefillingInterval, this, 1);
	m_mapReadDTFunc[u8"降温加液超时时间1"] = bind(&RepairModule::ReadDTCoolingAddingTimeout, this, 1);
	m_mapReadDTFunc[u8"加液超时时间2"] = bind(&RepairModule::ReadDTAddingTimeout, this, 2);
	m_mapReadDTFunc[u8"修复补液超时时间2"] = bind(&RepairModule::ReadDTRefillingTimeout, this, 2);
	m_mapReadDTFunc[u8"修复补液间隔时间2"] = bind(&RepairModule::ReadDTRefillingInterval, this, 2);
	m_mapReadDTFunc[u8"降温加液超时时间2"] = bind(&RepairModule::ReadDTCoolingAddingTimeout, this, 2);

	m_mapWriteDTFunc[u8"修复时间1"] = bind(&RepairModule::WriteDTRepairTime, this, 1, placeholders::_1);
	m_mapWriteDTFunc[u8"修复时间2"] = bind(&RepairModule::WriteDTRepairTime, this, 2, placeholders::_1);
	m_mapWriteDTFunc[u8"取架温度1"] = bind(&RepairModule::WriteDTTempPickShelf, this, 1, placeholders::_1);
	m_mapWriteDTFunc[u8"取架温度2"] = bind(&RepairModule::WriteDTTempPickShelf, this, 2, placeholders::_1);
	m_mapWriteDTFunc[u8"保持温度1"] = bind(&RepairModule::WriteDTTempKeep, this, 1, placeholders::_1);
	m_mapWriteDTFunc[u8"保持温度2"] = bind(&RepairModule::WriteDTTempKeep, this, 2, placeholders::_1);
	m_mapWriteDTFunc[u8"最低温度1"] = bind(&RepairModule::WriteDTTempLower, this, 1, placeholders::_1);
	m_mapWriteDTFunc[u8"最低温度2"] = bind(&RepairModule::WriteDTTempLower, this, 2, placeholders::_1);
	m_mapWriteDTFunc[u8"最高温度1"] = bind(&RepairModule::WriteDTTempUpper, this, 1, placeholders::_1);
	m_mapWriteDTFunc[u8"最高温度2"] = bind(&RepairModule::WriteDTTempUpper, this, 2, placeholders::_1);
	m_mapWriteDTFunc[u8"当前温度1"] = bind(&RepairModule::WriteDTCurrentTemp, this, 1, placeholders::_1);
	m_mapWriteDTFunc[u8"当前温度2"] = bind(&RepairModule::WriteDTCurrentTemp, this, 2, placeholders::_1);
	m_mapWriteDTFunc[u8"加液超时时间1"] = bind(&RepairModule::WriteDTAddingTimeout, this, 1, placeholders::_1);
	m_mapWriteDTFunc[u8"修复补液超时时间1"] = bind(&RepairModule::WriteDTRefillingTimeout, this, 1, placeholders::_1);
	m_mapWriteDTFunc[u8"修复补液间隔时间1"] = bind(&RepairModule::WriteDTRefillingInterval, this, 1, placeholders::_1);
	m_mapWriteDTFunc[u8"降温加液超时时间1"] = bind(&RepairModule::WriteDTCoolingAddingTimeout, this, 1, placeholders::_1);
	m_mapWriteDTFunc[u8"加液超时时间2"] = bind(&RepairModule::WriteDTAddingTimeout, this, 2, placeholders::_1);
	m_mapWriteDTFunc[u8"修复补液超时时间2"] = bind(&RepairModule::WriteDTRefillingTimeout, this, 2, placeholders::_1);
	m_mapWriteDTFunc[u8"修复补液间隔时间2"] = bind(&RepairModule::WriteDTRefillingInterval, this, 2, placeholders::_1);
	m_mapWriteDTFunc[u8"降温加液超时时间2"] = bind(&RepairModule::WriteDTCoolingAddingTimeout, this, 2, placeholders::_1);
}



/* slide shelf */

SlideShelf* ShelfHolder::TakeShelf()
{

	SlideShelf* p = m_pShelf;
	m_pShelf = nullptr;
	return p;
}

void ShelfHolder::PutShelf(SlideShelf* p)
{
	m_pShelf = p;
}

bool ShelfHolder::Empty()const
{
	return m_pShelf == nullptr;
}

bool ShelfHolder::Waiting()const
{
	if (m_pShelf != nullptr)
		return m_pShelf->m_iCurrentStep < m_pShelf->TotalStep();
	else
		return false;
}

bool ShelfHolder::NeedsRepair()const
{
	if (m_pShelf != nullptr)
		return m_pShelf->m_iRepairTime > 0;
	else
		return false;
}

bool ShelfHolder::HasSlideAt(int iIndex)const
{
	if (m_pShelf != nullptr)
		return m_pShelf->HasSlideAt(iIndex);
	else
		return false;
}

void ShelfHolder::Clear()
{
	if (m_pShelf != nullptr)
	{
		delete m_pShelf;
		m_pShelf = nullptr;
	}
}


/* transit */

void Transit::Pick(SlideShelf* p)
{
	assert(m_pShelf == nullptr);
	m_pShelf = p;
}

SlideShelf* Transit::Put()
{
	SlideShelf* p = m_pShelf;
	m_pShelf = nullptr;
	return p;
}

bool Transit::NeedsRepair()const
{
	if (m_pShelf == nullptr)
		return false;
	else
		return m_pShelf->m_iRepairTime > 0;
}

bool Transit::Empty()const
{
	return m_pShelf == nullptr;
}

bool Transit::Waiting()const
{
	if (m_pShelf != nullptr)
	{
		return m_pShelf->m_iCurrentStep < m_pShelf->TotalStep();
	}
	else
	{
		return false;
	}
}

bool Transit::HasSlideAt(int iIndex)const
{
	if (m_pShelf != nullptr)
	{
		return m_pShelf->HasSlideAt(iIndex);
	}
	else
	{
		return false;
	}
}

void Transit::Clear()
{
	if (m_pShelf != nullptr)
	{
		delete m_pShelf;
		m_pShelf = nullptr;
	}	
}
