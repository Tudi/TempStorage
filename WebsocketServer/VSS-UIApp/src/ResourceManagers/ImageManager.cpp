#include "stdafx.h"

ImageManager::ImageManager()
{
    memset(m_Images, NULL, sizeof(m_Images));
    m_bFinisedLoadingImages = false;
}

void ImageManager::DestructorCheckMemLeaks()
{
    for (int i = 0; i < II_MAX_VALUE; i++)
    {
        ImGui_RemoveTexture(&m_Images[i]);
    }

#ifdef _DEBUG
    delete& sImageManager;
#endif
}

void ImageManager::InitImages()
{
    AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceImageManager, 0, 0,
        "ImageManager:Started loading %d images", ImageIds::II_MAX_VALUE);

//    m_Images[ImageIds::II_AppIcon] = ImGui_LoadTextureFromFile("./Assets/Images/Lamp.png");
    m_Images[ImageIds::II_LoginBackground] = ImGui_LoadTextureFromFile("./Assets/Images/Login_Background.png");
    m_Images[ImageIds::II_LoginButtonHover] = ImGui_LoadTextureFromFile("./Assets/Images/Login_LoginButtonHover.png");

    // load the rest in a background thread because it may take time
    std::thread ImgLoaderThread([this]() {
        unsigned __int64 startStamp = GetTickCount64();

        m_Images[ImageIds::II_LoginInputInvalidPassw] = ImGui_LoadTextureFromFile("./Assets/Images/Login_InputInvalidPassw.png");

        m_Images[ImageIds::II_ResetPasswBackground] = ImGui_LoadTextureFromFile("./Assets/Images/Login_RstPasswBg.png");
        m_Images[ImageIds::II_ResetPasswSuccBackground] = ImGui_LoadTextureFromFile("./Assets/Images/Login_RstSuccess.png");
        m_Images[ImageIds::II_ResetPasswResetButtonHover] = ImGui_LoadTextureFromFile("./Assets/Images/Login_RstButtonHover.png");
        m_Images[ImageIds::II_ResetPasswCancelButtonHover] = ImGui_LoadTextureFromFile("./Assets/Images/Login_CancelButtonHover.png");
        m_Images[ImageIds::II_ResetPasswSuccesButtonHover] = ImGui_LoadTextureFromFile("./Assets/Images/Login_ReturnButtonHover.png");

        m_Images[ImageIds::II_NavigationBackground] = ImGui_LoadTextureFromFile("./Assets/Images/Navigation_Background.png");
        m_Images[ImageIds::II_DemoUserIcon] = ImGui_LoadTextureFromFile("./Assets/Images/Demo_UserImage.png");
        m_Images[ImageIds::II_NavigationDashboardButtonActive] = ImGui_LoadTextureFromFile("./Assets/Images/Navigation_DashboardButtonActive.png");
        m_Images[ImageIds::II_NavigationDashboardButtonHover] = ImGui_LoadTextureFromFile("./Assets/Images/Navigation_DashboardButtonHover.png");
        m_Images[ImageIds::II_NavigationFileButtonActive] = ImGui_LoadTextureFromFile("./Assets/Images/Navigation_FileButtonActive.png");
        m_Images[ImageIds::II_NavigationLocationButtonActive] = ImGui_LoadTextureFromFile("./Assets/Images/Navigation_LocationButtonActive.png");
        m_Images[ImageIds::II_NavigationAlertsButtonActive] = ImGui_LoadTextureFromFile("./Assets/Images/Navigation_AlertsButtonActive.png");
        m_Images[ImageIds::II_NavigationSettingsButonActive] = ImGui_LoadTextureFromFile("./Assets/Images/Navigation_SettingsButtonActive.png");

        m_Images[ImageIds::II_MainWindowBackground] = ImGui_LoadTextureFromFile("./Assets/Images/MainWindow_Background.png");
        m_Images[ImageIds::II_MainWindowLogoutButtonHover] = ImGui_LoadTextureFromFile("./Assets/Images/MainWindow_LogoutButtonHover.png");

//        m_Images[ImageIds::II_DashboardBackground] = ImGui_LoadTextureFromFile("./Assets/Images/Dashboard_Background.png");
        m_Images[ImageIds::II_DashboardRecentAlertsText] = ImGui_LoadTextureFromFile("./Assets/Images/Dashboard_RecentAlertsText.png");
        m_Images[ImageIds::II_DashboardViewMoreButton] = ImGui_LoadTextureFromFile("./Assets/Images/Dashboard_ViewMoreButton.png");
        m_Images[ImageIds::II_DashboardViewMoreButtonHover] = ImGui_LoadTextureFromFile("./Assets/Images/Dashboard_ViewMoreButtonHover.png");
        m_Images[ImageIds::II_DashboardAlertCardBackground] = ImGui_LoadTextureFromFile("./Assets/Images/Dashboard_AlertCardBackground.png");
        m_Images[ImageIds::II_DashboardAlertCardBackgroundHover] = ImGui_LoadTextureFromFile("./Assets/Images/Dashboard_AlertCardBackgroundHover.png");
        m_Images[ImageIds::II_DashboardAlertCardStatusDanger] = ImGui_LoadTextureFromFile("./Assets/Images/Dashboard_AlertCardStatusDanger.png");
        m_Images[ImageIds::II_DashboardAlertCardStatusCaution] = ImGui_LoadTextureFromFile("./Assets/Images/Dashboard_AlertCardStatusCaution.png");
        m_Images[ImageIds::II_DashboardAlertCardStatusInRange] = ImGui_LoadTextureFromFile("./Assets/Images/Dashboard_AlertCardStatusInRange.png");
        m_Images[ImageIds::II_DashboardRecentLocationsText] = ImGui_LoadTextureFromFile("./Assets/Images/Dashboard_RecentLocationsText.png");
        m_Images[ImageIds::II_DashboardActionsText] = ImGui_LoadTextureFromFile("./Assets/Images/Dashboard_ActionsText.png");
        m_Images[ImageIds::II_DashboardActionAlertCardBackgroundHover] = ImGui_LoadTextureFromFile("./Assets/Images/Dashboard_AlertCardBgHover.png");
        m_Images[ImageIds::II_DashboardActionLocationCardBackgroundHover] = ImGui_LoadTextureFromFile("./Assets/Images/Dashboard_LocationCardBgHover.png");
        m_Images[ImageIds::II_DashboardActionModuleCardBackgroundHover] = ImGui_LoadTextureFromFile("./Assets/Images/Dashboard_ModuleCardBgHover.png");

        m_Images[ImageIds::II_LocationsHeader] = ImGui_LoadTextureFromFile("./Assets/Images/Locations_Header.png");
        m_Images[ImageIds::II_LocationsGridViewButton] = ImGui_LoadTextureFromFile("./Assets/Images/Locations_GridViewButton.png");
        m_Images[ImageIds::II_LocationsGridViewButtonHover] = ImGui_LoadTextureFromFile("./Assets/Images/Locations_GridViewButtonHover.png");
        m_Images[ImageIds::II_LocationsGridTrashcanButton] = ImGui_LoadTextureFromFile("./Assets/Images/Locations_GridTrashcanButton.png");
        m_Images[ImageIds::II_LocationsGridDeleteButton] = ImGui_LoadTextureFromFile("./Assets/Images/Locations_GridDeleteButton.png");
        m_Images[ImageIds::II_LocationsGridCancelButton] = ImGui_LoadTextureFromFile("./Assets/Images/Locations_GridCancelButton.png");
        m_Images[ImageIds::II_LocationsGridDescOrder] = ImGui_LoadTextureFromFile("./Assets/Images/Locations_GridDescOrder.png");
        m_Images[ImageIds::II_LocationsGridAscOrder] = ImGui_LoadTextureFromFile("./Assets/Images/Locations_GridAscOrder.png");

        m_Images[ImageIds::II_AlertsHeader] = ImGui_LoadTextureFromFile("./Assets/Images/Alerts_Header.png");
        m_Images[ImageIds::II_AlertStatusDanger] = ImGui_LoadTextureFromFile("./Assets/Images/Alerts_StatusDanger.png");
        m_Images[ImageIds::II_AlertStatusCaution] = ImGui_LoadTextureFromFile("./Assets/Images/Alerts_StatusCaution.png");
        m_Images[ImageIds::II_AlertStatusInRange] = ImGui_LoadTextureFromFile("./Assets/Images/Alerts_StatusInRange.png");

        m_Images[ImageIds::II_SettingsWindowBackground] = ImGui_LoadTextureFromFile("./Assets/Images/Settings_Background.png");
        m_Images[ImageIds::II_SettingsActivityButtonHover] = ImGui_LoadTextureFromFile("./Assets/Images/Settings_ActivityButtonHover.png");
        m_Images[ImageIds::II_SettingsAccountButtonHover] = ImGui_LoadTextureFromFile("./Assets/Images/Settings_AccountButtonHover.png");
        m_Images[ImageIds::II_SettingsModulesButtonHover] = ImGui_LoadTextureFromFile("./Assets/Images/Settings_ModulesButtonHover.png");
        m_Images[ImageIds::II_ActivityLogHeader] = ImGui_LoadTextureFromFile("./Assets/Images/ActivityLog_Header.png");

        m_Images[ImageIds::II_DoplerBackgroundImage] = ImGui_LoadTextureFromFile("./Assets/Images/DoplerRadarBackground.png");
        m_Images[ImageIds::II_GreenPerson] = ImGui_LoadTextureFromFile("./Assets/Images/arrow_green.png");
        m_Images[ImageIds::II_YellowPerson] = ImGui_LoadTextureFromFile("./Assets/Images/arrow_yellow.png");
        m_Images[ImageIds::II_RedPerson] = ImGui_LoadTextureFromFile("./Assets/Images/arrow_red.png");

        unsigned __int64 endStamp = GetTickCount64();
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceImageManager, 0, 0,
            "ImageManager:Loaded all %d images in %lld ms", ImageIds::II_MAX_VALUE, endStamp - startStamp);

        m_bFinisedLoadingImages = true;
        });

    // allow it to self destruct in the background
    ImgLoaderThread.detach();
}

ImTextureID ImageManager::GetImage(ImageIds ii)
{ 
	return (ImTextureID)(m_Images[ii]->DS);
}

int ImageManager::GetImageWidth(ImageIds ii)
{
    return m_Images[ii]->Width;
}

int ImageManager::GetImageHeight(ImageIds ii)
{
    return m_Images[ii]->Height;
}

bool ImageManager::GetImageHit(ImageIds ii, int x, int y)
{
    return m_Images[ii]->ImageHitmap.getPixel(x, y);
}