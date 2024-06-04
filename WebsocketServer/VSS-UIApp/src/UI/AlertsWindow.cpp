#include "stdafx.h"
#include "ResourceManager/AsyncTaskManager.h"

AlertsWindow::AlertsWindow() 
{
    m_SearchFilter.SetBadValText("Invalid search string");
    m_SearchFilter.SetEmptyValueText("Search");
    m_dNextGridContentRefreshStamp = GetTickCount64();
}

void AlertsWindow::DestructorCheckMemLeaks()
{
    m_GridAlerts.DestructorCheckMemLeaks();
}

void AlertsWindow::ResetState()
{
    m_SearchFilter.IsFocusedByDefault();
    m_SearchFilter.SetFocus();
    // refresh window data from server, in case something changed
    // we need to do this in order to fetch data created by other users ( imagine one user creating locations )
    m_GridAlerts.RefreshData();
}

void AlertsWindow::OnUserLoggedIn()
{
    ResetState(); // fetch window data
}
#if defined(VER1_RENDERING)
int AlertsWindow::DrawWindow()
{
    int display_w, display_h;
    GetDrawAreaSize(display_w, display_h, true);
    float WindowWidth = (float)display_w - 15.0f;
    float WindowHeight = display_h - 55.0f;
    float TableRowSpacing = 32.0f;
    float WindowLeftBorder = 90.0f;

    if (ImGui::BeginChild(GetImGuiID("Alerts"), ImVec2(WindowWidth, WindowHeight), true))
    {
        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + 20.0f); // row spacing
        ImGui::SetCursorPosX(ImGui::GetCursorPos().x + WindowLeftBorder); // move text to the right
        ImGui::PushFont(sFontManager.GetFont(FontIds::FI_Medium));
        ImGui::PushStyleColor(ImGuiCol_Text, sStyles.GetUIntValue(StyleIds::Title_Alert_Text_Color));
        ImGui::Text("My Alerts");
        ImGui::PopStyleColor(1);

        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + TableRowSpacing); // row spacing
        ImGui::SetCursorPosX(ImGui::GetCursorPos().x + WindowLeftBorder); // move text to the right
        ImGui::Text("Below is a list of alerts from the locations you are assigned to.");
        ImGui::SetCursorPosX(ImGui::GetCursorPos().x + WindowLeftBorder); // move text to the right
        ImGui::Text("View a location to learn more and take action.");
        ImGui::PopFont();

        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + TableRowSpacing); // row spacing
        ImGui::SetCursorPosX(ImGui::GetCursorPos().x + WindowLeftBorder); // move text to the right

        if (m_dNextGridContentRefreshStamp < GetTickCount64())
        {
            m_GridAlerts.RefreshData(); // this should be a simple copy from cached data
            m_dNextGridContentRefreshStamp = GetTickCount64() + PAGE_CONTENT_REFRESH_CHECK_INTERVAL;
        }
        m_GridAlerts.DrawDataGrid();
    }
    ImGui::EndChild();
    return WindowManagerErrorCodes::WM_NO_ERROR;
}
#else
int AlertsWindow::DrawWindow()
{
    const int inputTextboxWidth = 220;

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    const ImVec2 imageHeaderSize = { 1674,257 };
    const ImVec2 LocationsHeader_TopLeft = { 113 + 32, 164 }; // magiv values that are based on drawn navigation bar
    const ImVec2 LocationsHeader_BottomRight = { LocationsHeader_TopLeft.x + imageHeaderSize.x, LocationsHeader_TopLeft.y + imageHeaderSize.y };
    drawList->AddImage(sImageManager.GetImage(ImageIds::II_AlertsHeader),
        LocationsHeader_TopLeft,
        LocationsHeader_BottomRight);

    ImGui::SetCursorPos(ImVec2(LocationsHeader_TopLeft.x + 210, LocationsHeader_TopLeft.y + 159));
    FORM_INVISIBLE_TXTI_FIELD(&m_SearchFilter, inputTextboxWidth, 0);

    m_GridAlerts.SetFilterString(m_SearchFilter.GetInputVal());
    ImGui::SetCursorPos(ImVec2(LocationsHeader_TopLeft.x, LocationsHeader_TopLeft.y + 210));
    m_GridAlerts.DrawDataGrid2();

    return WindowManagerErrorCodes::WM_NO_ERROR;
}
#endif

void AlertsGrid::AsyncTask_Init(void* params)
{
    AlertsGrid* self = typecheck_castL(AlertsGrid, params);

    // need to have images loaded
    while (sImageManager.FinishedLoadingImages() == false)
    {
        Sleep(1);
    }

    self->SetGridName("Alerts"); // needed for ImGUI
    self->SetShowBorder(false);
    self->SetRowsPerPage(ALERT_ROW_PER_PAGE);
    self->SetShowHeader(true);

    const int expectedRenderedColumns = 6;
    self->SetSize(expectedRenderedColumns, 0);

    self->SetHeaderData(0, "Name");
    self->SetHeaderData(1, "Location");
    self->SetHeaderData(2, "Date");
    self->SetHeaderData(3, "Time");
    self->SetHeaderData(4, "Status");
    self->SetHeaderData(5, "View Location"); // the "view location" button

    self->SetVisualColWidth(0, 385);
    self->SetVisualColWidth(1, 385);
    self->SetVisualColWidth(2, 335 / 2);
    self->SetVisualColWidth(3, 335 / 2);
    self->SetVisualColWidth(4, 180);
    self->SetVisualColWidth(5, 0); // 360

    const float expectedBorderX = 50;
    const float expectedGridStartY = 164 + 210;
    self->SetVisualSize(GetMainWindowDrawWidth() - expectedBorderX, GetMainWindowDrawHeight() - expectedGridStartY);

    self->SetCelDataSizeLimit(64);
    self->SetHeaderRenderingSizes(12, 10, 51);
    self->SetDataRowRenderingSizes(12, 10, 51);
    self->FillRemainingColWidths();

    // these are not rendered columns. Store data so we can fetch the data to view location
    self->SetExtraHiddenColumns(1);
    self->SetDBColName(6, "LocationId");

    for (int i = 0; i < _countof(m_ButtonViewLocation); i++)
    {
        char labelAndId[MAX_DB_STRING_LENGTH];
        sprintf_s(labelAndId, "View Location##%d", i);
        self->m_ButtonViewLocation[i].SetText(labelAndId);
        self->m_ButtonViewLocation[i].SetImageId(sImageManager.GetImage(ImageIds::II_LocationsGridViewButton));
        self->m_ButtonViewLocation[i].SetHoverImageId(sImageManager.GetImage(ImageIds::II_LocationsGridViewButtonHover));
        self->m_ButtonViewLocation[i].SetMinSize((float)sImageManager.GetImageWidth(ImageIds::II_LocationsGridViewButton),
            (float)sImageManager.GetImageHeight(ImageIds::II_LocationsGridViewButton));
        self->m_ButtonViewLocation[i].SetCallback(self, AlertsGrid::OnGridButtonClick);
    }

    self->SetActivePageColor(sStyles.GetUIntValue(StyleIds::Title_Alert_Text_Color));

    self->m_PrevValuesCRC = 0;
}

AlertsGrid::AlertsGrid()
{
    InitTypeInfo();

    // we need images to be loaded first. Push it to a worker thread to avoid blocking main thread
    AddAsyncTask(AsyncTask_Init, this);
}

void AlertsGrid::DestructorCheckMemLeaks()
{
    GenericDataGrid::DestructorCheckMemLeaks();
    for (int i = 0; i < _countof(m_ButtonViewLocation); i++)
    {
        m_ButtonViewLocation[i].DestructorCheckMemLeaks();
    }
}

void AlertsGrid::OnCellRender(uint32_t page, uint32_t col, uint32_t visualRow, uint32_t dataRow, const char* data)
{
    if(col == 5)
    {
        int page_row = visualRow % ALERT_ROW_PER_PAGE;
        m_ButtonViewLocation[page_row].DrawButton();
    }
    else if (col == 4)
    {
        ImVec2 imageHeaderSize = { 112, 22 };

        ImVec2 topLeft = ImGui::GetCursorPos(); 
        float extraYOffset = (ImGui::GetTextLineHeight() - imageHeaderSize.y) / 2;
        topLeft.y += extraYOffset;

        ImVec2 botRight = { topLeft.x + imageHeaderSize.x, topLeft.y + imageHeaderSize.y };

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        if (data[0] == 4)
        {
            drawList->AddImage(sImageManager.GetImage(ImageIds::II_AlertStatusInRange), topLeft, botRight);
        }
        else if (data[0] == 5)
        {
            drawList->AddImage(sImageManager.GetImage(ImageIds::II_AlertStatusCaution), topLeft, botRight);
        }
        else
        {
            drawList->AddImage(sImageManager.GetImage(ImageIds::II_AlertStatusDanger), topLeft, botRight);
        }
    }
    else
    {
        GenericDataGrid::OnCellRender(page, col, visualRow, dataRow, data);
    }
}

unsigned int AlertsGrid::GetRowIndexForButton(void* pBtn, AlertsGrid* grid)
{
    for (size_t i = 0; i < _countof(grid->m_ButtonViewLocation); i++)
    {
        if (&grid->m_ButtonViewLocation[i] == pBtn)
        {
            return (unsigned int)i;
        }
    }
    return 0xFFFF;
}

void AlertsGrid::OnGridButtonClick(GenericButton* pBtn, void* pParent)
{
    if (pBtn == NULL)
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceAlertsWindow, 0, 0,
            "Locations:Unexpected caller param. Aborting");
        return;
    }

    // the parent of this button
    AlertsGrid* grid = typecheck_castL(AlertsGrid, pParent);

    uint32_t rowNrOnPage = GetRowIndexForButton(pBtn, grid);
    uint32_t rowNr = grid->GetVisualRowDataRow((uint32_t)rowNrOnPage);
    if (rowNr >= grid->m_dRows)
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceAlertsWindow, 0, 0,
            "Locations:Tryng to access row %d. Max is %d. Aborting", rowNr, grid->m_dRows);
        return;
    }
    if (grid->m_DataRows == NULL)
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceAlertsWindow, 0, 0,
            "Locations:Tryng to access uninitialized row %d. Aborting", rowNr);
        return;
    }

    const char* sCellData = grid->GetCellData(rowNr, 6);
    int LocationId;
    if (sCellData != NULL)
    {
        LocationId = atoi(sCellData);
    }
    else
    {
        LocationId = 0;
    }
    sWindowManager.SetLocationViewWindowVisible(true, LocationId);
}

void AlertsGrid::RefreshData()
{
    if (sAlertsCache.GetSize() == 0)
    {
        return;
    }
    bool CheckCRC = true;
    uint64_t ValuesCRC = 0;
    for (size_t RepeatCount = 0; RepeatCount < 2; RepeatCount++)
    {
        uint32_t rowsAdded = 0;
        int32_t rowReadIndex = (int32_t)sAlertsCache.GetSize() - 1;
        const AlertHistoryData* hd;
        while (rowsAdded < MAX_ALERTS_SHOWN && rowReadIndex >= 0)
        {
            hd = sAlertsCache.GetCachedData(rowReadIndex);
            rowReadIndex--;
            if (hd == NULL)
            {
                break;
            }
            // update crc : the only 2 fields that can change over time
            if (CheckCRC == true)
            {
                ValuesCRC = crc64(ValuesCRC, &hd->alertId, sizeof(hd->alertId));
                ValuesCRC = crc64(ValuesCRC, &hd->statusFlags, sizeof(hd->statusFlags));
            }
            else
            {
//                SetData(0, rowsAdded, hd->alertName);
                SetData(0, rowsAdded, sLocalization.GetAlertTypeIdString((int)hd->alertTypeId));
                SetData(1, rowsAdded, hd->locationName);
                SetData(2, rowsAdded, hd->alertDateStr);
                SetData(3, rowsAdded, hd->alertTimeStr);
                SetData(4, rowsAdded, hd->alertTypeId);
                char locationIdAsStr[MAX_DB_STRING_LENGTH];
                sprintf_s(locationIdAsStr, "%d", (int)hd->locationId);
                SetData(6, rowsAdded, locationIdAsStr);
            }
            rowsAdded++;
        }
        // data has not changed, we will not update the grid with new data
        if (ValuesCRC == m_PrevValuesCRC)
        {
            break;
        }
        // we will update the grid
        m_PrevValuesCRC = ValuesCRC;
        SetLoadingState(true);
        ResetData();
        SetSize(0, rowsAdded);
        CheckCRC = false;
    }
    SetLoadingState(false);
}