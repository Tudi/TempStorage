#include "stdafx.h"

LocationViewWindow::LocationViewWindow() :
	m_ButtonEdit(this, OnWindowButtonClick, ButtonIds::BI_LOCATIONVIEW_EDIT, LocalizationRssIds::LocationView_Edit_Btn_Text)
{
	InitTypeInfo();

	m_sName[0] = 0;
	m_sDescription[0] = 0;
	m_sAddress1[0] = 0;
	m_sAddress2[0] = 0;
	m_sCity[0] = 0;
	m_sState[0] = 0;
	m_sCountryRegion[0] = 0;
	m_sCountry[0] = 0;
	m_dId = 0;
	m_sDescriptionMultiRow[0][0] = 0;
	m_sDescriptionMultiRow[1][0] = 0;
	m_StampStartedView = 0;
	m_Modules.SetCallback(this, CB_OnModulesDropdownChanged);
	m_nViewedModuleId = 0;
}

void LocationViewWindow::SetLocationData(const char* name, const char* desc, const char* addr1, const char* addr2,
	const char* city, const char* state, const char* countryRegion, const char* country)
{
	if (name != NULL)
	{
		strcpy_s(m_sName, name);
	}
	else
	{
		m_sName[0] = 0;
	}
	if (desc != NULL)
	{
		strcpy_s(m_sDescription, desc);
	}
	else
	{
		m_sDescription[0] = 0;
	}
	if (addr1 != NULL)
	{
		strcpy_s(m_sAddress1, addr1);
	}
	else
	{
		m_sAddress1[0] = 0;
	}
	if (addr2 != NULL)
	{
		strcpy_s(m_sAddress2, addr2);
	}
	else
	{
		m_sAddress2[0] = 0;
	}
	if (city != NULL)
	{
		strcpy_s(m_sCity, city);
	}
	else
	{
		m_sCity[0] = 0;
	}
	if (state != NULL)
	{
		strcpy_s(m_sState, state);
	}
	else
	{
		m_sState[0] = 0;
	}
	if (countryRegion != NULL)
	{
		strcpy_s(m_sCountryRegion, countryRegion);
	}
	else
	{
		m_sCountryRegion[0] = 0;
	}
	if (country != NULL)
	{
		strcpy_s(m_sCountry, country);
	}
	else
	{
		m_sCountry[0] = 0;
	}
	UpdateMultiRowDescription();
}

void LocationViewWindow::UpdateMultiRowDescription()
{

	// make sure description text is clipped to window width and max 256 chars
	memset(m_sDescriptionMultiRow, 0, sizeof(m_sDescriptionMultiRow));
	int charsFromSrc = 0;
	int charsInDstLine = 0;
	int dstLine = 0;
	float LocationDetailsWindowWidth = 900 - 280 - 50 - 50; // left + right border
	while (m_sDescription[charsFromSrc] != 0 && charsFromSrc < 256)
	{
		m_sDescriptionMultiRow[dstLine][charsInDstLine] = m_sDescription[charsFromSrc];
		if (ImGui::CalcTextSize(m_sDescriptionMultiRow[dstLine]).x >= LocationDetailsWindowWidth)
		{
			// delete last char that is too much to draw
			m_sDescriptionMultiRow[dstLine][charsInDstLine] = 0;
			// max 2 lines of text
			if (dstLine == 1)
			{
				break;
			}
			dstLine++;
			charsInDstLine = 0;
		}
		else
		{
			charsInDstLine++;
			charsFromSrc++;
		}
	}
}

void LocationViewWindow::ResetState(int id)
{
	m_dId = id;
	// refresh location data from DB
	if (id != 0)
	{
		WebApi_GetLocationsAsync(id, 2, LocationViewWindow::CB_AsyncDataArived, this);
		m_Alerts.SetLocationId(id);
		m_StampStartedView = GetTickCount64();
		m_DoplerModule.ResetState();
		// TODO : remove these later
		SetViewedModuleID(1);
		m_DoplerModule.SetModuleId(1); // temp code. Should use real module ID. Right now there is only 1 module
	}
}

void LocationViewWindow::OnWindowClosed()
{
	if (m_StampStartedView != 0)
	{
		uint64_t endStamp = GetTickCount64();
		uint64_t duration = endStamp - m_StampStartedView;
		if (duration > REPORT_LOCATIONVIEW_ACTIVITY_IF_LONGER)
		{
			AddLogEntry(LogDestinationFlags::LDF_SERVER, LogSeverityValue::LogSeverityNormal,
				LogSourceGroups::LogSourceLocationViewWindow, 0, 0, "Finished viewing location %s after %llu minutes",
				m_sName, duration / 60000);
		}
		m_StampStartedView = 0;
	}

	// TODO : remove these later
	SetViewedModuleID(0);
	m_DoplerModule.SetModuleId(0); // temp code. Should use real module ID. Right now there is only 1 module
}

bool LocationViewWindow::CB_OnDBRowExtractFinished_Modules(int rowIndex, ExtractDBColumnToBinary* rowColDataArr)
{
	DropdownEntry* ahd = (DropdownEntry*)rowColDataArr[0].cbDRF_userData1;
	GenericDropdown* dd = (GenericDropdown*)rowColDataArr[0].cbDRF_userData2;

	dd->SetEntryData(rowIndex, ahd->Text, (uint32_t)ahd->CallbackId);

	return true;
}

void LocationViewWindow::CB_AsyncDataArived(int CurlErr, char* response, void* userData)
{
	yyJSON(yydoc);
	if (ExtractDBColumnToBinary::DBH_APIResultValid(CurlErr, response, yydoc, LogSourceGroups::LogSourceLocationViewWindow, "") != WebApiErrorCodes::WAE_NoError)
	{
		return;
	}

	LocationViewWindow* wnd = (LocationViewWindow*)userData;

	{
		const char* arrayName = "Locations";
		ExtractDBColumnToBinary extractColumns[] = {
			{"LocationName", wnd->m_sName, sizeof(wnd->m_sName)},
			{"LocationDescription", wnd->m_sDescription, sizeof(wnd->m_sDescription)},
			{"LocationAddressLine1", wnd->m_sAddress1, sizeof(wnd->m_sAddress1)},
			{"LocationAddressLine2", wnd->m_sAddress2, sizeof(wnd->m_sAddress2)},
			{"LocationCity", wnd->m_sCity, sizeof(wnd->m_sCity)},
			{"LocationState", wnd->m_sState, sizeof(wnd->m_sState)},
			{"LocationCountyOrRegion", wnd->m_sCountryRegion, sizeof(wnd->m_sCountryRegion)},
			{"LocationCountry", wnd->m_sCountry, sizeof(wnd->m_sCountry)},
			{NULL} };

		ExtractDBColumnToBinary::DBH_ParseDBRowFromJSON(yydoc, arrayName, extractColumns, LogSourceGroups::LogSourceLocationViewWindow);
		wnd->UpdateMultiRowDescription();
	}

	{
		const char* arrayName = "LocationModules";
		DropdownEntry de;
		ExtractDBColumnToBinary extractColumns[] = {
			{"ModuleName", de.Text, sizeof(de.Text)},
			{"ModuleDefineID", &de.CallbackId},
			{NULL} };
		extractColumns[0].SetDestinationObj(&wnd->m_Modules);
		extractColumns[0].SetDataRowFinishedFunc(CB_OnDBRowExtractFinished_Modules, &de, &wnd->m_Modules);
		extractColumns[0].SetInitFunction(InitDropdownToStoreRows);

		ExtractDBColumnToBinary::DBH_ParseDBRowFromJSON(yydoc, arrayName, extractColumns, LogSourceGroups::LogSourceLocationViewWindow);
	}

	sLocationRecentManager.OnLocationView(wnd->m_sName, wnd->m_sDescription, wnd->m_dId);
}

void LocationViewWindow::DestructorCheckMemLeaks()
{
	m_ButtonEdit.DestructorCheckMemLeaks();
	m_Alerts.DestructorCheckMemLeaks();
	m_Modules.DestructorCheckMemLeaks();
	m_DoplerModule.DestructorCheckMemLeaks();
}

void LocationViewWindow::OnWindowButtonClick(GenericButton* btn, void* pParent)
{
	pParent;
	if (btn == NULL)
	{
		return;
	}

	if (btn->GetId() == ButtonIds::BI_LOCATIONVIEW_EDIT)
	{
		LocationViewWindow* wnd = typecheck_castL(LocationViewWindow, pParent);
		sWindowManager.SetLocationEditWindowVisible(true, wnd->m_dId);
		LocationEditWindow* wndEdit = sWindowManager.GetLocationEditWindow();
		wndEdit->SetLocationData(wnd->m_sName, wnd->m_sDescription, wnd->m_sAddress1, wnd->m_sAddress2,
			wnd->m_sCity, wnd->m_sState, wnd->m_sCountryRegion, wnd->m_sCountry);
	}
}

int LocationViewWindow::DrawWindow()
{
	int display_w, display_h;
	GetDrawAreaSize(display_w, display_h, true);
	float WindowWidth = (float)display_w;
	float WindowHeight = display_h - 80.0f;
	float WindowLeftBorder = 20.0f;
	const float LeftBorder = 50;
	const float TopBorder = 100;
	float TextRowSpacing = 5;

	if (ImGui::BeginChild(GetImGuiID("LocationView"), ImVec2(WindowWidth, WindowHeight), true))
	{
		m_DoplerModule.DrawWindow();

		ImGui::PushStyleColor(ImGuiCol_ChildBg, sStyles.GetUIntValue(StyleIds::Wnd_LocationViewOverlay_Bg_Color)); // half transparent window

		ImGui::SetNextWindowPos(ImVec2(LeftBorder, TopBorder), ImGuiCond_Always);
		if (ImGui::BeginChild(GetImGuiID("LocationViewDetails"), ImVec2(750, 290), false))
		{
			ImGui::SetCursorPosY(ImGui::GetCursorPos().y + 20.0f); // row spacing
			ImGui::SetCursorPosX(ImGui::GetCursorPos().x + WindowLeftBorder); // move text to the right
			ImGui::PushStyleColor(ImGuiCol_Text, sStyles.GetUIntValue(StyleIds::Title_Location_Text_Color));
			ImGui::Text("Location Details: ");
			ImGui::SameLine();
			ImGui::Text(m_sName);
			ImGui::PopStyleColor(1);
			ImGui::SetCursorPosY(ImGui::GetCursorPos().y + TextRowSpacing); // row spacing

			ImGui::SetCursorPosX(ImGui::GetCursorPos().x + WindowLeftBorder); // move text to the right
			ImGui::Text(m_sDescriptionMultiRow[0]);
			ImGui::SetCursorPosX(ImGui::GetCursorPos().x + WindowLeftBorder); // move text to the right
			ImGui::Text(m_sDescriptionMultiRow[1]);
			ImGui::SetCursorPosY(ImGui::GetCursorPos().y + TextRowSpacing); // row spacing

			ImGui::Columns(2, "LocationDetails2", false);
			ImGui::SetColumnWidth(0, 500);
			ImGui::SetColumnWidth(1, 300);

			ImGui::SetCursorPosY(ImGui::GetCursorPos().y + TextRowSpacing); // row spacing
			ImGui::SetCursorPosX(ImGui::GetCursorPos().x + WindowLeftBorder); // move text to the right
			ImGui::Text(m_sAddress1);
			ImGui::NextColumn();

			ImGui::SetCursorPosY(ImGui::GetCursorPos().y + TextRowSpacing); // row spacing
			ImGui::Text(m_sCountryRegion);
			ImGui::NextColumn();

			ImGui::SetCursorPosX(ImGui::GetCursorPos().x + WindowLeftBorder); // move text to the right
			ImGui::SetCursorPosY(ImGui::GetCursorPos().y + TextRowSpacing); // row spacing
			ImGui::Text(m_sAddress2);
			ImGui::NextColumn();
			ImGui::SetCursorPosY(ImGui::GetCursorPos().y + TextRowSpacing); // row spacing
			ImGui::Text(m_sCountry);
			ImGui::NextColumn();

			ImGui::SetCursorPosX(ImGui::GetCursorPos().x + WindowLeftBorder); // move text to the right
			ImGui::SetCursorPosY(ImGui::GetCursorPos().y + TextRowSpacing); // row spacing
			ImGui::Text(m_sCity);
			ImGui::NextColumn();
//			m_Modules.DrawDropdown(); // Disabled until reviwed by designer
			ImGui::NextColumn();

			ImGui::SetCursorPosX(ImGui::GetCursorPos().x + WindowLeftBorder); // move text to the right
			ImGui::SetCursorPosY(ImGui::GetCursorPos().y + TextRowSpacing); // row spacing
			ImGui::Text(m_sState);
			ImGui::NextColumn();
			ImGui::SetCursorPosY(ImGui::GetCursorPos().y + TextRowSpacing); // row spacing
			m_ButtonEdit.DrawButton();
			ImGui::NextColumn();

			ImGui::Columns(1);
		}
		ImGui::EndChild();

		ImGui::SetCursorPosX(0);
		ImGui::SetCursorPosY(WindowHeight - m_Alerts.GetVisualHeight() - 10.0f);
		m_Alerts.DrawDataGrid();
		m_Alerts.PeriodicCheckUpdate();

		ImGui::PopStyleColor(1); // colors
	}
	ImGui::EndChild();
	return WindowManagerErrorCodes::WM_NO_ERROR;
}

AlertsLocationGrid::AlertsLocationGrid()
{
	m_dLocationId = 0;
	m_PrevValuesCRC = 0;
	m_PrevUpdateStamp = 0;

	SetGridName("Alerts"); // needed for ImGUI
	SetShowBorder(false);
	SetRowsPerPage(ALERT_ROW_PER_PAGE);
	SetShowHeader(true);

	SetSize(3, 0);

	SetHeaderData(0, "Alerts");
	SetHeaderData(1, "Timestamp");
	SetHeaderData(2, "Status");

	SetVisualColWidth(0, (1920 - 50) * 0.6f);
	SetVisualColWidth(1, (1920 - 50) * 0.3f);
	SetVisualColWidth(2, (1920 - 50) * 0.1f);

	SetVisualSize(1980 - 50,
		20.0f + (ImGui::GetTextLineHeight() + m_dRowDistance) * (ALERTS_FOR_LOCATION + 1) + 20.0f);

	SetCelDataSizeLimit(64);

	SetShowPaginationButtons(false); // will use scrollbar
}

void AlertsLocationGrid::SetLocationId(int newId)
{
	if (m_dLocationId != newId)
	{
		m_dLocationId = newId;
		PeriodicCheckUpdate(true);
	}
}

void AlertsLocationGrid::PeriodicCheckUpdate(bool bForced)
{
	const ULONGLONG tickNow = GetTickCount64();
	if (bForced == true || tickNow > m_PrevUpdateStamp)
	{
		m_PrevUpdateStamp = tickNow + LOCATION_ALERT_UPDATE_INTERVAL;
		bool CheckCRC = true;
		uint64_t ValuesCRC = 0;
		int64_t lastIndex = sAlertsCache.GetSize() - 1;
		for (size_t RepeatCount = 0; RepeatCount < 2; RepeatCount++)
		{
			uint32_t rowsAdded = 0;
			int64_t rowReadIndex = 0;
			const AlertHistoryData* hd;
			while (rowsAdded < ALERTS_FOR_LOCATION)
			{
				hd = sAlertsCache.GetCachedData((int)(lastIndex - rowReadIndex));
				rowReadIndex++;
				if (hd == NULL)
				{
					break;
				}
				// we want to filter specific rows
				if (m_dLocationId != 0 && hd->locationId != m_dLocationId)
				{
					continue;
				}
				// update crc : the only 2 fields that can change over time
				if (CheckCRC == true)
				{
					ValuesCRC = crc64(ValuesCRC, &hd->alertId, sizeof(hd->alertId)); // detect new alerts
					ValuesCRC = crc64(ValuesCRC, &hd->statusFlags, sizeof(hd->statusFlags)); // detect if status changed
				}
				else
				{
//					SetData(0, rowsAdded, hd->alertName);
					SetData(0, rowsAdded, sLocalization.GetAlertTypeIdString((int)hd->alertTypeId));
					SetData(1, rowsAdded, hd->alertStampStr);
					SetData(2, rowsAdded, hd->alertStatusStr);
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
	}
	SetLoadingState(false);
}

void AlertsLocationGrid::OnCellRender(uint32_t page, uint32_t col, uint32_t visualRow, uint32_t dataRow, const char* data)
{
	if (col == 0)
	{
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 30.0f);
	}
	if (col != 2)
	{
		GenericDataGrid::OnCellRender(page, col, visualRow, dataRow, data);
	}
	else
	{
		if (data[0] != 'U') // user confirmed. Todo : add proper id check
		{
			ImGui::PushStyleColor(ImGuiCol_Text, sStyles.GetUIntValue(StyleIds::LocationView_StatusActive_Text_Color));
			ImGui::Text("Active");
			ImGui::PopStyleColor(1);
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_Text, sStyles.GetUIntValue(StyleIds::LocationView_StatusResolved_Text_Color));
			ImGui::Text("Resolved");
			ImGui::PopStyleColor(1);
		}
	}
}

void AlertsLocationGrid::OnHeaderCellRender(uint32_t page, uint32_t col, const char* data)
{
	if (col == 0)
	{
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 30.0f);
	}
	ImGui::PushStyleColor(ImGuiCol_Text, sStyles.GetUIntValue(StyleIds::Title_Alert_Text_Color));
	GenericDataGrid::OnHeaderCellRender(page, col, data);
	ImGui::PopStyleColor();
}

void LocationViewWindow::CB_OnModulesDropdownChanged(GenericDropdown* pBtn, void* pParent)
{
	LocationViewWindow* wnd = typecheck_castL(LocationViewWindow, pParent);
	const DropdownEntry *de = pBtn->GetSelectedEntry();
	if (de == NULL)
	{
		return;
	}

	wnd->SetViewedModuleID((int)de->CallbackId);
}

void LocationViewWindow::SetViewedModuleID(int newVal)
{ 
	if (m_nViewedModuleId != 0)
	{
		sDataSourceManager.UnSubscribeModuleFeed(m_nViewedModuleId);
	}
	m_nViewedModuleId = newVal; 
	if (m_nViewedModuleId != 0)
	{
		sDataSourceManager.SubscribeModuleFeed(m_nViewedModuleId);
	}
	m_DoplerModule.SetModuleId(m_nViewedModuleId);
}

void LocationViewWindow::OnModulePositionDataArrived(__int64 ModuleId, unsigned __int64 Timestamp, __int64 ObjectId, float x, float y)
{
	if (m_nViewedModuleId != ModuleId)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected,
			LogSourceGroups::LogSourceLocationViewWindow, 0, 0, 
			"LocationViewWindow:Module %lld data arived, but we are watching %lld", ModuleId, m_nViewedModuleId);
		return;
	}
	m_DoplerModule.OnModulePositionDataArrived(ModuleId, Timestamp, ObjectId, x, y);
}
