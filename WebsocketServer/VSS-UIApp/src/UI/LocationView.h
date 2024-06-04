#pragma once

#define ALERTS_FOR_LOCATION 4
#ifdef _DEBUG
	#define LOCATION_ALERT_UPDATE_INTERVAL 200 // given in milliseconds
#else
	#define LOCATION_ALERT_UPDATE_INTERVAL 200 // given in milliseconds
#endif
#define REPORT_LOCATIONVIEW_ACTIVITY_IF_LONGER 60000 // number of milliseconds the user viewed a locatio

#include "Util/ObjDescriptor.h"
class ExtractDBColumnToBinary;

class AlertsLocationGrid : public GenericDataGrid
{
public:
	AlertsLocationGrid();
	void OnCellRender(uint32_t page, uint32_t col, uint32_t visualRow, uint32_t dataRow, const char* data);
	void OnHeaderCellRender(uint32_t page, uint32_t col, const char* data);
	void SetLocationId(int newId);
	// fetch new alerts for this location
	void PeriodicCheckUpdate(bool bForced = false);
private:
	int m_dLocationId;
	uint64_t m_PrevValuesCRC;
	uint64_t m_PrevUpdateStamp;
};

class LocationViewWindow : public GenericWindow
{
public:
	REFLECT_TYPE(LocationViewWindow);
	LocationViewWindow();
	int DrawWindow();
	void ResetState() { ResetState(0); }
	void ResetState(int id);
	void OnWindowClosed();
	void DestructorCheckMemLeaks();
	// helper function. Can be used when called from locationView. Not a must to be used
	// helps to hide network latency
	void SetLocationData(const char* name, const char* desc, const char* addr1, const char* addr2,
		const char* city, const char* state, const char* countryRegion, const char* country);
	// Set the Module definition ID we intend to view
	void SetViewedModuleID(int newVal);
	// more then 1 module might be sending data. Relay it to the apropriate viewer
	void OnModulePositionDataArrived(__int64 ModuleId, unsigned __int64 Timestamp, __int64 ObjectId, float x, float y);
private:
	static void OnWindowButtonClick(class GenericButton* pBtn, void* pParent);
	// refresh data for a location from the DB
	static void CB_AsyncDataArived(int CurlErr, char* response, void* userData);
	static bool CB_OnDBRowExtractFinished_Modules(int rowIndex, ExtractDBColumnToBinary* rowColDataArr);
	static void CB_OnModulesDropdownChanged(class GenericDropdown* pBtn, void* pParent);
	void UpdateMultiRowDescription();
	FlatButton m_ButtonEdit;
	char m_sName[MAX_DB_STRING_LENGTH];
	char m_sDescription[MAX_DB_STRING_LENGTH];
	char m_sAddress1[MAX_DB_STRING_LENGTH];
	char m_sAddress2[MAX_DB_STRING_LENGTH];
	char m_sCity[MAX_DB_STRING_LENGTH];
	char m_sState[MAX_DB_STRING_LENGTH];
	char m_sCountryRegion[MAX_DB_STRING_LENGTH];
	char m_sCountry[MAX_DB_STRING_LENGTH];
	char m_sDescriptionMultiRow[2][MAX_DB_STRING_LENGTH];
	int m_dId;
	AlertsLocationGrid m_Alerts;
	uint64_t m_StampStartedView;
	GenericDropdown m_Modules;
	int m_nViewedModuleId;
	DoplerModuleView m_DoplerModule;
};