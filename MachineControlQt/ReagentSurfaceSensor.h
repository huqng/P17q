#pragma once
class ReagentSurfaceSensor
{
public:
	ReagentSurfaceSensor();
	void Reconnect();
	int Calibrate();
	bool SurfaceTouched();
	bool ReadVal();
	int GetLastReadValue()const;
	int GetBase()const;
private:
	int m_iReadValue;
	int m_iBase;
	VCI_INIT_CONFIG vic;
	VCI_CAN_OBJ vcoCalib;
	VCI_CAN_OBJ vcoRecv;
	mutex m_mtx;
};

