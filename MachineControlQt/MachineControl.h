#pragma once

#include "IMachineControl.h"

/* members */
#include "Motion.h"
#include "Modules.h"
#include "ReagentSurfaceSensor.h"

enum class RunState
{
	Running,
	Suspending,
	Stopped,
	Resetting,
	Exiting
};

enum class QuestionAnswer
{
	No,
	Yes,
	Waiting
};

class CMachineControl: public CIMachineControl
{
	Q_OBJECT
public:
	CMachineControl();
	virtual ~CMachineControl()override;
	virtual void Reset()override;
	virtual void Start()override;
	virtual void Abort()override;
	virtual void Suspend()override;
	virtual void Resume()override;
	virtual void Exit()override;

	virtual bool IsRunning()override;
	virtual bool IsSuspend()override;
	virtual bool IsResetting()override;
	virtual bool IsStopped()override;

	virtual void Test(int iArg)override;
	/* get slide state */
	virtual bool HoldsShelfAt(int iSlotIndex)override;


	virtual int GetShelfHolderState(int iIndex, int iSlideIndex)override;
	virtual int GetRepairModState(int iIndex, int iSlideIndex)override;
	virtual int TransitState(int iSlideIndex)override;
	virtual int GetReactionTotalStep(int iIndex, int iSlideIndex)override;
	virtual int GetReactionCompletedStep(int iIndex, int iSlideIndex)override;
	virtual void ClearShelfHolder(int iSlotIndex)override;
	virtual void ReloadShelfHolder(int iSlotIndex)override;
	virtual void ClearShelfHolder()override;
	virtual int ShelfCnt()override;
	virtual int HasSlideAtReactMod(int iModIndex, int iSlideIndex)override;

	virtual void ReloadConfig()override;
	virtual void ReloadCoord()override;
	virtual void ReloadReagentShelf()override;

	virtual int MoveToCoord(const char* pCoordName)override;
	virtual int MoveAxis(int iAxisNo, double dLength, bool bAbsolutly)override;
	virtual int HomeMoveAxis(int iAxisNo)override;
	virtual double GetAxisPos(int iAxisNo)override;
	virtual int ZAxisUp()override;

	virtual void SetElectroMagnet(bool bOn)override;
	virtual void SetNeedleCleanerPumpOn(bool bOn)override;
	virtual void SetReactModDrainPumpOn(int iIndex, bool bOn)override;
	virtual void SetCleanerSlotDrainPumpOn(bool bOn)override;

	virtual void HandleRepairModAlarm(const char* pRelayName)override;
	virtual void HandleRepairModAlarm2(int iAlarmIndex)override;
	virtual void AnswerQuestion(bool bYes)override;

	virtual void ClearReagentSurfaceAltitudeRecord(int iPos)override;
	virtual void WriteRelay(int iDevID, const char* pAddr, bool bVal)override;
	virtual bool ReadRelay(int iDevID, const char* pAddr)override;
	virtual void WriteDT(int iDevID, const char* pName, int iVal)override;
	virtual int ReadDT(int iDevID, const char* pName)override;
private:
	//boost::threadpool::pool	m_tp;
	boost::asio::thread_pool m_tp;
	
	RunState				m_state;
	mutex					m_mtxState;
	condition_variable		m_condRunning;

	void SetState(RunState s);
	bool CheckRunState();

	/* "flow" */
	void Dispatch();
	void ImplReset(int iStep, bool bHasRepairMod);
	void ImplCheckReactionModule(int iIndex, int iStep);
	void ImplCheckRepairModule(int iIndex, int iStep);
	void ImplCheckRepairModAlarm(int iStep);
	void ImplDrain(int iStep);
	void ImplCleanerSlotDrain(int iStep); /* call manually */
	void ImplPickShelf(int iStep);
	void ImplPickFromReaction(int iStep);
	void ImplPickFromRepair(int iStep);
	void ImplPutShelf(int iStep);
	void ImplPutToReaction(int iStep);
	void ImplPutToRepair(int iStep);
	void ImplPipet(int iStep, int iReactModIndex);

	void ImplTemperatureUpdate2(bool bHasRepairMod);
	void ImplAxisCoordUpdate();

	/* tests */
	void ImplTestingRepairMod(int iIndex, int iStep);
	void ImplTestSurfaceSensor(int iStep);
	atomic_bool m_bTestingFlag;

	/* ``subflow`` */
	void SubFlowZAxesUp(function<void()> prevTask, int iStep);
	void SubFlowXYHome(function<void()> prevTask, int iStep);
	void SubFlowZHome(function<void()> prevTask, int iStep);
	void SubFlowMultiAxisMove(function<void()> prevTask, int iStep, MultiAxisCoord);

	/* question */
	QuestionAnswer AskQuestion(QString strText, function<void()> fYes = [] {}, function<void()> fNo = [] {});
	mutex m_mtxQuestion;
	mutex m_mtxAnswer;
	condition_variable m_condAnswer;
	QuestionAnswer m_answer;

	/* member */
	Motion					m_motion;
	ReagentSurfaceSensor	m_SurfaceSensor;

	/* module */
	mutex m_mtxShelf;
	RepairModule* m_repairMod;
	map<int, ReactionModule*>	m_mapReactMods;
	mutex*			m_pMtxSerial;
	CISerialPort*	m_pSerial;
	atomic_int m_iUnhandledAlarmCnt;
	map<int, ShelfHolder*>		m_mapShelfHolders;
	Transit*					m_transit;



	/* pipetting */
	queue<int>	m_qReactionQueue;
	mutex		m_mtxReaction;
	enum class ReactionState
	{
		New,		/* 下一Step未开始（Step已完成） */
		Pending,	/* 已添加到队列等待操作的 */
		Pipetting,	/* 加液中 */
		Reacting	/* 反应/延时 */
	};
	map<int, ReactionState>				m_mapReactionState;
	map<int, double> m_mapLatestReagentSurfaceAltitude;

	/* config */ 
	MultiAxisCoord GetMultiAxisCoord(const string& strCoordName);
	double GetCommonConfig(const string& strGroup, const string& strKey); /* read a double from `config.ini` */
	int GetAxisNo(const string& strAxisName);

	/* utils */
	bool HasCompletedReactionModule();
	bool HasEmptyReactionModule();
	bool HasCompletedRepairModule();
	bool HasEmptyRepairModule();
	int GetOneCompletedReactionModule();
	int GetOneEmptyReactionModule();
	int GetOneCompletedRepairModule();
	int GetOneEmptyRepairModule();
	bool HasWaitingShelfHolder();
	bool HasEmptyShelfHolder();
	int GetOneWaitingShelfHolder();
	int GetOneEmptyShelfHolder();
	bool FirstWaitingShelfNeedsRepair();

	void ClearReactionMod(int iIndex);
	void ClearRepairMod(int iIndex);
	void ClearTransit();
private:
	inline void StartPickFromRepair() { 
			
		boost::asio::post(m_tp, bind(&CMachineControl::ImplPickFromRepair, this, 0));
		//  boost::asio::post(m_tp, bind(&CMachineControl::ImplPickFromRepair, this, 0)); 
	}
	inline void StartPickShelf() { boost::asio::post(m_tp, bind(&CMachineControl::ImplPickShelf, this, 0)); }


};

