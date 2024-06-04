#pragma once

enum WindowManagerErrorCodes {
	WM_NO_ERROR = 0,
	WM_FAILED_TO_CREATE_MAIN_WINDOW,
};

class WindowManager
{
public:
	inline static WindowManager& getInstance() {
		// This is a thread-safe way to create the instance
#ifdef _DEBUG
		static WindowManager *instance = new WindowManager;
		return *instance;
#else
		static WindowManager instance;
		return instance;
#endif
	}
	void DestructorCheckMemLeaks();
	WindowManagerErrorCodes DrawWindows();
	void SetLoginWindowVisible(bool newState);
	void SetDashboardWindowVisible(bool newState);
	void SetRstPasswWindowVisible(bool newState);
	void SetUserInfoWindowVisible(bool newState);
	void SetLocationsWindowVisible(bool newState);
	void SetLocationEditWindowVisible(bool newState, int id);
	void SetLocationViewWindowVisible(bool newState, int id);
	void SetAlertsWindowVisible(bool newState);
	void SetActivityLogWindowVisible(bool newState);
	void SetModulesWindowVisible(bool newState);
	void SetModulesBuyWindowVisible(bool newState);
	void SetSettingsWindowVisible(bool newState);
	/// <summary>
	/// Reset state of all windows
	/// </summary>
	void ResetState(); // example if you logout/relog
	/// <summary>
	/// When we open Window X, we might want to close all other windows
	/// </summary>
	void OnWindowOpenButtonPushed();
	/// <summary>
	/// When user logs in, prefetch window data so we can render them fast
	/// </summary>
	void OnUserLoggedIn();
	/// <summary>
	/// When user logs out, close windows, stop trackers...
	/// </summary>
	void OnUserLoggeOut();
	/// <summary>
	/// Used by locations window to set the dat to be shown
	/// </summary>
	LocationViewWindow* GetLocationViewWindow() { return &m_LocationViewWindow; }
	/// <summary>
	/// Used by locations view window to set the dat to be shown to hide network latency
	/// </summary>
	LocationEditWindow* GetLocationEditWindow() { return &m_LocationEditWindow; }
	/// <summary>
	/// When admin buys a module, the window data should be refreshed
	/// </summary>
	ModulesBuyWindow* GetModuleBuyWindow() { return &m_ModulesBuyWindow; }
	/// <summary>
	/// We draw some extra buttons on the menu bar if we are viewwing locations
	/// </summary>
	inline bool isDashboardWindowVisible() const { return m_bDrawDashboardWindow; }
	inline bool isFileWindowVisible() const { return m_bDrawFiledWindow; }
	inline bool isLocationsVisible() const { return m_bDrawLocationsScreen || m_bDrawLocationEditScreen || m_bDrawLocationViewScreen; }
	inline bool isLocationViewWindowVisible() const { return m_bDrawLocationViewScreen; }
	inline bool isAlertsWindowVisible() const { return m_bDrawAlertsWindowScreen; }
	inline bool isSettingsVisible() const { return m_bDrawSettingsWindow || m_bDrawActivityLogWindowScreen || m_bDrawModulesWindow || m_bDrawModulesBuyWindow; }
private:
	WindowManager()
	{
		ResetState();
	}
	WindowManager(const WindowManager&) = delete;
	WindowManager& operator=(const WindowManager&) = delete;

	MainWindow m_mainWindow;
	LoginWindow m_loginWindow;
	ResetPasswWindow m_rstPasswWindow;
	UserInfoWindow m_userInfoWindow;
	DashboardWelcomeWindow m_DashboardWelcomeWindow;
	LocationsWindow m_LocationsWindow;
	LocationEditWindow m_LocationEditWindow;
	LocationViewWindow m_LocationViewWindow;
	AlertsWindow m_AlertsWindow;
	ActivityLogWindow m_ActivityLogWindow;
	ModulesWindow m_ModulesWindow;
	ModulesBuyWindow m_ModulesBuyWindow;
	SettingsWindow m_SettingsWindow;
	bool m_bDrawLoginScreen;
	bool m_bDrawRstPasswScreen;
	bool m_bDrawUserInfoScreen;
	bool m_bDrawLocationsScreen;
	bool m_bDrawLocationEditScreen;
	bool m_bDrawLocationViewScreen;
	bool m_bDrawAlertsWindowScreen;
	bool m_bDrawActivityLogWindowScreen;
	bool m_bDrawModulesWindow;
	bool m_bDrawModulesBuyWindow;
	bool m_bDrawDashboardWindow;
	bool m_bDrawFiledWindow;
	bool m_bDrawSettingsWindow;
};

#define sWindowManager WindowManager::getInstance()