#include "stdafx.h"
#include "ResourceManager/AsyncTaskManager.h"

DashboardLocationsWindow::DashboardLocationsWindow() 
{
    InitTypeInfo();

    AddAsyncTask(AsyncTask_Init, this);
}

void DashboardLocationsWindow::ResetState()
{
}


void DashboardLocationsWindow::AsyncTask_Init(void* params)
{
    DashboardLocationsWindow* self = typecheck_castL(DashboardLocationsWindow, params);

    // need to have images loaded
    while (sImageManager.FinishedLoadingImages() == false)
    {
        Sleep(1);
    }

    self->ResetState();

    self->m_LocationsInfoButton.SetMinSize(14, 14);
    self->m_LocationsInfoButton.LoadHitMap("./Assets/Images/Dashboard_InfoButtonHitmap.png");

    self->m_ViewMoreButton.SetMinSize(64, 18);
    self->m_ViewMoreButton.SetCallback(self, OnWindowButtonClick);
    self->m_ViewMoreButton.SetId(ButtonIds::BI_DASHBOARD_VIEWMORE_LOCATIONS);
    self->m_ViewMoreButton.SetImageId(sImageManager.GetImage(II_DashboardViewMoreButton));
    self->m_ViewMoreButton.SetHoverImageId(sImageManager.GetImage(II_DashboardViewMoreButtonHover));
}

void DashboardLocationsWindow::DestructorCheckMemLeaks()
{
}

void DashboardLocationsWindow::OnWindowButtonClick(GenericButton* btn, void* pParent)
{
    pParent;
    if (btn->GetId() == ButtonIds::BI_DASHBOARD_VIEWMORE_LOCATIONS)
    {
        sWindowManager.SetLocationsWindowVisible(true);
    }
}

void DashboardLocationsWindow::DrawSingleLocationCard(ImVec2& imagePosition, int index)
{
    const LocationHystoryData* hd = sLocationRecentManager.GetHistoryData(index);
    if (hd == NULL || hd->id == 0)
    {
        return;
    }

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    static ImVec2 imageSize = { 0,0 };
    if (imageSize.x == 0)
    {
        imageSize.x = (float)sImageManager.GetImageWidth(ImageIds::II_DashboardAlertCardBackground);
        imageSize.y = (float)sImageManager.GetImageHeight(ImageIds::II_DashboardAlertCardBackground);
    }

    if (isMouseHoveredInArea_Size(imagePosition, imageSize))
    {
        drawList->AddImage(sImageManager.GetImage(ImageIds::II_DashboardAlertCardBackgroundHover),
            imagePosition, ImVec2(imagePosition.x + imageSize.x, imagePosition.y + imageSize.y));
        if (isMouseClickedInArea_Size(imagePosition, imageSize))
        {
            sWindowManager.SetLocationViewWindowVisible(true, hd->id);
        }
    }
    else
    {
        drawList->AddImage(sImageManager.GetImage(ImageIds::II_DashboardAlertCardBackground),
            imagePosition, ImVec2(imagePosition.x + imageSize.x, imagePosition.y + imageSize.y));
    }

    ImGui::SetCursorPos(ImVec2(imagePosition.x + 16, imagePosition.y + 16 + 51));
    ImGui::PushFont(sFontManager.GetFont(FontIds::FI_Medium));
    ImGui::PushStyleColor(ImGuiCol_Text, sStyles.GetUIntValue(StyleIds::Txt_DashboardMedium_Color));
    ImGui::Text(hd->name);
    ImGui::PopStyleColor(1);
    ImGui::PopFont();

    ImGui::PushFont(sFontManager.GetFont(FontIds::FI_Normal));
    ImGui::PushStyleColor(ImGuiCol_Text, sStyles.GetUIntValue(StyleIds::Txt_DashboardNormal_Color));
    ImGui::SetCursorPos(ImVec2(imagePosition.x + 16, imagePosition.y + 16 + 51 + 32 + 5));
    ImGui::Text(hd->description);
    ImGui::PopStyleColor(1);
    ImGui::PopFont();
}

int DashboardLocationsWindow::DrawWindow()
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddImage(sImageManager.GetImage(ImageIds::II_DashboardRecentLocationsText),
        ImVec2(113 + 32, 164 + 219), 
        ImVec2(113 + 32 + 298, 164 + 219 + 34));

    ImGui::SetCursorPos(ImVec2(113 + 32 + 298 - 14, 164 + 219 + 1));
    m_LocationsInfoButton.DrawButton();

    for (size_t i = 0; i < SPLASH_RECENTS_LOCATIONS; i++)
    {
        ImVec2 imagePosition = { (float)(113 + 32 + i * (408 + 16)), 164 + 219 + 34 + 16 };
        DrawSingleLocationCard(imagePosition, (int)i);
    }

    ImGui::SetCursorPos(ImVec2(113 + 32 + 1680 - 68, 164 + 219));
    m_ViewMoreButton.DrawButton(false);

	return WindowManagerErrorCodes::WM_NO_ERROR;
}
