#include "StdAfx.h"
#include "ResourceManager/AsyncTaskManager.h"

ResetPasswWindow::ResetPasswWindow() 
#if defined(VER1_RENDERING)
    :
    m_CancelForm(this, OnWindowButtonClick, ButtonIds::BI_RSTPSSW_CANCEL, LocalizationRssIds::RstPassw_Cancel_Btn_Text),
    m_ButtonResetPassw(this, OnWindowButtonClick, ButtonIds::BI_RSTPSSW_RESET, LocalizationRssIds::RstPassw_Reset_Btn_Text),
    m_Success(this, OnWindowButtonClick, ButtonIds::BI_RSTPSSW_SUCCESS, LocalizationRssIds::RstPassw_Success_Btn_Text)
#endif
{
    InitTypeInfo();
    AddAsyncTask(AsyncTask_Init, this);
}

void ResetPasswWindow::AsyncTask_Init(void* params)
{
    ResetPasswWindow* self = typecheck_castL(ResetPasswWindow, params);

    // need to have images loaded
    while (sImageManager.FinishedLoadingImages() == false)
    {
        Sleep(1);
    }
#if defined(VER1_RENDERING)
    m_CancelForm.SetColor(GenericButtonColors::GBCT_TextHover, sStyles.GetUIntValue(StyleIds::Btn_Cancel_TextHover_Color));
    m_CancelForm.SetColor(GenericButtonColors::GBCT_UnderlineHover, sStyles.GetUIntValue(StyleIds::Btn_Cancel_TextHover_Color));
#endif

    self->m_ButtonResetPassw2.SetMinSize(200, 32);
    self->m_ButtonResetPassw2.SetCallback(self, OnWindowButtonClick);
    self->m_ButtonResetPassw2.SetId(ButtonIds::BI_RSTPSSW_RESET);
    self->m_ButtonResetPassw2.LoadHitMap("./Assets/Images/Login_LoginButton.png");
    self->m_ButtonResetPassw2.SetHoverImageId(sImageManager.GetImage(II_ResetPasswResetButtonHover));

    self->m_CancelForm2.SetMinSize(200, 32);
    self->m_CancelForm2.SetCallback(self, OnWindowButtonClick);
    self->m_CancelForm2.SetId(ButtonIds::BI_RSTPSSW_CANCEL);
    self->m_CancelForm2.SetHoverImageId(sImageManager.GetImage(II_ResetPasswCancelButtonHover));

    self->m_Success2.SetMinSize(200, 32);
    self->m_Success2.SetCallback(self, OnWindowButtonClick);
    self->m_Success2.SetId(ButtonIds::BI_RSTPSSW_SUCCESS);
    self->m_Success2.LoadHitMap("./Assets/Images/Login_LoginButton.png");
    self->m_Success2.SetHoverImageId(sImageManager.GetImage(II_ResetPasswSuccesButtonHover));

    self->m_UserEmail.SetEmptyValueText("Enter your email");

    self->ResetState();
}

void ResetPasswWindow::DestructorCheckMemLeaks()
{
#if defined(VER1_RENDERING)
    m_CancelForm.DestructorCheckMemLeaks();
    m_ButtonResetPassw.DestructorCheckMemLeaks();
    m_Success.DestructorCheckMemLeaks();
#endif
    m_ButtonResetPassw2.DestructorCheckMemLeaks();
    m_CancelForm2.DestructorCheckMemLeaks();
    m_Success2.DestructorCheckMemLeaks();
}

void ResetPasswWindow::ResetState()
{
    m_bDrawAsEmbeded = false;
    m_UserEmail.ResetState("");
    m_UserEmail.SetFocus();
    m_UserEmail.IsFocusedByDefault();
    m_bBadEmail = false;
    m_bUnknownError = false;
    m_bShowSuccess = false;
    m_bSetDefaultFocus = true;
}

void ResetPasswWindow::OnWindowButtonClick(GenericButton* btn, void* pParent)
{
    if (btn == NULL)
    {
        return;
    }

    ResetPasswWindow* wnd = typecheck_castL(ResetPasswWindow, pParent);
    if (btn->GetId() == ButtonIds::BI_RSTPSSW_RESET)
    {

        //valid email can't be empty string
        const char* userEmail = wnd->m_UserEmail.GetInputBuff();
        if (strlen(userEmail) == 0)
        {
            wnd->m_bBadEmail = true;
            return;
        }

        // valid email always contains at least an @
        if (strchr(userEmail, '@') == NULL)
        {
            wnd->m_bBadEmail = true;
            return;
        }

        wnd->m_bBadEmail = false;

        wnd->m_bShowSuccess = true;
        WebApi_ResetPassReq(userEmail);
    }
    else if (btn->GetId() == ButtonIds::BI_RSTPSSW_SUCCESS)
    {
        if (wnd->m_bDrawAsEmbeded)
        {
            wnd->ResetState();
        }
        else
        {
			sWindowManager.SetLoginWindowVisible(true);
        }
    }
    else if (btn->GetId() == ButtonIds::BI_RSTPSSW_CANCEL)
    {
        sWindowManager.SetLoginWindowVisible(true);
    }
}
#if defined(VER1_RENDERING)
int ResetPasswWindow::DrawWindow()
{
    int display_w, display_h;
    GetDrawAreaSize(display_w, display_h, true);

    const int inputTextboxWidth = 500;
    const float WindowWidth = 1100;
    const float WindowHeight = 450;
    float TextRowSpacing = ImGui::GetTextLineHeightWithSpacing();
    const float RaiseInputField = 3;

    bool bCanDrawContent = true;
    if (m_bDrawAsEmbeded == false)
    {
        ImGui::SetNextWindowPos(ImVec2((display_w - WindowWidth) / 2.0f, (display_h - WindowHeight) / 2.0f), ImGuiCond_Always);
        bCanDrawContent = ImGui::BeginChild(GetImGuiID("RstPssw"), ImVec2(WindowWidth, WindowHeight), false, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings);
    }

    if (bCanDrawContent)
    {
        if (m_bShowSuccess)
        {
            ImGui::Text("Success");
            ImGui::Text(" ");
            ImGui::Text("You will receive instructions if the email you");
            ImGui::Text("provided is in the system.");
            ImGui::Text(" ");
            ImGui::Text(" ");
            m_Success.DrawButton();
        }
        else
        {
            ImGui::PushFont(sFontManager.GetFont(FontIds::FI_Medium));
            ImGui::Text("Reset Password");

            ImGui::SetCursorPosY(ImGui::GetCursorPos().y + TextRowSpacing); // row spacing
            ImGui::Text("Please enter the email associated with your account.");
            ImGui::PopFont();

            {
                ImGui::SetCursorPosY(ImGui::GetCursorPos().y + TextRowSpacing); // row spacing
                ImGui::Text("Email* ");
                ImGui::SameLine();

                ImGui::SetNextItemWidth(inputTextboxWidth - ImGui::GetCursorPosX());
                ImGui::SetCursorPosY(ImGui::GetCursorPos().y - RaiseInputField); // row spacing
                if (m_bSetDefaultFocus == true && m_bDrawAsEmbeded == false)
                {
                    ImGui::SetKeyboardFocusHere();
                    m_bSetDefaultFocus = false;
                }
                ImGui::InputText(GetImGuiID2("##Username"), m_sUserEmail, _countof(m_sUserEmail));
                ImGUIUnderlinePrevItem(sStyles.GetFloatValue(StyleIds::Input_Underline_Distance),
                    sStyles.GetFloatValue(StyleIds::Input_Underline_Width),
                    sStyles.GetUIntValue(StyleIds::Input_Underline_Color));

                if (m_bBadEmail)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text,
                        sStyles.GetUIntValue(StyleIds::Wnd_Txt_Error_Color));
                    ImGui::Text("Invalid email");
                    ImGui::PopStyleColor(1);
                }
                else
                {
                    ImGui::Text(" ");
                }
            }

            m_ButtonResetPassw.DrawButton();
            if (m_bDrawAsEmbeded == false)
            {
                ImGui::SetCursorPosY(ImGui::GetCursorPos().y + TextRowSpacing); // row spacing
                m_CancelForm.DrawButton();
            }
        }
    }

    if (m_bDrawAsEmbeded == false)
    {
        ImGui::EndChild();
    }

    return WindowManagerErrorCodes::WM_NO_ERROR;
}
#else
int ResetPasswWindow::DrawWindow()
{
    int display_w, display_h;
    GetDrawAreaSize(display_w, display_h, true);

    // center to the middle
    const ImVec2 imageSize = { 1856,976 };

    // top left corner position
    const ImVec2 imagePosition = { (display_w - imageSize.x) / 2.0f,
        (display_h - imageSize.y) / 2.0f };

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    if (m_bShowSuccess)
    {
        drawList->AddImage(sImageManager.GetImage(ImageIds::II_ResetPasswSuccBackground),
            imagePosition, ImVec2(imagePosition.x + imageSize.x, imagePosition.y + imageSize.y));

        ImGui::SetCursorPos(ImVec2(imagePosition.x + 832, imagePosition.y + 642));
        m_Success2.DrawButton();
    }
    else
    {
        const int inputTextboxWidth = 390;

        drawList->AddImage(sImageManager.GetImage(ImageIds::II_ResetPasswBackground),
            imagePosition, ImVec2(imagePosition.x + imageSize.x, imagePosition.y + imageSize.y));

        ImGui::SetCursorPos(ImVec2(imagePosition.x + 716, imagePosition.y + 556));
        FORM_INVISIBLE_TXTI_FIELD(&m_UserEmail, inputTextboxWidth, 0);

        ImGui::SetCursorPos(ImVec2(imagePosition.x + 832, imagePosition.y + 634));
        m_ButtonResetPassw2.DrawButton();

        ImGui::SetCursorPos(ImVec2(imagePosition.x + 832, imagePosition.y + 680));
        m_CancelForm2.DrawButton();
    }

    return WindowManagerErrorCodes::WM_NO_ERROR;
}
#endif