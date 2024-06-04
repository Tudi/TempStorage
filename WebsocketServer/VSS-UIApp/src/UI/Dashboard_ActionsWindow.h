#pragma once

#include "Util/ObjDescriptor.h"

class DashboardActionsWindow : public GenericWindow
{
public:
	REFLECT_TYPE(DashboardActionsWindow);
	DashboardActionsWindow();
	int DrawWindow();
	void ResetState();
	void DestructorCheckMemLeaks();
private:
	static void OnWindowButtonClick(GenericButton* pBtn, void* pParent);
	static void AsyncTask_Init(void* params);

	HoverOnlyButton m_ActionsButton;
	HoverOnlyButton m_LocationsButton;
	HoverOnlyButton m_ModulesButton;
};