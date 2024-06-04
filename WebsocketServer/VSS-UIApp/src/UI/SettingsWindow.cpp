#include "stdafx.h"
#include "ResourceManager/AsyncTaskManager.h"

SettingsWindow::SettingsWindow()
{
    InitTypeInfo();

    AddAsyncTask(AsyncTask_Init, this);
}


void SettingsWindow::AsyncTask_Init(void* params)
{
    SettingsWindow* self = typecheck_castL(SettingsWindow, params);

    // need to have images loaded
    while (sImageManager.FinishedLoadingImages() == false)
    {
        Sleep(1);
    }

    self->ResetState();

    self->m_bViewActivity.SetMinSize(549, 696);
    self->m_bViewActivity.SetCallback(self, OnWindowButtonClick);
    self->m_bViewActivity.SetId(ButtonIds::BI_SETTINGS_MYACTIVITY);
    self->m_bViewActivity.SetHoverImageId(sImageManager.GetImage(II_SettingsActivityButtonHover));

    self->m_bViewMyAccount.SetMinSize(549, 696);
    self->m_bViewMyAccount.SetCallback(self, OnWindowButtonClick);
    self->m_bViewMyAccount.SetId(ButtonIds::BI_SETTINGS_MYACCOUNT);
    self->m_bViewMyAccount.SetHoverImageId(sImageManager.GetImage(II_SettingsAccountButtonHover));

    self->m_bViewModules.SetMinSize(549, 696);
    self->m_bViewModules.SetCallback(self, OnWindowButtonClick);
    self->m_bViewModules.SetId(ButtonIds::BI_SETTINGS_MODULES);
    self->m_bViewModules.SetHoverImageId(sImageManager.GetImage(II_SettingsModulesButtonHover));
}

void SettingsWindow::DestructorCheckMemLeaks()
{
}

void SettingsWindow::ResetState()
{
}

int SettingsWindow::DrawWindow()
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const ImVec2 imagePositionSettingsBG = { 113 + 6 + 24 + 4, 133 + 0 };
    const ImVec2 imageSizeSettingsBG = { 1681, 831 };

    drawList->AddImage(sImageManager.GetImage(ImageIds::II_SettingsWindowBackground),
        imagePositionSettingsBG, 
        ImVec2(imagePositionSettingsBG.x + imageSizeSettingsBG.x, imagePositionSettingsBG.y + imageSizeSettingsBG.y));

    ImGui::SetCursorPos(ImVec2(imagePositionSettingsBG.x + 1 + (549 + 16) * 0, imagePositionSettingsBG.y + 134));
    m_bViewActivity.DrawButton();

    ImGui::SetCursorPos(ImVec2(imagePositionSettingsBG.x + 1 + (549 + 16) * 1, imagePositionSettingsBG.y + 134));
    m_bViewMyAccount.DrawButton();

    ImGui::SetCursorPos(ImVec2(imagePositionSettingsBG.x + 1 + (549 + 16) * 2, imagePositionSettingsBG.y + 134));
    m_bViewModules.DrawButton();

	return WindowManagerErrorCodes::WM_NO_ERROR;
}

void SettingsWindow::OnWindowButtonClick(GenericButton* pBtn, void* pParent)
{
    // sanity check
    SettingsWindow* pWnd = typecheck_castL(SettingsWindow, pParent);
    pWnd;
    // should never happen
    if (pBtn == NULL)
    {
        return;
    }

    if (pBtn->GetId() == ButtonIds::BI_SETTINGS_MYACTIVITY)
    {
        ImGui::CloseCurrentPopup();
        sWindowManager.SetActivityLogWindowVisible(true);
    }
    else if (pBtn->GetId() == ButtonIds::BI_SETTINGS_MYACCOUNT)
    {
        ImGui::CloseCurrentPopup();
        sWindowManager.SetUserInfoWindowVisible(true);
    }
    else if (pBtn->GetId() == ButtonIds::BI_SETTINGS_MODULES)
    {
        ImGui::CloseCurrentPopup();
        sWindowManager.SetModulesWindowVisible(true);
    }
}
