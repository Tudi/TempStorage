#pragma once

// I downloaded the fonts from here :
// https://www.1001freefonts.com/new-fonts.php

// Numeric representation of config keys
enum FontIds
{
    FI_UNINITIALIZED_VALUE = 0,
    FI_Menu,
    FI_Large,
    FI_Medium,
    FI_Normal,
    FI_Small,
    FI_Button,
    FI_MAX_VALUE
};

/// <summary>
/// Store multiple fonts with different sizes so that we can use them on the go
/// </summary>
class FontManager
{
public:
    inline static FontManager& getInstance() {
#ifdef _DEBUG
        static FontManager* instance = new FontManager;
        return *instance;
#else
        static FontManager instance;
        return instance;
#endif
    }

    /// <summary>
    /// Load fonts and their settings. Needs to be called after the ImGUI context has been initialized
    /// </summary>
    void InitFonts();

    /// <summary>
    /// Fetch store fonts with different sizes so that we can use them on the go
    /// </summary>
    ImFont* GetFont(FontIds fi) { return m_Fonts[fi]; }

    /// <summary>
    /// Only when checking if Application has memory leaks
    /// </summary>
    void DestructorCheckMemLeaks();
private:
    FontManager();
    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;

    ImFont* m_Fonts[FI_MAX_VALUE];
 };

#define sFontManager FontManager::getInstance()