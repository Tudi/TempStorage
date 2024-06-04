#include "stdafx.h"

SplashSessionWindow::SplashSessionWindow() :
    m_ButtonLocation(this, OnWindowButtonClick, ButtonIds::BI_SPLASHSESSION_LOCATION, LocalizationRssIds::WelcomeSession_Location_Btn_Text)
{
    m_ButtonLocation.SetDrawCentered(true);
}

void SplashSessionWindow::ResetState()
{
}

void SplashSessionWindow::OnWindowButtonClick(GenericButton* btn, void* pParent)
{
    btn; pParent;
    if (btn->GetId() == ButtonIds::BI_SPLASHSESSION_LOCATION)
    {
        int idStore = sLocationRecentManager.GetLastSessionLocationId();
        sWindowManager.SetLocationViewWindowVisible(true, idStore);
    }
}

void SplashSessionWindow::DestructorCheckMemLeaks()
{
    m_ButtonLocation.DestructorCheckMemLeaks();
}

int SplashSessionWindow::DrawWindow()
{
    int display_w, display_h;
    GetDrawAreaSize(display_w, display_h, true);
    float WindowWidth = display_w / 2.0f;
    float WindowHeight = display_h / 2.0f - 50;
    float WindowCenter = WindowWidth / 2;

    if (ImGui::BeginChild(GetImGuiID("SplashSessions"), ImVec2(WindowWidth, WindowHeight), true))
    {
        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + 50); // row spacing

        const char* TextToShow = "Last Sesssion";
        float TextWidth = ImGui::CalcTextSize(TextToShow).x;
        ImGui::SetCursorPosX(WindowCenter - TextWidth / 2.0f);
        ImGui::Text(TextToShow);

        const float ImageWidth = 225;
        ImGui::SetCursorPosX(WindowCenter - ImageWidth / 2.0f);
        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + 25); // row spacing
        ImGui::Image(sImageManager.GetImage(ImageIds::II_ResumeLarge), ImVec2(ImageWidth, 225));

        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + 35); // row spacing
        m_ButtonLocation.DrawButton();
    }

    ImGui::EndChild();

	return WindowManagerErrorCodes::WM_NO_ERROR;
}