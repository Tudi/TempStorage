#pragma once

#include "Util/ObjDescriptor.h"

#define SPLASH_RECENTS_LOCATIONS 4
class DashboardLocationsWindow : public GenericWindow
{
public:
	REFLECT_TYPE(DashboardLocationsWindow);
	DashboardLocationsWindow();
	int DrawWindow();
	void ResetState();
	void DestructorCheckMemLeaks();
private:
	static void OnWindowButtonClick(GenericButton* pBtn, void* pParent);
	void DrawSingleLocationCard(ImVec2& imagePosition, int index);
	static void AsyncTask_Init(void* params);

	HoverPopupButton m_LocationsInfoButton;
	MultiStateButton m_ViewMoreButton;
};