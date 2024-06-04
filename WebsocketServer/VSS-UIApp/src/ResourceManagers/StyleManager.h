#pragma once

// Numerical representation of the string keys from the INI file loaded
// These values will be accessed with 60 FPS. Cache them for fast access
enum StyleIds : int
{
    SI_VALUE_NOT_INITIALIZED = 0,
    Btn_BG_Color,
    Btn_Hover_Color,
    Btn_Active_Color,
    Btn_Text_Color,
    Btn_TextHover_Color,
    Btn_TextDisabled_Color,
    Btn_Cancel_TextHover_Color,
    Btn_Underline_Distance,
    Btn_Underline_Width,
    Btn_Underline_Color,
    Btn_UnderlineHover_Color,
    Wnd_Border_Width,
    Wnd_Border_Color,
    Wnd_Txt_Color,
    Wnd_Bg_Color,
    Hdr_Hovered_Color,
    Hdr_Active_Color,
    TitleBar_Bg_Color,
    TitleBar_Active_Color,
    Input_Bg_Color,
    Wnd_Splash_Bg_Color,
    Input_Underline_Distance,
    Input_Underline_Width,
    Input_Underline_Color,
    Input_Txt_ColorInactiveDefaultValue,
    Wnd_Txt_Error_Color,
    Title_Location_Text_Color,
    Title_Alert_Text_Color,
    Title_Settings_Text_Color,
    Menu_Txt_Font_Size,
    Large_Txt_Font_Size,
    Medium_Txt_Font_Size,
    Normal_Txt_Font_Size,
    Small_Txt_Font_Size,
    Button_Txt_Font_Size,
    Menu_Txt_Font_Spacing,
    Large_Txt_Font_Spacing,
    Medium_Txt_Font_Spacing,
    Normal_Txt_Font_Spacing,
    Small_Txt_Font_Spacing,
    Button_Txt_Font_Spacing,
    Wnd_LocationViewOverlay_Bg_Color,
    LocationView_StatusActive_Text_Color,
    LocationView_StatusResolved_Text_Color,
    Btn_RecentLocation_BG_Color,
    Btn_RecentLocation_Hover_Color,
    Btn_RecentLocation_Active_Color,
    Btn_RecentLocation_Text_Color,
    Btn_RecentLocation_TextHover_Color,
    Btn_RecentLocation_Underline_Color,
    Btn_RecentLocation_UnderlineHover_Color,
    DD_BG_Color,
    DD_Border_Color,
    DD_Hover_Color,
    DD_Active_Color,
    DD_Text_Color,
    DD_TextHover_Color,
    DD_TextDisabled_Color,
    Txt_DashboardMedium_Color,
    Txt_DashboardNormal_Color,
    SI_MAX_VAL
};

/// <summary>
/// Object that will hold in memory config values to be accessed globaly within the application
/// This Singleton is expected to be loaded on startup and remain unchanged while app is running
/// </summary>
class StyleManager
{
public:
    inline static StyleManager& getInstance() {
#ifdef _DEBUG
        static StyleManager* instance = new StyleManager;
        return *instance;
#else
        static StyleManager instance;
        return instance;
#endif
    }
    /// <summary>
    /// Loads an 'ini' file content in memory
    /// </summary>
    /// <param name="fileName"></param>
    void LoadStyles(const char *fileName);
    /// <summary>
    /// Get the key-value pair's value
    /// </summary>
    /// <param name="id"></param>
    /// <returns></returns>
    int GetIntValue(StyleIds id)
    {
        return m_dValues[id];
    }
    /// <summary>
    /// Get the key-value pair's value
    /// </summary>
    /// <param name="id"></param>
    /// <returns></returns>
    ImU32 GetUIntValue(StyleIds id)
    {
        return *(ImU32*)&m_dValues[id];
    }
    /// <summary>
    /// Get the key-value pair's value
    /// </summary>
    /// <param name="id"></param>
    /// <returns></returns>
    float GetFloatValue(StyleIds id)
    {
        return *(float*)&m_dValues[id];
    }
    /// <summary>
    /// Only when checking if Application has memory leaks
    /// </summary>
    void DestructorCheckMemLeaks();

    /// <summary>
    /// Only when checking if Application has memory leaks
    /// </summary>
    void ApplyGlobalDefaultStyle(bool bApply);
private:
    StyleManager();
    StyleManager(const StyleManager&) = delete;
    StyleManager& operator=(const StyleManager&) = delete;

    int m_dValues[StyleIds::SI_MAX_VAL];
};

#define sStyles StyleManager::getInstance()
