#include "stdafx.h"

ModulesWindow::ModulesWindow()
{
    m_AddModuleButton.SetText("+ Add Module to Customer Account");
    m_AddModuleButton.SetId(ButtonIds::BI_MODULES_ADD);
    m_AddModuleButton.SetCallback(this, OnButtonClick);
}

void ModulesWindow::DestructorCheckMemLeaks()
{
    m_GridModules.DestructorCheckMemLeaks();
}

void ModulesWindow::OnUserLoggedIn()
{
    m_GridModules.RefreshData();
}

void ModulesWindow::ResetState()
{
    m_GridModules.RefreshData();
}

int ModulesWindow::DrawWindow()
{
    int display_w, display_h;
    GetDrawAreaSize(display_w, display_h, true);
    float WindowWidth = (float)display_w;
    float WindowHeight = display_h - 50.0f;
    float TableRowSpacing = 32.0f;
    float WindowLeftBorder = 90.0f;

    if (ImGui::BeginChild(GetImGuiID("Modules"), ImVec2(WindowWidth, WindowHeight), true))
    {
        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + 20.0f); // row spacing
        ImGui::SetCursorPosX(ImGui::GetCursorPos().x + WindowLeftBorder); // move text to the right
        ImGui::PushFont(sFontManager.GetFont(FontIds::FI_Medium));
        ImGui::PushStyleColor(ImGuiCol_Text, sStyles.GetUIntValue(StyleIds::Title_Settings_Text_Color));
        ImGui::Text("Modules");
        ImGui::PopStyleColor(1);

        ImGui::SameLine();
        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + 5.0f); // adjust for font size difference
        ImGui::SetCursorPosX(ImGui::GetCursorPos().x + 110.0f);
        m_AddModuleButton.DrawButton();

        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + TableRowSpacing); // row spacing
        ImGui::SetCursorPosX(ImGui::GetCursorPos().x + WindowLeftBorder); // move text to the right
        ImGui::Text("These are security modules installed in your dashboard.");
        ImGui::PopFont();

        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + TableRowSpacing); // row spacing
        ImGui::SetCursorPosX(ImGui::GetCursorPos().x + WindowLeftBorder); // move text to the right
        m_GridModules.DrawDataGrid();
    }
    ImGui::EndChild();
    return WindowManagerErrorCodes::WM_NO_ERROR;
}

void ModulesWindow::OnButtonClick(GenericButton* pBtn, void* pParent)
{
    if (pBtn == NULL || pParent == NULL)
    {
        return;
    }
    if (pBtn->GetId() == ButtonIds::BI_MODULES_ADD)
    {
        sWindowManager.SetModulesBuyWindowVisible(true);
    }
}

ModulesGrid::ModulesGrid()
{
    InitTypeInfo();
    SetGridName("Modules"); // needed for ImGUI
    SetShowBorder(false);
    SetRowsPerPage(MODULES_ROW_PER_PAGE);
    SetShowHeader(true);

    SetSize(5, 0);

    SetHeaderData(0, "Name");
    SetDBColName(0, "ModuleName");
    SetHeaderData(1, "Device Name");
    SetDBColName(1, "ModuleTag");
    SetHeaderData(2, "Location");
    SetDBColName(2, "LocationName");
    SetHeaderData(3, "Status");
    SetDBColName(3, "ModuleStatusID");
    SetHeaderData(4, ""); // this is the view location button placeholder
    SetDBColName(4, "");

    SetVisualColWidth(0, (1920 - 50) * 0.2f);
    SetVisualColWidth(1, (1920 - 50) * 0.2f);
    SetVisualColWidth(2, (1920 - 50) * 0.2f);
    SetVisualColWidth(3, (1920 - 50) * 0.2f);
    SetVisualColWidth(4, (1920 - 50) * 0.2f);

    SetVisualSize(1920 - 50, 1080 - 450);
    SetCelDataSizeLimit(64);

    SetExtraHiddenColumns(2);
    SetDBColName(5, "LocationID");
    SetDBColName(6, "ModuleDefineID");

    for (int i = 0; i < _countof(m_ButtonJumpToPage); i++)
    {
        char labelAndId[MAX_DB_STRING_LENGTH];
        sprintf_s(labelAndId, "View Location##%d", i);
        m_ButtonJumpToPage[i].SetText(labelAndId);
        m_ButtonJumpToPage[i].SetCallback(this, ModulesGrid::OnGridButtonClick);
    }

    SetActivePageColor(sStyles.GetUIntValue(StyleIds::Title_Settings_Text_Color));

    m_PrevValuesCRCModules = 0;
}

void ModulesGrid::RefreshData()
{
    WebApi_GetOrganizationModulesAsync(sUserSession.GetOrganizationId(), MAX_MODULES_SHOWN, ModulesGrid::CB_AsyncDataArivedModules, this);
}

void ModulesGrid::CB_AsyncDataArivedModules(int CurlErr, char* response, void* userData)
{
    ModulesGrid* grid = (ModulesGrid*)userData;

    // refreshing many values induces a monitor flickering.
    // If we got the same values as before, do not refresh
    uint64_t crc64Val = crc64(0, response, strlen(response));
    if (grid->m_PrevValuesCRCModules == crc64Val)
    {
        return;
    }
    grid->m_PrevValuesCRCModules = crc64Val;

    yyJSON(yydoc);
    if (ExtractDBColumnToBinary::DBH_APIResultValid(CurlErr, response, yydoc, LogSourceGroups::LogSourceModulesWindow, "ModulesGrid") != WebApiErrorCodes::WAE_NoError)
    {
        return;
    }

    const char* arrayName = "OrganizationModules";
    ExtractDBColumnToBinary extractColumns[] = {
        {grid->GetDBColName(0), (uint32_t)0, grid},
        {grid->GetDBColName(1), (uint32_t)1, grid},
        {grid->GetDBColName(2), (uint32_t)2, grid},
        {grid->GetDBColName(3), (uint32_t)3, grid, ModuleStatusIdToStr},
        // col 4 does not need filling
        {grid->GetDBColName(5), (uint32_t)5, grid},
        {grid->GetDBColName(6), (uint32_t)6, grid},
        {NULL} };
    extractColumns[0].SetInitFunction(InitDatagridToStoreRows);

    grid->SetLoadingState(true);
    grid->ResetData();

    ExtractDBColumnToBinary::DBH_ParseDBRowFromJSON(yydoc, arrayName, extractColumns, LogSourceGroups::LogSourceModulesWindow);

    // ready to render the data
    grid->SetLoadingState(false);
}

void ModulesGrid::DestructorCheckMemLeaks()
{
    GenericDataGrid::DestructorCheckMemLeaks();
    for (int i = 0; i < _countof(m_ButtonJumpToPage); i++)
    {
        m_ButtonJumpToPage[i].DestructorCheckMemLeaks();
    }
}

void ModulesGrid::OnCellRender(uint32_t page, uint32_t col, uint32_t visualRow, uint32_t dataRow, const char* data)
{
    if (col != 4)
    {
        GenericDataGrid::OnCellRender(page, col, visualRow, dataRow, data);
    }
    else
    {
        int page_row = visualRow % MODULES_ROW_PER_PAGE;
        m_ButtonJumpToPage[page_row].DrawButton();
    }
}

void ModulesGrid::OnGridButtonClick(GenericButton* pBtn, void* pParent)
{
    if (pBtn == NULL)
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceModulesWindow, 0, 0,
            "Modules:Unexpected caller param. Aborting");
        return;
    }

    // the parent of this button
    ModulesGrid* grid = typecheck_castL(ModulesGrid, pParent);

    uint32_t rowNrOnPage = INT_MAX;
    for (size_t i = 0; i < _countof(m_ButtonJumpToPage); i++)
    {
        if (&grid->m_ButtonJumpToPage[i] == pBtn)
        {
            rowNrOnPage = (uint32_t)i;
            break;
        }
    }

    if (rowNrOnPage == INT_MAX)
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceModulesWindow, 0, 0,
            "Modules:Could not find button that was clicked. Aborting");
        return;
    }

    uint32_t rowNr = rowNrOnPage + grid->m_dRowsPerPage * grid->m_dCurPage;
    if (rowNr >= grid->m_dRows)
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceModulesWindow, 0, 0,
            "Modules:Tryng to access row %d. Max is %d. Aborting", rowNr, grid->m_dRows);
        return;
    }
    if (grid->m_DataRows == NULL)
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceModulesWindow, 0, 0,
            "Modules:Tryng to access uninitialized row %d. Aborting", rowNr);
        return;
    }

    const char* sCellData = grid->GetCellData(rowNr, 5);
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
