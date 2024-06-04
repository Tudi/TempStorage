#include "StdAfx.h"

LoginWindow::LoginWindow()
#if defined(VER1_RENDERING)
    :
    m_ButtonLogin(this, OnWindowButtonClick, ButtonIds::BI_LOGIN_BUTTON_ID, LocalizationRssIds::Login_Btn_Text),
    m_ButtonResetPassw(this, OnWindowButtonClick, ButtonIds::BI_RESET_PASSW_BUTTON_ID, LocalizationRssIds::Login_ResetPassw_Btn_Text)
#endif
{
    InitTypeInfo();

    m_cUserName.SetBadValText("Invalid email");
    m_cUserName.SetCheckValFunc(InputTextData::CheckStrIsEmail);
    m_cUserName.SetEmptyValueText("Enter your email");

    m_cPassword.SetBadValText("Invalid password");
    m_cPassword.SetCheckValFunc(InputTextData::CheckStrIsEmpty);
    m_cPassword.SetEmptyValueText("Enter your password");
    m_cPassword.SetFocus();
    m_cPassword.IsFocusedByDefault();


    m_ButtonLogin2.SetMinSize(200, 32);
    m_ButtonLogin2.SetCallback(this, OnWindowButtonClick);
    m_ButtonLogin2.SetId(ButtonIds::BI_LOGIN_BUTTON_ID);
    m_ButtonLogin2.SetHoverImageId(sImageManager.GetImage(II_LoginButtonHover));
    m_ButtonLogin2.LoadHitMap("./Assets/Images/Login_LoginButton.png");

    m_ButtonResetPassw2.SetMinSize(122, 16);
    m_ButtonResetPassw2.SetCallback(this, OnWindowButtonClick);
    m_ButtonResetPassw2.SetId(ButtonIds::BI_RESET_PASSW_BUTTON_ID);

    ResetState();
}

void LoginWindow::DestructorCheckMemLeaks()
{
#if defined(VER1_RENDERING)
    m_ButtonLogin.DestructorCheckMemLeaks();
    m_ButtonResetPassw.DestructorCheckMemLeaks();
#endif
    m_ButtonLogin2.DestructorCheckMemLeaks();
    m_ButtonResetPassw2.DestructorCheckMemLeaks();
}

void LoginWindow::ResetState()
{
    m_cUserName.ResetState(sConfigManager.GetLastActiveUser());
    m_cPassword.ResetState("");
    m_cPassword.IsFocusedByDefault();
    m_cPassword.SetFocus();
    m_bSetDefaultFocus = true;
    m_bTokenRequired = false;
}

void LoginWindow::OnWindowButtonClick(GenericButton* btn, void* pParent)
{
    if (btn == NULL)
    {
        return;
    }

    LoginWindow* wnd = typecheck_castL(LoginWindow, pParent);
    if (btn->GetId() == ButtonIds::BI_LOGIN_BUTTON_ID)
    {
        while (sAppSession.IsStartupPhaseDone() == false) { Sleep(1); }
        if (wnd->m_bTokenRequired == true)
        {
            if (wnd->m_cPassword.CheckNewValOk(NULL) == false)
            {
                return;
            }

            WebApiErrorCodes res = WebApi_LoginUser(wnd->m_cUserName.GetInputBuff(), wnd->m_cPassword.GetInputBuff(), true);
            auto errStrView = magic_enum::enum_name<WebApiErrorCodes>(res);
            AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceLoginWindow, 0, 0,
                "Login result is %d - %s", res, errStrView.data());

            if (res == WebApiErrorCodes::WAE_MFATokenInvalid)
            {
                wnd->m_cPassword.SetWarningValue(true);
            }
            else if (res == WebApiErrorCodes::WAE_NoError)
            {
                sWindowManager.SetLoginWindowVisible(false);
            }

            wnd->m_bTokenRequired = false;
            wnd->m_bSetDefaultFocus = true;
        }
        else
        {
            bool AllRequiredHaveValue = true;
            if (wnd->m_cUserName.CheckNewValOk(NULL) == false)
            {
                AllRequiredHaveValue = false;
            }
            if (wnd->m_cPassword.CheckNewValOk(NULL) == false)
            {
                AllRequiredHaveValue = false;
            }

            if (AllRequiredHaveValue == false)
            {
                return;
            }

            WebApiErrorCodes res = WebApi_LoginUser(wnd->m_cUserName.GetInputBuff(), wnd->m_cPassword.GetInputBuff(), false);
            auto errStrView = magic_enum::enum_name<WebApiErrorCodes>(res);
            AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceLoginWindow, 0, 0,
                "Login result is %d - %s", res, errStrView.data());

            if (res == WebApiErrorCodes::WAE_InvalidUserName)
            {
                wnd->m_cUserName.SetWarningValue(true);
            }
            else if (res == WebApiErrorCodes::WAE_InvalidPassword)
            {
                wnd->m_cPassword.SetWarningValue(true);
            }
            else if (res == WebApiErrorCodes::WAE_NoError)
            {
                sWindowManager.SetDashboardWindowVisible(true);
            }
            else if (res == WebApiErrorCodes::WAE_MFATokenRequired)
            {
                wnd->m_cPassword.ResetState("");
                wnd->m_bTokenRequired = true;
                wnd->m_bSetDefaultFocus = true;
            }
            // unhandled error. Show some generic error message ?
            else
            {
                wnd->m_cUserName.SetWarningValue(true);
            }
        }
    }
    else if (btn->GetId() == ButtonIds::BI_RESET_PASSW_BUTTON_ID)
    {
        sWindowManager.SetRstPasswWindowVisible(true);
    }
}

#if defined(VER1_RENDERING)
int LoginWindow::DrawWindow()
{
    int display_w, display_h;
    GetDrawAreaSize(display_w, display_h, true);

    const int inputTextboxWidth = 500;
    const float loginWindowWidth = 1100;
    const float loginWindowHeight = 450;
    float TextRowSpacing = ImGui::GetTextLineHeightWithSpacing();
    const float RaiseInputField = 3;

    ImGui::SetNextWindowPos(ImVec2((display_w - loginWindowWidth) / 2.0f, (display_h - loginWindowHeight) / 2.0f), ImGuiCond_Always);
    if (ImGui::BeginChild(GetImGuiID("Login"), ImVec2(loginWindowWidth, loginWindowHeight), false, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_Modal | ImGuiWindowFlags_NoSavedSettings))
    {
        ImGui::PushFont(sFontManager.GetFont(FontIds::FI_Medium));
        ImGui::Text("Start Session");

        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + TextRowSpacing); // row spacing
        ImGui::Text("Please login with your credentials below.");
        ImGui::PopFont();
        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + TextRowSpacing); // row spacing

        if (m_bTokenRequired == false)
        {
            FORM_FLAT_TXTI_FIELD("Email* ", m_cUserName);
            if (m_bSetDefaultFocus == true)
            {
                ImGui::SetKeyboardFocusHere();
                m_bSetDefaultFocus = false;
            }
            FORM_FLAT_TXTI_FIELD_F("Password* ", m_cPassword, ImGuiInputTextFlags_Password);
        }
        else
        {
            if (m_bSetDefaultFocus == true)
            {
                ImGui::SetKeyboardFocusHere();
                m_bSetDefaultFocus = false;
            }
            FORM_FLAT_TXTI_FIELD_F("Token* ", m_cPassword, ImGuiInputTextFlags_Password);
        }

        m_ButtonLogin.DrawButton();

        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + TextRowSpacing); // row spacing
        m_ButtonResetPassw.DrawButton();

        // Check for "Enter" keypress event
        if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)) 
        {
            OnWindowButtonClick(&m_ButtonLogin, this);
        }
    }

    ImGui::EndChild();

    return WindowManagerErrorCodes::WM_NO_ERROR;
}
#else
int LoginWindow::DrawWindow()
{
    int display_w, display_h;
    GetDrawAreaSize(display_w, display_h, true);

    // center to the middle
//    ImVec2 imageSize((float)sImageManager.GetImageWidth(ImageIds::II_LoginBackground), 
//        (float)sImageManager.GetImageHeight(ImageIds::II_LoginBackground));
    ImVec2 imageSize = { 1856,976 };

    // top left corner position
    ImVec2 imagePosition = ImVec2((display_w - imageSize.x) / 2.0f, (display_h - imageSize.y) / 2.0f);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddImage(sImageManager.GetImage(ImageIds::II_LoginBackground),
        imagePosition, ImVec2(imagePosition.x + imageSize.x, imagePosition.y + imageSize.y));

    if (m_cPassword.HasWarningValue())
    {
        const ImVec2 imagePosBadPassw = { imagePosition.x + 712,imagePosition.y + 558 };
        const ImVec2 imageSizeBadPassw = { 432,98 };
        drawList->AddImage(sImageManager.GetImage(ImageIds::II_LoginInputInvalidPassw),
            imagePosBadPassw,
            ImVec2(imagePosBadPassw.x + imageSizeBadPassw.x, imagePosBadPassw.y + imageSizeBadPassw.y));
    }

    const int inputTextboxWidth = 390;

    if (m_bTokenRequired == false)
    {
        // top left corner of the input text
        ImGui::SetCursorPos(ImVec2(imagePosition.x + 717, imagePosition.y + 503));
        FORM_INVISIBLE_TXTI_FIELD(&m_cUserName, inputTextboxWidth, 0);

        ImGui::SetCursorPos(ImVec2(imagePosition.x + 717, imagePosition.y + 593));
        FORM_INVISIBLE_TXTI_FIELD(&m_cPassword, inputTextboxWidth, ImGuiInputTextFlags_Password);
    }
/*    else
    {
        if (m_bSetDefaultFocus == true)
        {
            ImGui::SetKeyboardFocusHere();
            m_bSetDefaultFocus = false;
        }
        FORM_FLAT_TXTI_FIELD_F("Token* ", m_cPassword, ImGuiInputTextFlags_Password);
    }*/

    ImGui::SetCursorPos(ImVec2(imagePosition.x + 831, imagePosition.y + 696));
    m_ButtonLogin2.DrawButton();

    ImGui::SetCursorPos(ImVec2(imagePosition.x + 711, imagePosition.y + 640));
    m_ButtonResetPassw2.DrawButton();

    // Check for "Enter" keypress event
    if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter))
    {
        OnWindowButtonClick(&m_ButtonLogin2, this);
    }

    return WindowManagerErrorCodes::WM_NO_ERROR;
}
#endif