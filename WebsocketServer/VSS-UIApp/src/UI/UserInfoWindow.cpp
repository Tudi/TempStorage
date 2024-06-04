#include "StdAfx.h"

UserInfoWindow::UserInfoWindow() :
    m_buttonCancelForm(this, OnWindowButtonClick, ButtonIds::BI_USERINFO_CANCEL, LocalizationRssIds::UserInfo_Cancel_Btn_Text),
    m_buttonSave(this, OnWindowButtonClick, ButtonIds::BI_USERINFO_SAVE, LocalizationRssIds::UserInfo_Save_Btn_Text)
{
    InitTypeInfo();

    m_buttonCancelForm.SetColor(GenericButtonColors::GBCT_TextHover, sStyles.GetUIntValue(StyleIds::Btn_Cancel_TextHover_Color));
    m_buttonCancelForm.SetColor(GenericButtonColors::GBCT_UnderlineHover, sStyles.GetUIntValue(StyleIds::Btn_Cancel_TextHover_Color));

    m_sFirstName.SetBadValText("Uppercase and lowercase letters only");
    m_sFirstName.SetCheckValFunc(InputTextData::CheckStrIsCharOnly);
    
    m_sLastName.SetBadValText("Uppercase and lowercase letters only");
    m_sLastName.SetCheckValFunc(InputTextData::CheckStrIsCharOnly);
    
    m_sEmail.SetBadValText("Invalid email");
    m_sEmail.SetCheckValFunc(InputTextData::CheckStrIsEmail);

    m_sEmailConfirm.SetBadValText("Does not match");
    m_sEmailConfirm.SetCheckValFunc(InputTextData::CheckStrIsConfirm);

    m_sPassword.SetBadValText("Invalid password");
    m_sPassword.SetCheckValFunc(InputTextData::CheckStrIsPassword);

    m_sPasswordConfirm.SetBadValText("Does not match");
    m_sPasswordConfirm.SetCheckValFunc(InputTextData::CheckStrIsConfirm);

    ResetState();
}

void UserInfoWindow::DestructorCheckMemLeaks()
{
    m_buttonCancelForm.DestructorCheckMemLeaks();
    m_buttonSave.DestructorCheckMemLeaks();
}

void UserInfoWindow::ResetState()
{
    m_sFirstName.ResetState(sUserSession.GetFirstName());
    m_sLastName.ResetState(sUserSession.GetLastName());
    m_sEmail.ResetState(sUserSession.GetEmail());
    m_sEmailConfirm.ResetState(sUserSession.GetEmail());
    m_sRole.ResetState(sUserSession.GetRole());
    m_sPassword.ResetState("");
    m_sPasswordConfirm.ResetState("");
    m_bSetDefaultFocus = true;
}

void CB_UserDataUpdated(int CurlErr, char* response, void* userData)
{
    userData;
    yyJSON(yydoc);

    if (ExtractDBColumnToBinary::DBH_APIResultValid(CurlErr, response, yydoc, LogSourceGroups::LogSourceUserManagement, "UserInfoWindow") != WebApiErrorCodes::WAE_NoError)
    {
        return;
    }

    // notify user session that it should refresh it's data
    sUserSession.OnLoggedIn(NULL);
}

void UserInfoWindow::OnWindowButtonClick(GenericButton* btn, void* pParent)
{
    if (btn == NULL)
    {
        return;
    }

    UserInfoWindow* wnd = typecheck_castL(UserInfoWindow, pParent);
    if (btn->GetId() == ButtonIds::BI_USERINFO_SAVE)
    {
        bool ShouldCloseWindow = false;
        {
            // check if required fields have value
            InputTextData* MustHaveData[] = { &wnd->m_sFirstName, &wnd->m_sLastName };
            bool AllRequiredHaveValue = true;
            bool AnyValueChanged = false;
            for (int i = 0; i < _countof(MustHaveData); i++)
            {
                if (MustHaveData[i]->CheckNewValOk(NULL) == false)
                {
                    AllRequiredHaveValue = false;
                }
                else if (MustHaveData[i]->ValueChanged() == true)
                {
                    AnyValueChanged = true;
                }
            }

            if (wnd->m_sEmail.CheckNewValOk(NULL) == false)
            {
                AllRequiredHaveValue = false;
            }
            else if (wnd->m_sEmail.ValueChanged() == true)
            {
                AnyValueChanged = true;
            }
            if (wnd->m_sEmailConfirm.CheckNewValOk(wnd->m_sEmail.GetInputBuff()) == false)
            {
                AllRequiredHaveValue = false;
            }

            if (AllRequiredHaveValue == false)
            {
                return;
            }

            if (AnyValueChanged == true)
            {
                WebApi_SetUserInfoAsync(sUserSession.GetUserId(), wnd->m_sFirstName.GetInputBuff(),
                    wnd->m_sLastName.GetInputBuff(), wnd->m_sEmail.GetInputBuff(), CB_UserDataUpdated, NULL);

                ShouldCloseWindow = true;

                AddLogEntry(LogDestinationFlags::LDF_SERVER, LogSeverityValue::LogSeverityNormal,
                    LogSourceGroups::LogSourceUserManagement, 0, 0, "Updated user info");
            }
        }

        {
            if (wnd->m_sPassword.CheckNewValOk(NULL) == true &&
                wnd->m_sPasswordConfirm.CheckNewValOk(wnd->m_sPassword.GetInputBuff()) == true )
            {
                WebApi_PasswChangeAsync(sUserSession.GetUserId(), wnd->m_sPassword.GetInputBuff());

                ShouldCloseWindow = true;

                AddLogEntry(LogDestinationFlags::LDF_SERVER, LogSeverityValue::LogSeverityNormal,
                    LogSourceGroups::LogSourceUserManagement, 0, 0, "Updated user password");
            }
        }

        // close the window. Allow something else to be shown
        if (ShouldCloseWindow)
        {
            sWindowManager.SetUserInfoWindowVisible(false);
        }
    }
    else if (btn->GetId() == ButtonIds::BI_USERINFO_CANCEL)
    {
        // close the window. Allow something else to be shown
        sWindowManager.SetUserInfoWindowVisible(false);
    }
}

int UserInfoWindow::DrawWindow()
{
    int display_w, display_h;
    GetDrawAreaSize(display_w, display_h, true);
    const int inputTextboxWidth = 1000;
    const float LeftBorder = 100;
    const float TopBorder = 100;
    float TextRowSpacing = ImGui::GetTextLineHeightWithSpacing();
    float WindowWidth = (float)display_w - LeftBorder - 15;
    float WindowHeight = (float)display_h - TopBorder - 15;
    const float RaiseInputField = 3;

    ImGui::SetNextWindowPos(ImVec2(LeftBorder, TopBorder), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2((float)WindowWidth - LeftBorder, (float)WindowHeight - TopBorder), ImGuiCond_Always);

    if (ImGui::BeginChild(GetImGuiID("##UserInfo"), ImVec2(WindowWidth, WindowHeight), false, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove))
    {
        ImGui::PushFont(sFontManager.GetFont(FontIds::FI_Medium));
        ImGui::PushStyleColor(ImGuiCol_Text, sStyles.GetUIntValue(StyleIds::Title_Settings_Text_Color));
        ImGui::Text("My Account");
        ImGui::PopStyleColor(1);

        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + TextRowSpacing); // row spacing
        ImGui::Text("Update your account info below.");
        ImGui::PopFont();

        {
            ImGui::SetCursorPosY(ImGui::GetCursorPos().y + TextRowSpacing); // row spacing
            ImGui::Text("Role ");
            ImGui::SameLine();

            ImGui::SetNextItemWidth(inputTextboxWidth - ImGui::GetCursorPosX());
//                    ImGui::InputText(GetImGuiID2("##UserRole"), m_dUserRole, _countof(m_sUserEmail));
            ImGui::Text(m_sRole.GetInputBuff());
            ImGui::SetCursorPosY(ImGui::GetCursorPos().y + TextRowSpacing); // row spacing
        }

        FORM_FLAT_TXTI_FIELD("First Name ", m_sFirstName);
        FORM_FLAT_TXTI_FIELD("Last Name ", m_sLastName);
        FORM_FLAT_TXTI_FIELD("Email ", m_sEmail);
        FORM_FLAT_TXTI_FIELD("Confirm ", m_sEmailConfirm);

        ImGui::PushFont(sFontManager.GetFont(FontIds::FI_Medium));
        ImGui::Text("Create New Password");
        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + TextRowSpacing); // row spacing
        ImGui::Text("New Passwords require a minimum of 16-characters.");
        ImGui::PopFont();

        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + TextRowSpacing); // row spacing
        FORM_FLAT_TXTI_FIELD_F("Password ", m_sPassword, ImGuiInputTextFlags_Password);
        FORM_FLAT_TXTI_FIELD_F("Confirm ", m_sPasswordConfirm, ImGuiInputTextFlags_Password);

        ImGui::PushFont(sFontManager.GetFont(FontIds::FI_Small));
        ImGui::Text("Uppercase and lowercase letters");
        ImGui::Text("Numbers and Special Characters");
        ImGui::Text("No Repeating More Than Two in a Row");
        ImGui::PopFont();

        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + TextRowSpacing); // row spacing
        m_buttonSave.DrawButton();
        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + TextRowSpacing); // row spacing
        m_buttonCancelForm.DrawButton();

        ImGui::Text(" "); // making sure scroll bar will show the whole cancel button. Does not serve other role
    }

    ImGui::EndChild();

    return WindowManagerErrorCodes::WM_NO_ERROR;
}