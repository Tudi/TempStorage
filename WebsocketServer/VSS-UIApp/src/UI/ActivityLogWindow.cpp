#include "stdafx.h"
#include "json/yyjson.h"
#include "json/yyjson2.h"

ActivityLogWindow::ActivityLogWindow()
{
    m_SearchFilter.SetBadValText("Invalid search string");
    m_SearchFilter.SetEmptyValueText("Search");
}

void ActivityLogWindow::DestructorCheckMemLeaks()
{
    m_GridActivity.DestructorCheckMemLeaks();
}

void ActivityLogWindow::ResetState()
{
    m_GridActivity.ResetState();
}

#if defined(VER1_RENDERING)
int ActivityLogWindow::DrawWindow()
{
    int display_w, display_h;
    GetDrawAreaSize(display_w, display_h, true);
    float WindowWidth = (float)display_w;
    float WindowHeight = display_h - 50.0f;
    float TableRowSpacing = 32.0f;
    float WindowLeftBorder = 90.0f;

    if (ImGui::BeginChild(GetImGuiID("ActivityLogs"), ImVec2(WindowWidth, WindowHeight), true))
    {
        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + 20.0f); // row spacing
        ImGui::SetCursorPosX(ImGui::GetCursorPos().x + WindowLeftBorder); // move text to the right
        ImGui::PushFont(sFontManager.GetFont(FontIds::FI_Medium));
        ImGui::PushStyleColor(ImGuiCol_Text, sStyles.GetUIntValue(StyleIds::Title_Settings_Text_Color));
        ImGui::Text("My Activity Log");
        ImGui::PopStyleColor(1);

        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + TableRowSpacing); // row spacing
        ImGui::SetCursorPosX(ImGui::GetCursorPos().x + WindowLeftBorder); // move text to the right
        ImGui::Text("Below is a list the actions taken during your user sessions.");
        ImGui::PopFont();

        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + TableRowSpacing); // row spacing
        ImGui::SetCursorPosX(ImGui::GetCursorPos().x + WindowLeftBorder); // move text to the right
        m_GridActivity.DrawDataGrid();
    }
    ImGui::EndChild();
    return WindowManagerErrorCodes::WM_NO_ERROR;
}
#else
int ActivityLogWindow::DrawWindow()
{
    const int inputTextboxWidth = 220;

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImVec2 imageHeaderSize((float)sImageManager.GetImageWidth(ImageIds::II_ActivityLogHeader),
        (float)sImageManager.GetImageHeight(ImageIds::II_ActivityLogHeader));
    ImVec2 LocationsHeader_TopLeft = { 113 + 6 + 24 + 4, 133 + 0 }; // magiv values that are based on drawn navigation bar
    ImVec2 LocationsHeader_BottomRight = { LocationsHeader_TopLeft.x + imageHeaderSize.x, LocationsHeader_TopLeft.y + imageHeaderSize.y };
    drawList->AddImage(sImageManager.GetImage(ImageIds::II_ActivityLogHeader),
        LocationsHeader_TopLeft,
        LocationsHeader_BottomRight);

    ImGui::SetCursorPos(ImVec2(LocationsHeader_TopLeft.x + 236, LocationsHeader_TopLeft.y + 203));
    FORM_INVISIBLE_TXTI_FIELD(&m_SearchFilter, inputTextboxWidth, 0);
    m_GridActivity.SetFilterString(m_SearchFilter.GetInputVal());

    ImGui::SetCursorPos(ImVec2(LocationsHeader_TopLeft.x, LocationsHeader_TopLeft.y + 257));
    m_GridActivity.DrawDataGrid2();

    return WindowManagerErrorCodes::WM_NO_ERROR;
}
#endif
ActivityLogGrid::ActivityLogGrid()
{
    SetGridName("ActivityLog"); // needed for ImGUI
    SetShowBorder(false);
    SetRowsPerPage(ACTIVITY_ROW_PER_PAGE);
    SetShowHeader(true);

    const int expectedRenderedColumns = 2;
    SetSize(expectedRenderedColumns, 0);

    SetHeaderData(0, "Name");
    SetDBColName(0, "LogDetails");
    SetHeaderData(1, "TimeStamp");
    SetDBColName(1, "LogClientStamp");

    const int expectedGridStartX = 113 + 6 + 24 + 4 + 20;
    SetVisualColWidth(0, (1920 - expectedGridStartX) / expectedRenderedColumns);
    SetVisualColWidth(1, (1920 - expectedGridStartX) / expectedRenderedColumns);

    const int expectedGridStartY = 383 + 50;
    SetVisualSize(1920 - expectedGridStartX, 1080 - expectedGridStartY);
    SetCelDataSizeLimit(64);
    SetHeaderRenderingSizes(0, 0, 30);
    SetDataRowRenderingSizes(0, 0, 50);
    FillRemainingColWidths();

    SetActivePageColor(sStyles.GetUIntValue(StyleIds::Title_Settings_Text_Color));

    m_PrevValuesCRC = 0;
}

void ActivityLogGrid::ResetState()
{
    // refresh window data from server, in case something changed
    // we need to do this in order to fetch data created by other users ( imagine one user creating locations )
    WebApi_GetActivityLogsAsync(0, MAX_ACTIVITY_ROWS, ActivityLogGrid::CB_AsyncDataArived, this);
}

void ActivityLogGrid::CB_AsyncDataArived(int CurlErr, char* response, void* userData)
{
    ActivityLogGrid* grid = (ActivityLogGrid*)userData;

    // refreshing many values induces a monitor flickering.
    // If we got the same values as before, do not refresh
    uint64_t crc64Val = crc64(0, response, strlen(response));
    if (grid->m_PrevValuesCRC == crc64Val)
    {
        return;
    }
    grid->m_PrevValuesCRC = crc64Val;

    yyJSON(yydoc);
    if (ExtractDBColumnToBinary::DBH_APIResultValid(CurlErr, response, yydoc, LogSourceGroups::LogSourceActivityLogWindow, "ActivityLogGrid") != WebApiErrorCodes::WAE_NoError)
    {
        return;
    }

    const char* arrayName = "Activity";
    ExtractDBColumnToBinary extractColumns[] = {
        {grid->GetDBColName(0), (uint32_t)0, grid, LogToMsgOnly},
        {grid->GetDBColName(1), (uint32_t)1, grid, TimeStampToSortableStr},
        {NULL} };

    grid->SetLoadingState(true);
    grid->ResetData();
    extractColumns[0].SetInitFunction(InitDatagridToStoreRows);

    ExtractDBColumnToBinary::DBH_ParseDBRowFromJSON(yydoc, arrayName, extractColumns, LogSourceGroups::LogSourceActivityLogWindow);

    // ready to render the data
    grid->SetLoadingState(false);
}
