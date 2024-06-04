#pragma once

#define MAX_LOGIN_STRING_LENGTH	255

// forward declarations
class SplashRecentWindow;
class DashboardAlertsWindow;
class SplashSessionWindow;
class SplashModulesWindow;

class DashboardWelcomeWindow : public GenericWindow
{
public:
	DashboardWelcomeWindow() {};
	int DrawWindow();
	void ResetState();
	void DestructorCheckMemLeaks();
private:
	DashboardLocationsWindow m_SplashLocationsWindow;
	DashboardAlertsWindow m_SplashAlertWindow;
	DashboardActionsWindow m_SplashActionsWindow;
};