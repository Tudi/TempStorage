#pragma once

#ifdef _DEBUG
	#define Module_FEED_UPDATE_PERIOD 1000
#else
	#define Module_FEED_UPDATE_PERIOD 1000
#endif

//#define DRAW_DEMO_ALERT_BUTTON
//#define USE_POLLING_DEMO_DATA

#define PERSON_SIZE_PIXEL_HALF	14

class DoplerModulePointInfo
{
public:
	void UpdateIconDrawPosition();
	__int64 id;
	float x, y;
	float xp, yp; // previous values if available
	std::string Tags;
	bool isClicked;
	// calculate these when the new values arive. Used to render the image at the location
	double angle;
	double xi[4], yi[4];
	unsigned __int64 m_lldPrevModuleUpdateStamp;
	unsigned __int64 m_lldPrevModuleUpdatePrevStamp;
	unsigned __int64 m_lldLastUpdated;
};

class DoplerModuleView : public GenericWindow
{
public:
	DoplerModuleView();
	int DrawWindow();
	void ResetState();
	void DestructorCheckMemLeaks();
	std::vector<DoplerModulePointInfo>& GetPOI() { return m_ModulePOI; }
	void SetModuleId(int newId) { m_dModuleId = newId; }
	void OnModulePositionDataArrived(__int64 ModuleId, unsigned __int64 Timestamp, __int64 ObjectId, float x, float y);
private:
	static void OnButtonClick(GenericButton* pBtn, void* pParent);
#ifdef DRAW_DEMO_ALERT_BUTTON
	FlatButton m_ButtonTriggerAlert;
#endif
#ifdef USE_POLLING_DEMO_DATA
	unsigned __int64 m_lldPrevModuleUpdateTriggerStamp;
	static void CB_AsyncDataArived_DemoData(int CurlErr, char* response, void* userData);
#endif
	std::mutex m_POIListMutex;
	std::vector<DoplerModulePointInfo> m_ModulePOI;
	int m_dModuleId;
};