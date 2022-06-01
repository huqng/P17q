#include "stdafx.h"

#include <qstringlist.h>

#include "FormulaMgr.h"

string StrToHex(string strIn)
{
	static boost::format fmt("%02X");
	string strHex;
	for (unsigned char uc : strIn)
	{
		strHex.append((fmt % int(uc)).str());
	}
	return strHex;
}

FormulaInfo formulaInfo;

FormulaInfo::FormulaInfo()
{
	mINI::INIFile ini("./Formula/All");
	mINI::INIStructure allFml;
	ini.read(allFml);

	for (auto fmlIndex : allFml["AllFormula"])
	{
		mINI::INIFile iniFml(string("./Formula/").append(StrToHex(fmlIndex.first)));
		mINI::INIStructure fml;
		iniFml.read(fml);
		Formula vec;
		for (auto fmlRow : fml[fmlIndex.first])
		{
			string strRow = fmlRow.second;
			vector<string> vecSplit;
			boost::algorithm::split(vecSplit, strRow, boost::is_any_of(","));

			vec.push_back({ vecSplit[0], vecSplit[1], stoi(vecSplit[2]), stoi(vecSplit[3]) });
		}
		m_mapFormula[fmlIndex.first] = vec;
	}
}

int FormulaInfo::GetFormulaStepCnt(const string& strFormula)const
{
	return m_mapFormula.at(strFormula).size();
}
FormulaStep FormulaInfo::GetFormulaStep(const string& strFormula, int iStep)const
{
	return m_mapFormula.at(strFormula).at(iStep);
}

string FormulaInfo::GetReagentName(const string& strFormula, int iStep)const
{
	const Formula& fml = m_mapFormula.at(strFormula);
	if (fml.size() > iStep)
		return fml.at(iStep).reagent;
	else
		return "";
}

int FormulaInfo::GetQuantity(const string& strFormula, int iStep)const
{
	return m_mapFormula.at(strFormula).at(iStep).quantity;
}

int FormulaInfo::GetReactionTime(const string& strFormula, int iStep)const
{
	return m_mapFormula.at(strFormula).at(iStep).reactionTime;
}
