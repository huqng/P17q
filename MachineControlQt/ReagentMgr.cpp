#include "stdafx.h"
#include "ReagentMgr.h"

ReagentMgr rmgr;

ReagentMgr::ReagentMgr()
{
	this->Reload();
}

void ReagentMgr::Reload()
{
	m_rs.clear();
	mINI::INIFile ini("./Config/ReagentShelf.ini");
	ini.read(m_rs);
}

int ReagentMgr::GetReagentPosition(const string& strReagent)
{
	for (const auto& iter : m_rs["1"])
	{
		if (iter.second == strReagent && !IsUsedUp(stoi(iter.first)))
		{
			return stoi(iter.first);
		}
	}
	return -1;
}

bool ReagentMgr::IsUsedUp(int iPos)const
{
	if (m_mapUsedUp.find(iPos) != m_mapUsedUp.end() && m_mapUsedUp.at(iPos))
		return true;
	else
		return false;
}

void ReagentMgr::SetUsedUp(int iPos, bool bUsedUp)
{
	m_mapUsedUp[iPos] = bUsedUp;
}
