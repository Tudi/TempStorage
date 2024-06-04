#include "stdafx.h"

ModulesBuyWindow::ModulesBuyWindow()
{
}

void ModulesBuyWindow::DestructorCheckMemLeaks()
{
    m_GridModules.DestructorCheckMemLeaks();
}

void ModulesBuyWindow::OnUserLoggedIn()
{
    m_GridModules.RefreshData();
}

void ModulesBuyWindow::ResetState()
{
    m_GridModules.RefreshData();
}

void ModulesBuyWindow::RefreshDataModuleBuy()
{
    m_GridModules.RefreshDataModuleBuy();
}

void ModulesBuyWindow::RefreshInstanceLocation()
{
    m_GridModules.RefreshInstanceLocation();
}

int ModulesBuyWindow::DrawWindow()
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

        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + TableRowSpacing); // row spacing
        ImGui::SetCursorPosX(ImGui::GetCursorPos().x + WindowLeftBorder); // move text to the right
        ImGui::Text("Add a security module to this customer account, then assign it to a location.");
        ImGui::PopFont();

        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + TableRowSpacing); // row spacing
        ImGui::SetCursorPosX(ImGui::GetCursorPos().x + WindowLeftBorder); // move text to the right
        m_GridModules.DrawDataGrid();
    }
    ImGui::EndChild();
    return WindowManagerErrorCodes::WM_NO_ERROR;
}

ModulesBuyGrid::ModulesBuyGrid()
{
    InitTypeInfo();

    SetGridName("Modules"); // needed for ImGUI
    SetShowBorder(false);
    SetRowsPerPage(MODULES_ROW_PER_PAGE);
    SetShowHeader(true);

    SetSize(5, 0);

    SetHeaderData(0, "Module Name");
    SetDBColName(0, "ModuleName");
    SetHeaderData(1, "Developer");
    SetDBColName(1, "ModuleDeveloper");
    SetHeaderData(2, "Location");
    SetDBColName(2, ""); // this is a dropdown and we will generate the names later
    SetHeaderData(3, "Status");
    SetDBColName(3, "");
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
    SetDBColName(6, "ModuleInstanceID");

    for (int i = 0; i < _countof(m_ButtonBuyModule); i++)
    {
        char labelAndId[MAX_DB_STRING_LENGTH];
        sprintf_s(labelAndId, "##%d", i);
        m_ButtonBuyModule[i].SetText(labelAndId);
        m_ButtonBuyModule[i].SetCallback(this, ModulesBuyGrid::OnGridButtonClick);
    }

    for (int i = 0; i < _countof(m_ddSelectLocation); i++)
    {
        char labelAndId[MAX_DB_STRING_LENGTH];
        sprintf_s(labelAndId, "Select Location##%d", i);
        m_ddSelectLocation[i].SetCallback(this, ModulesBuyGrid::OnGridDropdownClick);
        m_ddSelectLocation[i].SetVisualSize(250.0f, 0.0f);
    }

    m_PrevValuesCRCOrganizationModules = 0;
    m_PrevValuesCRCModuleInstances = 0;
    m_PrevValuesCRCLocations = 0;

    m_bLocationsDataChangedSinceReinit = true;
    m_bOrganizationModuleDataChangedSinceReinit = true;
    m_bModuleInstanceDataChangedSinceReinit = true;
}

void ModulesBuyGrid::RefreshData()
{
    m_bLocationDataArrived = m_bOrganizationDataArrived = m_bModuleInstanceDataArrived = false;
    m_bLocationsDataChangedSinceReinit = m_bOrganizationModuleDataChangedSinceReinit = m_bModuleInstanceDataChangedSinceReinit = true;
    // refresh window data from server, in case something changed
    // we need to do this in order to fetch data created by other users ( imagine one user creating locations )
    WebApi_GetLocationsAsync(0, 3, ModulesBuyGrid::CB_AsyncDataArivedLocations, this);
    WebApi_GetOrganizationModulesAsync(sUserSession.GetOrganizationId(), 65535, ModulesBuyGrid::CB_AsyncDataArivedOrganizationModules, this);
    WebApi_GetModuleInstancesAsync(MAX_MODULES_SHOWN, ModulesBuyGrid::CB_AsyncDataArivedModuleInstances, this);
}

void ModulesBuyGrid::RefreshDataModuleBuy()
{
    m_bLocationDataArrived = m_bModuleInstanceDataArrived = true;
    m_bOrganizationDataArrived = false;
    m_bLocationsDataChangedSinceReinit = m_bModuleInstanceDataChangedSinceReinit = false;
    m_bOrganizationModuleDataChangedSinceReinit = true;
    // refresh window data from server, in case something changed
    // we need to do this in order to fetch data created by other users ( imagine one user creating locations )
    WebApi_GetOrganizationModulesAsync(sUserSession.GetOrganizationId(), 65535, ModulesBuyGrid::CB_AsyncDataArivedOrganizationModules, this);
}

void ModulesBuyGrid::RefreshInstanceLocation()
{
    m_bLocationDataArrived = m_bOrganizationDataArrived = true;
    m_bModuleInstanceDataArrived = false;
    m_bLocationsDataChangedSinceReinit = m_bOrganizationModuleDataChangedSinceReinit = false;
    m_bModuleInstanceDataChangedSinceReinit = true;
    // refresh window data from server, in case something changed
    // we need to do this in order to fetch data created by other users ( imagine one user creating locations )
    WebApi_GetModuleInstancesAsync(MAX_MODULES_SHOWN, ModulesBuyGrid::CB_AsyncDataArivedModuleInstances, this);
}

bool CB_OrganizationModuleRowDone(int rowIndex, ExtractDBColumnToBinary* rowColDataArr)
{
    rowIndex;
    std::set<int32_t>* out_set = (std::set<int32_t>*)rowColDataArr->cbDRF_userData1;
    uint64_t *in_data = (uint64_t*)rowColDataArr->cbDRF_userData2;
    out_set->insert((int32_t)*in_data);
    return true;
}

void ModulesBuyGrid::CB_AsyncDataArivedOrganizationModules(int CurlErr, char* response, void* userData)
{
    ModulesBuyGrid* grid = (ModulesBuyGrid*)userData;

    // refreshing many values induces a monitor flickering.
    // If we got the same values as before, do not refresh
    uint64_t crc64Val = crc64(0, response, strlen(response));
    if (grid->m_PrevValuesCRCOrganizationModules == crc64Val)
    {
        grid->m_bOrganizationDataArrived = true;
        grid->m_bOrganizationModuleDataChangedSinceReinit = false;
        grid->ReinitDropdownActiveSelections();
        return;
    }
    grid->m_PrevValuesCRCOrganizationModules = crc64Val;

    yyJSON(yydoc);
    if (ExtractDBColumnToBinary::DBH_APIResultValid(CurlErr, response, yydoc, LogSourceGroups::LogSourceModulesBuyWindow, "ModulesBuyGrid") != WebApiErrorCodes::WAE_NoError)
    {
        grid->m_bOrganizationDataArrived = true;
        grid->m_bOrganizationModuleDataChangedSinceReinit = false;
        grid->ReinitDropdownActiveSelections();
        return;
    }

    // flush old data
    grid->m_OrganizationModules.clear();

    const char* arrayName = "OrganizationModules";
    uint64_t tempColStore;
    ExtractDBColumnToBinary extractColumns[] = {
        { "ModuleInstanceID", &tempColStore},
                {NULL} };

    extractColumns[0].SetDataRowFinishedFunc(CB_OrganizationModuleRowDone, &grid->m_OrganizationModules, &tempColStore);

    ExtractDBColumnToBinary::DBH_ParseDBRowFromJSON(yydoc, arrayName, extractColumns, LogSourceGroups::LogSourceModulesBuyWindow);

    grid->m_bOrganizationDataArrived = true;
    grid->ReinitDropdownActiveSelections();
}

void ModulesBuyGrid::CB_AsyncDataArivedModuleInstances(int CurlErr, char* response, void* userData)
{
    ModulesBuyGrid* grid = (ModulesBuyGrid*)userData;

    // refreshing many values induces a monitor flickering.
    // If we got the same values as before, do not refresh
    uint64_t crc64Val = crc64(0, response, strlen(response));
    if (grid->m_PrevValuesCRCModuleInstances != crc64Val)
    {
        grid->m_PrevValuesCRCModuleInstances = crc64Val;
        grid->m_bModuleInstanceDataChangedSinceReinit = true;

        yyJSON(yydoc);
        if (ExtractDBColumnToBinary::DBH_APIResultValid(CurlErr, response, yydoc, LogSourceGroups::LogSourceModulesBuyWindow, "ModulesBuyGrid") != WebApiErrorCodes::WAE_NoError)
        {
            grid->m_bModuleInstanceDataArrived = true;
            grid->ReinitDropdownActiveSelections();
            return;
        }

        const char* arrayName = "ModuleInstances";
        ExtractDBColumnToBinary extractColumns[] = {
            {grid->GetDBColName(0), (uint32_t)0, grid}, // Module Name
            {grid->GetDBColName(1), (uint32_t)1, grid}, // Developer Name
            // col 4 does not need filling
            {grid->GetDBColName(5), (uint32_t)5, grid}, // LocationId
            {grid->GetDBColName(6), (uint32_t)6, grid}, // ModuleInstanceId
            {NULL} };
        extractColumns[0].SetInitFunction(InitDatagridToStoreRows);

        grid->SetLoadingState(true);
        grid->ResetData();

        ExtractDBColumnToBinary::DBH_ParseDBRowFromJSON(yydoc, arrayName, extractColumns, LogSourceGroups::LogSourceModulesBuyWindow);
    }
    else
    {
        grid->m_bModuleInstanceDataChangedSinceReinit = false;
    }

    grid->m_bModuleInstanceDataArrived = true;
    grid->ReinitDropdownActiveSelections();
}

void ModulesBuyGrid::ReinitDropdownActiveSelections()
{
    // wait for backend data to arrive
    if (m_bLocationDataArrived == false ||
        m_bOrganizationDataArrived == false ||
        m_bModuleInstanceDataArrived == false)
    {
        return;
    }

    // no need to reinit
    if (m_bLocationsDataChangedSinceReinit == false &&
        m_bOrganizationModuleDataChangedSinceReinit == false &&
        m_bModuleInstanceDataChangedSinceReinit == false)
    {
        return;
    }
    m_bLocationsDataChangedSinceReinit = false;
    m_bOrganizationModuleDataChangedSinceReinit = false;
    m_bModuleInstanceDataChangedSinceReinit = false;


    // update button texts and action
    for (size_t row = 0; row < GetRows() && row < _countof(m_ButtonBuyModule); row++)
    {
        const char* val = GetCellData((uint32_t)row, 6);
        int32_t moduleInstanceId = atoi(val);

        // check if this module instance is added to the current organization
        bool isBought = m_OrganizationModules.find(moduleInstanceId) != m_OrganizationModules.end();
        if (isBought) // status column is "Added"
        {
            SetData(3, (uint32_t)row, "Added");
            char labelAndId[MAX_DB_STRING_LENGTH];
            sprintf_s(labelAndId, "Remove Module##%llu", row);
            m_ButtonBuyModule[row].SetText(labelAndId);
        }
        else
        {
            SetData(3, (uint32_t)row, "   -");
            char labelAndId[MAX_DB_STRING_LENGTH];
            sprintf_s(labelAndId, "Add Module##%llu", row);
            m_ButtonBuyModule[row].SetText(labelAndId);
        }
    }

    // ready to render the data
    SetLoadingState(false);

    // reinit all dropdown selected values
    for (size_t i = 0; i < _countof(m_ddSelectLocation) && i < m_dRows; i++)
    {
        const char* szLocationId = GetCellData((uint32_t)i, 5);
        if (szLocationId == NULL || szLocationId[0] == 0)
        {
            continue;
        }
        int nLocationId = atoi(szLocationId);
        m_ddSelectLocation[i].SetSelectedRowFind(nLocationId);
    }
}

void ModulesBuyGrid::CB_InitDatagridToStoreLocations(size_t rowCount, ExtractDBColumnToBinary* colDef)
{
    ModulesBuyGrid* grid = (ModulesBuyGrid*)colDef->customDestinationObj;
    for (int i = 0; i < _countof(m_ddSelectLocation); i++)
    {
        grid->m_ddSelectLocation[i].SetSize((uint32_t)rowCount);
    }
}

bool ModulesBuyGrid::CB_OnLocationRowExtracted(int rowIndex, ExtractDBColumnToBinary* rowColDataArr)
{
    ModulesBuyGrid* grid = (ModulesBuyGrid*)rowColDataArr->customDestinationObj;
    DropdownEntry* de = (DropdownEntry*)rowColDataArr->cbDRF_userData1;
    if (de == NULL)
    {
        return false;
    }
    // on every row of the grid, we have the same dropdown, the only difference is the selected row
    for (int i = 0; i < _countof(m_ddSelectLocation); i++)
    {
        grid->m_ddSelectLocation[i].SetEntryData(rowIndex, de->Text, (uint32_t)de->CallbackId);
    }
    return true;
}

void ModulesBuyGrid::CB_AsyncDataArivedLocations(int CurlErr, char* response, void* userData)
{
    ModulesBuyGrid* grid = (ModulesBuyGrid*)userData;

    // refreshing many values induces a monitor flickering.
    // If we got the same values as before, do not refresh
    uint64_t crc64Val = crc64(0, response, strlen(response));
    if (grid->m_PrevValuesCRCLocations == crc64Val)
    {
        grid->m_bLocationDataArrived = true;
        grid->m_bLocationsDataChangedSinceReinit = false;
        grid->ReinitDropdownActiveSelections();
        return;
    }
    grid->m_PrevValuesCRCLocations = crc64Val;
    grid->m_bLocationsDataChangedSinceReinit = true;

    yyJSON(yydoc);
    if (ExtractDBColumnToBinary::DBH_APIResultValid(CurlErr, response, yydoc, LogSourceGroups::LogSourceModulesBuyWindow, "ModulesBuyGrid") != WebApiErrorCodes::WAE_NoError)
    {
        grid->m_bLocationDataArrived = true;
        grid->m_bLocationsDataChangedSinceReinit = false;
        grid->ReinitDropdownActiveSelections();
        return;
    }

    const char* arrayName = "Locations";
    DropdownEntry de;
    ExtractDBColumnToBinary extractColumns[] = {
        {"LocationName", de.Text, sizeof(de.Text)},
        {"LocationID", &de.CallbackId },
        {NULL} };
    extractColumns[0].SetDestinationObj(grid);
    extractColumns[0].SetInitFunction(CB_InitDatagridToStoreLocations);
    extractColumns[0].SetDataRowFinishedFunc(CB_OnLocationRowExtracted, &de, grid);

    ExtractDBColumnToBinary::DBH_ParseDBRowFromJSON(yydoc, arrayName, extractColumns, LogSourceGroups::LogSourceModulesBuyWindow);

    // wait for modules data to be fetched and do the refresh once instead twice
    grid->m_bLocationDataArrived = true;
    grid->ReinitDropdownActiveSelections();
}

void ModulesBuyGrid::DestructorCheckMemLeaks()
{
    GenericDataGrid::DestructorCheckMemLeaks();
    for (int i = 0; i < _countof(m_ButtonBuyModule); i++)
    {
        m_ButtonBuyModule[i].DestructorCheckMemLeaks();
    }
    for (int i = 0; i < _countof(m_ddSelectLocation); i++)
    {
        m_ddSelectLocation[i].DestructorCheckMemLeaks();
    }
}

void ModulesBuyGrid::OnCellRender(uint32_t page, uint32_t col, uint32_t visualRow, uint32_t dataRow, const char* data)
{
    if (col == 2)
    {
        int page_row = visualRow % MODULES_ROW_PER_PAGE;
        m_ddSelectLocation[page_row].DrawDropdown();
    }
    else if (col == 4)
    {
        int page_row = visualRow % MODULES_ROW_PER_PAGE;
        m_ButtonBuyModule[page_row].DrawButton();
    }
    else
    {
        GenericDataGrid::OnCellRender(page, col, visualRow, dataRow, data);
    }
}

void CB_ModuleAddDeleteReply(int CurlErr, char* response, void* userData)
{
    CurlErr; response; userData;
    // refresh data. Based on insert/create result, the data will be different
    sWindowManager.GetModuleBuyWindow()->RefreshDataModuleBuy();
}

void ModulesBuyGrid::OnGridButtonClick(GenericButton* pBtn, void* pParent)
{
    if (pBtn == NULL)
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceModulesBuyWindow, 0, 0,
            "ModulesBuy:Unexpected caller param. Aborting");
        return;
    }

    // the parent of this button
    ModulesBuyGrid* grid = (ModulesBuyGrid*)pParent;

    uint32_t rowNrOnPage = INT_MAX;
    for (size_t i = 0; i < _countof(m_ButtonBuyModule); i++)
    {
        if (&grid->m_ButtonBuyModule[i] == pBtn)
        {
            rowNrOnPage = (uint32_t)i;
            break;
        }
    }

    if (rowNrOnPage == INT_MAX)
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceModulesBuyWindow, 0, 0,
            "ModulesBuy:Could not find button that was clicked. Aborting");
        return;
    }

    uint32_t rowNr = rowNrOnPage + grid->m_dRowsPerPage * grid->m_dCurPage;
    if (rowNr >= grid->m_dRows)
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceModulesBuyWindow, 0, 0,
            "ModulesBuy:Tryng to access row %d. Max is %d. Aborting", rowNr, grid->m_dRows);
        return;
    }
    if (grid->m_DataRows == NULL)
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceModulesBuyWindow, 0, 0,
            "ModulesBuy:Tryng to access uninitialized row %d. Aborting", rowNr);
        return;
    }

    const char* sCellData;
    
    int LocationId;
    const DropdownEntry *de = grid->m_ddSelectLocation[rowNrOnPage].GetSelectedEntry();
    if (de != NULL)
    {
        LocationId = (int)de->CallbackId;
    }
    else
    {
        LocationId = 0;
    }

    sCellData = grid->GetCellData(rowNr, 6);
    int ModuleInstanceId;
    if (sCellData != NULL)
    {
        ModuleInstanceId = atoi(sCellData);
    }
    else
    {
        ModuleInstanceId = 0;
    }

    int OrganizationId = sUserSession.GetOrganizationId();

    bool isBought = grid->m_OrganizationModules.find(ModuleInstanceId) != grid->m_OrganizationModules.end();
    if (isBought == false)
    {
        WebApi_CreateOrgModuleAsync(OrganizationId, ModuleInstanceId, CB_ModuleAddDeleteReply, NULL);
        AddLogEntry(LogDestinationFlags::LDF_SERVER, LogSeverityValue::LogSeverityNormal,
            LogSourceGroups::LogSourceModulesBuyWindow, 0, 0, "New module has been added to the organization");
    }
    else
    {
        WebApi_DeleteOrgModuleAsync(OrganizationId, ModuleInstanceId, CB_ModuleAddDeleteReply, NULL);
        AddLogEntry(LogDestinationFlags::LDF_SERVER, LogSeverityValue::LogSeverityNormal,
            LogSourceGroups::LogSourceModulesBuyWindow, 0, 0, "Module has been removed from the organization");
    }
}

void ModulesBuyGrid::OnGridDropdownClick(GenericDropdown* pBtn, void* pParent)
{
    if (pBtn == NULL)
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceModulesBuyWindow, 0, 0,
            "ModulesBuy:Unexpected caller param. Aborting");
        return;
    }

    const DropdownEntry* de = pBtn->GetSelectedEntry();

    if (de == NULL)
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceModulesBuyWindow, 0, 0,
            "ModulesBuy:Dropdown selection is NULL. Aborting");
        return;
    }

    // the parent of this button
    ModulesBuyGrid* grid = typecheck_castL(ModulesBuyGrid, pParent);

    uint32_t rowNrOnPage = INT_MAX;
    for (size_t i = 0; i < _countof(m_ddSelectLocation); i++)
    {
        if (&grid->m_ddSelectLocation[i] == pBtn)
        {
            rowNrOnPage = (uint32_t)i;
            break;
        }
    }

    if (rowNrOnPage == INT_MAX)
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceModulesBuyWindow, 0, 0,
            "ModulesBuy:Could not find button that was clicked. Aborting");
        return;
    }

    uint32_t rowNr = rowNrOnPage + grid->m_dRowsPerPage * grid->m_dCurPage;
    if (rowNr >= grid->m_dRows)
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceModulesBuyWindow, 0, 0,
            "ModulesBuy:Tryng to access row %d. Max is %d. Aborting", rowNr, grid->m_dRows);
        return;
    }
    if (grid->m_DataRows == NULL)
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceModulesBuyWindow, 0, 0,
            "ModulesBuy:Tryng to access uninitialized row %d. Aborting", rowNr);
        return;
    }

    const char* sCellData = grid->GetCellData(rowNr, 6);
    int ModuleId;
    if (sCellData != NULL)
    {
        ModuleId = atoi(sCellData);
    }
    else
    {
        ModuleId = 0;
    }

    // update the selected LocationId for this Module
    int newLocationId = (int)pBtn->GetSelectedEntry()->CallbackId;
    WebApi_SetModuleLocationAsync(ModuleId, newLocationId, NULL, NULL);
}