#pragma once

#include "machinecontrolqt_global.h"

class MACHINECONTROLQT_EXPORT CIMachineControl: public QObject
{
	Q_OBJECT
public:
	virtual ~CIMachineControl() {}

	virtual void Reset() = 0;
	virtual void Start() = 0;
	virtual void Abort() = 0;
	virtual void Suspend() = 0;
	virtual void Resume() = 0;
	virtual void Exit() = 0;

	virtual bool IsRunning() = 0;
	virtual bool IsSuspend() = 0;
	virtual bool IsResetting() = 0;
	virtual bool IsStopped() = 0;

	virtual void Test(int iArg) = 0;

	/* get module/shelf/slide state */
	virtual bool HoldsShelfAt(int iShelfHolderIndex) = 0;
	virtual int GetShelfHolderState(int iIndex, int iSlideIndex) = 0;
	virtual int GetRepairModState(int iIndex, int iSlideIndex) = 0;
	virtual int TransitState(int iSlideIndex) = 0;
	virtual int GetReactionTotalStep(int iIndex, int iSlideIndex) = 0;
	virtual int GetReactionCompletedStep(int iIndex, int iSlideIndex) = 0;
	virtual void ClearShelfHolder(int iSlotIndex) = 0;
	virtual void ReloadShelfHolder(int iSlotIndex) = 0;
	virtual void ClearShelfHolder() = 0;
	virtual int ShelfCnt() = 0;
	virtual int HasSlideAtReactMod(int iModIndex, int iSlideIndex) = 0;

	/* debug / configure */
	virtual void ReloadConfig() = 0;
	virtual void ReloadCoord() = 0;
	virtual void ReloadReagentShelf() = 0;

	virtual int MoveToCoord(const char* pCoordName) = 0;
	virtual int MoveAxis(int iAxisNo, double dLength, bool bAbsolutly) = 0;
	virtual int HomeMoveAxis(int iAxisNo) = 0;
	virtual double GetAxisPos(int iAxisNo) = 0;
	virtual int ZAxisUp() = 0;

	virtual void SetElectroMagnet(bool bOn) = 0;
	virtual void SetNeedleCleanerPumpOn(bool bOn) = 0;
	virtual void SetReactModDrainPumpOn(int iIndex, bool bOn) = 0;
	virtual void SetCleanerSlotDrainPumpOn(bool bOn) = 0;

	virtual void HandleRepairModAlarm(const char* pRelayName) = 0;
	virtual void HandleRepairModAlarm2(int iAlarmIndex) = 0;
	virtual void AnswerQuestion(bool Yes) = 0;

	virtual void ClearReagentSurfaceAltitudeRecord(int iPos) = 0;
	virtual void WriteRelay(int iDevID, const char* pAddr, bool bVal) = 0;
	virtual bool ReadRelay(int iDevID, const char* pAddr) = 0;
	virtual void WriteDT(int iDevID, const char* pName, int iVal) = 0;
	virtual int ReadDT(int iDevID, const char* pName) = 0;
signals:
	void signalResetFinished();
	void signalUpdateAllView();
	void signalUpdateReactionModuleView(int iIndex);
	void signalUpdateRepairModuleView(int iIndex);
	void signalUpdateShelfHolderView(int iIndex);
	void signalUpdateTransitShelfView();

	void signalMsg(QString);									/* 显示消息信号 */
	void signalUpdateReactModTem(double dCDeg, int iIndex);		/* 更新反应模块温度信号 */
	void signalUpdateRepairModTem(double dCDeg, int iIndex);	/* 更新修复模块温度信号 */
	void signalUpdateCoord(double, double, double, double, double);					/* 更新（伪）实时坐标 */
	void signalInterrupt(QString);								/* 显示消息并暂停运行 */
	void signalRepairModAlarm(int);								/* 修复模块报警，选择写relay */
	void signalRepairModAlarm2(int);							/* 修复模块报警，清relay */

	void signalQuestion(QString);
};

extern "C" MACHINECONTROLQT_EXPORT CIMachineControl* GetMachineControlInstance();
