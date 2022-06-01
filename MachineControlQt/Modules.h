#pragma once

bool	PLCReadAndClearRelay(CIPLCCtrl* pPLC, const string& strSlave, const string& strSection, const string& strKey);
int		PLCWriteRelay(CIPLCCtrl* pPLC, const string& strSlave, const string& strSection, const string& strKey);
int		PLCWriteDT(CIPLCCtrl* pPLC, int iValue, const string& strSlave, const string& strSection, const string& strKey);
bool	PLCReadRelay(CIPLCCtrl* pPLC, const string& strSlave, const string& strSection, const string& strKey);
int		PLCClearRelay(CIPLCCtrl* pPLC, const string& strSlave, const string& strSection, const string& strKey);
int		PLCReadDT(CIPLCCtrl* pPLC, const string& strSlave, const string& strSection, const string& strKey);


struct Slide /* 玻片 */
{
	string strID;
	string strHospital;
	map<int, string> mapVReagent;
};

struct SlideShelf /* 玻片架 */
{
public:
	SlideShelf();
	bool HasSlideAt(int iIndex)const;
	void AddSlide(const string& strID, const string& strHospital, const map<int, string>&& mapVReagent);
	int TotalStep()const;

	int m_iCurrentStep;
	int m_iRepairTime;
	int m_iRepairTemp;
	map<int, Slide> m_mapSlides;
	string m_strFormula;
};

class ReactionModule /* 反应模块 */
{
public:	
	ReactionModule(int iIndex, mutex* pMtx, CISerialPort* pSerial);
	~ReactionModule();
	SlideShelf* TakeShelf();
	void PutShelf(SlideShelf*);
	bool Empty()const;
	bool Reacting()const;
	bool HasSlideAt(int iIndex)const;
	string FormulaName()const;
	string GetVReagent(int iSlideIndex, int iStep);
	int CurrentStep()const;
	int TotalStep()const;
	void IncStep();
	void DecStep();
	void Clear();

	/* for outside use */
	void WriteDT(const char* pName, int iVal);
	int ReadDT(const char* pName);
	void WriteRelay(const char* pAddr, bool bVal);
	bool ReadRelay(const char* pAddr);
		
	/* Reaction module PLC IO */
	/* Relay */ /* RAC: Read And Clear */
	int		WriteRelayStartTiming(int iSlideIndex, bool bVal = true);
	int		WriteRelayWashStart(bool bVal = true);
	bool	RACRelayWashCompletion();
	int		WriteRelayMoisturize(bool bVal = true);

	int		WriteRelayPBSStart(bool bVal = true);
	int		WriteRelayDelayStart(bool bVal = true);
	int		WriteRelayPBS(int iSlideIndex);
	
	int		WriteRelayPutShelfRequest(bool bVal = true);
	int		WriteRelayPutShelfCompletion(bool bVal = true);
	int		WriteRelayPickShelfRequest(bool bVal = true);
	int		WriteRelayPickShelfCompletion(bool bVal = true);
	int		WriteRelayPipetRequest(bool bVal = true);
	int		WriteRelayPipetting(bool bVal = true);
	int		WriteRelayPipetCompletion(bool bVal = true);
	bool	RACRelayPutShelfPermission();
	bool	RACRelayLidOpening();
	bool	RACRelayPickShelfPermission();
	bool	RACRelayPipetPermission();
	bool	RACRelayResetCompletion();

	bool	RACRelayReactionCompletion(int iSlideIndex);
	int		WriteRelayHeat(bool bVal = true);
	int		WriteRelayReset(bool bVal = true);
	/* DT */
	int WriteDTCurrentTemp(int iTemp);
	int WriteDTLowerTemp(int iTemp);
	int WriteDTHigherTemp(int iTemp);
	int WriteDTWashInterval(int iTime);
	int WriteDTWashCount(int iTime);
	int WriteDTAddPBS(int iVal);
	int WriteDTPBSDelay(int iVal);
	int WriteDTPBSReactionTime(int iTime);
	int WriteDTMoisturizationInterval(int iInterval);
	int WriteDTDelayTime(int iTime);
	int WriteDTReactionTime(int iSlideIndex, int iTime);
	int WriteDTLidMotorSpeed(int iSpeed);
	int WriteDTShelfMotorSpeed(int iSpeed);
	int WriteDTPBSMotorSpeed(int iSpeed);
	int WriteDTHorPos(int iPos);
	int WriteDTVerPos(int iPos);
	int ReadDTCurrentTemp();
	int ReadDTLowerTemp();
	int ReadDTHigherTemp();
	int ReadDTWashInterval();
	int ReadDTWashCount();
	int ReadDTAddPBS();
	int ReadDTPBSDelay();
	int ReadDTPBSReactionTime();
	int ReadDTMoisturizationInterval();
	int ReadDTDelayTime();
	int ReadDTReactionTime(int iSlideIndex);
	int ReadDTLidMotorSpeed();
	int ReadDTShelfMotorSpeed();
	int ReadDTPBSMotorSpeed();
	int ReadDTHorPos();
	int ReadDTVerPos();

	double GetCurrentTemp();

private:
	int m_iThisModIndex;	

	SlideShelf* m_pShelf;

	mutex* m_pMtxSerial;
	CISerialPort* m_pSerial;
	CIModBus* m_pModbus;
	CIPLCCtrl* m_pPLC;

	map<string, function<int()>> m_mapReadDTFunc;
	map<string, function<int(int)>> m_mapWriteDTFunc;
	void InitFuncTable();
};

enum class RepairModState
{
	Empty, Repairing, Completed
};
class RepairModule
{
public:
	RepairModule(mutex* pMtx, CISerialPort* pSerial);
	~RepairModule();
	SlideShelf* TakeShelf(int iIndex);
	SlideShelf* GetShelf(int iIndex);
	void PutShelf(int iIndex, SlideShelf*);
	bool Empty(int iIndex)const;
	bool HasSlideAt(int iIndex, int iSlideIndex)const;
	int WriteRepairParam(int iIndex);
	void Clear(int iIndex);

	bool IsRepairing(int iIndex)const;
	bool IsCompleted(int iIndex)const;
	void SetStateRepairing(int iIndex);
	void SetStateCompleted(int iIndex);

	void WriteDT(const char* pName, int iVal);
	int ReadDT(const char* pName);
	void WriteRelay(const char* pAddr, bool bVal);
	void WriteRelayByName(const char* pName, bool bVal);
	bool ReadRelay(const char* pAddr);

	/* IO */
	int WriteRelayContinueAddingWhenAddingTimeout(int iIndex);
	int WriteRelayStopAddingWhenAddingTimeout(int iIndex);
	int WriteRelayContinueAddingWhenAddingRefillingTimeout(int iIndex);
	int WriteRelayStopAddingWhenAddingRefillingTimeout(int iIndex);
	int WriteRelayContinueAddingWhenCoolingAddingTimeout(int iIndex);
	int WriteRelayStopAddingWhenCoolingAddingTimeout(int iIndex);

	bool RACRelayAlarm(int iAlarmIndex);
	bool ReadRelayAlarm(int iAlarmIndex);
	int ClearRelayAlarm(int iAlarmIndex);

	int WriteDTAddingTimeout(int iIndex, int iTime);
	int WriteDTRefillingTimeout(int iIndex, int iTime);
	int WriteDTRefillingInterval(int iIndex, int iTime);
	int WriteDTCoolingAddingTimeout(int iIndex, int iTime);
	int ReadDTAddingTimeout(int iIndex);
	int ReadDTRefillingTimeout(int iIndex);
	int ReadDTRefillingInterval(int iIndex);
	int ReadDTCoolingAddingTimeout(int iIndex);

	int WriteRelayReset();
	int WriteRelayFlowStart();

	bool RACRelayPutShelfPermission(int iIndex);
	int WriteRelayPutShelfCompletion(int iIndex);

	bool RACRelayPickShelfPermission(int iIndex);
	int WriteRelayPickShelfCompletion(int iIndex);

	int WriteDTRepairTime(int iIndex, int iTime);
	int WriteDTTempPickShelf(int iIndex, int iTemp);
	int WriteDTTempKeep(int iIndex, int iTemp);
	int WriteDTTempLower(int iIndex, int iTemp);
	int WriteDTTempUpper(int iIndex, int iTemp);
	int WriteDTCurrentTemp(int iIndex, int iTemp);
	int ReadDTRepairTime(int iIndex);
	int ReadDTTempPickShelf(int iIndex);
	int ReadDTTempKeep(int iIndex);
	int ReadDTTempLower(int iIndex);
	int ReadDTTempUpper(int iIndex);
	int ReadDTCurrentTemp(int iIndex);

	double GetCurrentTemp(int iIndex);
	
private:
	map<int, SlideShelf*> m_mapShelf;
	map<int, RepairModState> m_mapState;

	mutex* m_pMtxSerial;
	CISerialPort* m_pSerial;
	CIModBus* m_pModbus;
	CIPLCCtrl* m_pPLC;

	map<string, function<int()>> m_mapReadDTFunc;
	map<string, function<int(int)>> m_mapWriteDTFunc;
	void InitFuncTable();
};

class ShelfHolder /* '玻片架'架 */
{
public:
	SlideShelf* TakeShelf();
	void PutShelf(SlideShelf*);
	bool Empty()const;
	bool Waiting()const;
	bool NeedsRepair()const;
	bool HasSlideAt(int iIndex)const;
	void Clear();
private:
	SlideShelf* m_pShelf;
};

class Transit
{
public:
	void Pick(SlideShelf*);
	SlideShelf* Put();
	bool NeedsRepair()const;
	bool Empty()const;
	bool Waiting()const;
	bool HasSlideAt(int iIndex)const;
	void Clear();
private:
	SlideShelf* m_pShelf;
};
