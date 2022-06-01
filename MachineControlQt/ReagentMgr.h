#pragma once
class ReagentMgr
{
public:
	ReagentMgr();

	void Reload();
	int GetReagentPosition(const string& strReagent);
	bool IsUsedUp(int iPos)const;
	void SetUsedUp(int iPos, bool bUsedUp = true);
private:
	mINI::INIStructure m_rs;
	map<int, bool> m_mapUsedUp;

};

extern ReagentMgr rmgr;

