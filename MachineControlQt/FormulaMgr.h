#pragma once

struct FormulaStep
{
	std::string type;
	std::string reagent;
	int quantity, reactionTime;
};

class FormulaInfo
{
	using Formula = vector<FormulaStep>;
public:
	FormulaInfo();
	int GetFormulaStepCnt(const string& strFormula)const;
	FormulaStep GetFormulaStep(const string& strFormula, int iStep)const;
	string GetReagentName(const string& strFormula, int iStep)const;
	int GetQuantity(const string& strFormula, int iStep)const;
	int GetReactionTime(const string& strFormula, int iStep)const;

private:
	map<string, Formula> m_mapFormula; /* [formulaname]->formula */
};

extern FormulaInfo formulaInfo;
