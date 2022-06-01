#pragma once

class CLeadShine
{
public:
	CLeadShine();
	~CLeadShine();

	short InitCard(const string& strIp);
	short ResetCard(const WORD wCardNo, const int iType);	//iType 0: 表示硬件复位 1:表示软件复位
	short LoadConfigFile(const WORD wCardNo, const char* pcFileName);
	short IsCardConnect(const WORD wCardNo);

	//使能
	short SetAxisEnable(const WORD wCardNo, const WORD wAxisNo);
	short SetAxisDisable(const WORD wCardNo, const WORD wAxisNo);
	//

	//////////////////////////////////////////////////////////////////////////
	short SetPulseOutMode(const WORD wCardNo, const WORD wAxisNo, const WORD wOutMode);  //脉冲模式设置
	short GetPulseOutMode(const WORD wCardNo, const WORD wAxisNo, WORD& wOutMode);
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	short SetHomePinLogic(const WORD wCardNo, const WORD wAxisNo, const WORD wOrgLogic, const double dFilter);
	short GetHomePinLogic(const WORD wCardNo, const WORD wAxisNo, WORD& wOrgLogic, double& dFilter);
	short SetHomeMode(const WORD wCardNo, const WORD wAxisNo, const WORD wHomdDir, const double dSpeedMode, const WORD wHomeMode, const WORD wEZCount);  //dSpeedMode：回原速度模式0：低速 1：高速 HomeMode:回原方式 方式比较多有看函数说明
	short GetHomeMode(const WORD wCardNo, const WORD wAxisNo, WORD& wHomdDir, double& dSpeedMode, WORD& wHomeMode, WORD& wEZCount);
	short SetHomeELReturn(const WORD wCardNo, const WORD wAxisNo, const WORD wEnable);																	//设置回零遇到限位是否反找
	short SetHomePosition(const WORD wCardNo, const WORD wAxisNo, const WORD wEnable, const double dPosition);											//设置回零偏移量以及清零模式 wEanble：0 不清零 1回零完成后清零，再偏移 2 回零以及偏移完成后清零
	short GetHomePosition(const WORD wCardNo, const WORD wAxisNo, WORD& wEnable, double& dPosition);
	short HomeMove(const WORD wCardNo, const WORD wAxisNo);																								//回原点运动
	short GetHomeMoveResult(const WORD wCardNo, const WORD wAxisNo, WORD& wState);		//读取原点状态 wState: 1 回原点完成 0 回零未完成
	short SetHomeLatchMode(const WORD wCardNo, const WORD wAxisNo, const WORD wEnable, const WORD wLogic, const WORD wSource);//原点锁存函数
	short GetHomeLatchMode(const WORD wCardNo, const WORD wAxisNo, WORD& wEnable, WORD& wLogic, WORD& wSource);
	long GetHomeLatchFlag(const WORD wCardNo, const WORD wAxisNo); //返回值:0 未锁存 1 锁存
	long GetHomeLatchValue(const WORD wCardNo, const WORD wAxisNo); //返回值锁存值 单位Pulse
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	short SetELMode(const WORD wCardNo, const WORD wAxisNo, const WORD wELEnable, const WORD wELLogic, const WORD wELMode);//设置EL限位信号
	short GetELMode(const WORD wCardNo, const WORD wAxisNo, WORD& wELEnable, WORD& wELLogic, WORD& wELMode);
	short SetSoftLimit(const WORD wCardNo, const WORD wAxisNo, const WORD wEnable, const WORD wSourceSel, const WORD wSLAction, const double lNLimit, const double lPLimit);//设置软限位
	short GetSoftLimit(const WORD wCardNo, const WORD wAxisNo, WORD& wEnable, WORD& wSourceSel, WORD& wSLAction, double& lNLimit, double& lPLimit);
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	short SetPosition(const WORD wCardNo, const WORD wAxisNo, const double lCurrentPosition);	//设置指令脉冲位置  lCurrentPosition 指令脉冲位置 单位Pulse
	short GetPosition(const WORD wCardNo, const WORD wAxisNo, double& dPosition);
	short SetEncoder(const WORD wCardNo, const WORD wAxisNo, const double lCurrentPosition);
	short GetEncoder(const WORD wCardNo, const WORD wAxisNo, double& dPosition);
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	double GetCurrentSpeed(const WORD wCardNo, const WORD wAxisNo);		//读取指定轴的速度 单位:Pulse/s
	short GetLinkState(const WORD wCardNo, WORD& wLinkState);			//检测主卡与接线盒的连接状态 wLinkState 0 连接 1 断开
	short GetAxisDone(const WORD wCardNo, const WORD wAxisNo);			//检测指定轴运动状态  返回值 0 运动中 1 停止	注意：此函数适用于单轴、PVT运动
	short GetAxisDoneMultcoor(const WORD wCardNo, const WORD wCrd);		//检测坐标系运动状态  返回值 0 运动中 1 停止 [wCrd 指定控制卡上的坐标系号 （取值范围： 0~1] 注：此函数仅适用于插补运动
	DWORD GetAxisIoState(const WORD wCardNo, const WORD wAxisNo);		//读取指定轴有关运动信号的状态 返回值见说明书
	short AxisStop(const WORD wCardNo, const WORD wAxisNo, const WORD wStopMode);  //指定轴停止运动  wStopMode 0 减速停止 1 立即停止 注：此函数适用于单轴、PVT运动
	short AxisStopMultcoor(const WORD wCardNo, const WORD wCrd, const WORD wStopMode);//停止坐标系内所有轴  wStopMode 0 减速停止 1 立即停止  [wCrd 指定控制卡上的坐标系号 （取值范围： 0~1] 注：此函数仅适用于插补运动
	short EmgStop(const WORD wCardNo); //紧急停止所有轴
	long GetTargetPosition(const WORD wCardNo, const WORD wAxisNo);//读取正在运动的目标位置 返回值：目标位置 单位Pulse
	short LineMulticoor(WORD CardNo, WORD Crd, WORD axisNum, WORD* axisList, double* dDistList, WORD posi_mode); //差不运动
	//////////////////////////////////////////////////////////////////////////

	//单轴运动 速度 曲线 设置 函
	//////////////////////////////////////////////////////////////////////////
	short SetAxisProFile(const WORD wCardNo, const WORD wAxisNo, const double dMinSpeed, const double dMaxSpeed, const double dTAcc, const double dTDec, const double dStopSpeed);
	short GetAxisProFile(const WORD wCardNo, const WORD wAxisNo, double& dMinSpeed, double& dMaxSpeed, double& dTAcc, double& dTDec, double& dStopSpeed);
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	short AxisMove(const WORD wCardNo, const WORD wAxisNo, const double lDistance, const WORD wMoveMode);		//单轴移动wMoveMode 0：相对坐标 1：绝对坐标
	short AxisVMove(const WORD wCardNo, const WORD wAxisNo, const WORD wDir);  //单轴连续运动  wDir 0:正方向 1: 负方向
	short ChangSpeed(const WORD wCardNo, const WORD wAxisNo, const double dCurrentSpeed, const double dTAccDec); //在线改变指定轴的当前运动速度 dCurrentSpeed 为改变后的速度 单位Pulse/s
	short SetSpeed(const WORD wCardNo, const WORD wAxisNo, const double dTarSpeed, const double dTAccDec);
	short ResetTargetPosition(const WORD wCardNo, const WORD wAxisNo, const double lDistance, const WORD wMoveMode); //在线改变指定轴当前目标位置 wMoveMode 保留 注：只能在运动中
	short UpdateTargetPosition(const WORD wCardNo, const WORD wAxisNo, const double lDistance, const WORD wMoveMode); //强制改变指定轴当前目标位置 注：运动中、停止中都可以
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//IO 控制
	short ReadInBit(const WORD wCardNo, const WORD wBitNo, const WORD wNmcCardNo = 0);
	DWORD ReadInPort(const WORD wCardNo, const WORD wPortNo, const WORD wNmcCardNo = 0);
	short ReadOutBit(const WORD wCardNo, const WORD wBitNo, const WORD wNmcCardNo = 0);
	DWORD ReadOutPort(const WORD wCardNo, const WORD wPortNo, const WORD wNmcCardNo = 0);
	short WriteOutBit(const WORD wCardNo, const WORD wBitNo, const WORD wState, const WORD wNmcCardNo = 0);
	short WriteOutPort(const WORD wCardNo, const WORD wPortNo, const DWORD dwPortValue, const WORD wNmcCardNo = 0);

	short SetIOCountMode(const WORD wCardNo, const WORD wBitNo, const WORD wMode, const double dFilterTime);
	short GetIOCountMode(const WORD wCardNo, const WORD wBitNo, WORD& wMode, double& dFilterTime);
	short SetIOCountValue(const WORD wCardNo, const WORD wBitNo, const DWORD dwVaule);
	short GetIOCountValue(const WORD wCardNo, const WORD wBitNo, DWORD& dwVaule);

	DWORD GetAxisStuts(const WORD wCardNo, const WORD wAxisNo);
	//////////////////////////////////////////////////////////////////////////

private:
	mutex m_mtx;
};
