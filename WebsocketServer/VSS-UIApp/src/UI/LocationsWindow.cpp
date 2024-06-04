#include "stdafx.h"
#include "ResourceManager/AsyncTaskManager.h"

LocationsWindow::LocationsWindow()
#if defined(VER1_RENDERING)
    :m_ButtonAddLocation(this, OnWindowButtonClick, ButtonIds::BI_LOCATIONS_ADD_NEW, LocalizationRssIds::Locations_Add_Btn_Text)
#endif
{
    m_SearchFilter.SetBadValText("Invalid search string");
    m_SearchFilter.SetEmptyValueText("Search");

    m_ButtonAddLocation.SetMinSize(200,32);
    m_ButtonAddLocation.SetCallback(this, OnWindowButtonClick);
    m_ButtonAddLocation.SetId(ButtonIds::BI_LOCATIONS_ADD_NEW);
    m_ButtonAddLocation.LoadHitMap("./Assets/Images/Login_LoginButton.png"); // same as add button

    ResetState();
}

void LocationsWindow::DestructorCheckMemLeaks()
{
    m_ButtonAddLocation.DestructorCheckMemLeaks();
    m_GridLocations.DestructorCheckMemLeaks();
}

void LocationsWindow::ResetState()
{
    m_SearchFilter.IsFocusedByDefault();
    m_SearchFilter.SetFocus();
    m_GridLocations.ResetState();
}

void LocationsGrid::ResetState()
{
    // refresh window data from server, in case something changed
    // we need to do this in order to fetch data created by other users ( imagine one user creating locations )
    WebApi_GetLocationsAsync(0, 1, LocationsGrid::CB_AsyncDataArived, this);
}

void LocationsGrid::CB_AsyncDataArived(int CurlErr, char* response, void* userData)
{
    LocationsGrid* grd = typecheck_castL(LocationsGrid, userData);

    // refreshing many values induces a monitor flickering.
    // If we got the same values as before, do not refresh
    uint64_t crc64Val = crc64(0, response, strlen(response));
    if (grd->m_PrevValuesCRC == crc64Val)
    {
        return;
    }
    grd->m_PrevValuesCRC = crc64Val;

    yyJSON(yydoc);
    if (ExtractDBColumnToBinary::DBH_APIResultValid(CurlErr, response, yydoc, LogSourceGroups::LogSourceLocationsWindow, "LocationsGrid") != WebApiErrorCodes::WAE_NoError)
    {
        return;
    }

    const char* arrayName = "Locations";
    ExtractDBColumnToBinary extractColumns[] = {
        {grd->GetDBColName(0), (uint32_t)0, grd},
        {grd->GetDBColName(1), (uint32_t)1, grd},
        {grd->GetDBColName(2), (uint32_t)2, grd},
        // col 3 does not need filling. It's the "view" button
        // col 4 does not need filling. It stores the delete button state
        {grd->GetDBColName(5), (uint32_t)5, grd}, // location ID
        {NULL} };

    grd->SetLoadingState(true);
    grd->ResetData();
    extractColumns[0].SetInitFunction(InitDatagridToStoreRows);

    ExtractDBColumnToBinary::DBH_ParseDBRowFromJSON(yydoc, arrayName, extractColumns, LogSourceGroups::LogSourceLocationsWindow);

    // ready to render the data
    grd->SetLoadingState(false);
}


void LocationsGrid::CB_AsyncLocationDeleted(int CurlErr, char* response, void* userData)
{
    LocationsGrid* grd = typecheck_castL(LocationsGrid, userData);

    yyJSON(yydoc);
    if (ExtractDBColumnToBinary::DBH_APIResultValid(CurlErr, response, yydoc, LogSourceGroups::LogSourceLocationsWindow, "LocationsGrid") != WebApiErrorCodes::WAE_NoError)
    {
        // this is an API error. We deleted the row ourself, but should have not. Refresh list
        grd->ResetState();
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceLocationsWindow, 0, 0,
            "Locations:Unexpected API result. Refreshing table content");
        return;
    }

    yyjson_val* root = yyjson_doc_get_root(yydoc);
    yyjson_val* yyActionError = yyjson_obj_get(root, "ActionError");
    if (yyActionError == NULL)
    {
        // this is an API error. We deleted the row ourself, but should have not. Refresh list
        grd->ResetState();
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceLocationsWindow, 0, 0,
            "Locations:Unexpected API result. Refreshing table content");
        return;
    }
    int ActionError = yyjson_get_int(yyActionError);
    if (ActionError != 0)
    {
        grd->ResetState();
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceLocationsWindow, 0, 0,
            "Locations:Failed to delete the row. Putting it back");
    }
}

void LocationsWindow::OnUserLoggedIn()
{
    ResetState(); // fetch window data
}

void LocationsWindow::OnWindowButtonClick(GenericButton* btn, void* pParent)
{
    pParent;
    if (btn == NULL)
    {
        return;
    }

    if (btn->GetId() == ButtonIds::BI_LOCATIONS_ADD_NEW)
    {
        sWindowManager.SetLocationEditWindowVisible(true, 0);
    }
}

#if defined(VER1_RENDERING)
int LocationsWindow::DrawWindow()
{
    int display_w, display_h;
    GetDrawAreaSize(display_w, display_h, true);
    float WindowWidth = (float)display_w;
    float WindowHeight = display_h - 50.0f;
    float TableRowSpacing = 32.0f;
    float WindowLeftBorder = 90.0f;

    if (ImGui::BeginChild(GetImGuiID("Locations"), ImVec2(WindowWidth, WindowHeight), true))
    {
        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + 20.0f); // row spacing
        ImGui::SetCursorPosX(ImGui::GetCursorPos().x + WindowLeftBorder); // move text to the right
        ImGui::PushFont(sFontManager.GetFont(FontIds::FI_Medium));
        ImGui::PushStyleColor(ImGuiCol_Text, sStyles.GetUIntValue(StyleIds::Title_Location_Text_Color));
        ImGui::Text("Locations");
        ImGui::PopStyleColor(1);

        ImGui::SameLine();
        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + 5.0f); // adjust for font size difference
        ImGui::SetCursorPosX(ImGui::GetCursorPos().x + 60.0f);
        m_ButtonAddLocation.DrawButton();

        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + TableRowSpacing); // row spacing
        ImGui::SetCursorPosX(ImGui::GetCursorPos().x + WindowLeftBorder); // move text to the right
        ImGui::Text("Below is a list of locations that are currently in the system.");
        ImGui::SetCursorPosX(ImGui::GetCursorPos().x + WindowLeftBorder); // move text to the right
        ImGui::Text("View the location for details and to view the real-time radar.");
        ImGui::PopFont();

        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + TableRowSpacing); // row spacing
        ImGui::SetCursorPosX(ImGui::GetCursorPos().x + WindowLeftBorder); // move text to the right
        m_GridLocations.DrawDataGrid();
    }
    ImGui::EndChild();
    return WindowManagerErrorCodes::WM_NO_ERROR;
}
#else
int LocationsWindow::DrawWindow()
{
    const int inputTextboxWidth = 220;

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    const ImVec2 imageHeaderSize = { 1684,257 };
    const ImVec2 LocationsHeader_TopLeft = { 113 + 32, 164 }; // magiv values that are based on drawn navigation bar
    const ImVec2 LocationsHeader_BottomRight = { LocationsHeader_TopLeft.x + imageHeaderSize.x, LocationsHeader_TopLeft.y + imageHeaderSize.y };
    drawList->AddImage(sImageManager.GetImage(ImageIds::II_LocationsHeader),
        LocationsHeader_TopLeft,
        LocationsHeader_BottomRight);

    ImGui::SetCursorPos(ImVec2(LocationsHeader_TopLeft.x + 1394, LocationsHeader_TopLeft.y + 84));
    m_ButtonAddLocation.DrawButton();

    ImGui::SetCursorPos(ImVec2(LocationsHeader_TopLeft.x + 257, LocationsHeader_TopLeft.y + 157));
    FORM_INVISIBLE_TXTI_FIELD(&m_SearchFilter, inputTextboxWidth, 0);

    m_GridLocations.SetFilterString(m_SearchFilter.GetInputVal());
    ImGui::SetCursorPos(ImVec2(LocationsHeader_TopLeft.x, LocationsHeader_TopLeft.y + 200));
    m_GridLocations.DrawDataGrid2();

    return WindowManagerErrorCodes::WM_NO_ERROR;
}
#endif

void LocationsGrid::AsyncTask_Init(void* params)
{
    LocationsGrid *self = typecheck_castL(LocationsGrid, params);

    // need to have images loaded
    while (sImageManager.FinishedLoadingImages() == false)
    {
        Sleep(1);
    }

    self->SetGridName("Locations"); // needed for ImGUI
    self->SetShowBorder(false);
    self->SetRowsPerPage(LOCATIONS_ROW_PER_PAGE);
    self->SetShowHeader(true);

    const int expectedRenderedColumns = 4;
    self->SetSize(expectedRenderedColumns, 0);

    self->SetHeaderData(0, "Name");
    self->SetDBColName(0, "LocationName");
    self->SetHeaderData(1, "City");
    self->SetDBColName(1, "LocationCity");
    self->SetHeaderData(2, "State");
    self->SetDBColName(2, "LocationState");
    self->SetHeaderData(3, ""); // the "view" button
    self->SetDBColName(3, "");

    self->SetVisualColWidth(0, 410);
    self->SetVisualColWidth(1, 410);
    self->SetVisualColWidth(2, 410);
    self->SetVisualColWidth(3, 0); //440

    const float expectedBorderX = 50;
    const float expectedGridStartY = 164 + 210;
    self->SetVisualSize(GetMainWindowDrawWidth() - expectedBorderX, GetMainWindowDrawHeight() - expectedGridStartY);
    self->SetCelDataSizeLimit(64);
    self->SetHeaderRenderingSizes(12, 10, 51);
    self->SetDataRowRenderingSizes(12, 10, 51);
    self->FillRemainingColWidths();

    // these are not rendered columns. Store data so we can fetch the data to view location
    self->SetExtraHiddenColumns(2);
    self->SetDBColName(4, "");
    self->SetDBColName(5, "LocationID");

    self->SetActivePageColor(sStyles.GetUIntValue(StyleIds::Title_Location_Text_Color));

    self->m_PrevValuesCRC = 0;

    for (int i = 0; i < _countof(m_ButtonViewLocation); i++)
    {
        char labelAndId[MAX_DB_STRING_LENGTH];
        sprintf_s(labelAndId, "View Location##%d", i);
        self->m_ButtonViewLocation[i].SetText(labelAndId);
        self->m_ButtonViewLocation[i].SetImageId(sImageManager.GetImage(ImageIds::II_LocationsGridViewButton));
        self->m_ButtonViewLocation[i].SetHoverImageId(sImageManager.GetImage(ImageIds::II_LocationsGridViewButtonHover));
        self->m_ButtonViewLocation[i].SetMinSize((float)sImageManager.GetImageWidth(ImageIds::II_LocationsGridViewButton),
                (float)sImageManager.GetImageHeight(ImageIds::II_LocationsGridViewButton));
        self->m_ButtonViewLocation[i].SetCallback(self, LocationsGrid::OnGridButtonClick);
    }

    for (int i = 0; i < _countof(m_ButtonTrashcanLocation); i++)
    {
        self->m_ButtonTrashcanLocation[i].SetText(i);
        self->m_ButtonTrashcanLocation[i].SetImageId(sImageManager.GetImage(ImageIds::II_LocationsGridTrashcanButton));
        self->m_ButtonTrashcanLocation[i].SetMinSize((float)sImageManager.GetImageWidth(ImageIds::II_LocationsGridTrashcanButton),
                (float)sImageManager.GetImageHeight(ImageIds::II_LocationsGridTrashcanButton));
        self->m_ButtonTrashcanLocation[i].SetCallback(self, LocationsGrid::OnGridButtonClick);
    }
    for (int i = 0; i < _countof(m_ButtonDeleteLocation); i++)
    {
        self->m_ButtonDeleteLocation[i].SetText(i);
        self->m_ButtonDeleteLocation[i].SetImageId(sImageManager.GetImage(ImageIds::II_LocationsGridDeleteButton));
        self->m_ButtonDeleteLocation[i].SetMinSize((float)sImageManager.GetImageWidth(ImageIds::II_LocationsGridDeleteButton),
                (float)sImageManager.GetImageHeight(ImageIds::II_LocationsGridDeleteButton));
        self->m_ButtonDeleteLocation[i].SetCallback(self, LocationsGrid::OnGridButtonClick);
    }
    for (int i = 0; i < _countof(m_ButtonCancelDeleteLocation); i++)
    {
        self->m_ButtonCancelDeleteLocation[i].SetText(i);
        self->m_ButtonCancelDeleteLocation[i].SetImageId(sImageManager.GetImage(ImageIds::II_LocationsGridCancelButton));
        self->m_ButtonCancelDeleteLocation[i].SetMinSize((float)sImageManager.GetImageWidth(ImageIds::II_LocationsGridCancelButton),
                (float)sImageManager.GetImageHeight(ImageIds::II_LocationsGridCancelButton));
        self->m_ButtonCancelDeleteLocation[i].SetCallback(self, LocationsGrid::OnGridButtonClick);
    }
}

LocationsGrid::LocationsGrid()
{
    InitTypeInfo();

    // we need images to be loaded first. Push it to a worker thread to avoid blocking main thread
    AddAsyncTask(AsyncTask_Init, this);
}

void LocationsGrid::DestructorCheckMemLeaks()
{
    GenericDataGrid::DestructorCheckMemLeaks();
    for (int i = 0; i < _countof(m_ButtonViewLocation); i++)
    {
        m_ButtonViewLocation[i].DestructorCheckMemLeaks();
    }
    for (int i = 0; i < _countof(m_ButtonTrashcanLocation); i++)
    {
        m_ButtonTrashcanLocation[i].DestructorCheckMemLeaks();
    }
    for (int i = 0; i < _countof(m_ButtonDeleteLocation); i++)
    {
        m_ButtonDeleteLocation[i].DestructorCheckMemLeaks();
    }
    for (int i = 0; i < _countof(m_ButtonCancelDeleteLocation); i++)
    {
        m_ButtonCancelDeleteLocation[i].DestructorCheckMemLeaks();
    }
}

void LocationsGrid::OnCellRender(uint32_t page, uint32_t col, uint32_t visualRow, uint32_t dataRow, const char* data)
{
    if (col != 3)
    {
        GenericDataGrid::OnCellRender(page, col, visualRow, dataRow, data);
    }
    else
    {
        int page_row = visualRow % LOCATIONS_ROW_PER_PAGE;
        ImVec2 cellStart = ImGui::GetCursorPos();
        m_ButtonViewLocation[page_row].DrawButton();
        float myColWidth = GetVisualColWidth(col);
        const char* state = GetCellData(dataRow, 4);
        if (state == NULL || state[0] == 0)
        {
            float startRenderAtX = myColWidth - m_ButtonTrashcanLocation[page_row].GetMinWidth();
            float startRenderAtY = (GetVisualRowHeight() - m_ButtonTrashcanLocation[page_row].GetMinHeight())/2;
            ImGui::SetCursorPos(ImVec2(cellStart.x - m_fRowXOffset + startRenderAtX, cellStart.y - m_fRowYOffset + startRenderAtY));
            m_ButtonTrashcanLocation[page_row].DrawButton();
        }
        else if (state == NULL || state[0] == '1')
        {
            float startRenderAtX = myColWidth - 
                m_ButtonDeleteLocation[page_row].GetMinWidth() -
                m_ButtonCancelDeleteLocation[page_row].GetMinWidth();
            float startRenderAtY = (GetVisualRowHeight() - m_ButtonDeleteLocation[page_row].GetMinHeight()) / 2;
            ImGui::SetCursorPosY(cellStart.y - m_fRowYOffset + startRenderAtY);
            ImGui::SetCursorPosX(cellStart.x - m_fRowXOffset + startRenderAtX);
            m_ButtonDeleteLocation[page_row].DrawButton();
            ImGui::SetCursorPosX(cellStart.x - m_fRowXOffset + startRenderAtX + m_ButtonDeleteLocation[page_row].GetMinWidth());
            m_ButtonCancelDeleteLocation[page_row].DrawButton();
        }
    }
}

void LocationsGrid::GetRowIndexForButton(void* pBtn, LocationsGrid* grid, size_t& out_row, size_t& out_type)
{
    out_row = 0xFFFF;
    out_type = 0xFFFF;
    for (size_t i = 0; i < _countof(grid->m_ButtonViewLocation); i++)
    {
        if (&grid->m_ButtonViewLocation[i] == pBtn)
        {
            out_row = i;
            out_type = 0;
        }
    }
    for (size_t i = 0; i < _countof(grid->m_ButtonTrashcanLocation); i++)
    {
        if (&grid->m_ButtonTrashcanLocation[i] == pBtn)
        {
            out_row = i;
            out_type = 1;
        }
    }
    for (size_t i = 0; i < _countof(grid->m_ButtonDeleteLocation); i++)
    {
        if (&grid->m_ButtonDeleteLocation[i] == pBtn)
        {
            out_row = i;
            out_type = 2;
        }
    }
    for (size_t i = 0; i < _countof(grid->m_ButtonCancelDeleteLocation); i++)
    {
        if (&grid->m_ButtonCancelDeleteLocation[i] == pBtn)
        {
            out_row = i;
            out_type = 3;
        }
    }
}

void LocationsGrid::OnGridButtonClick(GenericButton* pBtn, void* pParent)
{
    if (pBtn == NULL)
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceLocationsWindow, 0, 0,
            "Locations:Unexpected caller param. Aborting");
        return;
    }

    // the parent of this button
    LocationsGrid* grid = typecheck_castL(LocationsGrid, pParent);

    size_t rowNrOnPage, rowBtnType;
    GetRowIndexForButton(pBtn, grid, rowNrOnPage, rowBtnType);
    uint32_t rowNr = grid->GetVisualRowDataRow((uint32_t)rowNrOnPage);
    if (rowNr >= grid->m_dRows)
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceLocationsWindow, 0, 0,
            "Locations:Tryng to access row %d. Max is %d. Aborting", rowNr, grid->m_dRows);
        return;
    }
    if (grid->m_DataRows == NULL)
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceLocationsWindow, 0, 0,
            "Locations:Tryng to access uninitialized row %d. Aborting", rowNr);
        return;
    }

    // view location
    if (rowBtnType == 0)
    {
        int LocationId = atoi(grid->GetCellData(rowNr, 5));
        sWindowManager.SetLocationViewWindowVisible(true, LocationId);
    }
    // click on trashcan
    else if (rowBtnType == 1)
    {
        const char *state = grid->GetCellData(rowNr, 4);
        if (state == NULL || state[0] == 0)
        {
            grid->SetData(4, rowNr, "1");
        }
    }
    else if (rowBtnType == 2)
    {
        // actually delete the row
        grid->SetData(4, rowNr, "");
        int LocationId = atoi(grid->GetCellData(rowNr, 5));
        WebApi_DeleteLocationsAsync(LocationId, CB_AsyncLocationDeleted, grid);
        
        GridRowDeleteAsyncParams* params = (GridRowDeleteAsyncParams*)InternalMalloc(sizeof(GridRowDeleteAsyncParams));
        params->InitTypeInfo();
        params->grid = grid;
        params->row = rowNr;
        AddAsyncTask(LocationsGrid::AsyncTask_DeleteRow, params);
    }
    // click on cancel delete
    else if (rowBtnType == 3)
    {
        grid->SetData(4, rowNr, "");
    }
}
