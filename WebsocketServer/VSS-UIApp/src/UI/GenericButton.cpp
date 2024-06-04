#include "StdAfx.h"

void GenericButton::Init()
{
	m_dId = ButtonIds::BI_UNINITIALIZED_VALUE;
	m_CallbackParent = NULL;
	m_CallBack = NULL;
	m_dNameId = LocalizationRssIds::LRSS_NOT_INITIALIZED;
	m_uBGColor = 0xFF000000;
	m_uHoverColor = 0xFF0000FF;
	m_uActiveColor = 0xFF00FF00;
	m_uTextColor = 0xFF888888;
	m_uTextDisabledColor = 0xFF666666;
	m_uTextHoverColor = 0xFFFFFFFF;
	m_uUnderlineColor = 0xFF888888;
	m_uUnderlineHoverColor = 0xFFFFFFFF;
	m_sButtonText = InternalStrDup("");
	m_dButtonText = 0;
	m_sUserData = NULL;
	m_bDisabled = false;
}

GenericButton::GenericButton()
{
	Init();
}

GenericButton::GenericButton(void* owner, ButtonCallback cb, ButtonIds id, LocalizationRssIds txtId)
{
	Init();

	m_dId = id;
	m_CallbackParent = owner;
	m_CallBack = cb;
	SetTextLocalized(txtId);

	m_forcedMinSize = { 0.0f, 0.0f };
}

void GenericButton::DestructorCheckMemLeaks()
{
	InternalFree(m_sButtonText);
	InternalFree(m_sUserData);
}

GenericButton::~GenericButton()
{
	DestructorCheckMemLeaks();
}

void GenericButton::SetColor(GenericButtonColors colorIndex, ImU32 newColor)
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
	case GenericButtonColors::GBCT_Underline:
	{
		m_uUnderlineColor = newColor;
	}break;
	case GenericButtonColors::GBCT_UnderlineHover:
	{
		m_uUnderlineHoverColor = newColor;
	}break;
	case GenericButtonColors::GBCT_DisabledText:
	{
		m_uTextDisabledColor = newColor;
	}break;
	default:
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceUI, 0, 0,
			"ButtonStyle:Trying to set invalid color type %d to %X", colorIndex, newColor);
	}
	}
	AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceUI, 0, 0,
		"ButtonStyle:Set color type %d to %X", colorIndex, newColor);
}

void GenericButton::SetText(const char* newText)
{
	if (strcmp(newText, m_sButtonText) != 0)
	{
		InternalFree(m_sButtonText);
		m_sButtonText = InternalStrDup(newText);
	}
	if (m_sButtonText == NULL || m_sButtonText[0] == 0)
	{
		m_sButtonText = InternalStrDup("##");
	}
}

void GenericButton::SetText(const int newText)
{
	if (newText == m_dButtonText)
	{
		return;
	}
	m_dButtonText = newText;

	char strBuff[50];
	sprintf_s(strBuff, " %d ", newText);
	SetText(strBuff);
}

void GenericButton::SetTextLocalized(LocalizationRssIds txtId)
{
	m_dNameId = txtId;
	InternalFree(m_sButtonText);
	m_sButtonText = InternalStrDup(sLocalization.GetString(m_dNameId));
	if (m_sButtonText == NULL || m_sButtonText[0] == 0)
	{
		m_dNameId = LocalizationRssIds::LRSS_NOT_INITIALIZED;
		m_sButtonText = InternalStrDup("## ");
	}
}

void GenericButton::SetCallback(void* owner, ButtonCallback cb)
{
	if (m_CallbackParent != NULL && m_CallbackParent != owner)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceUI, 0, 0,
			"Button:Callback parent was already set.");
	}
	m_CallbackParent = owner;
	m_CallBack = cb;
}

void GenericButton::OnPush()
{
	if (m_CallBack)
	{
		m_CallBack(this, m_CallbackParent);
	}
}

void GenericButton::SetUserData(const char* data)
{
	InternalFree(m_sUserData);
	m_sUserData = InternalStrDup(data);
}