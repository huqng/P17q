#include "stdafx.h"
#include "Motion.h"

Motion::Motion()
{

}

int Motion::Init(const string& strIPAddr)
{
	return this->InitCard(strIPAddr);
}

int Motion::MultiAxisMoveSimple(const MultiAxisCoord& pos)
{
	int iRet = 0;
	for (pair<int, double> p : pos)
	{
		iRet = SingleAxisMoveAbs(p.first, p.second);
		if (iRet != 0)
		{
			break;
		}
	}

	return iRet;
}

int Motion::SingleAxisMoveRel(int iAxisNo, double dRelPos)
{
	if (this->AxisMove(0, iAxisNo, dRelPos, 0) == 0)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

int Motion::SingleAxisMoveAbs(int iAxisNo, double dAbsPos)
{
	if (this->AxisMove(0, iAxisNo, dAbsPos, 1) == 0)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

int Motion::AxisHome(int iAxisNo)
{
	if (this->HomeMove(0, iAxisNo) == 0)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

bool Motion::IsHomeCompleted(int iAxisNo)
{
	WORD wTemp = 0;
	this->GetHomeMoveResult(0, iAxisNo, wTemp);
	return (wTemp != 0);
}

bool Motion::IsAxisBusy(const MultiAxisCoord& pos)
{
	for (pair<int, double> p : pos)
	{
		if (IsAxisBusy(p.first))
			return true;
	}
	return false;
}

bool Motion::IsAxisBusy(int iAxisNo)
{
	return (this->GetAxisDone(0, iAxisNo) == 0);
}
