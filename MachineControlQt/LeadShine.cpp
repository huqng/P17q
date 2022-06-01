#include "stdafx.h"
#include "LeadShine.h"

#pragma comment(lib, "LTsmc.lib")

CLeadShine::CLeadShine()
{
	smc_board_close(0);
}

CLeadShine::~CLeadShine()
{
	smc_board_close(0);
}

short CLeadShine::InitCard(const string& strIp)
{
	short shRet = 0;
	m_mtx.lock();
	shRet |= smc_board_init(0, 2, const_cast<char*>(strIp.c_str()), 0);
	m_mtx.unlock();
	return shRet;
}

short CLeadShine::ResetCard(const WORD wCardNo, const int iType)
{
	if (iType == 0)
	{
		short sRet = 0;
		m_mtx.lock();
		sRet = smc_soft_reset(wCardNo);
		m_mtx.unlock();
		return sRet;
	}
	else
	{
		short sRet = 0;
		m_mtx.lock();
		sRet = smc_board_reset(wCardNo);
		m_mtx.unlock();
		return sRet;
	}
}

short CLeadShine::IsCardConnect(const WORD wCardNo)
{
	short sRet = 0;
	WORD sState = 0;
	m_mtx.lock();
	sRet = smc_get_connect_status(wCardNo);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::LoadConfigFile(const WORD wCardNo, const char* pcFileName)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_download_file(wCardNo, pcFileName, "Config", 2);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::SetPulseOutMode(const WORD wCardNo, const WORD wAxisNo, const WORD wOutMode)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_set_pulse_outmode(wCardNo, wAxisNo, wOutMode);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::GetPulseOutMode(const WORD wCardNo, const WORD wAxisNo, WORD& wOutMode)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_get_pulse_outmode(wCardNo, wAxisNo, &wOutMode);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::SetHomePinLogic(const WORD wCardNo, const WORD wAxisNo, const WORD wOrgLogic, const double dFilter)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_set_home_pin_logic(wCardNo, wAxisNo, wOrgLogic, dFilter);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::GetHomePinLogic(const WORD wCardNo, const WORD wAxisNo, WORD& wOrgLogic, double& dFilter)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_get_home_pin_logic(wCardNo, wAxisNo, &wOrgLogic, &dFilter);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::GetHomeMode(const WORD wCardNo, const WORD wAxisNo, WORD& wHomdDir, double& dSpeedMode, WORD& wHomeMode, WORD& wEZCount)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_get_homemode(wCardNo, wAxisNo, &wHomdDir, &dSpeedMode, &wHomeMode, &wEZCount);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::SetHomeMode(const WORD wCardNo, const WORD wAxisNo, const WORD wHomdDir, const double dSpeedMode, const WORD wHomeMode, const WORD wEZCount)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_set_homemode(wCardNo, wAxisNo, wHomdDir, dSpeedMode, wHomeMode, wEZCount);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::SetHomeELReturn(const WORD wCardNo, const WORD wAxisNo, const WORD wEnable)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_set_el_home(wCardNo, wAxisNo, wEnable);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::SetHomePosition(const WORD wCardNo, const WORD wAxisNo, const WORD wEnable, const double dPosition)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_set_home_position_unit(wCardNo, wAxisNo, wEnable, dPosition);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::GetHomePosition(const WORD wCardNo, const WORD wAxisNo, WORD& wEnable, double& dPosition)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_get_home_position_unit(wCardNo, wAxisNo, &wEnable, &dPosition);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::HomeMove(const WORD wCardNo, const WORD wAxisNo)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_home_move(wCardNo, wAxisNo);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::GetHomeMoveResult(const WORD wCardNo, const WORD wAxisNo, WORD& wState)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_get_home_result(wCardNo, wAxisNo, &wState);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::SetHomeLatchMode(const WORD wCardNo, const WORD wAxisNo, const WORD wEnable, const WORD wLogic, const WORD wSource)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_set_homelatch_mode(wCardNo, wAxisNo, wEnable, wLogic, wSource);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::GetHomeLatchMode(const WORD wCardNo, const WORD wAxisNo, WORD& wEnable, WORD& wLogic, WORD& wSource)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_get_homelatch_mode(wCardNo, wAxisNo, &wEnable, &wLogic, &wSource);
	m_mtx.unlock();
	return sRet;
}

long CLeadShine::GetHomeLatchFlag(const WORD wCardNo, const WORD wAxisNo)
{
	long sRet = 0;
	m_mtx.lock();
	sRet = smc_get_homelatch_flag(wCardNo, wAxisNo);
	m_mtx.unlock();
	return sRet;
}

long CLeadShine::GetHomeLatchValue(const WORD wCardNo, const WORD wAxisNo)
{
	long sRet = 0;
	double dPos = 0.0;
	m_mtx.lock();
	sRet = smc_get_homelatch_value_unit(wCardNo, wAxisNo, &dPos);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::SetELMode(const WORD wCardNo, const WORD wAxisNo, const WORD wELEnable, const WORD wELLogic, const WORD wELMode)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_set_el_mode(wCardNo, wAxisNo, wELEnable, wELLogic, wELMode);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::GetELMode(const WORD wCardNo, const WORD wAxisNo, WORD& wELEnable, WORD& wELLogic, WORD& wELMode)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_get_el_mode(wCardNo, wAxisNo, &wELEnable, &wELLogic, &wELMode);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::SetSoftLimit(const WORD wCardNo, const WORD wAxisNo, const WORD wEnable, const WORD wSourceSel, const WORD wSLAction, const double lNLimit, const double lPLimit)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_set_softlimit_unit(wCardNo, wAxisNo, wEnable, wSourceSel, wSLAction, lNLimit, lPLimit);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::GetSoftLimit(const WORD wCardNo, const WORD wAxisNo, WORD& wEnable, WORD& wSourceSel, WORD& wSLAction, double& lNLimit, double& lPLimit)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_get_softlimit_unit(wCardNo, wAxisNo, &wEnable, &wSourceSel, &wSLAction, &lNLimit, &lPLimit);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::SetPosition(const WORD wCardNo, const WORD wAxisNo, const double lCurrentPosition)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_set_position_unit(wCardNo, wAxisNo, lCurrentPosition);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::GetPosition(const WORD wCardNo, const WORD wAxisNo, double& dPosition)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_get_position_unit(wCardNo, wAxisNo, &dPosition);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::SetEncoder(const WORD wCardNo, const WORD wAxisNo, const double lCurrentPosition)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_set_encoder_unit(wCardNo, wAxisNo, lCurrentPosition);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::GetEncoder(const WORD wCardNo, const WORD wAxisNo, double& dPosition)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_get_encoder_unit(wCardNo, wAxisNo, &dPosition);
	m_mtx.unlock();
	return sRet;
}

double CLeadShine::GetCurrentSpeed(const WORD wCardNo, const WORD wAxisNo)
{
	double sRet = 0;

	m_mtx.lock();
	smc_read_current_speed_unit(wCardNo, wAxisNo, &sRet);
	m_mtx.unlock();
	return sRet;
}


short CLeadShine::GetAxisDone(const WORD wCardNo, const WORD wAxisNo)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_check_done(wCardNo, wAxisNo);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::GetAxisDoneMultcoor(const WORD wCardNo, const WORD wCrd)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_check_done_multicoor(wCardNo, wCrd);
	m_mtx.unlock();
	return sRet;
}

DWORD CLeadShine::GetAxisIoState(const WORD wCardNo, const WORD wAxisNo)
{
	DWORD sRet = 0;
	m_mtx.lock();
	sRet = smc_axis_io_status(wCardNo, wAxisNo);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::AxisStop(const WORD wCardNo, const WORD wAxisNo, const WORD wStopMode)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_stop(wCardNo, wAxisNo, wStopMode);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::AxisStopMultcoor(const WORD wCardNo, const WORD wCrd, const WORD wStopMode)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_stop_multicoor(wCardNo, wCrd, wStopMode);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::EmgStop(const WORD wCardNo)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_emg_stop(wCardNo);
	m_mtx.unlock();
	return sRet;
}

long CLeadShine::GetTargetPosition(const WORD wCardNo, const WORD wAxisNo)
{
	long sRet = 0;
	double dPosition = 0.0;
	m_mtx.lock();
	sRet = smc_get_target_position_unit(wCardNo, wAxisNo, &dPosition);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::SetAxisProFile(const WORD wCardNo, const WORD wAxisNo, const double dMinSpeed, const double dMaxSpeed, const double dTAcc, const double dTDec, const double dStopSpeed)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_set_profile_unit(wCardNo, wAxisNo, dMinSpeed, dMaxSpeed, dTAcc, dTDec, dStopSpeed);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::GetAxisProFile(const WORD wCardNo, const WORD wAxisNo, double& dMinSpeed, double& dMaxSpeed, double& dTAcc, double& dTDec, double& dStopSpeed)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_get_profile_unit(wCardNo, wAxisNo, &dMinSpeed, &dMaxSpeed, &dTAcc, &dTDec, &dStopSpeed);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::AxisMove(const WORD wCardNo, const WORD wAxisNo, const double lDistance, const WORD wMoveMode)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_pmove_unit(wCardNo, wAxisNo, lDistance, wMoveMode);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::AxisVMove(const WORD wCardNo, const WORD wAxisNo, const WORD wDir)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_vmove(wCardNo, wAxisNo, wDir);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::ChangSpeed(const WORD wCardNo, const WORD wAxisNo, const double dCurrentSpeed, const double dTAccDec)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_change_speed_unit(wCardNo, wAxisNo, dCurrentSpeed, dTAccDec);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::SetSpeed(const WORD wCardNo, const WORD wAxisNo, const double dTarSpeed, const double dTAccDec)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_change_speed_unit(wCardNo, wAxisNo, dTarSpeed, dTAccDec);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::ResetTargetPosition(const WORD wCardNo, const WORD wAxisNo, const double lDistance, const WORD wMoveMode)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_reset_target_position_unit(wCardNo, wAxisNo, lDistance);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::UpdateTargetPosition(const WORD wCardNo, const WORD wAxisNo, const double lDistance, const WORD wMoveMode)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_update_target_position_unit(wCardNo, wAxisNo, lDistance);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::ReadInBit(const WORD wCardNo, const WORD wBitNo, const WORD wNmcCardNo)
{
	if (wNmcCardNo == 0)
	{
		short sRet = 0;
		m_mtx.lock();
		sRet = smc_read_inbit(wCardNo, wBitNo);
		m_mtx.unlock();
		return sRet;
	}
	else
	{
		WORD wValue = 0;
		short sRet = 0;
		// 		m_cslock.lock();
		// 		sRet = nmcs_read_inbit(wCardNo, wNmcCardNo, wBitNo, &wValue);
		// 		m_cslock.unlock();
		// 		if (0 == sRet)
		// 		{
		// 			if (1 == wValue)
		// 			{
		// 				return 1;
		// 			}
		// 			else
		// 			{
		// 				return 0;
		// 			}
		// 
		// 		}
		// 		else
		{
			return sRet;
		}

	}

}

DWORD CLeadShine::ReadInPort(const WORD wCardNo, const WORD wPortNo, const WORD wNmcCardNo)
{
	if (wNmcCardNo == 0)
	{
		DWORD sRet = 0;
		m_mtx.lock();
		sRet = smc_read_inport(wCardNo, wPortNo);
		m_mtx.unlock();
		return sRet;
	}
	else
	{
		DWORD dwValue = 0;
		short sRet = 0;
		// 		m_cslock.lock();
		// 		sRet = nmcs_read_inport(wCardNo, wNmcCardNo, wPortNo, &dwValue);
		// 		m_cslock.unlock();
		// 		if (0 == sRet)
		// 		{
		// 			return dwValue;
		// 
		// 		}
		// 		else
		{
			return sRet;
		}
	}

}

short CLeadShine::ReadOutBit(const WORD wCardNo, const WORD wBitNo, const WORD wNmcCardNo)
{
	if (wNmcCardNo == 0)
	{
		DWORD sRet = 0;
		m_mtx.lock();
		sRet = smc_read_outbit(wCardNo, wBitNo);
		m_mtx.unlock();
		return sRet;
	}
	else
	{
		WORD wValue = 0;
		short sRet = 0;
		// 		m_cslock.lock();
		// 		sRet = nmcs_read_outbit(wCardNo, wNmcCardNo, wBitNo, &wValue);
		// 		m_cslock.unlock();
		// 		if (0 == sRet)
		// 		{
		// 			if (1 == wValue)
		// 			{
		// 				return 1;
		// 			}
		// 			else
		// 			{
		// 				return 0;
		// 			}
		// 
		// 		}
		// 		else
		{
			return sRet;
		}
	}

}

DWORD CLeadShine::ReadOutPort(const WORD wCardNo, const WORD wPortNo, const WORD wNmcCardNo /* = 0 */)
{
	if (wNmcCardNo == 0)
	{
		DWORD sRet = 0;
		m_mtx.lock();
		sRet = smc_read_outport(wCardNo, wPortNo);
		m_mtx.unlock();
		return sRet;
	}
	else
	{
		DWORD dwValue = 0;
		short sRet = 0;
		// 		m_cslock.lock();
		// 		sRet = nmcs_read_outport(wCardNo, wNmcCardNo, wPortNo, &dwValue);
		// 		m_cslock.unlock();
		// 		if (0 == sRet)
		// 		{
		// 			return dwValue;
		// 
		// 		}
		// 		else
		{
			return sRet;
		}
	}

}

short CLeadShine::WriteOutBit(const WORD wCardNo, const WORD wBitNo, const WORD wState, const WORD wNmcCardNo)
{
	if (wNmcCardNo == 0)
	{
		short sRet = 0;
		m_mtx.lock();
		sRet = smc_write_outbit(wCardNo, wBitNo, wState);
		m_mtx.unlock();
		return sRet;
	}
	else
	{
		short sRet = 0;
		// 		m_cslock.lock();
		// 		sRet = nmcs_write_outbit(wCardNo, wNmcCardNo, wBitNo, wState);
		// 		m_cslock.unlock();
		return sRet;
	}

}

short CLeadShine::WriteOutPort(const WORD wCardNo, const WORD wPortNo, const DWORD dwPortValue, const WORD wNmcCardNo)
{
	if (wNmcCardNo == 0)
	{
		short sRet = 0;
		m_mtx.lock();
		sRet = smc_write_outport(wCardNo, wPortNo, dwPortValue);
		m_mtx.unlock();
		return sRet;
	}
	else
	{
		short sRet = 0;
		// 		m_cslock.lock();
		// 		sRet = nmcs_write_outport(wCardNo, wNmcCardNo, wPortNo, dwPortValue);
		// 		m_cslock.unlock();
		return sRet;
	}

}

short CLeadShine::SetIOCountMode(const WORD wCardNo, const WORD wBitNo, const WORD wMode, const double dFilterTime)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_set_io_count_mode(wCardNo, wBitNo, wMode, dFilterTime);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::GetIOCountMode(const WORD wCardNo, const WORD wBitNo, WORD& wMode, double& dFilterTime)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_get_io_count_mode(wCardNo, wBitNo, &wMode, &dFilterTime);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::SetIOCountValue(const WORD wCardNo, const WORD wBitNo, const DWORD dwVaule)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_set_io_count_value(wCardNo, wBitNo, dwVaule);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::GetIOCountValue(const WORD wCardNo, const WORD wBitNo, DWORD& dwVaule)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_get_io_count_value(wCardNo, wBitNo, &dwVaule);
	m_mtx.unlock();
	return sRet;
}

DWORD CLeadShine::GetAxisStuts(const WORD wCardNo, const WORD wAxisNo)
{
	DWORD sRet = 0;
	m_mtx.lock();
	sRet = smc_axis_io_status(wCardNo, wAxisNo);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::SetAxisEnable(const WORD wCardNo, const WORD wAxisNo)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_write_sevon_pin(wCardNo, wAxisNo, 0);
	m_mtx.unlock();
	return sRet;
}

short  CLeadShine::SetAxisDisable(const WORD wCardNo, const WORD wAxisNo)
{
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_write_sevon_pin(wCardNo, wAxisNo, 1);
	m_mtx.unlock();
	return sRet;
}

short CLeadShine::LineMulticoor(WORD CardNo, WORD Crd, WORD axisNum, WORD* axisList, double* dDistList, WORD posi_mode)
{
	/*********************变量定义****************************/
	//short MyCardNo = 0; //连接号
	//short ret; //返回错误码
	//WORD Myposi_mode = 0; //0:相对模式，1：绝对模式
	//WORD MyCrd = 0; //参与插补运动的坐标系
	//WORD AxisArray[2]; //定义轴
	//AxisArray[0] = 2; //定义插补 0 轴为 X 轴
	//AxisArray[1] = 4; //定义插补 1 轴为 Y 轴
	//double MyMin_Vel = 0; //起始速度 0
	//double MyMax_Vel = 3000; //插补运动最大速度
	//double MyTacc = 0.2; //插补运动加速时间
	//double MyTdec = 0.1; //插补运动减速时间
	//double MyStop_Vel = 0; //插补运动停止速度
	//WORD MySmode = 0; //保留参数，固定值为 0
	//double MySpara = 0.05; //平滑时间为 0.05s
	//WORD MyaxisNum = 2; //插补运动轴数为 2
	//double Dist[2];
	//Dist[0] = 2000; //定义 X 轴运动距离
	//Dist[1] = 70000; //Y 轴运动距离
	///*********************函数调用执行**************************/
	////第一步、设置插补运动速度参数
	//ret = smc_set_vector_profile_unit(MyCardNo, MyCrd, MyMin_Vel, MyMax_Vel, MyTacc, MyTdec, MyStop_Vel);
	////第二步、设置插补运动平滑参数
	//ret = smc_set_vector_s_profile(MyCardNo, MyCrd, MySmode, MySpara);
	//smc_set_arc_limit(0, 0, 0, 0, 0);
	////第三步、启动插补运动
	//ret = smc_line_unit(MyCardNo, MyCrd, MyaxisNum, AxisArray, Dist, Myposi_mode);
	//return 0;
	short sRet = 0;
	m_mtx.lock();
	sRet = smc_line_unit(CardNo, Crd, axisNum, axisList, dDistList, posi_mode);
	m_mtx.unlock();
	return sRet;
}