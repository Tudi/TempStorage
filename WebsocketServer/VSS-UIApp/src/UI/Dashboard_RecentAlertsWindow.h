#pragma once

#include "Util/ObjDescriptor.h"

#define MAX_LOGIN_STRING_LENGTH	255
#define CACHE_REFRESH_MS		100

struct AlertHistoryData;
class DashboardAlertsWindow : public GenericWindow
{
public:
	REFLECT_TYPE(DashboardAlertsWindow);
	DashboardAlertsWindow();
	int DrawWindow();
	void ResetState();
	void DestructorCheckMemLeaks();
private:
	void DrawSingleAlertCard(ImVec2& imagePosition, AlertHistoryData *alert);
	static void AsyncTask_Init(void* params);
	static void OnWindowButtonClick(GenericButton* btn, void* pParent);
	unsigned __int64 m_dNextCachedStamp;

	HoverPopupButton m_AlertsInfoButton;
	MultiStateButton m_ViewMoreButton;
};