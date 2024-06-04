#include "stdafx.h"

void DashboardWelcomeWindow::ResetState()
{
	m_SplashLocationsWindow.ResetState();
	m_SplashAlertWindow.ResetState();
	m_SplashActionsWindow.ResetState();
}

void DashboardWelcomeWindow::DestructorCheckMemLeaks()
{
	m_SplashLocationsWindow.DestructorCheckMemLeaks();
	m_SplashAlertWindow.DestructorCheckMemLeaks();
	m_SplashActionsWindow.DestructorCheckMemLeaks();
}

#if defined(VER1_RENDERING)
int DashboardWelcomeWindow::DrawWindow()
{
	int display_w, display_h;
	GetDrawAreaSize(display_w, display_h, true);

	if (ImGui::BeginChild(GetImGuiID("DashboardWelcome"), ImVec2(0, 0), true, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar))
	{
		m_SplashRecentWindow.DrawWindow();
		ImGui::SameLine();
		m_SplashAlertWindow.DrawWindow();

		m_SplashSessionWindow.DrawWindow();
		ImGui::SameLine();
		m_SplashModulesWindow.DrawWindow();
	}

	ImGui::EndChild();

	return WindowManagerErrorCodes::WM_NO_ERROR;
}
#else
int DashboardWelcomeWindow::DrawWindow()
{
	m_SplashAlertWindow.DrawWindow();
	m_SplashLocationsWindow.DrawWindow();
	m_SplashActionsWindow.DrawWindow();
	return WindowManagerErrorCodes::WM_NO_ERROR;
}
#endif