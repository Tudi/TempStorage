#include "StdAfx.h"

void FlatButton::Init()
{
    InitTypeInfo();
    m_buttonRectMin = { 0,0 };
    m_buttonRectMax = { 0,0 };
    m_uBGColor = sStyles.GetUIntValue(StyleIds::Btn_BG_Color);
    m_uHoverColor = sStyles.GetUIntValue(StyleIds::Btn_Hover_Color);
    m_uActiveColor = sStyles.GetUIntValue(StyleIds::Btn_Active_Color);
    m_uTextColor = sStyles.GetUIntValue(StyleIds::Btn_Text_Color);
    m_uTextHoverColor = sStyles.GetUIntValue(StyleIds::Btn_TextHover_Color);
    m_uTextDisabledColor = sStyles.GetUIntValue(StyleIds::Btn_TextDisabled_Color);
    m_uUnderlineColor = sStyles.GetUIntValue(StyleIds::Btn_Underline_Color);
    m_uUnderlineHoverColor = sStyles.GetUIntValue(StyleIds::Btn_UnderlineHover_Color);
    m_bDrawWindowCentered = false;
    m_fCenterX = 0.0f;
    m_bDrawUnderline = true;
}

FlatButton::FlatButton()
{
    Init();
}

FlatButton::FlatButton(void* owner, ButtonCallback cb, ButtonIds id, LocalizationRssIds txtId):
    GenericButton(owner, cb, id, txtId)
{
    Init();
}

void FlatButton::DrawButton(ImGuiID id)
{
    ImFont* defaultFont = sFontManager.GetFont(FontIds::FI_Button);
    ImGui::PushFont(defaultFont);

    ImGui::PushStyleColor(ImGuiCol_Button, m_uBGColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, m_uHoverColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, m_uActiveColor);

    ImU32 buttonTextColor = m_uTextColor;
    ImU32 buttonUnderlineColor = m_uUnderlineColor;
    if (m_bDisabled)
    {
        buttonTextColor = m_uTextDisabledColor;
        buttonUnderlineColor = m_uTextDisabledColor;
    }
    else if (ImGui::IsMouseHoveringRect(m_buttonRectMin, m_buttonRectMax))
    {
        buttonTextColor = m_uTextHoverColor;
        buttonUnderlineColor = m_uUnderlineHoverColor;
    }

    ImGui::PushStyleColor(ImGuiCol_Text, buttonTextColor);

    if (m_bDrawWindowCentered == true && m_fCenterX == 0.0f)
    {
        ImGui_CenterCursorForText(m_sButtonText);
        m_fCenterX = ImGui::GetCursorPosX();
    }
    if (m_fCenterX != 0.0f)
    {
        ImGui::SetCursorPosX(m_fCenterX);
    }
    if (m_sButtonText)
    {
        if (id == 0)
        {
            id = GetImGuiIDPointer(this);
        }
        if (ImGui::Button(id, m_sButtonText, m_forcedMinSize) && m_bDisabled == false)
        {
            OnPush();
        }
    }
    if (m_bDrawUnderline)
    {
        if (m_fCenterX != 0.0f)
        {
            ImGui::SetCursorPosX(m_fCenterX);
        }
        ImGUIUnderlinePrevItem(sStyles.GetFloatValue(StyleIds::Btn_Underline_Distance),
            sStyles.GetFloatValue(StyleIds::Btn_Underline_Width),
            buttonUnderlineColor);
    }

    // these values are only available after drawing the button content. 
    // Remeber the values so on next cycle we can guess how large the button will get
    m_buttonRectMin = ImGui::GetItemRectMin();
    m_buttonRectMax = ImGui::GetItemRectMax();

    ImGui::PopStyleColor(4);

    // pop default font(button)
    ImGui::PopFont();
}

void FlatButton::DrawButtonAsTextKnownSize()
{
    ImU32 buttonTextColor = m_uTextColor;
    ImVec2 topleft = ImGui::GetCursorPos();
    if (m_bDisabled)
    {
        buttonTextColor = m_uTextDisabledColor;
    }
    else if (isMouseHoveredInArea_Size(topleft, m_forcedMinSize))
    {
        buttonTextColor = m_uTextHoverColor;

        if (ImGui::IsMouseClicked(0))
        {
            OnPush();
        }
    }

    ImGui::PushStyleColor(ImGuiCol_Text, buttonTextColor);

    ImGui::Text(m_sButtonText);

    ImGui::PopStyleColor(1);
}