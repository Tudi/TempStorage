#include "stdafx.h"

SplashModulesWindow::SplashModulesWindow() :
    m_ButtonModules(this, OnWindowButtonClick, ButtonIds::BI_SPLASHMODULES_MODULE, LocalizationRssIds::WelcomeModule_Add_Btn_Text)
{
    m_ButtonModules.SetDrawCentered(true);
}

void SplashModulesWindow::ResetState()
{
}

void SplashModulesWindow::OnWindowButtonClick(GenericButton* btn, void* pParent)
{
    btn; pParent;
    if (btn->GetId() == ButtonIds::BI_SPLASHMODULES_MODULE)
    {
        sWindowManager.SetModulesWindowVisible(true);
    }
}

void SplashModulesWindow::DestructorCheckMemLeaks()
{
    m_ButtonModules.DestructorCheckMemLeaks();
}

int SplashModulesWindow::DrawWindow()
{
    int display_w, display_h;
    GetDrawAreaSize(display_w, display_h, true);
    float WindowWidth = display_w / 2.0f - 35.0f;
    float WindowHeight = display_h / 2.0f - 50.0f;
    float WindowCenter = WindowWidth / 2;

    if (ImGui::BeginChild(GetImGuiID("SplashModules"), ImVec2(WindowWidth, WindowHeight), true))
    {
        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + 50); // row spacing
        const char* TextToShow = "Explore Security Modules";
        float TextWidth = ImGui::CalcTextSize(TextToShow).x;
        ImGui::SetCursorPosX(WindowCenter - TextWidth / 2.0f);
        ImGui::Text(TextToShow);

        const float ImageWidth = 225;
        ImGui::SetCursorPosX(WindowCenter - ImageWidth / 2.0f);
        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + 25); // row spacing
        ImGui::Image(sImageManager.GetImage(ImageIds::II_AddModuleLarge), ImVec2(ImageWidth, 225));

        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + 35); // row spacing
        m_ButtonModules.DrawButton();
    }

    ImGui::EndChild();

	return WindowManagerErrorCodes::WM_NO_ERROR;
}
