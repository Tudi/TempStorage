#pragma once

// Access Images using IDs
enum ImageIds
{
    II_UNINITIALIZED_VALUE = 0,
    II_LoginBackground,
    II_LoginButtonHover,
    II_LoginInputInvalidPassw,
    II_ResetPasswBackground,
    II_ResetPasswSuccBackground,
    II_ResetPasswResetButtonHover,
    II_ResetPasswCancelButtonHover,
    II_ResetPasswSuccesButtonHover,

    II_MainWindowBackground,
    II_MainWindowLogoutButtonHover,

    II_NavigationBackground,
    II_DashboardBackground,
    II_DashboardViewMoreButton,
    II_DashboardViewMoreButtonHover,
    II_DashboardRecentAlertsText,
    II_DashboardAlertCardBackground,
    II_DashboardAlertCardBackgroundHover,
    II_DashboardRecentLocationsText,
    II_DashboardAlertCardStatusDanger,
    II_DashboardAlertCardStatusCaution,
    II_DashboardAlertCardStatusInRange,
    II_DashboardActionsText,
    II_DashboardActionAlertCardBackgroundHover,
    II_DashboardActionLocationCardBackgroundHover,
    II_DashboardActionModuleCardBackgroundHover,


    II_DemoUserIcon,

    II_NavigationDashboardButtonActive,
    II_NavigationDashboardButtonHover,
    II_NavigationFileButtonActive,
    II_NavigationFileButtonHover,
    II_NavigationLocationButtonActive,
    II_NavigationLocationButtonHover,
    II_NavigationAlertsButtonActive,
    II_NavigationAlertsButtonHover,
    II_NavigationSettingsButonActive,
    II_NavigationSettingsButonHover,
    II_NavigationLogoutButonHover,

    II_LocationsHeader,
    II_LocationsGridViewButton,
    II_LocationsGridViewButtonHover,
    II_LocationsGridTrashcanButton,
    II_LocationsGridDeleteButton,
    II_LocationsGridCancelButton,
    II_LocationsGridDescOrder,
    II_LocationsGridAscOrder,

    II_AlertsHeader,
    II_AlertStatusDanger,
    II_AlertStatusCaution,
    II_AlertStatusInRange,

    II_SettingsWindowBackground,
    II_SettingsActivityButtonHover,
    II_SettingsAccountButtonHover,
    II_SettingsModulesButtonHover,
    II_ActivityLogHeader,

    II_DoplerBackgroundImage,
    II_GreenPerson,
    II_YellowPerson,
    II_RedPerson,
    II_AppLogoPlaceholder,
    II_MAX_VALUE
};

struct VSSImageStore;

/// <summary>
/// Store multiple images that ImGUI can render
/// </summary>
class ImageManager
{
public:
    inline static ImageManager& getInstance() {
#ifdef _DEBUG
        static ImageManager* instance = new ImageManager;
        return *instance;
#else
        static ImageManager instance;
        return instance;
#endif
    }

    /// <summary>
    /// Load fonts and their settings. Needs to be called after the ImGUI context has been initialized
    /// </summary>
    void InitImages();

    /// <summary>
    /// Fetch store fonts with different sizes so that we can use them on the go
    /// </summary>
    ImTextureID GetImage(ImageIds ii);

    /// <summary>
    /// Get the size of a loaded image. Used for dynamic rendering only
    /// </summary>
    int GetImageWidth(ImageIds ii);
    int GetImageHeight(ImageIds ii);

    /// <summary>
    /// Check if an image x,y alpha channel is different than 0
    /// </summary>
    bool GetImageHit(ImageIds ii, int x, int y);

    /// <summary>
    /// Only when checking if Application has memory leaks
    /// </summary>
    void DestructorCheckMemLeaks();

    bool FinishedLoadingImages() { return m_bFinisedLoadingImages; }
private:
    ImageManager();
    ImageManager(const ImageManager&) = delete;
    ImageManager& operator=(const ImageManager&) = delete;

    VSSImageStore* m_Images[II_MAX_VALUE];
    bool m_bFinisedLoadingImages; // because of async call
};

#define sImageManager ImageManager::getInstance()