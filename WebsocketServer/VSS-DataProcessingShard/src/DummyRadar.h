#pragma once

#include <vector>

#define TRIGGER_ALERT_INTERVAL	5000 // don't trigger the same alert for the same status until this amount of time passes

// pretends to be a radar and injects data into DSManager
class DummyRadar
{
public:
	DummyRadar(__int64 dModuleId, unsigned __int64 dUpdatePeriod, __int64 dObjectsMoving);
	void DisconnectRadar();
private:
	// periodically does things
	void AsyncExecuteThread(class  WatchdogThreadData* wtd);
	// emulate a radar input behavior
	void UpdateObjectPositions();
	// emulate data received from data
	void EmulateRadarRead();
	// run scripts on radar feed. These scripts are based on AlertDefinition table
	void RunAlertDetectScripts();
	__int64 m_dModuleId;
	unsigned __int64 m_dUpdatePeriod;
	unsigned __int64 m_dLastUpdatedStamp;
	unsigned __int64 m_dBlockAlertUntilStamp;
	__int64 m_dObjectsMoving;
	bool m_bThreadRunning; // to be able to kill it externally
	typedef struct MovingObjectData
	{
		MovingObjectData() { x = y = px = py = 0.0f; }
		float x, y;
		float px, py;
		bool alertIsOn;
	}MovingObjectData;
	std::vector<MovingObjectData> m_Objects;
	WatchdogThreadData *m_pWorkerThread;
};