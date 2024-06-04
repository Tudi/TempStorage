#include "StdAfx.h"
#include "ResourceManager/AsyncTaskManager.h"

MainWindow::MainWindow()
#if defined(VER1_RENDERING)
    :
    m_Menu_Locations_ViewAll(this, OnWindowButtonClick, ButtonIds::BI_MENU_LOCATION_VIEW_ALL, LocalizationRssIds::Menu_Locations_ViewAll_Text),
    m_Menu_Locations_Add(this, OnWindowButtonClick, ButtonIds::BI_MENU_LOCATION_ADD, LocalizationRssIds::Menu_Locations_Add_Text),
    m_Menu_Settings_MyActivityLog(this, OnWindowButtonClick, ButtonIds::BI_MENU_SETTINGS_MYACTIVITYLOG, LocalizationRssIds::Menu_Settings_MyActivityLog_Text),
    m_Menu_Settings_MyAccount(this, OnWindowButtonClick, ButtonIds::BI_MENU_SETTINGS_MYACCOUNT, LocalizationRssIds::Menu_Settings_MyAccount_Text),
    m_Menu_Settings_Modules(this, OnWindowButtonClick, ButtonIds::BI_MENU_SETTINGS_MODULES, LocalizationRssIds::Menu_Settings_Modules_Text)
#endif
{
    InitTypeInfo();
#if defined(VER1_RENDERING)
    m_Menu_Locations_ViewAll.SetColor(GenericButtonColors::GBCT_TextHover, sStyles.GetUIntValue(StyleIds::Title_Location_Text_Color));
    m_Menu_Locations_ViewAll.SetColor(GenericButtonColors::GBCT_UnderlineHover, sStyles.GetUIntValue(StyleIds::Title_Location_Text_Color));
    m_Menu_Locations_Add.SetColor(GenericButtonColors::GBCT_TextHover, sStyles.GetUIntValue(StyleIds::Title_Location_Text_Color));
    m_Menu_Locations_Add.SetColor(GenericButtonColors::GBCT_UnderlineHover, sStyles.GetUIntValue(StyleIds::Title_Location_Text_Color));

    m_Menu_Settings_MyActivityLog.SetColor(GenericButtonColors::GBCT_TextHover, sStyles.GetUIntValue(StyleIds::Title_Settings_Text_Color));
    m_Menu_Settings_MyActivityLog.SetColor(GenericButtonColors::GBCT_UnderlineHover, sStyles.GetUIntValue(StyleIds::Title_Settings_Text_Color));
    m_Menu_Settings_MyAccount.SetColor(GenericButtonColors::GBCT_TextHover, sStyles.GetUIntValue(StyleIds::Title_Settings_Text_Color));
    m_Menu_Settings_MyAccount.SetColor(GenericButtonColors::GBCT_UnderlineHover, sStyles.GetUIntValue(StyleIds::Title_Settings_Text_Color));
    m_Menu_Settings_Modules.SetColor(GenericButtonColors::GBCT_TextHover, sStyles.GetUIntValue(StyleIds::Title_Settings_Text_Color));
    m_Menu_Settings_Modules.SetColor(GenericButtonColors::GBCT_UnderlineHover, sStyles.GetUIntValue(StyleIds::Title_Settings_Text_Color));
#endif
    AddAsyncTask(AsyncTask_Init, this);
}

void MainWindow::AsyncTask_Init(void* params)
{
    MainWindow* self = typecheck_castL(MainWindow, params);

    // need to have images loaded
    while (sImageManager.FinishedLoadingImages() == false)
    {
        Sleep(1);
    }

    self->ResetState();
    self->m_Navigation_Dashboard.SetMinSize(73, 49);
    self->m_Navigation_Dashboard.SetCallback(self, OnWindowButtonClick);
    self->m_Navigation_Dashboard.SetId(ButtonIds::BI_MENU_DASHBOARD);
    self->m_Navigation_Dashboard.SetHoverImageId(sImageManager.GetImage(II_NavigationDashboardButtonHover));
    self->m_Navigation_Dashboard.SetActiveImageId(sImageManager.GetImage(II_NavigationDashboardButtonActive));

    self->m_Navigation_File.SetMinSize(73, 49);
    self->m_Navigation_File.SetCallback(self, OnWindowButtonClick);
    self->m_Navigation_File.SetId(ButtonIds::BI_MENU_FILE);
//    self->m_Navigation_File.SetHoverImageId(sImageManager.GetImage(II_NavigationFileButtonHover));
    self->m_Navigation_File.SetActiveImageId(sImageManager.GetImage(II_NavigationFileButtonActive));

    self->m_Navigation_Locations.SetMinSize(73, 49);
    self->m_Navigation_Locations.SetCallback(self, OnWindowButtonClick);
    self->m_Navigation_Locations.SetId(ButtonIds::BI_MENU_LOCATION_VIEW_ALL);
//    self->m_Navigation_Locations.SetHoverImageId(sImageManager.GetImage(II_NavigationLocationButtonHover));
    self->m_Navigation_Locations.SetActiveImageId(sImageManager.GetImage(II_NavigationLocationButtonActive));

    self->m_Navigation_Alerts.SetMinSize(73, 49);
    self->m_Navigation_Alerts.SetCallback(self, OnWindowButtonClick);
    self->m_Navigation_Alerts.SetId(ButtonIds::BI_MENU_ALERTS_VIEW_ALL);
//    self->m_Navigation_Alerts.SetHoverImageId(sImageManager.GetImage(II_NavigationAlertsButtonHover));
    self->m_Navigation_Alerts.SetActiveImageId(sImageManager.GetImage(II_NavigationAlertsButtonActive));

    self->m_Navigation_Settings.SetMinSize(73, 49);
    self->m_Navigation_Settings.SetCallback(self, OnWindowButtonClick);
    self->m_Navigation_Settings.SetId(ButtonIds::BI_MENU_SETTINGS_VIEW_ALL);
//    self->m_Navigation_Settings.SetHoverImageId(sImageManager.GetImage(II_NavigationSettingsButonHover));
    self->m_Navigation_Settings.SetActiveImageId(sImageManager.GetImage(II_NavigationSettingsButonActive));

    self->m_Navigation_Logout.SetMinSize(201, 42);
    self->m_Navigation_Logout.SetCallback(self, OnWindowButtonClick);
    self->m_Navigation_Logout.SetId(ButtonIds::BI_MENU_LOGOUT);
    self->m_Navigation_Logout.LoadHitMap("./Assets/Images/MainWindow_LogoutButtonHover.png");
    self->m_Navigation_Logout.SetHoverImageId(sImageManager.GetImage(II_MainWindowLogoutButtonHover));

    self->m_Navigation_Logout2.SetMinSize(20, 20);
    self->m_Navigation_Logout2.SetCallback(self, OnWindowButtonClick);
    self->m_Navigation_Logout2.SetId(ButtonIds::BI_MENU_LOGOUT);
}

void MainWindow::ResetState()
{
    m_bIsOpen = true;
}

void MainWindow::DestructorCheckMemLeaks()
{
#if defined(VER1_RENDERING)
    m_Menu_Locations_ViewAll.DestructorCheckMemLeaks();
    m_Menu_Locations_Add.DestructorCheckMemLeaks();
    m_Menu_Settings_MyActivityLog.DestructorCheckMemLeaks();
    m_Menu_Settings_MyAccount.DestructorCheckMemLeaks();
    m_Menu_Settings_Modules.DestructorCheckMemLeaks();
#endif
}

int MainWindow::DrawMenu()
{
#if defined(VER1_RENDERING)
    ImFont* defaultFont = sFontManager.GetFont(FontIds::FI_Menu);
    ImGui::PushFont(defaultFont);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(30, 0));

    // Menu Bar
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            ImGui::EndMenu();
        }

        static bool bIsLocationsMenuOpen = false;
        if (bIsLocationsMenuOpen == true)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, sStyles.GetUIntValue(StyleIds::Title_Location_Text_Color));
        }
        if (ImGui::BeginMenu("Location"))
        {
            if (bIsLocationsMenuOpen == true)
            {
                ImGui::PopStyleColor(1);
            }
            bIsLocationsMenuOpen = true;

            m_Menu_Locations_ViewAll.DrawButton();
            ImGui::SameLine();
            m_Menu_Locations_Add.DrawButton();
            ImGui::EndMenu();
        }
        else
        {
            if (bIsLocationsMenuOpen == true)
            {
                ImGui::PopStyleColor(1);
            }
            bIsLocationsMenuOpen = false;
        }

        static bool bIsAlertsMenuOpen = false;
        if (bIsAlertsMenuOpen == true)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, sStyles.GetUIntValue(StyleIds::Title_Alert_Text_Color));
        }
        if (ImGui::BeginMenu("Alerts"))
        {
            if (bIsAlertsMenuOpen == true)
            {
                ImGui::PopStyleColor(1);
            }
            bIsAlertsMenuOpen = true;

            // this means the items was open and we clicked on it
            if (ImGui::IsItemClicked(0) && ImGui::IsItemHovered())
            {
                ImGui::CloseCurrentPopup();
                sWindowManager.SetAlertsWindowVisible(true);
            }

            ImGui::EndMenu();
        }
        else
        {
            // funcky. This means the menu is closed but we still clicked on it
            if (ImGui::IsItemClicked(0) && ImGui::IsItemHovered())
            {
                ImGui::CloseCurrentPopup();
                sWindowManager.SetAlertsWindowVisible(true);
            }

            if (bIsAlertsMenuOpen == true)
            {
                ImGui::PopStyleColor(1);
            }
            bIsAlertsMenuOpen = false;
        }

        static bool bIsSettingsMenuOpen = false;
        if (bIsSettingsMenuOpen == true)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, sStyles.GetUIntValue(StyleIds::Title_Settings_Text_Color));
        }
        if (ImGui::BeginMenu("Settings"))
        {
            if (bIsSettingsMenuOpen == true)
            {
                ImGui::PopStyleColor(1);
            }
            bIsSettingsMenuOpen = true;

            m_Menu_Settings_MyActivityLog.DrawButton();
            ImGui::SameLine();
            m_Menu_Settings_MyAccount.DrawButton();
            ImGui::SameLine();
            m_Menu_Settings_Modules.DrawButton();
            ImGui::EndMenu();
        }
        else
        {
            if (bIsSettingsMenuOpen == true)
            {
                ImGui::PopStyleColor(1);
            }
            bIsSettingsMenuOpen = false;
        }

        // draw recent 3 windows on the menubar. Custom ...
        if (sWindowManager.isLocationViewWindowVisible())
        {
            sWindowManager.GetLocationRecentsWindow()->DrawWindow();
        }

        // Logout is only visible if user is logged in
        if (sAppSession.IsUserLoggedIn())
        {
            ImFont* terminateSessionFont = sFontManager.GetFont(FontIds::FI_Normal);
            ImGui::PushFont(terminateSessionFont);

            float redLineWidth = 4.0f;
            // Calculate the width of the "Logout" button
            float buttonWidth = ImGui::CalcTextSize("TERMINATE SESSION").x + ImGui::GetStyle().FramePadding.x * 2.0f + redLineWidth;

            // Calculate the available width in the menu bar
            float menuBarWidth = ImGui::GetWindowWidth();
            float availableWidth = menuBarWidth - ImGui::GetStyle().ItemSpacing.x;

            // Calculate the position to align the "Logout" button to the right
            float posX = availableWidth - buttonWidth;

            // Set the draw position for the button
            ImGui::SetCursorPosX(posX);
 
            // draw the red line before the button
            {
                // Calculate the width and height of the red line
                float lineHeight = ImGui::GetTextLineHeightWithSpacing();
                // Calculate the position for the red line
                ImVec2 lineStartPos;
                lineStartPos.x = posX - 25;
                lineStartPos.y = lineHeight + ImGui::GetStyle().FramePadding.y;
                ImVec2 lineEndPos(lineStartPos.x, lineStartPos.y + lineHeight + 2 * ImGui::GetStyle().FramePadding.y);
                // Draw the red line
                ImGui::GetWindowDrawList()->AddLine(lineStartPos, lineEndPos, ImColor(255, 0, 0, 255), redLineWidth);
            }

            // Add the "Logout" button
            ImGui::PushStyleColor(ImGuiCol_Button, sStyles.GetUIntValue(StyleIds::Btn_BG_Color));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, sStyles.GetUIntValue(StyleIds::Btn_Hover_Color));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, sStyles.GetUIntValue(StyleIds::Btn_Active_Color));
            if (ImGui::Button("TERMINATE SESSION"))
            {
                sWindowManager.OnUserLoggeOut();
            }
            ImGui::PopStyleColor(3);
            ImGui::PopFont();
        }

        ImGui::EndMenuBar();
    }

    // pop menu font
    ImGui::PopFont();
    ImGui::PopStyleVar(1);
#endif
    return WindowManagerErrorCodes::WM_NO_ERROR;
}

int MainWindow::DrawAppIcon()
{
#if defined(VER1_RENDERING)
    ImVec2 imagePosition = ImVec2(15, 10);
    ImVec2 imageSize(32, 11);
    ImGui::GetForegroundDrawList()->AddImage(sImageManager.GetImage(ImageIds::II_AppIcon),
        imagePosition, ImVec2(imagePosition.x + imageSize.x, imagePosition.y + imageSize.y));
#endif
    return WindowManagerErrorCodes::WM_NO_ERROR;
}

#if defined(VER1_RENDERING)
int MainWindow::DrawWindow()
{
    // set default font for all windows
    ImFont* defaultFont = sFontManager.GetFont(FontIds::FI_Normal);
    ImGui::PushFont(defaultFont);

    int display_w, display_h;
    GetDrawAreaSize(display_w, display_h, true);

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2((float)display_w, (float)display_h), ImGuiCond_Always);

    if (!ImGui::Begin("##VSS-Dashboard", &m_bIsOpen, ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings))
    {
        return WindowManagerErrorCodes::WM_FAILED_TO_CREATE_MAIN_WINDOW;
    }
    ImGui::PushStyleColor(ImGuiCol_Button, sStyles.GetUIntValue(StyleIds::Btn_BG_Color));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, sStyles.GetUIntValue(StyleIds::Btn_Hover_Color));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, sStyles.GetUIntValue(StyleIds::Btn_Active_Color));

    if (sUserSession.GetUserId() != 0)
    {
        DrawMenu();
    }

    DrawAppIcon();

    // if we close the main window, start shutting down the main application
    if (m_bIsOpen == false)
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityNormal, LogSourceGroups::LogSourceMainWindow, 0, 0,
            "Close button pressed. Starting shutdown");

        sAppSession.SetApplicationRunning(false);
    }

    return WindowManagerErrorCodes::WM_NO_ERROR;
}
#else
int MainWindow::DrawWindow()
{
    int display_w, display_h;
    GetDrawAreaSize(display_w, display_h, true);

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2((float)display_w, (float)display_h), ImGuiCond_Always);
    if (!ImGui::Begin("##VSS-Dashboard", &m_bIsOpen, ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNavFocus))
    {
        return WindowManagerErrorCodes::WM_FAILED_TO_CREATE_MAIN_WINDOW;
    }

    if (sUserSession.GetUserId() != 0)
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        // navigation pane
        const ImVec2 imageSizeNavigation = { 113, 976 };
        // top left corner position
//        ImVec2 imagePositionNavigation = ImVec2(0, 32); // in case flag ImGuiWindowFlags_NoTitleBar is not used
        const ImVec2 imagePositionNavigation = { 2, 4 };
        {
            drawList->AddImage(sImageManager.GetImage(ImageIds::II_NavigationBackground),
                imagePositionNavigation, ImVec2(imagePositionNavigation.x + imageSizeNavigation.x, imagePositionNavigation.y + imageSizeNavigation.y));

            ImGui::SetCursorPos(ImVec2(imagePositionNavigation.x + 15, imagePositionNavigation.y + 163));
            m_Navigation_Dashboard.DrawButton(sWindowManager.isDashboardWindowVisible());

            ImGui::SetCursorPos(ImVec2(imagePositionNavigation.x + 15, imagePositionNavigation.y + 163 + 73*1));
            m_Navigation_File.DrawButton(sWindowManager.isFileWindowVisible());

            ImGui::SetCursorPos(ImVec2(imagePositionNavigation.x + 15, imagePositionNavigation.y + 163 + 73*2));
            m_Navigation_Locations.DrawButton(sWindowManager.isLocationsVisible());

            ImGui::SetCursorPos(ImVec2(imagePositionNavigation.x + 15, imagePositionNavigation.y + 163 + 73*3));
            m_Navigation_Alerts.DrawButton(sWindowManager.isAlertsWindowVisible());

            ImGui::SetCursorPos(ImVec2(imagePositionNavigation.x + 15, imagePositionNavigation.y + 163 + 73*4));
            m_Navigation_Settings.DrawButton(sWindowManager.isSettingsVisible());

            // user icon
            {
                const ImVec2 imageSizeUserIcon((float)sImageManager.GetImageWidth(ImageIds::II_DemoUserIcon),
                    (float)sImageManager.GetImageHeight(ImageIds::II_DemoUserIcon));

                // top left corner position
                const ImVec2 imagePosition2 = ImVec2(imagePositionNavigation.x + 9 + 4, imagePositionNavigation.y + 911 + 4);

                drawList->AddImage(sImageManager.GetImage(ImageIds::II_DemoUserIcon),
                    imagePosition2, ImVec2(imagePosition2.x + imageSizeUserIcon.x, imagePosition2.y + imageSizeUserIcon.y));

                ImGui::SetCursorPos(ImVec2(imagePosition2.x + -4 + 60, imagePosition2.y -4 + 18));
                m_Navigation_Logout2.DrawButton();
            } 
        }

        // background image
        {
            const ImVec2 imageSizeBackground = { GetMainWindowDrawWidthWithBorder(),GetMainWindowDrawHeightWithBorder() };

            // top left corner position
            const ImVec2 imagePosition = ImVec2(imagePositionNavigation.x + imageSizeNavigation.x + 6, 
                                            imagePositionNavigation.y);

            drawList->AddImage(sImageManager.GetImage(ImageIds::II_MainWindowBackground),
                imagePosition, ImVec2(imagePosition.x + imageSizeBackground.x, imagePosition.y + imageSizeBackground.y));

            ImGui::SetCursorPos(ImVec2(imagePosition.x + 1528, imagePosition.y));
            m_Navigation_Logout.DrawButton();

            // user icon
            {
                ImVec2 imageSizeUserIcon((float)sImageManager.GetImageWidth(ImageIds::II_DemoUserIcon),
                    (float)sImageManager.GetImageHeight(ImageIds::II_DemoUserIcon));

                // top left corner position
                ImVec2 imagePosition2 = ImVec2(imagePosition.x + 24 + 4, imagePosition.y + 24 + 6);

                drawList->AddImage(sImageManager.GetImage(ImageIds::II_DemoUserIcon),
                    imagePosition2, ImVec2(imagePosition2.x + imageSizeUserIcon.x, imagePosition2.y + imageSizeUserIcon.y));
            }

            // user name
            ImGui::PushFont(sFontManager.GetFont(FontIds::FI_Large));
            ImGui::SetCursorPos(ImVec2(imagePosition.x + 24 + 72, imagePosition.y + 24 + 23));
            ImGui::Text(sUserSession.GetFirstName());
            ImGui::PopFont();
        } 
    }

    // if we close the main window, start shutting down the main application
    if (m_bIsOpen == false)
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityNormal, LogSourceGroups::LogSourceMainWindow, 0, 0,
            "Close button pressed. Starting shutdown");

        sAppSession.SetApplicationRunning(false);
    }

    ImGui::PushStyleColor(ImGuiCol_Text, sStyles.GetUIntValue(StyleIds::Txt_DashboardNormal_Color));

    return WindowManagerErrorCodes::WM_NO_ERROR;
}
#endif

#if defined(VER1_RENDERING)
void MainWindow::FinishDraw()
{
    // pop default font(normal)
    ImGui::PopFont();
    ImGui::PopStyleColor(3);

    ImGui::End(); // main window end. All other windows should be child
}
#else
void MainWindow::FinishDraw()
{
    ImGui::PopStyleColor(1);
    ImGui::End(); // main window end. All other windows should be child
}
#endif

void MainWindow::OnWindowButtonClick(GenericButton* btn, void* pParent)
{
    pParent;
    if (btn == NULL)
    {
        return;
    }

    if (btn->GetId() == ButtonIds::BI_MENU_LOCATION_VIEW_ALL)
    {
        ImGui::CloseCurrentPopup();
        sWindowManager.SetLocationsWindowVisible(true);
    }
    else if (btn->GetId() == ButtonIds::BI_MENU_DASHBOARD)
    {
        ImGui::CloseCurrentPopup();
        sWindowManager.SetDashboardWindowVisible(true);
    }
    else if (btn->GetId() == ButtonIds::BI_MENU_LOCATION_ADD)
    {
        ImGui::CloseCurrentPopup();
        sWindowManager.SetLocationEditWindowVisible(true, 0);
    }
    else if (btn->GetId() == ButtonIds::BI_MENU_SETTINGS_MYACTIVITYLOG)
    {
        ImGui::CloseCurrentPopup();
        sWindowManager.SetActivityLogWindowVisible(true);
    }
    else if (btn->GetId() == ButtonIds::BI_MENU_SETTINGS_MYACCOUNT)
    {
        ImGui::CloseCurrentPopup();
        sWindowManager.SetUserInfoWindowVisible(true);
    }
    else if (btn->GetId() == ButtonIds::BI_MENU_SETTINGS_MODULES)
    {
        ImGui::CloseCurrentPopup();
        sWindowManager.SetModulesWindowVisible(true);
    }
    else if (btn->GetId() == ButtonIds::BI_MENU_ALERTS_VIEW_ALL)
    {
        ImGui::CloseCurrentPopup();
        sWindowManager.SetAlertsWindowVisible(true);
    }
    else if (btn->GetId() == ButtonIds::BI_MENU_SETTINGS_VIEW_ALL)
    {
        ImGui::CloseCurrentPopup();
        sWindowManager.SetSettingsWindowVisible(true);
    }
    else if (btn->GetId() == ButtonIds::BI_MENU_LOGOUT)
    {
        ImGui::CloseCurrentPopup();
        sWindowManager.OnUserLoggeOut();
    }
}

