// AM_PLCCtrl.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "AM_PanasoniPLC.h"
//#include "AM_PanasoniTcp.h"
//#include "AM_SieMens.h"

void AM_CreatePLCCtrl(CIPLCCtrl **pPLC, const int iMode, mutex& m, CISerialPort* pISerialPort)
{
	if (*pPLC)
	{
		delete *pPLC;	
	}

	*pPLC = NULL;
	CIPLCCtrl *pPLCTemp = NULL;
	switch (iMode)
	{
	case 0:
		{
			*pPLC = new CAM_PanasoniPLC(m, pISerialPort);
		}
		break;

	//case 1:
	//	{
	//		*pPLC = new CAM_PanasoniTcp();
	//	}
	//	break;

	//case 2:
	//	{
	//		*pPLC = new CAM_Siemens();
	//	}
	//	break;

	default:
		{
			*pPLC = new CAM_PanasoniPLC(m, pISerialPort);
		}
		break;
	}


}

void AM_ReleasePLCCtrl(CIPLCCtrl *pPLC)
{
	if (pPLC)
	{
		delete pPLC;	
	}

	pPLC = NULL;
}