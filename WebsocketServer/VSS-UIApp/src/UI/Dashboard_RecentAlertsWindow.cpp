#include "stdafx.h"
#include "ResourceManager/AsyncTaskManager.h"

DashboardAlertsWindow::DashboardAlertsWindow() 
{
    InitTypeInfo();
    m_dNextCachedStamp = 0;
    AddAsyncTask(AsyncTask_Init, this);
}

void DashboardAlertsWindow::AsyncTask_Init(void* params)
{
    DashboardAlertsWindow* self = typecheck_castL(DashboardAlertsWindow, params);

    // need to have images loaded
    while (sImageManager.FinishedLoadingImages() == false)
    {
        Sleep(1);
    }

    self->ResetState();

    self->m_AlertsInfoButton.SetMinSize(14, 14);
    self->m_AlertsInfoButton.LoadHitMap("./Assets/Images/Dashboard_InfoButtonHitmap.png");

    self->m_ViewMoreButton.SetMinSize(64, 18);
    self->m_ViewMoreButton.SetCallback(self, OnWindowButtonClick);
    self->m_ViewMoreButton.SetId(ButtonIds::BI_DASHBOARD_VIEWMORE_ALERTS);
    self->m_ViewMoreButton.SetImageId(sImageManager.GetImage(II_DashboardViewMoreButton));
    self->m_ViewMoreButton.SetHoverImageId(sImageManager.GetImage(II_DashboardViewMoreButtonHover));
}

void DashboardAlertsWindow::OnWindowButtonClick(GenericButton* btn, void* pParent)
{
    pParent;
    if (btn == NULL)
    {
        return;
    }

    if (btn->GetId() == ButtonIds::BI_DASHBOARD_VIEWMORE_ALERTS)
    {
        sWindowManager.SetAlertsWindowVisible(true);
    }
}

void DashboardAlertsWindow::ResetState()
{
}

void DashboardAlertsWindow::DestructorCheckMemLeaks()
{
}

void DashboardAlertsWindow::DrawSingleAlertCard(ImVec2 &imagePosition, AlertHistoryData* alert)
{
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
    }
    else
    {
        drawList->AddImage(sImageManager.GetImage(ImageIds::II_DashboardAlertCardBackground),
            imagePosition, ImVec2(imagePosition.x + imageSize.x, imagePosition.y + imageSize.y));
    }

    ImGui::SetCursorPos(ImVec2(imagePosition.x + 16, imagePosition.y + 16 + 51));
    ImGui::PushFont(sFontManager.GetFont(FontIds::FI_Medium));
    ImGui::PushStyleColor(ImGuiCol_Text, sStyles.GetUIntValue(StyleIds::Txt_DashboardMedium_Color));
    ImGui::Text(alert->alertTypeName);
    ImGui::PopStyleColor(1);
    ImGui::PopFont();

    ImGui::PushFont(sFontManager.GetFont(FontIds::FI_Normal));
    ImGui::PushStyleColor(ImGuiCol_Text, sStyles.GetUIntValue(StyleIds::Txt_DashboardNormal_Color));
    ImGui::SetCursorPos(ImVec2(imagePosition.x + 281 - 6, imagePosition.y + 16));
    ImGui::Text(alert->alertStampStrCard);
//    ImGui::SetCursorPos(ImVec2(imagePosition.x + 16, imagePosition.y + 16 + 51 + 25));
    ImGui::SetCursorPos(ImVec2(imagePosition.x + 16, imagePosition.y + 16 + 51 + 32 + 5));
    ImGui::Text(alert->alertTypeDescription);
    ImGui::PopStyleColor(1);
    ImGui::PopFont();

    ImVec2 statusPos = imagePosition;
    statusPos.x += 16 + 150;
    statusPos.y += 16 + 51 + 6;
    if (alert->alertTypeId == 4)
    {
        drawList->AddImage(sImageManager.GetImage(ImageIds::II_DashboardAlertCardStatusInRange),
            statusPos, ImVec2(statusPos.x + (float)sImageManager.GetImageWidth(ImageIds::II_DashboardAlertCardStatusInRange),
                statusPos.y + (float)sImageManager.GetImageHeight(ImageIds::II_DashboardAlertCardStatusInRange)));
    }
    else if (alert->alertTypeId == 5)
    {
        drawList->AddImage(sImageManager.GetImage(ImageIds::II_DashboardAlertCardStatusCaution),
            statusPos, ImVec2(statusPos.x + (float)sImageManager.GetImageWidth(ImageIds::II_DashboardAlertCardStatusCaution),
                statusPos.y + (float)sImageManager.GetImageHeight(ImageIds::II_DashboardAlertCardStatusCaution)));
    }
    else if (alert->alertTypeId == 6)
    {
        drawList->AddImage(sImageManager.GetImage(ImageIds::II_DashboardAlertCardStatusDanger),
            statusPos, ImVec2(statusPos.x + (float)sImageManager.GetImageWidth(ImageIds::II_DashboardAlertCardStatusDanger),
                statusPos.y + (float)sImageManager.GetImageHeight(ImageIds::II_DashboardAlertCardStatusDanger)));
    }
}

int DashboardAlertsWindow::DrawWindow()
{
#if 0
    int display_w, display_h;
    GetDrawAreaSize(display_w, display_h, true);
    float WindowWidth = display_w / 2.0f - 35.0f;
    float WindowHeight = display_h / 2.0f - 50.0f;
    float WindowCenter = WindowWidth / 2;

    if (ImGui::BeginChild(GetImGuiID("DashboardAlerts"), ImVec2(WindowWidth, WindowHeight), true))
    {
        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + 20); // row spacing
        const char* TextToShow = "Respond to Alerts";
        float TextWidth = ImGui::CalcTextSize(TextToShow).x;
        ImGui::SetCursorPosX(WindowCenter - TextWidth / 2.0f);
        ImGui::Text(TextToShow);
            
        const float ImageWidth = 225;
        ImGui::SetCursorPosX(WindowCenter - ImageWidth / 2.0f);
        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + 25); // row spacing
        ImGui::Image(sImageManager.GetImage(ImageIds::II_AlertLarge), ImVec2(ImageWidth, 225));

        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + 55); // row spacing
        m_ButtonAlerts.DrawButton();
    }

    ImGui::EndChild();
#endif

    static AlertHistoryData recentAlerts[4];
    static size_t haveAlerts = 0;
    if (m_dNextCachedStamp < GetTickCount64())
    {
        // all fixed ... because of design
        haveAlerts = 0;
        m_dNextCachedStamp = GetTickCount64() + CACHE_REFRESH_MS;
        for (size_t i = 0; i < 4; i++)
        {
            if (sAlertsCache.GetCachedData((int)i, recentAlerts[i]))
            {
                haveAlerts++;
            }
        }
    }

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddImage(sImageManager.GetImage(ImageIds::II_DashboardRecentAlertsText),
        ImVec2(113 + 32, 164), 
        ImVec2(113 + 32 + 249, 164 + 34));

    ImGui::SetCursorPos(ImVec2(113 + 32 + 249 - 14 - 2, 165));
    m_AlertsInfoButton.DrawButton();

    for (size_t i = 0; i < haveAlerts; i++)
    {
        ImVec2 imagePosition = { (float)(113 + 32 + i * (408 + 16)), 164 + 34 + 16 };
        DrawSingleAlertCard(imagePosition, &recentAlerts[i]);
    }

    ImGui::SetCursorPos(ImVec2(113 + 32 + 1680 - 68, 164));
    m_ViewMoreButton.DrawButton(false);

    return WindowManagerErrorCodes::WM_NO_ERROR;
}
