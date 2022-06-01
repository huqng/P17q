#pragma once

#include "def.h"
#include "LeadShine.h"

class Motion: public CLeadShine
{
public:
	Motion();
	int Init(const string& strIPAddr = "192.168.5.11");
	int MultiAxisMoveSimple(const MultiAxisCoord& pos);
	int SingleAxisMoveRel(int iAxisNo, double dRelPos);
	int SingleAxisMoveAbs(int iAxisNo, double dAbsPos);
	int AxisHome(int iAxisNo);
	bool IsHomeCompleted(int iAxisNo);

	bool IsAxisBusy(const MultiAxisCoord& pos);
	bool IsAxisBusy(int iAxisNo);
};
