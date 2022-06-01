#include "stdafx.h"

/* global var */
#include "config.h"
#include "ReagentMgr.h"
#include "FormulaMgr.h"

#include "MachineControl.h"

/* ms */
long long int GetProgramTime()
{
	return (long long)clock() * 1000 / CLOCKS_PER_SEC;
}

#define CHECK_MOTION_TIMEOUT 

CMachineControl::CMachineControl() :
	m_tp(8),
	m_state(RunState::Stopped),
	m_mapReactionState({ { 1,ReactionState::New }, { 2,ReactionState::New } }),
	m_pMtxSerial(new mutex),
	m_iUnhandledAlarmCnt(0)
{
	this->ReloadConfig();
	this->ReloadCoord();

	AM_CreateSerialPort(&m_pSerial);
	m_pSerial->SetBaud(19200);
	m_pSerial->SetPortNo(stoi(iniCfg[u8"模块"][u8"串口号"]));
	m_pSerial->SetPort(8, 0, 0);
	m_pSerial->OpenPort();
	this->m_mapReactMods.emplace(1, new ReactionModule(1, m_pMtxSerial, m_pSerial));
	this->m_mapReactMods.emplace(2, new ReactionModule(2, m_pMtxSerial, m_pSerial));
	m_repairMod = new RepairModule(m_pMtxSerial, m_pSerial);

	this->m_mapShelfHolders.emplace(1, new ShelfHolder());
	this->m_mapShelfHolders.emplace(2, new ShelfHolder());
	this->m_mapShelfHolders.emplace(3, new ShelfHolder());
	this->m_mapShelfHolders.emplace(4, new ShelfHolder());
	this->m_transit = new Transit();

	ClearShelfHolder();
	ClearReactionMod(1);
	ClearReactionMod(2);
	ClearRepairMod(1);
	ClearRepairMod(2);
	ClearTransit();

	m_motion.Init(iniCfg[u8"运动控制卡"][u8"地址"]);

	/* updaters */
	boost::asio::post(m_tp, bind(&CMachineControl::ImplTemperatureUpdate2, this, (stoi(iniCfg[u8"模块"][u8"有修复模块"]) == 1)));
	boost::asio::post(m_tp, bind(&CMachineControl::ImplAxisCoordUpdate, this));

	emit signalUpdateAllView();
}

CMachineControl::~CMachineControl()
{
	unique_lock<mutex> lk(m_mtxState);

	m_state = RunState::Exiting;
	m_condRunning.notify_all(); /* for those who are waiting on `condRunning' */

	lk.unlock();
	m_tp.wait();

	if (m_repairMod != nullptr && stoi(iniCfg[u8"模块"][u8"有修复模块"]) == 1)
	{
		m_repairMod->WriteRelayReset();
		delete m_repairMod;
		m_repairMod = nullptr;
	}
	for (const auto& iter : m_mapReactMods)
	{
		if (iter.second != nullptr)
		{
			iter.second->WriteRelayReset();
			delete iter.second;
		}
	}
	for (const auto& iter : m_mapShelfHolders)
	{
		if (iter.second != nullptr)
		{
			delete iter.second;
		}
	}
	if (m_transit != nullptr)
	{
		delete m_transit;
	}

	for (int i = 0; i < 12; i++)
	{
		m_motion.WriteOutBit(0, i, 1, 0);
	}
}

void CMachineControl::Reset()
{
	unique_lock<mutex> lk(m_mtxState);
	if (m_state == RunState::Stopped) 
	{
		m_state = RunState::Resetting;
		m_condRunning.notify_all();
		lk.unlock();

		boost::asio::post(m_tp, bind(&CMachineControl::ImplReset, this, 0, stoi(iniCfg[u8"模块"][u8"有修复模块"]) == 1));
	}
}

void CMachineControl::Start()
{
	lock_guard<mutex> lk(m_mtxState);
	if (m_state == RunState::Stopped)
	{
		m_state = RunState::Running;

		/* start background work */
		boost::asio::post(m_tp, bind(&CMachineControl::ImplCheckReactionModule, this, 2, 0));
		boost::asio::post(m_tp, bind(&CMachineControl::ImplCheckReactionModule, this, 1, 0));

		if (GetCommonConfig(u8"模块", u8"有修复模块") == 1)
		{
			boost::asio::post(m_tp, bind(&CMachineControl::ImplCheckRepairModule, this, 2, 0));
			boost::asio::post(m_tp, bind(&CMachineControl::ImplCheckRepairModule, this, 1, 0));
			boost::asio::post(m_tp, bind(&CMachineControl::ImplCheckRepairModAlarm, this, 0));
		}
		boost::asio::post(m_tp, bind(&CMachineControl::ImplDrain, this, 0));
	
		/* main workflow */
		boost::asio::post(m_tp, bind(&CMachineControl::Dispatch, this)); 
	}
}

void CMachineControl::Abort()
{
	emit signalMsg(QString::fromUtf8(u8"手动停止"));

	m_motion.EmgStop(0);
	lock_guard<mutex> lk(m_mtxState);
	m_state = RunState::Stopped;
	// ...

}

void CMachineControl::Suspend()
{
	lock_guard<mutex> lk(m_mtxState);
	if (m_state == RunState::Running)
	{
		m_state = RunState::Suspending;
		// ...
	}
}

void CMachineControl::Resume()
{
	lock_guard<mutex> lk(m_mtxState);
	if (m_state == RunState::Suspending)
	{
		m_state = RunState::Running;
		m_condRunning.notify_all();		
	}
}

void CMachineControl::Exit()
{
	unique_lock<mutex> lk(m_mtxState);

	m_state = RunState::Exiting;
	m_condRunning.notify_all(); /* for those who are waiting on `condRunning' */
	
	lk.unlock();	

	m_tp.wait();
}

bool CMachineControl::IsRunning()
{
	return m_state == RunState::Running;
}

bool CMachineControl::IsSuspend()
{
	return m_state == RunState::Suspending;
}

bool CMachineControl::IsResetting()
{
	return m_state == RunState::Resetting;
}

bool CMachineControl::IsStopped()
{
	return m_state == RunState::Stopped;
}

void CMachineControl::Test(int iArg)
{
	this->Abort();
	if (iArg == 1)
	{
		m_bTestingFlag = true;
		boost::asio::post(m_tp, bind(&CMachineControl::ImplTestSurfaceSensor, this, 0));
	}
	else
	{
		m_bTestingFlag = false;
	}
}

bool CMachineControl::HoldsShelfAt(int iSlotIndex)
{
	if (m_mapShelfHolders.find(iSlotIndex) != m_mapShelfHolders.end())
	{
		return !m_mapShelfHolders[iSlotIndex]->Empty();
	}
	else
	{
		return false;
	}
}

int CMachineControl::GetShelfHolderState(int iIndex, int iSlideIndex)
{
	if (m_mapShelfHolders.find(iIndex) == m_mapShelfHolders.end() || !m_mapShelfHolders[iIndex]->HasSlideAt(iSlideIndex) || m_mapShelfHolders[iIndex]->Empty())
	{
		return 0;
	}
	else if (m_mapShelfHolders[iIndex]->Waiting())
	{
		return 1;
	}
	else
	{
		return 2;
	}
}

int CMachineControl::GetRepairModState(int iIndex, int iSlideIndex)
{
	if (!m_repairMod->HasSlideAt(iIndex, iSlideIndex) || m_repairMod->Empty(iIndex))
	{
		return 0;
	}
	else if (m_repairMod->IsRepairing(iIndex))
	{
		return 1;
	}
	else
	{
		return 2;
	}
}

int CMachineControl::TransitState(int iSlideIndex)
{
	if (m_transit->Empty() || !m_transit->HasSlideAt(iSlideIndex))
	{
		return 0;
	}
	else if (m_transit->Waiting())
	{
		return 1;
	}
	else
	{
		return 2;
	}
}

int CMachineControl::GetReactionTotalStep(int iIndex, int iSlideIndex)
{
	if (m_mapReactMods.find(iIndex) != m_mapReactMods.end())
	{
		return m_mapReactMods[iIndex]->TotalStep();
	}
	else
	{
		return 0;
	}
}

int CMachineControl::GetReactionCompletedStep(int iIndex, int iSlideIndex)
{
	if (m_mapReactMods.find(iIndex) != m_mapReactMods.end())
	{
		return m_mapReactMods[iIndex]->CurrentStep();
	}
	else
	{
		return 0;
	}
}

void CMachineControl::ClearShelfHolder(int iSlotIndex)
{
	if (m_mapShelfHolders.find(iSlotIndex) != m_mapShelfHolders.end())
	{
		m_mapShelfHolders[iSlotIndex]->Clear();		
	}
}

void CMachineControl::ReloadShelfHolder(int iSlotIndex)
{
	if (m_mapShelfHolders.find(iSlotIndex) != m_mapShelfHolders.end())
	{
		m_mapShelfHolders[iSlotIndex]->Clear();
	}

	mINI::INIFile iniFile(string(u8"./Product/SS").append(to_string(iSlotIndex)).append(".pro"));
	mINI::INIStructure iniStruct;
	iniFile.read(iniStruct);

	if (iniStruct.size() > 0)
	{
		SlideShelf* p = new SlideShelf();

		const int iRepairTime = stoi(iniStruct["Config"]["RepairTime"]);
		const int iRepairTemp = stoi(iniStruct["Config"]["RepairTemp"]);
		const string& strFormula = iniStruct["Config"]["Formula"];
		p->m_iRepairTime = iRepairTime;
		p->m_iRepairTemp = iRepairTemp;
		p->m_strFormula = strFormula;

		for (int i = 1; i <= iniStruct.size() - 1; i++) /* for each slide */
		{
			const string& strIndex = to_string(i);
			auto& iniSection = iniStruct[strIndex];
			map<int, string> mapVReagents;
			for (auto row : iniSection) /* for each vreagent */
			{
				if (row.first.find("vReagent") == 0)
				{
					mapVReagents[stoi(row.first.substr(8))] = row.second;
				}
			}


			p->AddSlide(iniSection["ID"], iniSection["Hospital"], move(mapVReagents));
		}
		m_mapShelfHolders[iSlotIndex]->PutShelf(p);
	}	
}

void CMachineControl::ClearShelfHolder()
{
	for (int i = 1; i <= 4; i++)
	{
		ClearShelfHolder(i);
	}
}

int CMachineControl::ShelfCnt()
{
	int iCnt = 0;
	for (const auto& react : m_mapReactMods)
	{
		if (!react.second->Empty())
		{
			iCnt++;
		}
	}

	for (int i = 1; i <= 2; i++)
	{
		if (!m_repairMod->Empty(i))
		{
			iCnt++;
		}
	}

	for (const auto& holder : m_mapShelfHolders)
	{
		if (!holder.second->Empty())
		{
			iCnt++;
		}
	}

	if (!m_transit->Empty())
	{
		iCnt++;
	}

	return iCnt;
}

int CMachineControl::HasSlideAtReactMod(int iModIndex, int iSlideIndex)
{
	return m_mapReactMods[iModIndex]->HasSlideAt(iSlideIndex);
}

int CMachineControl::MoveToCoord(const char* pCoordName)
{
	return this->m_motion.MultiAxisMoveSimple(this->GetMultiAxisCoord(pCoordName));
}

int CMachineControl::MoveAxis(int iAxisNo, double dLength, bool bAbsolutly)
{
	dLength = dLength * stod(iniCfg[STR_SECTION_RESOLUTION][to_string(iAxisNo)]); // resolution
	if (bAbsolutly)
	{
		return m_motion.SingleAxisMoveAbs(iAxisNo, dLength);
	}
	else
	{
		return m_motion.SingleAxisMoveRel(iAxisNo, dLength);
	}
}

int CMachineControl::HomeMoveAxis(int iAxisNo)
{
	return m_motion.AxisHome(iAxisNo);
}

double CMachineControl::GetAxisPos(int iAxisNo)
{
	double dPos = 0;
	m_motion.GetPosition(0, iAxisNo, dPos);
	return dPos;
}

int CMachineControl::ZAxisUp()
{
	int iRet = 0;
	iRet |= m_motion.MultiAxisMoveSimple(GetMultiAxisCoord(u8"针头安全高度"));
	iRet |= m_motion.MultiAxisMoveSimple(GetMultiAxisCoord(u8"提架安全高度"));
	return iRet;
}

void CMachineControl::ClearReactionMod(int iIndex)
{
	m_mapReactMods[iIndex]->Clear();
	emit signalUpdateReactionModuleView(iIndex);
}

void CMachineControl::ClearRepairMod(int iIndex)
{
	m_repairMod->Clear(iIndex);
	emit signalUpdateRepairModuleView(iIndex);
}

void CMachineControl::ClearTransit()
{
	m_transit->Clear();
	emit signalUpdateTransitShelfView();
}

void CMachineControl::SetState(RunState s)
{
	lock_guard<mutex> lk(m_mtxState);
	m_state = s;
}

void CMachineControl::ImplReset(int iStep, bool bHasRepairMod)
{
	static long long int s_llStartTime = 0;
	static int s_iPumpAxisNo = GetAxisNo(STR_AXIS_PIP);

	if (m_state != RunState::Resetting)
		return;

	switch (iStep)
	{
	case 0:
	{
		if (m_motion.AxisHome(GetAxisNo(STR_AXIS_NDL)) == 0)
		{
			emit signalMsg(QString::fromUtf8(u8"针头轴复位"));
			iStep = 1000001;
		}
		break;
	}
	case 1000001:
	{
		emit signalMsg(QString::fromUtf8(u8"关闭通用输出"));
		for (int i = 0; i < 12; i++)
		{
			m_motion.WriteOutBit(0, i, 1, 0);
		}
		iStep = 1;
		break;
	}
	case 1:
	{
		if (m_motion.AxisHome(GetAxisNo(STR_AXIS_PICK)) == 0)
		{
			emit signalMsg(QString::fromUtf8(u8"提架轴复位"));
			iStep = 2;
		}
		break;
	}	
	case 2:
	{
		if (m_motion.IsHomeCompleted(GetAxisNo(STR_AXIS_NDL))
			&& m_motion.IsHomeCompleted(GetAxisNo(STR_AXIS_PICK)))
		{
			emit signalMsg(QString::fromUtf8(u8"针头轴、提架轴复位完成"));
			iStep = 3;
		}
		break;
	}
	case 3:
	{
		m_motion.Init(iniCfg[u8"运动控制卡"][u8"地址"]);
		//ClearShelfHolder();
		ClearReactionMod(1);
		ClearReactionMod(2);
		ClearRepairMod(1);
		ClearRepairMod(2);
		ClearTransit();
		emit signalMsg(QString::fromUtf8(u8"反应模块复位"));
		emit signalUpdateAllView();
		iStep = 1003001;
		break;
	}
	case 1003001:
	{
		int iRet1 = bHasRepairMod ? m_repairMod->WriteRelayReset() : 0;
		if (!bHasRepairMod)
		{
			emit signalMsg(QString::fromUtf8(u8"无修复模块"));
		}
		int iRet2 = m_mapReactMods[1]->WriteRelayReset();
		int iRet3 = m_mapReactMods[2]->WriteRelayReset();
		if ((iRet1 | iRet2 | iRet3) == 0)
		{
			iStep = 4;
		}
		break;
	}
	case 4:
	{
		if (m_motion.SingleAxisMoveRel(GetAxisNo(STR_AXIS_X), 5000) == 0)
		{
			iStep = 5;
			emit signalMsg(QString::fromUtf8(u8"X轴偏移"));
			s_llStartTime = GetProgramTime();
		}
		break;
	}
	case 5:
	{
		if (!m_motion.IsAxisBusy(GetAxisNo(STR_AXIS_X)))
		{
			emit signalMsg(QString::fromUtf8(u8"X轴偏移完成"));
			iStep = 6;
		}
		break;
	}
	case 6:
	{

		function<void()> nextStep = bind(&CMachineControl::ImplReset, this, 7, bHasRepairMod);
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowXYHome, this, nextStep, 0));
		iStep = -2;
		break;
	}
	case 7:
	{
		/* check if xy axis */
		double dPosX = 99999, dPosY = 99999;
		m_motion.GetPosition(0, GetAxisNo(STR_AXIS_X), dPosX);
		m_motion.GetPosition(0, GetAxisNo(STR_AXIS_Y), dPosY);
		if (dPosX == 0 && dPosY == 0)
		{
			iStep = 8;
		}
		else
		{
			iStep = 6; /* if not, home again */
		}
		break;
	}
	case 8:
	{
		iStep = 9;
		break;
	}
	case 9:
	{
		function<void()> nextStep = bind(&CMachineControl::ImplReset, this, 10, bHasRepairMod);
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowMultiAxisMove, this, nextStep, 0, GetMultiAxisCoord(u8"清洗位置")));
		iStep = -2;
		break;
	}
	case 10:
	{
		function<void()> nextStep = bind(&CMachineControl::ImplReset, this, 11, bHasRepairMod);
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowMultiAxisMove, this, nextStep, 0, GetMultiAxisCoord(u8"清洗高度")));
		iStep = -2;
		break;
	}
	case 11:
	{
		iStep = 12;
		break;
	}
	case 12:
	{
		iStep = 13;
		break;
	}
	case 13:
	{
		if (m_motion.AxisHome(s_iPumpAxisNo) == 0)
		{
			emit signalMsg(QString::fromUtf8(u8"移液轴复位"));
			iStep = 14;
		}
		break;
	}
	case 14:
	{
		if (m_motion.IsHomeCompleted(s_iPumpAxisNo))
		{
			emit signalMsg(QString::fromUtf8(u8"移液轴复位完成"));
			iStep = 15;
		}
		break;
	}
	case 15:
	{
		if(m_motion.MultiAxisMoveSimple(GetMultiAxisCoord(u8"针头安全高度")) == 0)
		{
			emit signalMsg(QString::fromUtf8(u8"往针头安全高度"));
			Sleep(200);
			iStep = 16;
		}	
		break;
	}
	case 16:
	{
		double dVol = GetCommonConfig(STR_SECTION_PUMPPARAM, u8"留空容积");
		double dReso = GetCommonConfig(STR_SECTION_RESOLUTION, to_string(s_iPumpAxisNo));

		if (m_motion.SingleAxisMoveRel(s_iPumpAxisNo, dVol * dReso) == 0)
		{
			emit signalMsg(QString::fromUtf8(u8"柱塞泵（移液轴）往留空位置"));
			iStep = 17;
		}
		break;
	}
	case 17:
	{
		if (!m_motion.IsAxisBusy(s_iPumpAxisNo))
		{
			iStep = 18;
			emit signalMsg(QString::fromUtf8(u8"校准液位传感器"));
		}
		break;
	}
	case 18:
	{
		if (m_SurfaceSensor.Calibrate() == 0)
		{
			emit signalMsg(QString::fromUtf8(u8"液位传感器校准完成：%1").arg(m_SurfaceSensor.GetBase()));
			iStep = 19;
		}
		else
		{
			emit signalMsg(QString::fromUtf8(u8"校准液位传感器失败，重试中"));
			m_SurfaceSensor.Reconnect();
			Sleep(100);
		}
		break;
	}
	case 19:
	{
		if (!m_motion.IsAxisBusy(GetAxisNo(STR_AXIS_NDL)))
		{
			emit signalMsg(QString::fromUtf8(u8"到针头安全高度"));
			iStep = 20;
		}
		else
		{
			//Check Move TimeOut
		}
		break;
	}
	case 20:
	{
		if (m_mapReactMods[1]->RACRelayResetCompletion())
		{
			emit signalMsg(QString::fromUtf8(u8"反应模块1复位完成确认"));
			iStep = 21;
		}
		break;
	}
	case 21:
	{
		if (m_mapReactMods[2]->RACRelayResetCompletion())
		{
			emit signalMsg(QString::fromUtf8(u8"反应模块2复位完成确认"));
			iStep = 22;
		}
		break;
	}
	

	default:
	{
		iStep = -1;
		break;
	}
	}

	if (iStep >= 0)
	{
		boost::asio::post(m_tp, bind(&CMachineControl::ImplReset, this, iStep, bHasRepairMod));
	}
	else if(iStep == -1) /* completed */
	{
		emit signalMsg(QString::fromUtf8(u8"复位完成"));
		SetState(RunState::Stopped);

		boost::asio::post(m_tp, bind(&CMachineControl::ImplTemperatureUpdate2, this, bHasRepairMod));
		boost::asio::post(m_tp, bind(&CMachineControl::ImplAxisCoordUpdate, this));

		emit signalResetFinished();
		emit signalUpdateAllView();
	}
	else if (iStep == -2) /* need to continue */
	{

	}
	else /* TODO - error */
	{

	}
}

void CMachineControl::ImplCheckReactionModule(int iIndex, int iStep)
{
	if (!CheckRunState())
		return;

	switch (iStep)
	{
	case 0:
	{
		/* wait for module */
		if (!m_mapReactMods[iIndex]->Empty())
		{
			iStep = 1;
		}
		else
		{
			Sleep(10);
		}

		break;
	}
	case 1: 
	{
		if (m_mapReactionState[iIndex] == ReactionState::New) 
		{
			iStep = 2;
		}
		else if (m_mapReactionState[iIndex] == ReactionState::Reacting)
		{
			/* check whether the reaction is completed */
			bool bReactCompleted = m_mapReactMods[iIndex]->RACRelayWashCompletion();

			if (bReactCompleted)
			{

				m_mapReactMods[iIndex]->IncStep();
				emit signalUpdateReactionModuleView(iIndex);

				m_mapReactionState[iIndex] = ReactionState::New;
			}
			else
			{
				Sleep(20);
			}
		}
		else
		{
			Sleep(20);
		}
		break;
	}
	case 2:
	{
		lock_guard<mutex> lk(m_mtxReaction);

		if (!m_mapReactMods[iIndex]->Reacting())
		{
			/* all steps finished */
			// nop
		}
		else
		{
			/* enqueue */
			const string strFormula = m_mapReactMods[iIndex]->FormulaName();
			const int iCurStep = m_mapReactMods[iIndex]->CurrentStep();
			const FormulaStep step = formulaInfo.GetFormulaStep(strFormula, iCurStep);
			const string& strType = step.type;
			if (strType == OPTYPE_CREAGENT || strType == OPTYPE_VREAGENT)
			{
				m_qReactionQueue.push(iIndex);
				emit signalMsg(QString::fromUtf8(u8"反应模块%1准备加液").arg(iIndex));
				m_mapReactionState[iIndex] = ReactionState::Pending;
			}
			else if (strType == OPTYPE_WASH)
			{
				emit signalMsg(QString::fromUtf8(u8"反应模块%1清洗（未实现)").arg(iIndex));
				Sleep(1000);
				m_mapReactMods[iIndex]->IncStep();
			}
			else
			{
				emit signalMsg(QString::fromUtf8(u8"反应模块%1步骤类型%2").arg(iIndex).arg(QString::fromUtf8(strType)));
				Sleep(1000);
				m_mapReactMods[iIndex]->IncStep();

			}
		}

		iStep = 3;
		break;
	}
	
	default:
		Sleep(20);
		iStep = -1;
		break;
	}

	boost::asio::post(m_tp, bind(&CMachineControl::ImplCheckReactionModule, this, iIndex, iStep >= 0 ? iStep : 0)); /* run forever */
}

void CMachineControl::ImplCheckRepairModule(int iIndex, int iStep)
{
	if (!CheckRunState())
		return;

	switch (iStep)
	{
	case 0:
	{
		/* waiting for repair */
		if (!m_repairMod->Empty(iIndex) && m_repairMod->IsRepairing(iIndex))
		{
			iStep = 1;
		}
		else
		{
			Sleep(50);
		}
		break;
	}
	case 1: /* wait for completion */
	{
		if (m_repairMod->RACRelayPickShelfPermission(iIndex))
		{
			emit signalMsg(QString::fromUtf8(u8"修复模块%1完成，允许取架").arg(iIndex));
			iStep = 2;
		}
		break;
	}
	case 2: 
	{
		/* update state to dispatch */
		m_repairMod->SetStateCompleted(iIndex);
		iStep = 3;
		break;
	}
	default:
		iStep = -1;
		break;
	}

	boost::asio::post(m_tp, bind(&CMachineControl::ImplCheckRepairModule, this, iIndex, iStep >= 0 ? iStep : 0)); /* run forever */
}

void CMachineControl::ImplCheckRepairModAlarm(int iStep)
{
	if (m_state == RunState::Exiting)
		return;

	static int s_iAlarmCode = 0;
	static int s_iAlarmCode2 = 0;

#ifndef _DEBUG
	if (!CheckRunState())
		return;
#endif

	switch (iStep)
	{
	case 0:
	{
		s_iAlarmCode = 0;
		for (int i = 1; i <= 6; i++)
		{
			if (m_repairMod->ReadRelayAlarm(i))
			{
				s_iAlarmCode |= (1 << (i - 1));
			}
		}
		iStep = 1;
		break;
	}
	case 1:
	{
		if (s_iAlarmCode != 0)
		{
			list<int> listAlarmCode;
			for (int i = 1; i <= 6; i++)
			{
				if (s_iAlarmCode & (1 << (i - 1)))
				{
					m_iUnhandledAlarmCnt++;
					listAlarmCode.push_back(i);
					emit signalRepairModAlarm(i); // TODO - use a single signal instead
				}
			}

			auto iter = listAlarmCode.begin();
			QString strAlarmCodeMsg = QString::number(*iter);
			++iter;
			while (iter != listAlarmCode.end())
			{
				strAlarmCodeMsg.append(QString(", %1").arg(*iter));
				++iter;
			}

			emit signalMsg(QString::fromUtf8(u8"修复模块报警：%1").arg(strAlarmCodeMsg));
			iStep = 2;
		}
		else
		{
			iStep = 1001001;
			Sleep(50);
		}
		break;
	}
	case 1001001:
	{
		s_iAlarmCode2 = 0;
		if (m_repairMod->ReadRelayAlarm(7))
		{
			s_iAlarmCode2 = 1;
			emit signalRepairModAlarm2(7);
			m_iUnhandledAlarmCnt++;
		}
		if (m_repairMod->ReadRelayAlarm(8))
		{
			s_iAlarmCode2 |= 2;
			m_iUnhandledAlarmCnt++;
			emit signalRepairModAlarm2(8);
		}
		if (s_iAlarmCode2 != 0)
		{
			iStep = 2;
		}
		else
		{
			iStep = 0;
		}
		break;

	}
	case 2:
	{
		if (m_iUnhandledAlarmCnt == 0)
		{
			for (int i = 1; i <= 6; i++)
			{
				if (s_iAlarmCode & (1 << (i - 1)))
				{
					m_repairMod->ClearRelayAlarm(i);
				}
			}

			for (int i = 1; i <= 2; i++)
			{
				if (s_iAlarmCode2 & (1 << (i - 1)))
				{
					m_repairMod->ClearRelayAlarm(i + 6);
				}
			}

			emit signalMsg(QString::fromUtf8(u8"修复模块报警：处理完成"));
			iStep = 3;
		}
		else
		{
			Sleep(50);
		}
		break;
	}
	default:
	{
		iStep = -1;
		break;
	}
	}

	boost::asio::post(m_tp, bind(&CMachineControl::ImplCheckRepairModAlarm, this, iStep >= 0 ? iStep : 0)); /* run forever */
}

void CMachineControl::ImplDrain(int iStep)
{
	if (!CheckRunState())
	{
		this->SetReactModDrainPumpOn(1, false);
		this->SetReactModDrainPumpOn(2, false);
		return;
	}

	double dDuration = GetCommonConfig(u8"模块", u8"反应模块排液时长");
	double dInterval = GetCommonConfig(u8"模块", u8"反应模块排液间隔");
	static long long s_llTimeStart;

	switch (iStep)
	{
	case 0:
	{
		if (m_mapReactMods[1]->Empty() && m_mapReactMods[2]->Empty())
		{
			Sleep(500 + rand() % 100);
		}
		else
		{
			iStep = 1000001;
		}	
		break;
	}
	case 1000001:
	{
		//emit signalMsg(QString::fromUtf8(u8"反应模块开始排废液"));
		this->SetReactModDrainPumpOn(1, true);
		this->SetReactModDrainPumpOn(2, true);
		s_llTimeStart = GetProgramTime();
		iStep = 1;
		break;
	}
	case 1:
	{
		if (GetProgramTime() - s_llTimeStart > dDuration)
		{
			//emit signalMsg(QString::fromUtf8(u8"反应模块停止排废液"));
			this->SetReactModDrainPumpOn(1, false);
			this->SetReactModDrainPumpOn(2, false);
			s_llTimeStart = GetProgramTime();
			iStep = 2;
		}
		else
		{
			Sleep(10);
		}
		break;
	}
	case 2:
	{
		if (GetProgramTime() - s_llTimeStart > dInterval)
		{
			iStep = 3;
		}
		else
		{
			Sleep(10);
		}
		break;
	}
	default:
	{
		iStep = -1;
		break;
	}
	}

	boost::asio::post(m_tp, bind(&CMachineControl::ImplDrain, this, iStep >= 0 ? iStep : 0)); /* run forever */
}

void CMachineControl::ImplCleanerSlotDrain(int iStep)
{
	if (!CheckRunState())
	{
		this->SetNeedleCleanerPumpOn(false);
		return;
	}

	static long long s_llSimeStart;

	switch (iStep)
	{
	case 0:
	{
		this->SetCleanerSlotDrainPumpOn(true);
		s_llSimeStart = GetProgramTime();
		iStep = 1;
		break;
	}
	case 1:
	{
		if (GetProgramTime() - s_llSimeStart > GetCommonConfig(u8"加液", u8"清洗槽排液时长"))
		{
			this->SetCleanerSlotDrainPumpOn(false);
			iStep = -1;
		}
		else
		{
			Sleep(10);
		}
		break;
	}
	default:
	{
		iStep = -1;
		break;
	}
	}

	if (iStep >= 0)
	{
		boost::asio::post(m_tp, bind(&CMachineControl::ImplCleanerSlotDrain, this, iStep)); 
	}
}

void CMachineControl::ImplPickShelf(int iStep)
{
	static int s_iPickFailCnt = 0;
	static int s_iShelfIndex = 0;
	static long long int s_llStartTime = 0;
	if (!CheckRunState())
		return;

	switch (iStep)
	{
	case 0:
	{		
		if (this->HasWaitingShelfHolder())
		{
			s_iShelfIndex = this->GetOneWaitingShelfHolder();
			emit signalMsg(QString::fromUtf8(u8"玻片架%1取架开始").arg(s_iShelfIndex));
			iStep = 1;
		}
		break;
	}
	case 1:
	{
		function<void()> nextStep = bind(&CMachineControl::ImplPickShelf, this, 2);
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowZHome, this, nextStep, 0));
		iStep = -1;
		break;
	}
	case 2:
	{
		iStep = 3;
		break;
	}
	case 3:
	{
		/* 到提架位置 */
		function<void()> nextStep = bind(&CMachineControl::ImplPickShelf, this, 4);
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowMultiAxisMove, this, nextStep, 0, GetMultiAxisCoord(STR_SHELF_POS(s_iShelfIndex))));
		iStep = -1;
		break;
	}
	case 4:
	{
		iStep = 5;
		break;
	}
	case 5: /* 下降 */
	{
		function<void()> nextStep = bind(&CMachineControl::ImplPickShelf, this, 6);
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowMultiAxisMove, this, nextStep, 0, GetMultiAxisCoord(STR_SHELF_HEIGHT(s_iShelfIndex))));
		iStep = -1;
		break;
	}
	case 6:
	{
		iStep = 7;
		break;
	}
	case 7: /* 开电磁铁 */
	{
		iStep = 11; 
		SetElectroMagnet(true);
		break;
	}
	case 11: /* 上升 */
	{
		function<void()> nextStep = bind(&CMachineControl::ImplPickShelf, this, 1011001);
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowZAxesUp, this, nextStep, 0));
		iStep = -1;
		break;
	}
	case 1011001:
	{
		bool bShelfSensor = m_motion.ReadInBit(0, stoi(iniIO[u8"运动控制卡通用输出"][u8"提架传感器"])) == 0;
		if (bShelfSensor)
		{
			iStep = 12;
		}
		else
		{
			if (s_iPickFailCnt < 10)
			{
				iStep = 5;
				s_iPickFailCnt++;
				break;
			}
			else
			{
				s_iPickFailCnt = 0;
				int iGotoRetry = 5;
				int iGotoIgnore = 12;
				if (this->AskQuestion(QString::fromUtf8(u8"提架时未检测到传感器信号。重试（Yes）/忽略（No）？")) == QuestionAnswer::Yes)
				{
					iStep = iGotoRetry;
				}
				else
				{
					iStep = iGotoIgnore;
				}
			}
		}
		break;
	}
	case 12:
	{
		unique_lock<mutex> lk(m_mtxShelf);
		m_transit->Pick(m_mapShelfHolders[s_iShelfIndex]->TakeShelf());
		lk.unlock();

		emit signalMsg(QString::fromUtf8(u8"玻片架%1取架完成").arg(s_iShelfIndex));
		emit signalUpdateShelfHolderView(s_iShelfIndex);
		emit signalUpdateTransitShelfView();
		if (m_transit->NeedsRepair() && stoi(iniCfg[u8"模块"][u8"有修复模块"]) == 1)
		{
			boost::asio::post(m_tp, bind(&CMachineControl::ImplPutToRepair, this, 0));
		}
		else
		{
			boost::asio::post(m_tp, bind(&CMachineControl::ImplPutToReaction, this, 0));
		}
		iStep = 13;
		break;
	}
	default:
		iStep = -1;
		break;
	}
	
	Sleep(1);
	if (iStep >= 0)
	{
		boost::asio::post(m_tp, bind(&CMachineControl::ImplPickShelf, this, iStep));
	}
}

void CMachineControl::ImplPickFromReaction(int iStep)
{
	static int s_iPickFailCnt = 0;
	static int s_iReactModIndex = 0;
	static long long int s_llStartTime = 0;
	if (!CheckRunState())
		return;

	switch (iStep)
	{
	case 0:
	{
		s_iReactModIndex = GetOneCompletedReactionModule();
		emit signalMsg(QString::fromUtf8(u8"反应模块%1开始取架").arg(s_iReactModIndex));
		// maybe open the lid
		iStep = 1;
		break;
	}
	case 1:
	{
		function<void()> nextStep = bind(&CMachineControl::ImplPickFromReaction, this, 2);
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowZHome, this, nextStep, 0));
		iStep = -1;
		break;
	}
	case 2:
	{
		function<void()> nextStep = bind(&CMachineControl::ImplPickFromReaction, this, 3);
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowMultiAxisMove, this, nextStep, 0, GetMultiAxisCoord(STR_REACTMOD_POS(s_iReactModIndex))));
		iStep = -1;
		break;
	}
	case 3:
	{
		iStep = 4;
		break;
	}
	case 4:
	{
		if (m_mapReactMods[s_iReactModIndex]->WriteRelayPickShelfRequest() == 0)
		{
			iStep = 1004001;
		}
		else
		{
			Sleep(20);
		}
		break;
	}	
	case 1004001:
	{
		if (m_mapReactMods[s_iReactModIndex]->RACRelayPickShelfPermission())
		{
			iStep = 5;
		}
		else
		{
			Sleep(20);
		}
		break;
	}
	case 5:
	{
		function<void()> nextStep = bind(&CMachineControl::ImplPickFromReaction, this, 6);
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowMultiAxisMove, this, nextStep, 0, GetMultiAxisCoord(STR_REACTMOD_PICK_HEIGHT(s_iReactModIndex))));
		iStep = -1;
		break;
		
	}
	case 6:
	{
		iStep = 7;
		break;
	}
	case 7:
	{
		SetElectroMagnet(true);
		iStep = 8;
		break;
	}
	case 8:
	{
		function<void()> nextStep = bind(&CMachineControl::ImplPickFromReaction, this, 1008001);
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowZAxesUp, this, nextStep, 0));
		iStep = -1;
		break;
	}
	case 1008001:
	{
		bool bShelfSensor = m_motion.ReadInBit(0, stoi(iniIO[u8"运动控制卡通用输出"][u8"提架传感器"])) == 0;
		if (bShelfSensor)
		{
			iStep = 9;
		}
		else
		{
			if (s_iPickFailCnt < 10)
			{
				iStep = 5;
				s_iPickFailCnt++;
				break;
			}
			else
			{
				s_iPickFailCnt = 0;
				int iGotoRetry = 5;
				int iGotoIgnore = 9;
				if (this->AskQuestion(QString::fromUtf8(u8"提架时未检测到传感器信号。重试（Yes）/忽略（No）？")) == QuestionAnswer::Yes)
				{
					iStep = iGotoRetry;
				}
				else
				{
					iStep = iGotoIgnore;
				}
			}
		}
		break;
	}
	case 9:
	{
		if (m_mapReactMods[s_iReactModIndex]->WriteRelayPickShelfCompletion() == 0)
		{
			iStep = 10;
		}
		else
		{
			Sleep(20);
		}
		break;
	}
	case 10:
	{
		unique_lock<mutex> lk(m_mtxShelf);
		m_transit->Pick(m_mapReactMods[s_iReactModIndex]->TakeShelf());
		emit signalMsg(QString::fromUtf8(u8"反应模块%1取架完成").arg(s_iReactModIndex));
		lk.unlock();
		emit signalUpdateTransitShelfView();
		emit signalUpdateReactionModuleView(s_iReactModIndex);

		boost::asio::post(m_tp, bind(&CMachineControl::ImplPutShelf, this, 0));
		iStep = 11;
		break;
	}

	default:
		iStep = -1;
		break;
	}

	Sleep(1);
	if (iStep >= 0)
	{
		boost::asio::post(m_tp, bind(&CMachineControl::ImplPickFromReaction, this, iStep));
	}
	
}

void CMachineControl::ImplPickFromRepair(int iStep)
{
	static int s_iPickFailCnt = 0;
	static int s_iRepairModIndex = 0;
	static long long int s_llStartTime = 0;
	if (!CheckRunState())
		return;

	switch (iStep)
	{
	case 0:
	{
		s_iRepairModIndex = GetOneCompletedRepairModule();
		emit signalMsg(QString::fromUtf8(u8"修复模块%1开始取架").arg(s_iRepairModIndex));
		iStep = 1;
		break;
	}
	case 1:
	{
		function<void()> nextStep = bind(&CMachineControl::ImplPickFromRepair, this, 2);
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowZHome, this, nextStep, 0));
		iStep = -1;
		break;
	}
	case 2:
	{
		iStep = 3;
		break;
	}
	case 3:
	{
		function<void()> nextStep = bind(&CMachineControl::ImplPickFromRepair, this, 4);
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowMultiAxisMove, this, nextStep, 0, GetMultiAxisCoord(STR_REPAIRMOD_POS(s_iRepairModIndex))));
		iStep = -1;
		break;
	}
	case 4:
	{
		iStep = 5;
		break;
	}
	case 5:
	{
		function<void()> nextStep = bind(&CMachineControl::ImplPickFromRepair, this, 6);
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowMultiAxisMove, this, nextStep, 0, GetMultiAxisCoord(STR_REPAIRMOD_HEIGHT(s_iRepairModIndex))));
		iStep = -1;
		break;
	}
	case 6:
	{
		iStep = 7;
		break;
	}
	case 7:
	{
		SetElectroMagnet(true);
		iStep = 8;
		break;
	}
	case 8:
	{

		function<void()> nextStep = bind(&CMachineControl::ImplPickFromRepair, this, 1008001);
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowZAxesUp, this, nextStep, 0));
		iStep = -1;
		break;
	}
	case 1008001:
	{		
		bool bShelfSensor = m_motion.ReadInBit(0, stoi(iniIO[u8"运动控制卡通用输出"][u8"提架传感器"])) == 0;
		if (bShelfSensor)
		{
			iStep = 9;
		}
		else
		{
			if (s_iPickFailCnt < 10)
			{
				iStep = 5;
				s_iPickFailCnt++;
				break;
			}
			else
			{
				s_iPickFailCnt = 0;
				int iGotoRetry = 5;
				int iGotoIgnore = 9;
				if (this->AskQuestion(QString::fromUtf8(u8"提架时未检测到传感器信号。重试（Yes）/忽略（No）？")) == QuestionAnswer::Yes)
				{
					iStep = iGotoRetry;
				}
				else
				{
					iStep = iGotoIgnore;
				}
			}
		}
		break;
	}
	case 9:
	{
		if (m_repairMod->WriteRelayPickShelfCompletion(s_iRepairModIndex) == 0)
		{
			iStep = 10;
		}
		break;
	}
	case 10:
	{
		unique_lock<mutex> lk(m_mtxShelf);
		m_transit->Pick(m_repairMod->TakeShelf(s_iRepairModIndex));
		emit signalMsg(QString::fromUtf8(u8"修复模块%1取架完成").arg(s_iRepairModIndex));
		lk.unlock();
		emit signalUpdateRepairModuleView(s_iRepairModIndex);
		emit signalUpdateTransitShelfView();
		iStep = 11;
		break;
	}
	case 11:
	{
		iStep = 12;
		break;
	}
	case 12:
	{		
		boost::asio::post(m_tp, bind(&CMachineControl::ImplPutToReaction, this, 0));
		iStep = 13;
		break;
	}

	default:
		iStep = -1;
		break;
	}

	Sleep(1);
	if (iStep >= 0)
	{
		boost::asio::post(m_tp, bind(&CMachineControl::ImplPickFromRepair, this, iStep));
	}
}

void CMachineControl::ImplPutShelf(int iStep)
{
	static int s_iShelfIndex = 0;
	static long long int s_llStartTime = 0;
	if (!CheckRunState())
		return;

	switch (iStep)
	{
	case 0:
	{
		s_iShelfIndex = GetOneEmptyShelfHolder();
		emit signalMsg(QString::fromUtf8(u8"玻片架%1开始放架").arg(s_iShelfIndex));
		iStep = 1;
		break;
	}
	case 1:
	{
		iStep = 2;
		break;
	}
	case 2:
	{
		function<void()> nextStep = bind(&CMachineControl::ImplPutShelf, this, 3);
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowZAxesUp, this, nextStep, 0));
		iStep = -1;
		break;
	}
	case 3: /* move to shelf holder */
	{
		function<void()> nextStep = bind(&CMachineControl::ImplPutShelf, this, 4);
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowMultiAxisMove, this, nextStep, 0, GetMultiAxisCoord(STR_SHELF_POS(s_iShelfIndex))));
		iStep = -1;
		break;
	}
	case 4:
	{
		iStep = 5;
		break;
	}
	case 5: /* to correct altitude */
	{
		function<void()> nextStep = bind(&CMachineControl::ImplPutShelf, this, 6);
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowMultiAxisMove, this, nextStep, 0, GetMultiAxisCoord(STR_SHELF_HEIGHT(s_iShelfIndex))));
		iStep = -1;
		break;
	}
	case 6:
	{
		iStep = 7;
		break;
	}
	case 7: /* put */
	{		
		this->SetElectroMagnet(false);
		iStep = 8;
		break;
	}
	case 8: 
	{
		function<void()> nextStep = bind(&CMachineControl::ImplPutShelf, this, 9);
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowZAxesUp, this, nextStep, 0));
		iStep = -1;
		break;
	}
	case 9:
	{
		unique_lock<mutex> lk(m_mtxShelf);
		m_mapShelfHolders[s_iShelfIndex]->PutShelf(m_transit->Put());
		emit signalMsg(QString::fromUtf8(u8"玻片架%1放架完成").arg(s_iShelfIndex));
		lk.unlock();
		emit signalUpdateShelfHolderView(s_iShelfIndex);
		emit signalUpdateTransitShelfView();
		function<void()> nextStep = bind(&CMachineControl::Dispatch, this);
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowZAxesUp, this, nextStep, 0));
		iStep = 10;
		break;
	}

	default:
		iStep = -1;
	}
	
	Sleep(1);
	if (iStep >= 0)
	{
		boost::asio::post(m_tp, bind(&CMachineControl::ImplPutShelf, this, iStep));
	}
}

void CMachineControl::ImplPutToReaction(int iStep)
{
	static int s_iReactModIndex = 0;
	static long long int s_llStartTime = 0;
	if (!CheckRunState())
		return;

	switch (iStep)
	{
	case 0:
	{
		function<void()> nextStep = bind(&CMachineControl::ImplPutToReaction, this, 1);
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowZAxesUp, this, nextStep, 0));
		iStep = -1;
		break;
	}
	case 1:
	{
		s_iReactModIndex = GetOneEmptyReactionModule();
		emit signalMsg(QString::fromUtf8(u8"反应模块%1放架开始").arg(s_iReactModIndex));
		iStep = 2;
		break;
	}
	case 2:
	{
		if (m_mapReactMods[s_iReactModIndex]->WriteRelayPutShelfRequest() == 0)
		{
			iStep = 1002001;
		}
		else
		{
			Sleep(1);
		}
		break;
	}
	case 1002001:
	{
		if (m_mapReactMods[s_iReactModIndex]->RACRelayPutShelfPermission())
		{
			iStep = 1002002;
		}
		else
		{
			Sleep(1);
		}
		break;
	}
	case 1002002:
	{
		if (m_mapReactMods[s_iReactModIndex]->RACRelayLidOpening())
		{
			iStep = 1002003;
		}
		else
		{
			iStep = 1002003; // !
			Sleep(1);
		}
		break;
	}
	case 1002003:
	{
		function<void()> nextStep = bind(&CMachineControl::ImplPutToReaction, this, 1002004);
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowMultiAxisMove, this, nextStep, 0, GetMultiAxisCoord(STR_REACTMOD_PRE_POS(s_iReactModIndex))));
		iStep = -1;
		break;
	}
	case 1002004:
	{
		function<void()> nextStep = bind(&CMachineControl::ImplPutToReaction, this, 3);
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowMultiAxisMove, this, nextStep, 0, GetMultiAxisCoord(STR_REACTMOD_PRE_PUT_HEIGHT(s_iReactModIndex))));
		iStep = -1;
		break;
	}
	case 3:
	{
		function<void()> nextStep = bind(&CMachineControl::ImplPutToReaction, this, 4);
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowMultiAxisMove, this, nextStep, 0, GetMultiAxisCoord(STR_REACTMOD_POS(s_iReactModIndex))));
		iStep = -1;
		break;
	}
	case 4:
	{
		iStep = 5;
		break;
	}
	case 5:
	{
		function<void()> nextStep = bind(&CMachineControl::ImplPutToReaction, this, 6);
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowMultiAxisMove, this, nextStep, 0, GetMultiAxisCoord(STR_REACTMOD_PUT_HEIGHT(s_iReactModIndex))));
		iStep = -1;
		break;
	}
	case 6:
	{
		iStep = 7;
		break;
	}
	case 7:
	{
		/* 关电磁铁 */
		SetElectroMagnet(false);
		iStep = 8;
		break;
	}
	case 8:
	{
		function<void()> nextStep = bind(&CMachineControl::ImplPutToReaction, this, 9);
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowZAxesUp, this, nextStep, 0));
		iStep = -1;
		break;
	}
	case 9:
	{
		if (m_mapReactMods[s_iReactModIndex]->WriteRelayPutShelfCompletion() == 0)
		{
			iStep = 10;
		}
		break;
	}
	case 10:
	{
		unique_lock<mutex> lk1(m_mtxReaction);
		m_mapReactionState[s_iReactModIndex] = ReactionState::New;
		lk1.unlock();

		unique_lock<mutex> lk(m_mtxShelf);
		m_mapReactMods[s_iReactModIndex]->PutShelf(m_transit->Put());
		emit signalMsg(QString::fromUtf8(u8"反应模块%1放架完成").arg(s_iReactModIndex));
		lk.unlock();

		emit signalUpdateReactionModuleView(s_iReactModIndex);
		emit signalUpdateTransitShelfView();
		boost::asio::post(m_tp, bind(&CMachineControl::Dispatch, this));
		iStep = 17;
		break;
	}

	default:
		iStep = -1;
		break;
	}

	Sleep(1);
	if (iStep >= 0)
	{
		boost::asio::post(m_tp, bind(&CMachineControl::ImplPutToReaction, this, iStep));
	}
}

void CMachineControl::ImplPutToRepair(int iStep)
{
	static int s_iRepairModIndex = 0;
	static long long int s_llStartTime = 0;
	if (!CheckRunState())
		return;

	switch (iStep)
	{
	case 0:
	{
		s_iRepairModIndex = GetOneEmptyRepairModule();
		emit signalMsg(QString::fromUtf8(u8"修复模块%1放架开始").arg(s_iRepairModIndex));
		iStep = 1;
		break;
	}
	case 1:
	{
		function<void()> nextStep = bind(&CMachineControl::ImplPutToRepair, this, 2);
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowMultiAxisMove, this, nextStep, 0, GetMultiAxisCoord(STR_REPAIRMOD_POS(s_iRepairModIndex))));
		iStep = -1;
		break;
	}
	case 2:
	{
		iStep = 1002001;
		break;
	}
	case 1002001:
	{
		if (m_repairMod->WriteRelayFlowStart() == 0)
		{
			iStep = 1002002;
		}
		else
		{
			Sleep(20);
		}
		break;
	}
	case 1002002:
	{
		if (m_repairMod->RACRelayPutShelfPermission(s_iRepairModIndex))
		{
			iStep = 3;
		}
		break;
	}
	case 3:
	{
		function<void()> nextStep = bind(&CMachineControl::ImplPutToRepair, this, 4);
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowMultiAxisMove, this, nextStep, 0, GetMultiAxisCoord(STR_REPAIRMOD_PUT_POS(s_iRepairModIndex))));
		iStep = -1;
		break;
	}
	case 4:
	{
		iStep = 5;
		break;
	}
	case 5:
	{
		/* 关电磁铁 */
		SetElectroMagnet(false);
		iStep = 6;
		break;
	}
	case 6: 
	{
		function<void()> nextStep = bind(&CMachineControl::ImplPutToRepair, this, 7);
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowZAxesUp, this, nextStep, 0));
		iStep = -1;
		break;
	}
	case 7:
	{
		unique_lock<mutex> lk(m_mtxShelf);
		m_repairMod->PutShelf(s_iRepairModIndex, m_transit->Put());
		emit signalMsg(QString::fromUtf8(u8"修复模块%1放架完成").arg(s_iRepairModIndex));
		lk.unlock();
		iStep = 8;
		break;
	}
	case 8:
	{
		if (m_repairMod->WriteRepairParam(s_iRepairModIndex) == 0) /* params stored in repairMod->shelf */
		{
			int iTemp = m_repairMod->GetShelf(s_iRepairModIndex)->m_iRepairTemp;
			int iTime = m_repairMod->GetShelf(s_iRepairModIndex)->m_iRepairTime;
			emit signalMsg(QString::fromUtf8(u8"修复模块%1：温度%2℃，时间%3s").arg(s_iRepairModIndex).arg(iTemp).arg(iTime));
			iStep = 12;
		}
		break;
	}
	case 12:
	{
		/* should be constant, but still re-write each time */
		if (m_repairMod->WriteDTTempPickShelf(s_iRepairModIndex, GetCommonConfig(u8"模块", u8"修复模块提架温度")) == 0)  
		{
			iStep = 13;
		}
		break;
	}
	case 13:
	{
		if (m_repairMod->WriteRelayPutShelfCompletion(s_iRepairModIndex) == 0) /* complete putting and start timing */
		{
			iStep = 14;
		}
		break;
	}
	case 14:
	{
		m_repairMod->SetStateRepairing(s_iRepairModIndex);
		emit signalUpdateRepairModuleView(s_iRepairModIndex);
		emit signalUpdateTransitShelfView();
		boost::asio::post(m_tp, bind(&CMachineControl::Dispatch, this));
		iStep = 15;
		break;
	}


	default:
		iStep = -1;
		break;
	}

	Sleep(1);
	if (iStep >= 0)
	{
		boost::asio::post(m_tp, bind(&CMachineControl::ImplPutToRepair, this, iStep));
	}	
}

void CMachineControl::ImplPipet(int iStep, int iReactModIndex)
{
	static long long int s_llStartTime = 0;

	struct ReagentUsage
	{
		string strReagent;
		double dTime;
		double dQuantity;
	};
	static map<int, ReagentUsage> s_mapReagentUsage;
	static map<int, ReagentUsage>::iterator s_iterReagentUsage;
	static int s_iReagentPos;
	/* 
		(add one reagent one step)
		for each reagent
			move to get reagent
			add reagent to slide
	*/
	if (!CheckRunState())
		return;

	switch (iStep)
	{	
	case 0: /* setup 'how to pipet' */
	{
		lock_guard<mutex> lk(m_mtxReaction);
		if (m_mapReactionState[iReactModIndex] == ReactionState::Pending)
		{
			m_mapReactionState[iReactModIndex] = ReactionState::Pipetting;
			int iCurReactStep = m_mapReactMods[iReactModIndex]->CurrentStep();
			emit signalMsg(QString::fromUtf8(u8"反应模块%1开始加液：步骤%2/%3").arg(iReactModIndex).arg(1 + iCurReactStep).arg(m_mapReactMods[iReactModIndex]->TotalStep()));

			s_mapReagentUsage.clear();
			for (int i = 1; i <= 10; i++)
			{
				if (m_mapReactMods[iReactModIndex]->HasSlideAt(i))
				{
					const string strFormulaName = m_mapReactMods[iReactModIndex]->FormulaName();
					FormulaStep step = formulaInfo.GetFormulaStep(strFormulaName, iCurReactStep);

					ReagentUsage usage;
					usage.dQuantity = step.quantity;
					usage.dTime = step.reactionTime;

					if (step.type == OPTYPE_CREAGENT)
					{
						usage.strReagent = step.reagent;
					}
					else if (step.type == OPTYPE_VREAGENT)
					{
						usage.strReagent = m_mapReactMods[iReactModIndex]->GetVReagent(i, iCurReactStep);
					}
										
					s_mapReagentUsage[i] = usage;
				}
			}
			iStep = 1000001;
		}
		break;
	}
	case 1000001:
	{
		if (m_mapReactMods[iReactModIndex]->WriteRelayPipetRequest() == 0)
		{
			iStep = 1000002;
		}
		break;
	}
	case 1000002:
	{
		if (m_mapReactMods[iReactModIndex]->RACRelayPipetPermission())
		{
			iStep = 1000003;
		}
		break;
	}
	case 1000003:
	{
		if (m_mapReactMods[iReactModIndex]->WriteRelayPipetting() == 0)
		{
			iStep = 1;
		}
		break;
	}
	case 1:
	{
		s_iterReagentUsage = s_mapReagentUsage.begin();
		iStep = 2;
		break;
	}
	case 2: /* begin of pipet-loop */
	{
		const string& strReagentName = s_iterReagentUsage->second.strReagent;
		if (strReagentName.empty())
		{
			iStep = 24;
			++s_iterReagentUsage;
			break;
		}
		else
		{
			double dQuantity = s_iterReagentUsage->second.dQuantity;
			s_iReagentPos = rmgr.GetReagentPosition(strReagentName);
			if (s_iReagentPos > 0)
			{
				iStep = 3;
			}
			else
			{
				/* can't find reagent */
				this->Suspend();
				emit signalInterrupt(QString::fromUtf8(u8"找不到试剂%1").arg(QString::fromUtf8(strReagentName)));
				//abort();
			}

			break;
		}
	}
	case 3:
	{
		if (m_motion.MultiAxisMoveSimple(GetMultiAxisCoord(string(u8"试剂架试剂").append(to_string(s_iReagentPos)))) == 0)
		{
			iStep = 4;
			s_llStartTime = GetProgramTime();
		}
		break;
	}
	case 4:
	{
		if (m_motion.IsAxisBusy(GetMultiAxisCoord(string(u8"试剂架试剂").append(to_string(s_iReagentPos)))))
		{
			CHECK_MOTION_TIMEOUT
		}
		else
		{
			iStep = 5;
		}
		break;
	}
	case 5:
	{
		iStep = 1005001;
		break;
	}
	case 1005001:
	{
		iStep = 1005002;
		break;
	}
	case 1005002:
	{
		iStep = 1005003;
		break;
	}
	case 1005003: /* 寻找液面 */
	{
		if (m_motion.MultiAxisMoveSimple(GetMultiAxisCoord(u8"试剂架试剂高度下界")) == 0)
		{
			/* skip looking for surface and directly goto buttom */
			iStep = 1005008;
			break;

			int iNeedleAxisNo = GetAxisNo(STR_AXIS_NDL);
			double dReso = GetCommonConfig(STR_SECTION_RESOLUTION, to_string(iNeedleAxisNo));
			double dAttemptBeginHeight = (m_mapLatestReagentSurfaceAltitude.find(s_iReagentPos) != m_mapLatestReagentSurfaceAltitude.end()) ? (m_mapLatestReagentSurfaceAltitude[s_iReagentPos] - 15 * dReso) : (GetMultiAxisCoord(u8"试剂架试剂高度上界").at(iNeedleAxisNo) - 5 * dReso);
			while (true) /* 往特定高度，减速 */
			{
				double dPos = GetAxisPos(iNeedleAxisNo);
				if (dPos > dAttemptBeginHeight)
				{
					int iRet = m_motion.ChangSpeed(0, iNeedleAxisNo, 4000, 0.2);
					break;
				}
			}

			while (true) /* 检测液面 */
			{
				if (m_SurfaceSensor.SurfaceTouched())
				{
					m_motion.AxisStop(0, iNeedleAxisNo, 1);
					int iRet = m_motion.ChangSpeed(0, iNeedleAxisNo, 30000, 0.2);
					double dPos = GetAxisPos(iNeedleAxisNo);
					//emit signalMsg(QString::fromUtf8(u8"液面：%1").arg(dPos));
					m_mapLatestReagentSurfaceAltitude[s_iReagentPos] = dPos;
					iStep = 1005007;
					break;
				}
				else if (!m_motion.IsAxisBusy(iNeedleAxisNo)) /* axis stopped; can't find reagent */
				{
					rmgr.SetUsedUp(s_iReagentPos);
					m_motion.ChangSpeed(0, iNeedleAxisNo, 30000, 0.1);
					m_motion.MultiAxisMoveSimple(GetMultiAxisCoord(u8"针头安全高度"));
					//int iPipAxisNo = GetAxisNo(STR_AXIS_PIP); /* 抽空气 */
					//m_motion.SingleAxisMoveRel(iPipAxisNo, -1000.0 * GetCommonConfig(STR_SECTION_RESOLUTION, to_string(iPipAxisNo)));
					iStep = 1005006;
					break;
					emit signalMsg(QString::fromUtf8(u8"位置[%1] 试剂[%2] 用完").arg(s_iReagentPos).arg(QString::fromUtf8(s_iterReagentUsage->second.strReagent)));
				}
				else
					continue;
				
			}
		}
		break;
	}
	case 1005006: /* 未找到液面，回2重找 */
	{
		int iPipAxisNo = GetAxisNo(STR_AXIS_PIP);
		if (!m_motion.IsAxisBusy(GetMultiAxisCoord(u8"针头安全高度")) && !m_motion.IsAxisBusy(iPipAxisNo))
		{
			iStep = 2; 
		}
		break;
	}
	case 1005007: /* 找到液面 */
	{
		/*下降2mm */
		double dPos =
			GetCommonConfig(STR_SECTION_RESOLUTION, to_string(GetAxisNo(STR_AXIS_NDL))) * 2;
		if (m_motion.SingleAxisMoveRel(GetAxisNo(STR_AXIS_NDL), dPos) == 0)
		{
			iStep = 1005008;
		}
		break;
	}
	case 1005008:
	{
		if (!m_motion.IsAxisBusy(GetMultiAxisCoord(u8"试剂架试剂高度下界")))
		{
			iStep = 6;
		}
		break;
	}
	case 6:
	{
		double dNeedleHeightFactor = 
			GetCommonConfig(STR_SECTION_RESOLUTION, to_string(GetAxisNo(STR_AXIS_NDL))) 
			/ GetCommonConfig(u8"加液", u8"试管横截面积");		// height per ul
		double dPumpAxisFactor = GetCommonConfig(STR_SECTION_RESOLUTION, to_string(GetAxisNo(STR_AXIS_PIP))); // units/ul
		double dQuantity = s_iterReagentUsage->second.dQuantity;

		WORD wAxes[2] = { stoi(iniCfg[STR_SECTION_AXIS][STR_AXIS_NDL]), stoi(iniCfg[STR_SECTION_AXIS][STR_AXIS_PIP]) };
		double dDist[2] = { dQuantity * dNeedleHeightFactor, dQuantity * dPumpAxisFactor };
		if (m_motion.LineMulticoor(0, 1, 2, wAxes, dDist, 0) == 0)
		{
			iStep = 7;
			s_llStartTime = GetProgramTime();
		}
		double dPos = -1;
		m_motion.GetPosition(0, stoi(iniCfg[STR_SECTION_AXIS][STR_AXIS_NDL]), dPos);
		m_mapLatestReagentSurfaceAltitude[s_iReagentPos] = dPos;
		emit signalMsg(QString::fromUtf8(u8"反应模块%1加液：%2ul试剂【%3】").arg(iReactModIndex).arg(dQuantity).arg(QString::fromUtf8(s_iterReagentUsage->second.strReagent)));
		break;
	}
	case 7:
	{
		if (m_motion.IsAxisBusy(GetAxisNo(STR_AXIS_NDL)) || m_motion.IsAxisBusy(stoi(iniCfg[STR_SECTION_AXIS][STR_AXIS_PIP])))
		{
			CHECK_MOTION_TIMEOUT
		}
		else
		{
			iStep = 8;
		}
		break;
	}
	case 8:
	{
		function<void()> nextStep = bind(&CMachineControl::ImplPipet, this, 9, iReactModIndex);
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowZAxesUp, this, nextStep, 0));
		iStep = -1;
		break;
	}
	case 9: /* move to slide */
	{
		int iSlidePos = s_iterReagentUsage->first;
		const MultiAxisCoord& coord = GetMultiAxisCoord(string(u8"反应模块").append(to_string(iReactModIndex)).append(u8"玻片").append(to_string(iSlidePos)));		

		if (m_motion.MultiAxisMoveSimple(coord) == 0)
		{
			iStep = 10;
			s_llStartTime = GetProgramTime();
		}
		break;
	}
	case 10:
	{
		int iSlidePos = s_iterReagentUsage->first;
		const MultiAxisCoord& coord = GetMultiAxisCoord(string(u8"反应模块").append(to_string(iReactModIndex)).append(u8"玻片").append(to_string(iSlidePos)));

		if (m_motion.IsAxisBusy(coord))
		{
			CHECK_MOTION_TIMEOUT
		}
		else
		{
			iStep = 11;
		}
		break;
	}
	case 11: /* to pipet height */
	{
		if (m_motion.MultiAxisMoveSimple(GetMultiAxisCoord(STR_PIPET_HEIGHT(iReactModIndex))) == 0)
		{
			iStep = 12;
			s_llStartTime = GetProgramTime();
		}
		break;
	}
	case 12:
	{
		if (m_motion.IsAxisBusy(GetMultiAxisCoord(STR_PIPET_HEIGHT(iReactModIndex))))
		{
			CHECK_MOTION_TIMEOUT
		}
		else
		{
			iStep = 13;
		}
		break;
	}
	case 13:
	{
		/* 加液 */
		double dXAxisFactor = GetCommonConfig(STR_SECTION_RESOLUTION, to_string(GetAxisNo(STR_AXIS_X)));
		double dPumpAxisFactor = GetCommonConfig(STR_SECTION_RESOLUTION, to_string(GetAxisNo(STR_AXIS_PIP)));
		double dQuantity = s_iterReagentUsage->second.dQuantity;

		WORD wAxes[2] = { GetAxisNo(STR_AXIS_X), GetAxisNo(STR_AXIS_PIP) };
		double dDist[2] = { 0 * 20.0 * dXAxisFactor, -(dQuantity * dPumpAxisFactor) }; /* needn't move x-axis */
		if (m_motion.LineMulticoor(0, 1, 2, wAxes, dDist, 0) == 0)
		{
			emit signalMsg(QString::fromUtf8(u8"反应模块%1加液：玻片%2").arg(iReactModIndex).arg(s_iterReagentUsage->first));
			iStep = 1013001;
			s_llStartTime = GetProgramTime();
		}
		break;
	}

	case 1013001:
	{
		/* 加液 */
		if (m_motion.GetAxisDoneMultcoor(0, 1) == 0)
		{
			CHECK_MOTION_TIMEOUT
		}
		else
		{
			iStep = 14;
		}
		break;
	}

	case 14:
	{
		iStep = 1014001;
		break;
	}
	case 1014001:
	{
		if (m_mapReactMods[iReactModIndex]->WriteDTReactionTime(s_iterReagentUsage->first, s_iterReagentUsage->second.dTime) == 0) /* 写反应时间 */
		{
			iStep = 1014002;
		}
		break;
	}
	case 1014002:
	{
		if (m_mapReactMods[iReactModIndex]->WriteRelayStartTiming(s_iterReagentUsage->first) == 0) /* 开始计时 */
		{
			iStep = 15;
		}
		break;
	}
	case 15:
	{
		int iNextStep = 16;
		auto nextIter = s_iterReagentUsage;
		++nextIter;
		/* if same reagent as previous step, skip cleaning */
		if (nextIter != s_mapReagentUsage.end() && nextIter->second.strReagent == s_iterReagentUsage->second.strReagent)
		{
			iNextStep = 24;
		}
		function<void()> nextStep = bind(&CMachineControl::ImplPipet, this, iNextStep, iReactModIndex);
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowZAxesUp, this, nextStep, 0));
		iStep = -1;
		break;
	}
	case 16: /* 到清洗位置 */
	{
		if (m_motion.MultiAxisMoveSimple(GetMultiAxisCoord(STR_REACT_CLEAN_POS(iReactModIndex))) == 0)
		{
			iStep = 17;
			s_llStartTime = GetProgramTime();
		}
		break;
	}
	case 17:
	{
		if (m_motion.IsAxisBusy(GetMultiAxisCoord(STR_REACT_CLEAN_POS(iReactModIndex))))
		{
			CHECK_MOTION_TIMEOUT
		}
		else
		{
			iStep = 18;
		}
		break;
	}
	case 18: /* 到清洗高度 */
	{
		if (m_motion.MultiAxisMoveSimple(GetMultiAxisCoord(STR_REACT_CLEAN_HEIGHT(iReactModIndex))) == 0)
		{
			iStep = 19;
			s_llStartTime = GetProgramTime();
		}
		break;
	}
	case 19:
	{
		if (m_motion.IsAxisBusy(GetMultiAxisCoord(STR_REACT_CLEAN_HEIGHT(iReactModIndex))))
		{
			CHECK_MOTION_TIMEOUT
		}
		else
		{
			iStep = 20;
		}
		break;
	}
	case 20:
	{
		/* start clean */

		emit signalMsg(QString::fromUtf8(u8"针头清洗开始"));
		SetNeedleCleanerPumpOn(true);

		int iAxisNo = GetAxisNo(STR_AXIS_PIP);
		double dEmptyVol = GetCommonConfig(STR_SECTION_PUMPPARAM, u8"留空容积");
		double dReso = GetCommonConfig(STR_SECTION_RESOLUTION, to_string(iAxisNo));
		//m_motion.SingleAxisMoveRel(iAxisNo, -dEmptyVol * dReso);
		//Change by ZhangWeiqi 直接采用绝对位置
		m_motion.SingleAxisMoveAbs(iAxisNo, 0); 
		s_llStartTime = GetProgramTime();
		iStep = 21;
		break;
	}
	case 21:
	{
		/* wait clean */

		int iAxisNo = GetAxisNo(STR_AXIS_PIP);
		double dEmptyVol = GetCommonConfig(STR_SECTION_PUMPPARAM, u8"留空容积");
		double dReso = GetCommonConfig(STR_SECTION_RESOLUTION, to_string(iAxisNo));
		if (GetProgramTime() - s_llStartTime > GetCommonConfig(u8"加液", u8"清洗时间") && !m_motion.IsAxisBusy(iAxisNo))
		{
			SetNeedleCleanerPumpOn(false);
			//Change by ZhangWeiqi 直接采用绝对位置
			m_motion.SingleAxisMoveAbs(iAxisNo, dEmptyVol * dReso);
			emit signalMsg(QString::fromUtf8(u8"针头清洗完成"));
			iStep = 22;
		}
		else
		{
			Sleep(10);
		}
		break;
	}
	case 22:
	{
		function<void()> nextStep = bind(&CMachineControl::ImplPipet, this, 23, iReactModIndex);
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowZAxesUp, this, nextStep, 0));
		boost::asio::post(m_tp, bind(&CMachineControl::ImplCleanerSlotDrain, this, 0)); /* start drain */
		iStep = -1;
		break;
	}
	case 23:
	{
		int iAxisNo = GetAxisNo(STR_AXIS_PIP);
		if (!m_motion.IsAxisBusy(iAxisNo))
		{
			iStep = 24;
		}
		else
		{
			Sleep(10);
		}
		break;
	}
	case 24: /* end of a cycle, loop or break */
	{
		/* iterator has been updated */
		++s_iterReagentUsage;
		if (s_iterReagentUsage == s_mapReagentUsage.end())
		{
			iStep =1024001;
		}
		else
		{
			iStep = 2;
		}
		break;
	}
	case 1024001:
	{
		if (m_mapReactMods[iReactModIndex]->WriteRelayPipetCompletion() == 0)
		{
			emit signalMsg(QString::fromUtf8(u8"反应模块%1加液完成").arg(iReactModIndex));
			iStep = 25;
		}
		break;
	}
	case 25:
	{
		iStep = 26;
		break;
	}
	case 26:
	{
		lock_guard<mutex> lk(m_mtxReaction);
		m_mapReactionState[iReactModIndex] = ReactionState::Reacting;
		iStep = 27;
		break;
	}
	case 27:
	{
		iStep = 28;
		break;
	}
	case 28:
	{
		/* end */
		boost::asio::post(m_tp, bind(&CMachineControl::Dispatch, this));
		iStep = 29;
		break;
	}
	default:
		iStep = -1;
		break;
	}

	if (iStep >= 0)
	{
		boost::asio::post(m_tp, bind(&CMachineControl::ImplPipet, this, iStep, iReactModIndex));
	}
}

void CMachineControl::ImplTemperatureUpdate2(bool bHasRepairMod)
{
	static int iCnt = 0;
	if (m_state == RunState::Exiting || m_state == RunState::Resetting)
		return;

	if (bHasRepairMod)
	{
		/* read from temperature sensorand write into module PLC */
		double dTempRepair1 = m_repairMod->GetCurrentTemp(1) / 10;
		double dTempRepair2 = m_repairMod->GetCurrentTemp(2) / 10;
		if (dTempRepair1 >= 0 && dTempRepair1 <= 100)
		{
			m_repairMod->WriteDTCurrentTemp(1, dTempRepair1);
			emit signalUpdateRepairModTem(dTempRepair1, 1);
		}
		if (dTempRepair2 >= 0 && dTempRepair2 <= 100)
		{
			m_repairMod->WriteDTCurrentTemp(2, dTempRepair2);
			emit signalUpdateRepairModTem(dTempRepair2, 2);
		}
	}

	double dTempReact1 = m_mapReactMods[1]->GetCurrentTemp() / 10;
	double dTempReact2 = m_mapReactMods[2]->GetCurrentTemp() / 10;
	/* update temperature display */
	if (dTempReact1 >= 0 && dTempReact1 <= 100)
	{
		emit signalUpdateReactModTem(dTempReact1, 1);
	}
	if (dTempReact2 >= 0 && dTempReact2 <= 100)
	{
		emit signalUpdateReactModTem(dTempReact2, 2);
	}

	Sleep(500);

	boost::asio::post(m_tp, bind(&CMachineControl::ImplTemperatureUpdate2, this, bHasRepairMod));
}

void CMachineControl::ImplAxisCoordUpdate()
{
	static int iCnt = 0;
	if (m_state == RunState::Exiting || m_state == RunState::Resetting)
		return;

	double d[5];
	for (int i = 0; i < 5; i++)
	{
		m_motion.GetPosition(0, i, d[i]);
	}
	emit signalUpdateCoord(d[0], d[1], d[2], d[3], d[4]);

	Sleep(500);

	boost::asio::post(m_tp, bind(&CMachineControl::ImplAxisCoordUpdate, this));
}

void CMachineControl::ImplTestingRepairMod(int iIndex, int iStep)
{
	if (m_state == RunState::Exiting || !m_bTestingFlag)
	{
		m_repairMod->WriteRelayReset();
		return;
	}

	int iRet = -1;
	switch (iStep)
	{
	case 0:
	{
		//if (m_repairMod->WriteRelayFlowStart() == 0)
		{
			iStep = 1;
		}
		break;
	}
	case 1:
	{
		if (m_repairMod->RACRelayPutShelfPermission(iIndex))
		{
			emit signalMsg(QString::fromUtf8(u8"修复模块%1允许放架").arg(iIndex));
			iStep = 8;
		}
		break;
	}
	case 8:
	{
		if (m_repairMod->WriteDTRepairTime(iIndex, GetCommonConfig(u8"模块", u8"修复时间")) == 0) 
		{
			emit signalMsg(QString::fromUtf8(u8"修复模块%1设置修复时间%2").arg(iIndex).arg(GetCommonConfig(u8"模块", u8"修复时间")));
			iStep = 9;
		}
		break;
	}
	case 9:
	{
		if (m_repairMod->WriteDTTempLower(iIndex, GetCommonConfig(u8"模块", u8"修复模块低温")) == 0) /* temperature */
		{
			emit signalMsg(QString::fromUtf8(u8"修复模块%1设置低温%2").arg(iIndex).arg(GetCommonConfig(u8"模块", u8"修复模块低温")));
			iStep = 10;
		}
		break;
	}
	case 10:
	{
		if (m_repairMod->WriteDTTempKeep(iIndex, GetCommonConfig(u8"模块", u8"修复模块保持温度")) == 0) /* temperature */
		{
			emit signalMsg(QString::fromUtf8(u8"修复模块%1设置保持温度%2").arg(iIndex).arg(GetCommonConfig(u8"模块", u8"修复模块保持温度")));
			iStep = 11;
		}
		break;
	}
	case 11:
	{
		if (m_repairMod->WriteDTTempUpper(iIndex, GetCommonConfig(u8"模块", u8"修复模块高温")) == 0) /* temperature */
		{
			emit signalMsg(QString::fromUtf8(u8"修复模块%1设置高温%2").arg(iIndex).arg(GetCommonConfig(u8"模块", u8"修复模块高温")));
			iStep = 12;
		}
		break;
	}
	case 12:
	{
		if (m_repairMod->WriteDTTempPickShelf(iIndex, GetCommonConfig(u8"模块", u8"修复模块提架温度")) == 0)  /* temperature */
		{
			emit signalMsg(QString::fromUtf8(u8"修复模块%1设置提架温度%2").arg(iIndex).arg(GetCommonConfig(u8"模块", u8"修复模块提架温度")));
			iStep = 13;
		}
		break;
	}
	case 13:
	{
		if (m_repairMod->WriteRelayPutShelfCompletion(iIndex) == 0) /* complete putting and start timing */
		{
			emit signalMsg(QString::fromUtf8(u8"修复模块%1放架完成").arg(iIndex));
			iStep = 14;
		}
		break;
	}
	case 14:
	{
		if (m_repairMod->RACRelayPickShelfPermission(iIndex))
		{
			emit signalMsg(QString::fromUtf8(u8"修复模块%1修复完成，允许取架").arg(iIndex));
			iStep = 15;
		}
		break;
	}
	case 15:
	{
		if (m_repairMod->WriteRelayPickShelfCompletion(iIndex) == 0)
		{
			emit signalMsg(QString::fromUtf8(u8"修复模块%1取架完成").arg(iIndex));
			iStep = 16;
		}
		break;
	}
	default:
	{
		iStep = -1;
		break;
	}
	}

	Sleep(100);

	if (iStep >= 0)
	{
		boost::asio::post(m_tp, bind(&CMachineControl::ImplTestingRepairMod, this, iIndex, iStep));
	}
	else
	{
		boost::asio::post(m_tp, bind(&CMachineControl::ImplTestingRepairMod, this, iIndex, 0));
	}
}

void CMachineControl::ImplTestSurfaceSensor(int iStep)
{
	if (m_state != RunState::Stopped || !m_bTestingFlag)
	{
		emit signalMsg(QString::fromUtf8(u8"测试针头传感器结束"));
		return;
	}

	static int s_iCnt = 0;
	static ofstream fout("./Log/record", ios::out | ios::app);

	switch (iStep)
	{
	case 0:
	{
		emit signalMsg(QString::fromUtf8(u8"测试针头液面传感器100次"));
		s_iCnt = 0;
		function<void()> nextStep = bind(&CMachineControl::ImplTestSurfaceSensor, this, 1);
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowZAxesUp, this, nextStep, 0));
		iStep = -1;
		break;
	}
	case 1:
	{
		function<void()> nextStep = bind(&CMachineControl::ImplTestSurfaceSensor, this, 2);
		emit signalMsg(QString::fromUtf8(u8"往试剂架1"));
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowMultiAxisMove, this, nextStep, 0, GetMultiAxisCoord(u8"试剂架试剂1")));
		iStep = -1;
		break;
	}
	case 2: /* 到高度上界 */
	{
		function<void()> nextStep = bind(&CMachineControl::ImplTestSurfaceSensor, this, 3);
		//emit signalMsg(QString::fromUtf8(u8"往试剂架高度上界"));
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowMultiAxisMove, this, nextStep, 0, GetMultiAxisCoord(u8"试剂架试剂高度上界")));
		iStep = -1;
		break;
	}
	case 3: /* 往高度下界 */
	{
		//emit signalMsg(QString::fromUtf8(u8"往试剂架高度下界"));
		if (m_motion.MultiAxisMoveSimple(GetMultiAxisCoord(u8"试剂架试剂高度下界")) == 0)
		{
			iStep = 4;
		}
		break;
	}
	case 4: /* 到特定高度后减速 */
	{
		int iAxisNo = GetAxisNo(STR_AXIS_NDL);
		double dPos = GetAxisPos(iAxisNo);
		double dReso = GetCommonConfig(STR_SECTION_RESOLUTION, to_string(iAxisNo));
		double dAttemptBeginHeight =
			(m_mapLatestReagentSurfaceAltitude.find(1) != m_mapLatestReagentSurfaceAltitude.end())
			? (m_mapLatestReagentSurfaceAltitude[1] - 15 * dReso) /* last pos - 15mm */
			: (GetMultiAxisCoord(u8"试剂架试剂高度上界").at(iAxisNo) - 5 * dReso); /* upper bound - 5mm */

		if (dPos > dAttemptBeginHeight)
		{
			int iRet = m_motion.ChangSpeed(0, iAxisNo, 4000, 0.2);
			//emit signalMsg(QString::fromUtf8(u8"changedspeed4000 = %1").arg(iRet));
			iStep = 5;
		}
		else
		{

		}
		Sleep(100);
		break;
	}
	case 5: /* 检测到液面则停止 */
	{
		int iAxisNo = GetAxisNo(STR_AXIS_NDL);
		double dReso = GetCommonConfig(STR_SECTION_RESOLUTION, to_string(iAxisNo));
		if (m_SurfaceSensor.SurfaceTouched())
		{
			m_motion.AxisStop(0, iAxisNo, 1);
			int iRet = m_motion.ChangSpeed(0, iAxisNo, 30000, 0.1);
			double dPos = GetAxisPos(iAxisNo);
			emit signalMsg(QString::fromUtf8(u8"液面：+%1mm").arg(dPos / dReso));
			fout << dPos / dReso << endl;
			m_mapLatestReagentSurfaceAltitude[1] = dPos;
			iStep = 6; /* touched */
		}
		else /* 未检测到液面且到达下界则认为未检测到液面 */
		{
			if (!m_motion.IsAxisBusy(iAxisNo)) /* can't find reagent */
			{
				rmgr.SetUsedUp(1);
				m_motion.ChangSpeed(0, iAxisNo, 30000, 0.1);
				int iPipAxisNo = GetAxisNo(STR_AXIS_PIP); /* 抽空气 */
				emit signalMsg(QString::fromUtf8(u8"未检测到液面，重试"));
				fout << 999999 << endl;
				iStep = 6; /* wait for stop */
			}
			else
			{
				/* wait for touching */
			}
		}
		break;
	}
	case 6: /* 等待到达安全高度，循环 */
	{
		function<void()> nextStep = bind(&CMachineControl::ImplTestSurfaceSensor, this, 7);
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowZAxesUp, this, nextStep, 0));
		iStep = -1;
		break;
	}
	case 7:
	{
		s_iCnt++;
		if (s_iCnt >= 100)
		{
			iStep = -1; /* stop */
		}
		else
		{
			iStep = 2; /* continue */
		}
		break;
	}


	default:
	{
		iStep = -1;
		break;
	}
	}
	Sleep(1);

	if (iStep >= 0)
	{
		boost::asio::post(m_tp, bind(&CMachineControl::ImplTestSurfaceSensor, this, iStep));
	}
}


void CMachineControl::SubFlowZAxesUp(function<void()> prevTask, int iStep)
{
	if (m_state == RunState::Exiting || m_state == RunState::Stopped)
		return;

	static long long int s_llStartTime;
	switch (iStep)
	{
	case 0:
	{
		if (m_motion.MultiAxisMoveSimple(GetMultiAxisCoord(u8"针头安全高度")) == 0)
		//if (m_motion.AxisHome(GetAxisNo(STR_AXIS_NDL)) == 0)
		{
			iStep = 1;
			s_llStartTime = GetProgramTime();
		}
		break;
	}
	case 1:
	{
		if (m_motion.MultiAxisMoveSimple(GetMultiAxisCoord(u8"提架安全高度")) == 0)
		{
			iStep = 2;
			s_llStartTime = GetProgramTime();
		}
		break;
	}
	case 2:
	{
		if (m_motion.IsAxisBusy(GetMultiAxisCoord(u8"针头安全高度"))
			|| m_motion.IsAxisBusy(GetMultiAxisCoord(u8"提架安全高度")))
		{
			CHECK_MOTION_TIMEOUT
		}
		else
		{
			iStep = 3;
		}
		break;
	}
	default:
		iStep = -1;
		break;
	}

	Sleep(1);
	if (iStep >= 0)
	{
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowZAxesUp, this, prevTask, iStep));
	}
	else
	{
		boost::asio::post(m_tp, prevTask);
	}
}

void CMachineControl::SubFlowXYHome(function<void()> prevTask, int iStep)
{
	if (m_state == RunState::Exiting || m_state == RunState::Stopped)
		return;

	static long long int s_llStartTime;

	switch (iStep)
	{
	case 0:
	{
		smc_set_home_position_unit(0, GetAxisNo(STR_AXIS_X), 1, 0);
		smc_set_home_position_unit(0, GetAxisNo(STR_AXIS_Y), 1, 0);
		iStep = 1000001;
		break;
	}
	case 1000001:
	{
		if (m_motion.AxisHome(GetAxisNo(STR_AXIS_X)) == 0)
		{
			iStep = 1;
		}
		break;
	}
	case 1:
	{
		if (m_motion.AxisHome(GetAxisNo(STR_AXIS_Y)) == 0)
		{
			iStep = 2;
		}
		break;
	}
	case 2:
	{
		if (m_motion.IsHomeCompleted(GetAxisNo(STR_AXIS_X))
			&& m_motion.IsHomeCompleted(GetAxisNo(STR_AXIS_Y)))
		{
			iStep = 3;
		}
		break;
	}
	default:
	{
		iStep = -1;
		break;
	}
	}

	Sleep(1);
	if (iStep >= 0)
	{
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowXYHome, this, prevTask, iStep));
	}
	else
	{
		boost::asio::post(m_tp, prevTask);
	}
}

void CMachineControl::SubFlowZHome(function<void()> prevTask, int iStep)
{
	if (m_state == RunState::Exiting || m_state == RunState::Stopped)
		return;

	switch (iStep)
	{
	case 0:
	{
		smc_set_home_position_unit(0, GetAxisNo(STR_AXIS_PICK), 1, 0);
		smc_set_home_position_unit(0, GetAxisNo(STR_AXIS_NDL), 1, 0);
		iStep = 1000001;
		break;
	}
	case 1000001:
	{
		if (m_motion.AxisHome(GetAxisNo(STR_AXIS_PICK)) == 0)
		{
			iStep = 1;
		}
		break;
	}
	case 1:
	{
		if (m_motion.AxisHome(GetAxisNo(STR_AXIS_NDL)) == 0)
		{
			iStep = 2;
		}
		break;
	}
	case 2:
	{
		if (m_motion.IsHomeCompleted(GetAxisNo(STR_AXIS_PICK))
			&& m_motion.IsHomeCompleted(GetAxisNo(STR_AXIS_NDL)))
		{
			iStep = 3;
		}
		break;
	}
	default:
	{
		iStep = -1;
		break;
	}
	}

	Sleep(1);
	if (iStep >= 0)
	{
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowZHome, this, prevTask, iStep));
	}
	else
	{
		boost::asio::post(m_tp, prevTask);
	}

}

void CMachineControl::SubFlowMultiAxisMove(function<void()> prevTask, int iStep, MultiAxisCoord coord)
{
	if (m_state == RunState::Exiting || m_state == RunState::Stopped)
		return;

	static long long int s_llStartTime;

	switch (iStep)
	{
	case 0:
	{
		if (m_motion.MultiAxisMoveSimple(coord) == 0)
		{
			iStep = 1;
		}
		break;
	}
	case 1:
	{
		if (!m_motion.IsAxisBusy(coord))
		{
			iStep = 2;
		}
		else
		{
			CHECK_MOTION_TIMEOUT
		}
		break;
	}
	default:
	{
		iStep = -1;
		break;
	}
	}

	Sleep(1);
	if (iStep >= 0)
	{
		boost::asio::post(m_tp, bind(&CMachineControl::SubFlowMultiAxisMove, this, prevTask, iStep, coord));
	}
	else
	{
		boost::asio::post(m_tp, prevTask);
	}
}

QuestionAnswer CMachineControl::AskQuestion(QString strText, function<void()> fYes, function<void()> fNo)
{
	lock_guard<mutex> lk{ m_mtxQuestion };
	unique_lock<mutex> lkAnswer{ m_mtxAnswer };
	m_answer = QuestionAnswer::Waiting;

	emit signalQuestion(strText);
	while (m_answer == QuestionAnswer::Waiting)
	{
		m_condAnswer.wait(lkAnswer);
	}
	(m_answer == QuestionAnswer::Yes ? fYes : fNo)();
	return m_answer;
}

void CMachineControl::SetElectroMagnet(bool bOn)
{
	int iBitNo = stoi(iniIO[u8"运动控制卡通用输出"][u8"电磁铁"]);
	m_motion.WriteOutBit(0, iBitNo, !bOn);
}

void CMachineControl::SetNeedleCleanerPumpOn(bool bOn)
{
	int iBitNo = stoi(iniIO[u8"运动控制卡通用输出"][u8"针头清洗"]);
	m_motion.WriteOutBit(0, iBitNo, !bOn);
}

void CMachineControl::SetReactModDrainPumpOn(int iIndex, bool bOn)
{
	int iBitNo = stoi(iniIO[u8"运动控制卡通用输出"][string(u8"反应模块").append(to_string(iIndex)).append("排液")]);
	m_motion.WriteOutBit(0, iBitNo, !bOn);
}

void CMachineControl::SetCleanerSlotDrainPumpOn(bool bOn)
{
	int iBitNo = stoi(iniIO[u8"运动控制卡通用输出"][u8"针头清洗排液"]);
	m_motion.WriteOutBit(0, iBitNo, !bOn);
}

void CMachineControl::HandleRepairModAlarm(const char* pRelayName)
{
	m_repairMod->WriteRelayByName(pRelayName, true);
	m_iUnhandledAlarmCnt--;
	emit signalMsg(QString::fromUtf8(pRelayName));
}

void CMachineControl::HandleRepairModAlarm2(int iAlarmIndex)
{
	m_repairMod->WriteRelayByName(string(u8"报警继续").append(to_string(iAlarmIndex)).c_str(), true);
	m_iUnhandledAlarmCnt--;
	emit signalMsg(QString::fromUtf8(u8"清除修复模块报警%1").arg(iAlarmIndex));
}

void CMachineControl::AnswerQuestion(bool bYes)
{
	lock_guard<mutex> lk(m_mtxAnswer);
	m_answer = bYes ? QuestionAnswer::Yes : QuestionAnswer::No;
	m_condAnswer.notify_one();
}

void CMachineControl::ClearReagentSurfaceAltitudeRecord(int iPos)
{
	m_mapLatestReagentSurfaceAltitude.erase(iPos);
	rmgr.SetUsedUp(iPos, false);
}

void CMachineControl::WriteDT(int iDevID, const char* pName, int iVal)
{
	if (iDevID == stoi(iniIO["Station"][u8"修复模块"]))
	{
		return m_repairMod->WriteDT(pName, iVal);
	}
	else if (iDevID == stoi(iniIO["Station"][u8"反应模块1"]))
	{
		return m_mapReactMods[1]->WriteDT(pName, iVal);
	}
	else if (iDevID == stoi(iniIO["Station"][u8"反应模块2"]))
	{
		return m_mapReactMods[2]->WriteDT(pName, iVal);
	}
}

int CMachineControl::ReadDT(int iDevID, const char* pName)
{
	if (iDevID == stoi(iniIO["Station"][u8"修复模块"]))
	{
		return m_repairMod->ReadDT(pName);
	}
	else if (iDevID == stoi(iniIO["Station"][u8"反应模块1"]))
	{
		return m_mapReactMods[1]->ReadDT(pName);
	}
	else if (iDevID == stoi(iniIO["Station"][u8"反应模块2"]))
	{
		return m_mapReactMods[2]->ReadDT(pName);
	}
	else
	{
		return 0;
	}
}

void CMachineControl::WriteRelay(int iDevID, const char* pAddr, bool bVal)
{
	if (iDevID == stoi(iniIO["Station"][u8"修复模块"]))
	{
		m_repairMod->WriteRelay(pAddr, bVal);
	}
	else if (iDevID == stoi(iniIO["Station"][u8"反应模块1"]))
	{
		m_mapReactMods[1]->WriteRelay(pAddr, bVal);
	}
	else if (iDevID == stoi(iniIO["Station"][u8"反应模块2"]))
	{
		m_mapReactMods[2]->WriteRelay(pAddr, bVal);
	}
}

bool CMachineControl::ReadRelay(int iDevID, const char* pAddr)
{
	if (iDevID == stoi(iniIO["Station"][u8"修复模块"]))
	{
		return m_repairMod->ReadRelay(pAddr);
	}
	else if (iDevID == stoi(iniIO["Station"][u8"反应模块1"]))
	{
		return m_mapReactMods[1]->ReadRelay(pAddr);
	}
	else if (iDevID == stoi(iniIO["Station"][u8"反应模块2"]))
	{
		return m_mapReactMods[2]->ReadRelay(pAddr);
	}
	else
	{
		return false;
	}
}

void CMachineControl::Dispatch()
{
	if (!CheckRunState()) 
		return;
		
	if (this->HasEmptyReactionModule())
	{
		if (this->HasCompletedRepairModule())
		{
			StartPickFromRepair();
			return;
		}
		else if (HasWaitingShelfHolder() 
			&& (!FirstWaitingShelfNeedsRepair() || (stoi(iniCfg[u8"模块"][u8"有修复模块"]) == 0)))
		{
			StartPickShelf();
			return;
		}
	}
	
	/* if need repair */
	if (HasEmptyRepairModule() && stoi(iniCfg[u8"模块"][u8"有修复模块"]) == 1)
	{
		if (HasWaitingShelfHolder() && FirstWaitingShelfNeedsRepair())
		{
			boost::asio::post(m_tp, bind(&CMachineControl::ImplPickShelf, this, 0));
			return;
		}
	}

	/* lock for check_reaction_module thread */
	unique_lock<mutex> lk(m_mtxReaction);
	if (!m_qReactionQueue.empty())
	{
		int iReactModIndex = m_qReactionQueue.front();
		emit signalMsg(QString::fromUtf8(u8"反应模块%1准备加液").arg(iReactModIndex));
		m_qReactionQueue.pop();
		boost::asio::post(m_tp, bind(&CMachineControl::ImplPipet, this, 0, iReactModIndex));
		return;
	}
	lk.unlock();

	if (HasCompletedReactionModule())
	{
		if (HasEmptyShelfHolder())
		{
			boost::asio::post(m_tp, bind(&CMachineControl::ImplPickFromReaction, this, 0));
			return;
		}
		else /* should not be here */
		{
			this->Suspend();
			emit signalInterrupt(u8"已完成反应的玻片没有位置回收");
		}
	}

	Sleep(20); /* if nothing to schedule, sleep for a while, and schedule again */
	boost::asio::post(m_tp, bind(&CMachineControl::Dispatch, this));	
}

bool CMachineControl::CheckRunState() /* if exiting, return, if running, Start, else, wait */
{
	unique_lock<mutex> lk(m_mtxState); 
	/* if not running, wait for running */
	if (m_state == RunState::Suspending || m_state == RunState::Stopped)
		m_condRunning.wait(lk);
	/* if exiting ro resetting, exit */
	if (m_state == RunState::Exiting || m_state == RunState::Resetting)
		return false;
	else
		return true;		
}

void CMachineControl::ReloadConfig()
{
	LoadIniConfig();
	LoadIniIO();
}

void CMachineControl::ReloadCoord()
{
	LoadIniCoord();
}

void CMachineControl::ReloadReagentShelf()
{
	rmgr.Reload();
}

MultiAxisCoord CMachineControl::GetMultiAxisCoord(const string& strCoordName)
{
	MultiAxisCoord ret;
	const int iAxisBitmap = stoi(iniCoord[strCoordName]["AxisNos"]);
	for (int i = 0; i < 5; i++)
	{
		if (iAxisBitmap & (1 << i))
		{
			ret[i] = stod(iniCoord[strCoordName][to_string(i)]);
		}
	}
	return ret;
}

double CMachineControl::GetCommonConfig(const string& strGroup, const string& strName)
{
	return stod(iniCfg[strGroup][strName]);
}

int CMachineControl::GetAxisNo(const string& strAxisName)
{
	return static_cast<int>(GetCommonConfig(STR_SECTION_AXIS, strAxisName));
}

bool CMachineControl::HasCompletedReactionModule()
{
	auto iter = m_mapReactMods.begin();
	while (iter != m_mapReactMods.end())
	{
		if (!iter->second->Empty() && !iter->second->Reacting())
		{
			return true;
		}
		++iter;
	}
	return false;

	//for (const auto& iter : m_mapReactMods)
	//{
	//	if (!iter.second->Empty() && !iter.second->Reacting())
	//	{
	//		return true;
	//	}
	//}
	//return false;
}

bool CMachineControl::HasEmptyReactionModule()
{
	auto iter = m_mapReactMods.begin();
	while (iter != m_mapReactMods.end())
	{
		if (iter->second->Empty())
		{
			return true;
		}
		++iter;
	}
	return false;

	//for (const auto& iter : m_mapReactMods)
	//{
	//	if (iter.second->Empty())
	//	{
	//		return true;
	//	}
	//}
	//return false;
}

bool CMachineControl::HasCompletedRepairModule()
{
	for (int i = 1; i <= 2; i++)
	{
		if (!m_repairMod->Empty(i) && m_repairMod->IsCompleted(i))
		{
			return true;
		}
	}
	return false;
}

bool CMachineControl::HasEmptyRepairModule()
{
	for (int i = 1; i <= 2; i++)
	{
		if (m_repairMod->Empty(i))
		{
			return true;
		}
	}
	return false;
}

int CMachineControl::GetOneCompletedReactionModule()
{
	auto iter = m_mapReactMods.begin();
	while (iter != m_mapReactMods.end())
	{
		if (!iter->second->Empty() && !iter->second->Reacting())
		{
			return iter->first;
		}
		++iter;
	}
	return -1;

	//for (const auto& iter : m_mapReactMods)
	//{
	//	if (!iter.second->Empty() && !iter.second->Reacting())
	//	{
	//		return iter.first;
	//	}
	//}
	//return -1;
}

int CMachineControl::GetOneEmptyReactionModule()
{
	auto iter = m_mapReactMods.begin();
	while (iter != m_mapReactMods.end())
	{
		if (iter->second->Empty())
		{
			return iter->first;
		}
		++iter;
	}
	return -1;

	//for (const auto& iter : m_mapReactMods)
	//{
	//	if (iter.second->Empty())
	//	{
	//		return iter.first;
	//	}
	//}
	//return -1;
}

int CMachineControl::GetOneCompletedRepairModule()
{
	for (int i = 1; i <= 2; i++)
	{
		if (!m_repairMod->Empty(i) && m_repairMod->IsCompleted(i))
		{
			return i;
		}
	}
	return -1;
}

int CMachineControl::GetOneEmptyRepairModule()
{
	for (int i = 1; i <= 2; i++)
	{
		if (m_repairMod->Empty(i))
		{
			return i;
		}
	}
	return -1;
}

bool CMachineControl::HasWaitingShelfHolder()
{
	for (auto iter : m_mapShelfHolders)
	{
		if (iter.second->Waiting())
		{
			return true;
		}
	}
	return false;
}

bool CMachineControl::HasEmptyShelfHolder()
{
	for (auto iter : m_mapShelfHolders)
	{
		if (iter.second->Empty())
		{
			return true;
		}
	}
	return false;
}

int CMachineControl::GetOneWaitingShelfHolder()
{
	for (auto iter : m_mapShelfHolders)
	{
		if (iter.second->Waiting())
		{
			return iter.first;
		}
	}
	return -1;
}

int CMachineControl::GetOneEmptyShelfHolder()
{
	for (auto iter : m_mapShelfHolders)
	{
		if (iter.second->Empty())
		{
			return iter.first;
		}
	}
	return -1;
}

bool CMachineControl::FirstWaitingShelfNeedsRepair()
{
	for (auto iter : m_mapShelfHolders)
	{
		if (iter.second->Waiting())
		{
			return iter.second->NeedsRepair();
		}
	}
	return false;
}

CIMachineControl* GetMachineControlInstance()
{
	static CMachineControl instance;
	return &instance;
}
