#include "stdafx.h"

GenericDropdown::GenericDropdown(void* owner, DropDownCallback cb, DropDownIds id)
{
	Init();
	m_CallbackParent = owner;
	m_CallBack = cb;
	m_dId = id;
	m_fVisualWidth = m_fVisualHeight = 0.0f;
}

GenericDropdown::GenericDropdown()
{
	Init();
}

void GenericDropdown::Init()
{
    m_dId = DropDownIds::DI_UNINITIALIZED_VALUE;
    m_CallbackParent = NULL;
	m_CallBack = NULL;
    m_uBGColor = sStyles.GetUIntValue(StyleIds::DD_BG_Color);
	m_uBorderColor = sStyles.GetUIntValue(StyleIds::DD_Border_Color);
    m_uHoverColor = sStyles.GetUIntValue(StyleIds::DD_Hover_Color);
    m_uActiveColor = sStyles.GetUIntValue(StyleIds::DD_Active_Color);
    m_uTextColor = sStyles.GetUIntValue(StyleIds::DD_Text_Color);
    m_uTextHoverColor = sStyles.GetUIntValue(StyleIds::DD_TextHover_Color);
    m_uTextDisabledColor = sStyles.GetUIntValue(StyleIds::DD_TextDisabled_Color);
    m_Entries = NULL;
    m_EntryCount = 0;
    m_sUserData = NULL;
    m_forcedMinSize = { 0,0 };
    m_bDisabled = false;
	m_SelectedEntry = NULL;
	sprintf_s(m_DropdownName, "##myDropdown%p", this);
}

GenericDropdown::~GenericDropdown()
{
	DestructorCheckMemLeaks();
}

void GenericDropdown::DestructorCheckMemLeaks()
{
	InternalFree(m_Entries);
	m_EntryCount = 0;
	InternalFree(m_sUserData);
	m_SelectedEntry = NULL;
}

void GenericDropdown::SetColor(GenericButtonColors colorIndex, ImU32 newColor)
{
	switch (colorIndex)
	{
	case GenericButtonColors::GBCT_Background:
	{
		m_uBGColor = newColor;
	}break;
	case GenericButtonColors::GBCT_Hover:
	{
		m_uHoverColor = newColor;
	}break;
	case GenericButtonColors::GBCT_Active:
	{
		m_uActiveColor = newColor;
	}break;
	case GenericButtonColors::GBCT_Text:
	{
		m_uTextColor = newColor;
	}break;
	case GenericButtonColors::GBCT_TextHover:
	{
		m_uTextHoverColor = newColor;
	}break;
	default:
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceDropDown, 0, 0,
			"DropdownStyle:Trying to set invalid color type %d to %X", colorIndex, newColor);
	}
	}
	AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceDropDown, 0, 0,
		"DropdownStyle:Set color type %d to %X", colorIndex, newColor);
}

void GenericDropdown::SetCallback(void* owner, DropDownCallback cb)
{
	if (m_CallbackParent != NULL && m_CallbackParent != owner)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceUI, 0, 0,
			"Dropdown:Callback parent was already set.");
	}
	m_CallbackParent = owner;
	m_CallBack = cb;
}

void GenericDropdown::OnPush()
{
	if (m_CallBack)
	{
		m_CallBack(this, m_CallbackParent);
	}
}

void GenericDropdown::SetUserData(const char* data)
{
	InternalFree(m_sUserData);
	m_sUserData = InternalStrDup(data);
}

void GenericDropdown::DrawDropdown()
{
	ImGui::PushStyleColor(ImGuiCol_PopupBg, m_uBGColor);
	ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, m_uHoverColor);
	ImGui::PushStyleColor(ImGuiCol_FrameBgActive, m_uHoverColor);
	ImGui::PushStyleColor(ImGuiCol_Border, 0);

	if(m_fVisualWidth != 0.0f || m_fVisualHeight != 0.0f)
	{
		// this is the popup window size
		ImGui::SetNextWindowSize(ImVec2(m_fVisualWidth, m_fVisualHeight));
		// this sets the width of the dropdown ( non expanded )
		ImGui::SetNextItemWidth(m_fVisualWidth);
	}

	// Begin the combo box
	bool bIsExpanded = false;
	ImVec2 clipMin, clipMax;
	if (ImGui::BeginCombo(m_DropdownName, m_SelectedEntry, ImGuiComboFlags_NoArrowButton))
	{
		bIsExpanded = true;
		// Loop through each item in the combo
		for (size_t i = 0; i < m_EntryCount; i++)
		{
			// Check if the current item is selected
			const bool isSelected = (m_SelectedEntry == m_Entries[i].Text);

			// Display the item in the combo
			if (ImGui::Selectable(m_Entries[i].Text, isSelected))
			{
				// If the item is selected, set the current item
				m_SelectedEntry = m_Entries[i].Text;
				OnPush();
			}

			// If the current item is selected, set the combo preview text
			if (isSelected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}

		ImGui::GetWindowClipRect(clipMin, clipMax);

		// End the combo
		ImGui::EndCombo();
	}

	ImDrawList* drawList = ImGui::GetWindowDrawList();

	ImVec2 ComboBoxBoundingRectMin = ImGui::GetItemRectMin();
	ImVec2 ComboBoxBoundingRectMax = ImGui::GetItemRectMax();

	ComboBoxBoundingRectMin.x -= 1.0f; // or else popup would render over it
	ComboBoxBoundingRectMax.x += 1.0f; // this is the scrollbar ? What if there is no scrollbar ?

	// frame around the combo box
	if (bIsExpanded)
	{
		drawList->AddRect(
			ImVec2(ComboBoxBoundingRectMin.x - 0.0f, ComboBoxBoundingRectMin.y),
			ImVec2(ComboBoxBoundingRectMax.x + 0.0f, clipMax.y + 2.0f),
			m_uBorderColor);
	}
	else
	{
		drawList->AddRect(ComboBoxBoundingRectMin, ComboBoxBoundingRectMax, m_uBorderColor);
	}

	// Draw arrow
	// Calculate arrow position
	ImVec2 arrowSize = ImVec2(ImGui::GetTextLineHeight() * 1.5f, 
		ComboBoxBoundingRectMax.y - ComboBoxBoundingRectMin.y);
	ImVec2 arrowPos = ImVec2(ComboBoxBoundingRectMax.x - arrowSize.x + 1.0f, ComboBoxBoundingRectMin.y);

	// Draw arrow background
	drawList->AddRectFilled(arrowPos, 
		ImVec2(arrowPos.x + arrowSize.x, arrowPos.y + arrowSize.y), 
		m_uBGColor);
	// Draw arrow frame
	drawList->AddRect(ImVec2(arrowPos.x, arrowPos.y),
		ImVec2(ComboBoxBoundingRectMax.x, ComboBoxBoundingRectMax.y),
		m_uBorderColor);

	const float triangleDistCoef1 = 0.3f;
	const float triangleDistCoef1Inv = 1.0f - triangleDistCoef1;
	drawList->AddTriangleFilled(
		ImVec2(arrowPos.x + arrowSize.x * triangleDistCoef1, arrowPos.y + arrowSize.y * triangleDistCoef1),
		ImVec2(arrowPos.x + arrowSize.x * triangleDistCoef1Inv, arrowPos.y + arrowSize.y * triangleDistCoef1),
		ImVec2(arrowPos.x + arrowSize.x * 0.5f, arrowPos.y + arrowSize.y * triangleDistCoef1Inv),
		m_uTextColor);
 
	ImGui::PopStyleColor(4);
}

void GenericDropdown::SetSize(uint32_t entryCount)
{
	// make sure we do not early exist and create a crash
	InternalFree(m_Entries);
	m_EntryCount = 0;
	m_SelectedEntry = NULL;

	// alloc new store
	m_Entries = (DropdownEntry*)InternalMalloc(entryCount * sizeof(DropdownEntry));
	if (m_Entries == NULL)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceUI, 0, 0,
			"Dropdown:Failed to allocated memory.");
		return;
	}

	// reinit store
	memset(m_Entries, 0, entryCount * sizeof(DropdownEntry));
	m_EntryCount = entryCount;

	// to avoid Imgui crashing
	m_SelectedEntry = m_Entries[0].Text;
}

void GenericDropdown::SetEntryData(uint32_t rowNr, const char* newText, uint32_t callbackId)
{
	if (rowNr >= m_EntryCount)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceUI, 0, 0,
			"Dropdown:Trying to write on row %. Max is %d.", rowNr, m_EntryCount);
		return;
	}

	if (newText != NULL)
	{
		strcpy_s(m_Entries[rowNr].Text, newText);
	}
	else
	{
		strcpy_s(m_Entries[rowNr].Text, "");
	}

	m_Entries[rowNr].CallbackId = callbackId;
}

void GenericDropdown::SetSelectedRow(uint32_t rowNr)
{
	if (rowNr >= m_EntryCount)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceUI, 0, 0,
			"Dropdown:Trying to select row %. Max is %d.", rowNr, m_EntryCount);
		return;
	}

	m_SelectedEntry = m_Entries[rowNr].Text;
}

const DropdownEntry* GenericDropdown::SetSelectedRowFind(uint64_t nCallbackId)
{
	for (size_t i = 0; i < m_EntryCount; i++)
	{
		if (nCallbackId == m_Entries[i].CallbackId)
		{
			m_SelectedEntry = m_Entries[i].Text;
			return &m_Entries[i];
		}
	}

	return NULL;
}

const DropdownEntry* GenericDropdown::GetSelectedEntry()
{
	for (size_t i = 0; i < m_EntryCount; i++)
	{
		if (m_SelectedEntry == m_Entries[i].Text)
		{
			return &m_Entries[i];
		}
	}

	return NULL;
}