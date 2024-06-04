#include "StdAfx.h"

StyleManager::StyleManager()
{
	memset(m_dValues, 0, sizeof(m_dValues));
}

void StyleManager::DestructorCheckMemLeaks()
{
#ifdef _DEBUG
	delete& sStyles;
#endif

}

static StyleIds GetStyleId(const char* keyName)
{
	if (keyName == NULL)
	{
		return StyleIds::SI_VALUE_NOT_INITIALIZED;
	}
	auto keyname = magic_enum::enum_cast<StyleIds>(keyName, magic_enum::case_insensitive);
	if (keyname.has_value() && StyleIds::SI_VALUE_NOT_INITIALIZED < keyname && keyname < StyleIds::SI_MAX_VAL)
	{
		return keyname.value();
	}
	return StyleIds::SI_VALUE_NOT_INITIALIZED;
}

enum StyleValueTypes
{
	STV_INVALID,
	STV_HEX_VALUE,
	STV_TUPLE_COLOR,
	STV_FLOAT_VALUE,
	STV_NUMERIC,
	STV_STRING
};

static StyleValueTypes GuessStyleValueType(const char *str)
{
	if (str == NULL)
	{
		return StyleValueTypes::STV_INVALID;
	}
	if (str[0] == '0' && str[1] == 'x')
	{
		return StyleValueTypes::STV_HEX_VALUE;
	}

	int dotCount = 0;
	int periodCount = 0;
	bool isNumeric = 1;
	while (*str != 0)
	{
		if (*str == '.')
		{
			dotCount++;
		}
		else if (*str == ',')
		{
			periodCount++;
		}
		else if (!std::isdigit(*str))
		{
			isNumeric = 0;
		}
		str++;
	}
	if (dotCount == 1 && periodCount == 0 && isNumeric == 1)
	{
		return StyleValueTypes::STV_FLOAT_VALUE;
	}
	if(dotCount==0 && periodCount == 0 && isNumeric == 1)
	{
		return StyleValueTypes::STV_NUMERIC;
	}
	if (dotCount == 0 && periodCount == 3 && isNumeric == 1)
	{
		return StyleValueTypes::STV_TUPLE_COLOR;
	}

	return StyleValueTypes::STV_INVALID;
}

void StyleManager::LoadStyles(const char* fileName)
{
	AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityNormal, LogSourceGroups::LogSourceConfig, 0, 0,
		"StyleManager:Started loading style values from %s", fileName);

	IniFile ini = parseIniFile(fileName);
	for (auto sectionsItr = ini.sections.begin(); sectionsItr != ini.sections.end(); sectionsItr++)
	{
		for (auto keyval : sectionsItr->second)
		{
			StyleIds id = GetStyleId(keyval.first.c_str());
			if (id == StyleIds::SI_VALUE_NOT_INITIALIZED)
			{
				continue;
			}
			std::string localVal = keyval.second;
			// remove spaces from the value string
			localVal.erase(std::remove_if(localVal.begin(), localVal.end(), ::isspace), localVal.end());
			// try to guess what type of value is
			StyleValueTypes vt = GuessStyleValueType(localVal.c_str());

			AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceConfig, 0, 0,
				"StyleManager:Assigned id-val %d = %s type %d", id, keyval.second.c_str(), vt);

			if (vt == StyleValueTypes::STV_INVALID)
			{
				continue;
			}
			if (vt == StyleValueTypes::STV_NUMERIC)
			{
				m_dValues[id] = std::stoi(keyval.second);
			}
			else if (vt == StyleValueTypes::STV_FLOAT_VALUE)
			{
				*(float*)&m_dValues[id] = std::stof(keyval.second);
			}
			else if (vt == StyleValueTypes::STV_HEX_VALUE)
			{
				m_dValues[id] = std::stoul(keyval.second, nullptr, 16);
			}
			else if (vt == StyleValueTypes::STV_TUPLE_COLOR)
			{
				unsigned int RGBA[4];
				sscanf_s(keyval.second.c_str(), "%u,%u,%u,%u", &RGBA[0], &RGBA[1], &RGBA[2], &RGBA[3]);
				m_dValues[id] = IM_COL32(RGBA[0], RGBA[1], RGBA[2], RGBA[3]);
			}
		}
	}
}

void StyleManager::ApplyGlobalDefaultStyle(bool bApply)
{
	if (bApply == true)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize,
			sStyles.GetFloatValue(StyleIds::Wnd_Border_Width));

		ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize,
			sStyles.GetFloatValue(StyleIds::Wnd_Border_Width));

		ImGui::PushStyleColor(ImGuiCol_Border,
			sStyles.GetUIntValue(StyleIds::Wnd_Border_Color));

		ImGui::PushStyleColor(ImGuiCol_Text,
			sStyles.GetUIntValue(StyleIds::Wnd_Txt_Color));

		ImGui::PushStyleColor(ImGuiCol_WindowBg,
			sStyles.GetUIntValue(StyleIds::Wnd_Bg_Color));

		ImGui::PushStyleColor(ImGuiCol_ChildBg,
			sStyles.GetUIntValue(StyleIds::Wnd_Bg_Color));

		ImGui::PushStyleColor(ImGuiCol_HeaderHovered,
			sStyles.GetUIntValue(StyleIds::Hdr_Hovered_Color));

		ImGui::PushStyleColor(ImGuiCol_Header,
			sStyles.GetUIntValue(StyleIds::Hdr_Active_Color));

		ImGui::PushStyleColor(ImGuiCol_TitleBg,
			sStyles.GetUIntValue(StyleIds::TitleBar_Bg_Color));

		ImGui::PushStyleColor(ImGuiCol_TitleBgActive,
			sStyles.GetUIntValue(StyleIds::TitleBar_Active_Color));

		ImGui::PushStyleColor(ImGuiCol_FrameBg,
			sStyles.GetUIntValue(StyleIds::Input_Bg_Color));

		// set default row spacing
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 8.0f));
	}
	else
	{
		// pop font spacing, border size, child border size
		ImGui::PopStyleVar(3);
		// border color, text color, bg color
		ImGui::PopStyleColor(9); // colors
	}
}
