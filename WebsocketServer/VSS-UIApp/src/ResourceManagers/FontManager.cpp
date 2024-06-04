#include "stdafx.h"
#include <imgui.h>

// fonts will only be available with specific sizes
bool checkAndReportFontSizeAcceptable(float fontSize)
{
	const float acceptableSizes[] = { 12.0f, 18.0f, 24.0f, 36.0f, 48.0f, 60.0f, 72.0f, -1 };
	for (int i = 0; acceptableSizes[i] > 0; i++)
	{
		if (fontSize == acceptableSizes[i])
		{
			return true;
		}
	}

	AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceFontManager, 0, 0,
		"FontManager:Tried to load font size %f. Not a native resolution", fontSize);

	return false;
}

FontManager::FontManager()
{
	memset(m_Fonts, 0, sizeof(m_Fonts));
}

void FontManager::DestructorCheckMemLeaks()
{
	for (int i = 0; i < _countof(m_Fonts); i++)
	{
		m_Fonts[i] = NULL; // will be handled by ImGUI
	}
	if (ImGui::GetCurrentContext())
	{
		ImGuiIO& io = ImGui::GetIO();
		io.Fonts->Clear();
	}

#ifdef _DEBUG
	delete& sFontManager;
#endif
}

void FontManager::InitFonts()
{
	ImGuiIO& io = ImGui::GetIO();
	const char* fname;
	float fontSize;

	fname = sConfigManager.GetString(ConfigOptionIds::MenuTextFontFile);
	if (fname != NULL)
	{
		fontSize = sStyles.GetFloatValue(StyleIds::Menu_Txt_Font_Size);
		checkAndReportFontSizeAcceptable(fontSize);
		ImFontConfig font_cfg;
		font_cfg.GlyphExtraSpacing.x = sStyles.GetFloatValue(StyleIds::Menu_Txt_Font_Spacing);
		m_Fonts[FontIds::FI_Menu] = io.Fonts->AddFontFromFileTTF(fname, fontSize, &font_cfg);
	}
	if (fname == NULL || m_Fonts[FontIds::FI_Menu] == NULL)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceFontManager, 0, 0,
			"FontManager:Failed to load font file for Menu fonts");
	}

	fname = sConfigManager.GetString(ConfigOptionIds::LargeTextFontFile);
	if (fname != NULL)
	{
		fontSize = sStyles.GetFloatValue(StyleIds::Large_Txt_Font_Size);
		checkAndReportFontSizeAcceptable(fontSize);
		ImFontConfig font_cfg;
		font_cfg.GlyphExtraSpacing.x = sStyles.GetFloatValue(StyleIds::Large_Txt_Font_Spacing);
		m_Fonts[FontIds::FI_Large] = io.Fonts->AddFontFromFileTTF(fname, fontSize, &font_cfg);
	}
	if (fname == NULL || m_Fonts[FontIds::FI_Large] == NULL)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceFontManager, 0, 0,
			"FontManager:Failed to load font file for large fonts");
	}

	fname = sConfigManager.GetString(ConfigOptionIds::MediumTextFontFile);
	if (fname != NULL)
	{
		fontSize = sStyles.GetFloatValue(StyleIds::Medium_Txt_Font_Size);
		checkAndReportFontSizeAcceptable(fontSize);
		ImFontConfig font_cfg;
		font_cfg.GlyphExtraSpacing.x = sStyles.GetFloatValue(StyleIds::Medium_Txt_Font_Spacing);
		m_Fonts[FontIds::FI_Medium] = io.Fonts->AddFontFromFileTTF(fname, fontSize, &font_cfg);
	}
	if (fname == NULL || m_Fonts[FontIds::FI_Medium] == NULL)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceFontManager, 0, 0,
			"FontManager:Failed to load font file for medium fonts");
	}

	fname = sConfigManager.GetString(ConfigOptionIds::NormalTextFontFile);
	if (fname != NULL)
	{
		fontSize = sStyles.GetFloatValue(StyleIds::Normal_Txt_Font_Size);
		checkAndReportFontSizeAcceptable(fontSize);
		ImFontConfig font_cfg;
		font_cfg.GlyphExtraSpacing.x = sStyles.GetFloatValue(StyleIds::Normal_Txt_Font_Spacing);
		m_Fonts[FontIds::FI_Normal] = io.Fonts->AddFontFromFileTTF(fname, fontSize, &font_cfg);
	}
	if (fname == NULL || m_Fonts[FontIds::FI_Normal] == NULL)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceFontManager, 0, 0,
			"FontManager:Failed to load font file for normal fonts");
	}

	fname = sConfigManager.GetString(ConfigOptionIds::ButtonTextFontFile);
	if (fname != NULL)
	{
		fontSize = sStyles.GetFloatValue(StyleIds::Button_Txt_Font_Size);
		checkAndReportFontSizeAcceptable(fontSize);
		ImFontConfig font_cfg;
		font_cfg.GlyphExtraSpacing.x = sStyles.GetFloatValue(StyleIds::Button_Txt_Font_Spacing);
		m_Fonts[FontIds::FI_Button] = io.Fonts->AddFontFromFileTTF(fname, fontSize, &font_cfg);
	}
	if (fname == NULL || m_Fonts[FontIds::FI_Button] == NULL)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceFontManager, 0, 0,
			"FontManager:Failed to load font file for normal-sparse fonts");
	}

	fname = sConfigManager.GetString(ConfigOptionIds::SmallTextFontFile);
	if (fname != NULL)
	{
		fontSize = sStyles.GetFloatValue(StyleIds::Small_Txt_Font_Size);
		checkAndReportFontSizeAcceptable(fontSize);
		ImFontConfig font_cfg;
		font_cfg.GlyphExtraSpacing.x = sStyles.GetFloatValue(StyleIds::Small_Txt_Font_Spacing);
		m_Fonts[FontIds::FI_Small] = io.Fonts->AddFontFromFileTTF(fname, fontSize, &font_cfg);
	}
	if (fname == NULL || m_Fonts[FontIds::FI_Small] == NULL)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceFontManager, 0, 0,
			"FontManager:Failed to load font file for normal-sparse fonts");
	}
	io.Fonts->Build();
}
