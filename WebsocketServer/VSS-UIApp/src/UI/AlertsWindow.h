#pragma once

#define ALERT_ROW_PER_PAGE					9
#define MAX_ALERTS_SHOWN					1000 // can't be larger than number of cached alerts
#define PAGE_CONTENT_REFRESH_CHECK_INTERVAL 200

#include "Util/ObjDescriptor.h"

class AlertsGrid : public GenericDataGrid
{
public:
	REFLECT_TYPE(AlertsGrid);
	AlertsGrid();
	void OnCellRender(uint32_t page, uint32_t col, uint32_t visualRow, uint32_t dataRow, const char* data);
	void DestructorCheckMemLeaks();
	void RefreshData();
private:
	static void OnGridButtonClick(GenericButton* pBtn, void* pParent);
	static unsigned int GetRowIndexForButton(void* pBtn, AlertsGrid* grid);
	static void AsyncTask_Init(void* params);

	MultiStateButton m_ButtonViewLocation[ALERT_ROW_PER_PAGE]; // could / should have made it dynamic ?
	uint64_t m_PrevValuesCRC;
};

class AlertsWindow : public GenericWindow
{
public:
	AlertsWindow();
	int DrawWindow();
	void ResetState();
	void DestructorCheckMemLeaks();
	void OnUserLoggedIn();
private:

	InputTextData m_SearchFilter;
	AlertsGrid m_GridAlerts;
	unsigned __int64 m_dNextGridContentRefreshStamp;
};