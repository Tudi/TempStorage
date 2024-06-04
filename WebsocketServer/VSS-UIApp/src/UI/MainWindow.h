#pragma once

#include "Util/ObjDescriptor.h"

class MainWindow : public GenericWindow
{
public:
	REFLECT_TYPE(MainWindow);
	MainWindow();
	int DrawWindow();
	void ResetState();
	void FinishDraw();
	void DestructorCheckMemLeaks();
private:
	int DrawMenu();
	int DrawAppIcon();
	static void OnWindowButtonClick(GenericButton* pBtn, void* pParent);
#if defined(VER1_RENDERING)
	FlatButton m_Menu_Locations_ViewAll;
	FlatButton m_Menu_Locations_Add;

	FlatButton m_Menu_Settings_MyActivityLog;
	FlatButton m_Menu_Settings_MyAccount;
	FlatButton m_Menu_Settings_Modules;
#endif
	static void AsyncTask_Init(void* userData); // because we need to wait for images to get loaded
	MultiStateButton m_Navigation_Dashboard;
	MultiStateButton m_Navigation_File;
	MultiStateButton m_Navigation_Locations;
	MultiStateButton m_Navigation_Alerts;
	MultiStateButton m_Navigation_Settings;
	MultiStateButton m_Navigation_Logout;
	MultiStateButton m_Navigation_Logout2;

	bool m_bIsOpen;
};

// the draw area where we will draw our tables and forms
constexpr float GetMainWindowDrawWidthWithBorder() { return 1729; }
constexpr float GetMainWindowDrawHeightWithBorder() { return 977; }
constexpr float GetMainWindowDrawWidth() { return GetMainWindowDrawWidthWithBorder() - 3 - 3; }
constexpr float GetMainWindowDrawHeight() { return GetMainWindowDrawHeightWithBorder() - 3 - 3; }
