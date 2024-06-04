#pragma once

#include "Util/ObjDescriptor.h"

class SettingsWindow : public GenericWindow
{
public:
	REFLECT_TYPE(SettingsWindow);
	SettingsWindow();
	int DrawWindow();
	void ResetState();
	void DestructorCheckMemLeaks();
private:
	static void OnWindowButtonClick(GenericButton* pBtn, void* pParent);
	static void AsyncTask_Init(void* params);
	MultiStateButton m_bViewActivity;
	MultiStateButton m_bViewMyAccount;
	MultiStateButton m_bViewModules;
};