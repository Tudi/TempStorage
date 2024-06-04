#include "stdafx.h"
#include "ResourceManager/AsyncTaskManager.h"

DashboardActionsWindow::DashboardActionsWindow()
{
    InitTypeInfo();

    AddAsyncTask(AsyncTask_Init, this);
}

void DashboardActionsWindow::ResetState()
{
}

void DashboardActionsWindow::AsyncTask_Init(void* params)
{
    DashboardActionsWindow* self = typecheck_castL(DashboardActionsWindow, params);

    // need to have images loaded
    while (sImageManager.FinishedLoadingImages() == false)
    {
        Sleep(1);
    }

    self->ResetState();

    self->m_ActionsButton.SetMinSize(550, 320);
    self->m_ActionsButton.SetCallback(self, OnWindowButtonClick);
    self->m_ActionsButton.SetId(ButtonIds::BI_DASHBOARD_VIEWMORE_ALERTS);
    self->m_ActionsButton.SetHoverImageId(sImageManager.GetImage(II_DashboardActionAlertCardBackgroundHover));

    self->m_LocationsButton.SetMinSize(550, 320);
    self->m_LocationsButton.SetCallback(self, OnWindowButtonClick);
    self->m_LocationsButton.SetId(ButtonIds::BI_DASHBOARD_VIEWMORE_LOCATIONS);
    self->m_LocationsButton.SetHoverImageId(sImageManager.GetImage(II_DashboardActionLocationCardBackgroundHover));

    self->m_ModulesButton.SetMinSize(550, 320);
    self->m_ModulesButton.SetCallback(self, OnWindowButtonClick);
    self->m_ModulesButton.SetId(ButtonIds::BI_DASHBOARD_VIEWMORE_MODULES);
    self->m_ModulesButton.SetHoverImageId(sImageManager.GetImage(II_DashboardActionModuleCardBackgroundHover));
}

void DashboardActionsWindow::DestructorCheckMemLeaks()
{
}

void DashboardActionsWindow::OnWindowButtonClick(GenericButton* btn, void* pParent)
{
    pParent;
    if (btn->GetId() == ButtonIds::BI_DASHBOARD_VIEWMORE_ALERTS)
    {
        sWindowManager.SetAlertsWindowVisible(true);
    }
    else if (btn->GetId() == ButtonIds::BI_DASHBOARD_VIEWMORE_LOCATIONS)
    {
        sWindowManager.SetLocationsWindowVisible(true);
    }
    else if (btn->GetId() == ButtonIds::BI_DASHBOARD_VIEWMORE_MODULES)
    {
        sWindowManager.SetModulesWindowVisible(true);
    }
}

int DashboardActionsWindow::DrawWindow()
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddImage(sImageManager.GetImage(ImageIds::II_DashboardActionsText),
        ImVec2(113 + 32, 164 + 219 + 219),
        ImVec2(113 + 32 + 1680, 164 + 219 + 219 + 381));

    ImGui::SetCursorPos(ImVec2(113 + 31.5 + (549 + 16) * 0, 164 + 219 + 219 + 58));
    m_ActionsButton.DrawButton();

    ImGui::SetCursorPos(ImVec2(113 + 31.5 + (549 + 16) * 1, 164 + 219 + 219 + 58));
    m_LocationsButton.DrawButton();

    ImGui::SetCursorPos(ImVec2(113 + 31.5 + (549 + 16) * 2, 164 + 219 + 219 + 58));
    m_ModulesButton.DrawButton();

    return WindowManagerErrorCodes::WM_NO_ERROR;
}
