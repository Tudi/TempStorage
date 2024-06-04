#include "StdAfx.h"

void WindowManager::DestructorCheckMemLeaks()
{
    m_mainWindow.DestructorCheckMemLeaks();
    m_loginWindow.DestructorCheckMemLeaks();
    m_rstPasswWindow.DestructorCheckMemLeaks();
    m_userInfoWindow.DestructorCheckMemLeaks();
    m_DashboardWelcomeWindow.DestructorCheckMemLeaks();
    m_LocationsWindow.DestructorCheckMemLeaks();
    m_LocationEditWindow.DestructorCheckMemLeaks();
    m_LocationViewWindow.DestructorCheckMemLeaks();
    m_AlertsWindow.DestructorCheckMemLeaks();
    m_ActivityLogWindow.DestructorCheckMemLeaks();
    m_ModulesWindow.DestructorCheckMemLeaks();
    m_ModulesBuyWindow.DestructorCheckMemLeaks();
    m_SettingsWindow.DestructorCheckMemLeaks();
#ifdef _DEBUG
    delete & sWindowManager;
#endif
}

void WindowManager::ResetState()
{
    static bool AntiCircularCall = false;
    if (AntiCircularCall == true)
    {
        return;
    }
    AntiCircularCall = true;

    m_mainWindow.ResetState();
    SetLoginWindowVisible(sUserSession.GetUserId() == 0);
    SetRstPasswWindowVisible(false);
    SetUserInfoWindowVisible(false);
    SetLocationsWindowVisible(false);
    SetLocationEditWindowVisible(false, 0xFFFFFF);
    SetLocationViewWindowVisible(false, 0);
    SetAlertsWindowVisible(false);
    SetActivityLogWindowVisible(false);
    SetModulesWindowVisible(false);
    SetModulesBuyWindowVisible(false);
    SetDashboardWindowVisible(false);
    SetSettingsWindowVisible(false);
    m_bDrawFiledWindow = false; // no window for it yet

    AntiCircularCall = false;
}

// reset state on all windows as we consider them closed
// We will open a specific one later
void WindowManager::OnWindowOpenButtonPushed()
{
    ResetState();
}

void WindowManager::OnUserLoggedIn()
{
    // Load server defined string constatnts
    sLocalization.FetchServerStringDefines();
    sAlertsCache.OnUserLoggedIn();

    // load location with values that we will reload after window open
    m_LocationsWindow.OnUserLoggedIn();
    m_AlertsWindow.OnUserLoggedIn();
    m_ModulesWindow.OnUserLoggedIn();

    // Not the best place
    sDataSourceManager.OnUserLoggedIn();

    // load recents data
    sLocationRecentManager.OnUserLogin();
}

void WindowManager::OnUserLoggeOut()
{
    AddLogEntry(LogDestinationFlags::LDF_SERVER, LogSeverityValue::LogSeverityNormal,
        LogSourceGroups::LogSourceUserManagement, 0, 0, "Logged out");
    Sleep(20); // should invent something else here

    sAppSession.SessionIdInit(0);
    sUserSession.OnLoggedOut();
    sDataSourceManager.OnUserLogout();
    sAlertsCache.OnUserLoggedOut();
    sWindowManager.ResetState();
}

//#define DEBUG_SPLASH_WINDOW_LAYOUT
#if defined(_DEBUG) && defined(DEBUG_SPLASH_WINDOW_LAYOUT)
static void DrawSplashOverlayCheckUIMatch()
{
    static VSSImageStore* ExpectedLayout = NULL;
    if (ExpectedLayout == NULL)
    {
        ExpectedLayout = ImGui_LoadTextureFromFile("../ExpectedLayout_Splash.png");
    }
    if (ExpectedLayout != NULL)
    {
        // Calculate the position to draw the image in the title bar (adjust as needed)
        float titleBarHeight = ImGui::GetTextLineHeightWithSpacing(); // Get the title bar height
        int width = 1920, height = 1080;
        GetDrawAreaSize(width, height, true);
        ImVec2 imagePosition = ImVec2(0, 0 + titleBarHeight * 1);
        ImVec2 imageSize((float)width, (float)height - titleBarHeight);
        ImGui::GetForegroundDrawList()->AddImage((ImTextureID)ExpectedLayout->DS,
            imagePosition, ImVec2(imagePosition.x + imageSize.x, imagePosition.y + imageSize.y));
    }
}
#endif

WindowManagerErrorCodes WindowManager::DrawWindows()
{
    // Draw main menu and set generic style settings
    // The background for most windows
    m_mainWindow.DrawWindow();

    // Priority over the login windows. Exclusive to logon window
    if (m_bDrawRstPasswScreen)
    {
        m_rstPasswWindow.DrawWindow();
    }
    // Used to obtain a user session
    else if (m_bDrawLoginScreen)
    {
        m_loginWindow.DrawWindow();
    }
    else if (m_bDrawUserInfoScreen)
    {
        m_userInfoWindow.DrawWindow();
    }
    else if (m_bDrawLocationsScreen)
    {
        m_LocationsWindow.DrawWindow();
    }
    else if (m_bDrawLocationEditScreen)
    {
        m_LocationEditWindow.DrawWindow();
    }
    else if (m_bDrawLocationViewScreen)
    {
        m_LocationViewWindow.DrawWindow();
//        m_LocationRecentsWindow.DrawWindow();
    }
    else if (m_bDrawAlertsWindowScreen)
    {
        m_AlertsWindow.DrawWindow();
    }
    else if (m_bDrawActivityLogWindowScreen)
    {
        m_ActivityLogWindow.DrawWindow();
    }
    else if (m_bDrawModulesWindow)
    {
        m_ModulesWindow.DrawWindow();
    }
    else if (m_bDrawModulesBuyWindow)
    {
        m_ModulesBuyWindow.DrawWindow();
    }
    // if no other window is drawn and the user is logged in, draw the splash screen
    else if(m_bDrawDashboardWindow)
    {
        m_DashboardWelcomeWindow.DrawWindow();
    }
    else if (m_bDrawSettingsWindow)
    {
        m_SettingsWindow.DrawWindow();
    }

    // clean up styles
    m_mainWindow.FinishDraw();

#if defined(_DEBUG) && defined(DEBUG_SPLASH_WINDOW_LAYOUT)
    DrawSplashOverlayCheckUIMatch();
#endif

	return WindowManagerErrorCodes::WM_NO_ERROR;
}

static void GenericSetWindowVisble(WindowManager *mgr, bool newState, GenericWindow &wnd, bool &oldState, const char *ErrStr, bool bNeedsLoggedInUser = true)
{
    if (bNeedsLoggedInUser == true)
    {
        if (newState == true && sUserSession.GetUserId() == 0)
        {
            AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected,
                LogSourceGroups::LogSourceWindowManager, 0, 0, ErrStr);
            return;
        }
    }

    if (newState == true)
    {
        mgr->OnWindowOpenButtonPushed(); // close any other non important window
        wnd.ResetState();
    }
    else if (oldState != newState)
    {
        wnd.OnWindowClosed();
    }
    oldState = newState;
}

void WindowManager::SetRstPasswWindowVisible(bool newState)
{
    GenericSetWindowVisble(this, newState, m_rstPasswWindow, m_bDrawRstPasswScreen, "", false);
}

void WindowManager::SetLoginWindowVisible(bool newState)
{
    GenericSetWindowVisble(this, newState, m_loginWindow, m_bDrawLoginScreen, 
        "WindowManager:User is trying to log in, but old user has not logged out. ", false);
}

void WindowManager::SetUserInfoWindowVisible(bool newState)
{
    GenericSetWindowVisble(this, newState, m_userInfoWindow, m_bDrawUserInfoScreen, 
        "WindowManager:Trying to show user info window, but old user has not logged out. ");
}

void WindowManager::SetLocationsWindowVisible(bool newState)
{
    GenericSetWindowVisble(this, newState, m_LocationsWindow, m_bDrawLocationsScreen, 
        "WindowManager:Trying to show locations window, but old user has not logged out. ");
}

void WindowManager::SetLocationEditWindowVisible(bool newState, int id)
{
    GenericSetWindowVisble(this, newState, m_LocationEditWindow, m_bDrawLocationEditScreen, 
        "WindowManager:Trying to show locationEdit window, but old user has not logged out. ");
    if (newState == true)
    {
        m_LocationEditWindow.ResetState(id);
    }
}

void WindowManager::SetLocationViewWindowVisible(bool newState, int id)
{
    if (newState == true && sUserSession.GetUserId() == 0)
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceWindowManager, 0, 0,
            "WindowManager:Trying to show location view window, but there is no logged in user. ");
        return;
    }

    if (newState == true)
    {
        OnWindowOpenButtonPushed(); // close any other non important window
        m_LocationViewWindow.ResetState(id);
    }
    // closing it now, and it was not open. Close event might come even if it's already closed
    else if(m_bDrawLocationViewScreen != newState)
    {
        m_LocationViewWindow.OnWindowClosed();
    }
    m_bDrawLocationViewScreen = newState;
}

void WindowManager::SetAlertsWindowVisible(bool newState)
{
    GenericSetWindowVisble(this, newState, m_AlertsWindow, m_bDrawAlertsWindowScreen, 
        "WindowManager:Trying to show Alerts window, but old user has not logged out. ");
}

void WindowManager::SetActivityLogWindowVisible(bool newState)
{
    GenericSetWindowVisble(this, newState, m_ActivityLogWindow, m_bDrawActivityLogWindowScreen, 
        "WindowManager:Trying to show Activity window, but old user has not logged out. ");
}

void WindowManager::SetModulesWindowVisible(bool newState)
{
    GenericSetWindowVisble(this, newState, m_ModulesWindow, m_bDrawModulesWindow, 
        "WindowManager:Trying to show Modules window, but old user has not logged out. ");
}

void WindowManager::SetModulesBuyWindowVisible(bool newState)
{
    GenericSetWindowVisble(this, newState, m_ModulesBuyWindow, m_bDrawModulesBuyWindow, 
        "WindowManager:Trying to show ModulesBuy window, but old user has not logged out. ");
}

void WindowManager::SetDashboardWindowVisible(bool newState)
{
    GenericSetWindowVisble(this, newState, m_DashboardWelcomeWindow, m_bDrawDashboardWindow,
        "WindowManager:Trying to show Dashboard window, but user has not logged in. ");
}

void WindowManager::SetSettingsWindowVisible(bool newState)
{
    GenericSetWindowVisble(this, newState, m_SettingsWindow, m_bDrawSettingsWindow,
        "WindowManager:Trying to show Settings window, but user has not logged in. ");
}
