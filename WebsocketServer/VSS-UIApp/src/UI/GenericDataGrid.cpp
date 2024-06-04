#include "stdafx.h"

static_assert(BI_DATAGRID_PAGINATION_START + PAGES_JUMP_LARGE * 2 + 1 < BI_DATAGRID_PAGINATION_END);

int GetViewablePageCount(int dataRows, int RowsPerPage)
{
	int mod = dataRows % RowsPerPage;
	int pages = dataRows / RowsPerPage;
	if (mod != 0)
	{
		pages++;
	}
	return pages;
}

// struct means it does not have a constructor + can be initialized in bulk + does not have a destructor + can be copied directly
typedef struct DataGridCell
{
	void DestroyData()
	{
		InternalFree(this->m_sData);
		InternalFree(this->m_sDataLimited);
	}
	void Set(const char* newVal, const size_t dataSize, uint32_t dataLimit)
	{
		if (dataSize + 1 > this->m_allocatedSize)
		{
			InternalFree(this->m_sData);
			InternalFree(this->m_sDataLimited);

			// alloc buffer to store the value
			this->m_sData = (char*)InternalMalloc(dataSize + 1);
			if (this->m_sData == NULL)
			{
				return;
			}
			this->m_allocatedSize = dataSize + 1;
		}

		// duplicate the data as value
		memcpy(this->m_sData, newVal, dataSize);

		// this is for strings only. For the rest make sure you pass data limit large enough
		if (dataSize > dataLimit)
		{
			// backup char we are going to overwrite
			char t = this->m_sData[dataLimit];
			// temp make the full string terminate early
			this->m_sData[dataLimit] = 0;
			// duplicate it
			this->m_sDataLimited = InternalStrDup(this->m_sData);
			// restore temp deleted char
			this->m_sData[dataLimit] = t;
		}
		// we are not using limited string
		else
		{
			this->m_sDataLimited = NULL;
		}
	}
	char* GetFull() { return this->m_sData; }
	char* GetVisual() 
	{ 
		// visual data matches real data ? Might need to complicate this logic later
		if (this->m_sDataLimited == NULL)
		{
			return this->m_sData;
		}
		return this->m_sDataLimited; 
	}
	char* m_sData;
	size_t m_allocatedSize;
	char* m_sDataLimited;
}DataGridCell;

class DataGridRow
{
public:
	DataGridRow()
	{
//		m_bIsHidden = m_bIsColapsed = m_bIsSelected = false;
//		m_dRow = 0;
		m_Cells = NULL;
		m_dCols = m_dColsHiddenData = 0;
	}
	void Init(int row, int cols, int hiddenCols)
	{
		row;
//		m_dRow = row;
		m_dCols = cols;
		m_dColsHiddenData = hiddenCols;
		m_Cells = (DataGridCell*)InternalMalloc(sizeof(DataGridCell) * (cols + hiddenCols));
		memset(m_Cells, 0, sizeof(DataGridCell) * (cols + hiddenCols));
	}
	~DataGridRow()
	{
		for (int i = 0; i < m_dCols + m_dColsHiddenData; i++)
		{
			m_Cells[i].DestroyData();
		}
		InternalFree(m_Cells);
	}
	inline DataGridCell* GetCell(int col) 
	{ 
#ifdef _DEBUG
		if (col >= (m_dCols + m_dColsHiddenData))
		{
			AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceDataGrids, 0, 0,
				"DataGrid:Trying to fetch GridData column %d but max is %d.", col, m_dCols);
			return NULL;
		}
#endif
		return &m_Cells[col]; 
	}
	// used by delete row
	void SwapWith(DataGridRow* other)
	{
		DataGridCell* tCells = other->m_Cells;
		other->m_Cells = m_Cells;
		m_Cells = tCells;

		int tval = other->m_dCols;
		other->m_dCols = m_dCols;
		m_dCols = tval;

		tval = other->m_dColsHiddenData;
		other->m_dColsHiddenData = m_dColsHiddenData;
		m_dColsHiddenData = tval;
	}
private:
//	bool m_bIsHidden;
//	bool m_bIsColapsed;
//	bool m_bIsSelected;
	DataGridCell* m_Cells;
	int m_dCols;
//	int m_dRow; // my initial row number in an array of rows. Might need this as unique row ID
	int m_dColsHiddenData;
};

GenericDataGrid::GenericDataGrid()
{
	InitTypeInfo();
	strcpy_s(m_sGridName, "");
	m_bShowBroder = false;
	m_fWidthPixels = 0;
	m_fHeightPixels = 0;
	m_dRows = 0;
	m_dCols = 0;
	m_dColsHiddenData = 0;
	m_dRowDistance = 15;
	m_dRowsPerPage = 15;
	m_dCurPage = 0;
	m_dPaginationFirstPage = 0;
	m_DataRows = NULL;
	m_bShowHeader = false;
	m_bShowFooter = false;
	m_dSortByCol = MAX_GRIDDATA_COLUMNS;
	m_dSortOrder = DataGridSortTypes::NotSorted;
	m_bHasColumnWidthsSet = false;
	memset(m_fColWidths, 0, sizeof(m_fColWidths));
	m_bShowFilterRow = false;
	m_uCellDataLimit = MAX_GRID_STRING_LEN;
	memset(m_cHideColumn, 0, sizeof(m_cHideColumn));
	m_bIsLoading = false;

	m_ButtonPrevPrev.SetTextLocalized(LocalizationRssIds::DataGrid_PrevPrev_Btn_Text);
	m_ButtonPrevPrev.SetCallback(this, OnPaginationButtonClick);
	m_ButtonPrevPrev.SetId(ButtonIds::BI_DATAGRID_PREVPREV);
	m_ButtonPrevPrev.ToggleUnderline(false);
	m_ButtonPrevPrev.SetMinSize(30, 30);
	m_ButtonPrev.SetTextLocalized(LocalizationRssIds::DataGrid_Prev_Btn_Text);
	m_ButtonPrev.SetCallback(this, OnPaginationButtonClick);
	m_ButtonPrev.SetId(ButtonIds::BI_DATAGRID_PREV);
	m_ButtonPrev.ToggleUnderline(false);
	m_ButtonPrev.SetMinSize(30, 30);
	m_ButtonNext.SetTextLocalized(LocalizationRssIds::DataGrid_Next_Btn_Text);
	m_ButtonNext.SetCallback(this, OnPaginationButtonClick);
	m_ButtonNext.SetId(ButtonIds::BI_DATAGRID_NEXT);
	m_ButtonNext.ToggleUnderline(false);
	m_ButtonNext.SetMinSize(30, 30);
	m_ButtonNextNext.SetTextLocalized(LocalizationRssIds::DataGrid_NextNext_Btn_Text);
	m_ButtonNextNext.SetCallback(this, OnPaginationButtonClick);
	m_ButtonNextNext.SetId(ButtonIds::BI_DATAGRID_NEXTNEXT);
	m_ButtonNextNext.ToggleUnderline(false);
	m_ButtonNextNext.SetMinSize(30, 30);

//	m_ButtonJumpToPage[PAGES_JUMP_LARGE / 2].SetDisabled(true);

	// reset button texts : 1 2 3 4
	for (int i = 0; i < _countof(m_ButtonJumpToPage); i++)
	{
		int pageIndex = i + m_dPaginationFirstPage + 1;
		m_ButtonJumpToPage[i].SetText(pageIndex);
		m_ButtonJumpToPage[i].SetCallback(this, OnPaginationButtonClick);
		m_ButtonJumpToPage[i].SetId((ButtonIds)(ButtonIds::BI_DATAGRID_PAGINATION_START + i));
		m_ButtonJumpToPage[i].ToggleUnderline(false);
		m_ButtonJumpToPage[i].SetMinSize(30, 30);
	}

	m_bEnablePagination = true;

	m_uColorActivePage = 0;
	InternalNew(m_Filter, GenericDataGrid::GenericDataGridFilter, this);

	m_fHeaderYOffset = m_fHeaderXOffset = m_fHeaderHeight = 0.0f;
	m_fRowYOffset = m_fRowXOffset = m_fRowHeight = 0.0f;

	m_bIsRendering = false;
}

void GenericDataGrid::DestructorCheckMemLeaks()
{
	if (m_DataRows)
	{
		InternalDeleteA(m_DataRows);
	}
	if (m_Filter)
	{
		InternalDelete(m_Filter);
	}
	m_dRows = 0;
	m_dCols = 0;
	m_dColsHiddenData = 0;
}

GenericDataGrid::~GenericDataGrid()
{
	DestructorCheckMemLeaks();
}

void GenericDataGrid::ResetData()
{
	m_dRows = 0;
	m_dCurPage = 0;
	m_dPaginationFirstPage = 0;
	if (m_DataRows)
	{
		InternalDeleteA(m_DataRows);
	}

//	OnActivePageChanged(); // update pagination button texts
}

void GenericDataGrid::DrawDataGrid()
{
	if (ImGui::BeginChild(m_sGridName, ImVec2(m_fWidthPixels, m_fHeightPixels), m_bShowBroder))
	{
		ImGui::Columns(m_dCols, m_sGridName, m_bShowBroder);

		// do we have resolution specific column widths ?
		if (m_bHasColumnWidthsSet == true)
		{
			for (uint32_t tcol = 0; tcol < m_dCols; tcol++)
			{
				if (m_fColWidths[tcol] != 0)
				{
					ImGui::SetColumnWidth(tcol, m_fColWidths[tcol]);
				}
			}
		}

		// if desired, render the header
		if (m_bShowHeader)
		{
			for (uint32_t col = 0; col < m_dCols; col++)
			{
				ImGui::TableSetColumnIndex(col);
				if (ImGui::IsItemClicked()) 
				{
					OnHeaderCellClick(col);
				}
				ImGui::SetCursorPosY(ImGui::GetCursorPos().y + m_dRowDistance); // row spacing
				OnHeaderCellRender(m_dCurPage, col, m_HeaderStrings[col].c_str());
				ImGui::TableNextColumn();
				ImGui::NextColumn();
			}
		}

		// in case we wish to filter data shown
		if(m_bShowFilterRow)
		{
		}

		if (m_bIsLoading == false && m_DataRows != NULL)
		{
			std::vector<unsigned short> selectedRows = m_Filter->GetSelectedRows();

			// render specific page of data
			size_t pageStart = m_dCurPage * m_dRowsPerPage;
			size_t pageEnd = (m_dCurPage + 1) * m_dRowsPerPage;
			if (pageEnd > GetViewableRowCount())
			{
				pageEnd = GetViewableRowCount();
			}
			for (size_t row = pageStart; row < pageEnd; row++)
			{
				uint32_t unfilteredRowNumber = selectedRows[row];
				for (uint32_t col = 0; col < m_dCols; col++)
				{
					// data is within the grid, but it's not shown
					if (m_cHideColumn[col])
					{
						continue;
					}
					ImGui::TableSetColumnIndex(col);
					if (ImGui::IsItemClicked())
					{
						OnCellClick(m_dCurPage, col, unfilteredRowNumber);
					}

					ImGui::SetCursorPosY(ImGui::GetCursorPos().y + m_dRowDistance); // row spacing
					char* TxtToRender = NULL;
					if (m_DataRows[unfilteredRowNumber].GetCell(col))
					{
						TxtToRender = m_DataRows[unfilteredRowNumber].GetCell(col)->GetVisual();
					}
					OnCellRender(m_dCurPage, col, (uint32_t)row, unfilteredRowNumber, TxtToRender);
					ImGui::TableNextColumn();
					ImGui::NextColumn();
				}
			}
		}

		// maybe also render a footer ?
		if (m_bShowFooter)
		{

		}

		// back to single column rendering
		ImGui::Columns(1);

		uint32_t numberOfPages = GetViewablePageCount((int)GetViewableRowCount(), m_dRowsPerPage);
		if (numberOfPages > 1) // only show pagination if there are more than 1 page
		{
			// jump to the low side of the grid
			if (m_fHeightPixels != 0.0f)
			{
				ImGui::SetCursorPosY(m_fHeightPixels - ImGui::GetTextLineHeight() - ImGui::GetStyle().FramePadding.y - 10.0f);
			}
			// handle pagination
			if (m_bEnablePagination == true) // sometimes we might want to use scrolling instead
			{
				// render -X,+X page jumps
				int32_t minStart = (int)m_dPaginationFirstPage;

				ImGui::Columns(2 + PAGES_JUMP_LARGE + 2, "##paginationbuttons", false);
				// all other columns only use single digit ? Might need to revisit this later
				for (uint32_t tcol = 0; tcol < 2 + PAGES_JUMP_LARGE + 2; tcol++)
				{
					ImGui::SetColumnWidth(tcol, 2.2f * ImGui::GetTextLineHeight());
				}

				// render the jump more than 1 button : <<
				m_ButtonPrevPrev.DrawButton();
				ImGui::NextColumn();

				// render the jump back 1 button : <
				m_ButtonPrev.DrawButton();
				ImGui::NextColumn();

				for (int32_t buttonIndex = 0; buttonIndex < _countof(m_ButtonJumpToPage); buttonIndex++)
				{
					int32_t pageIndex = buttonIndex + minStart;
					// empty cell if this page does not exist
					if (pageIndex < 0 || pageIndex >= (int)numberOfPages)
					{
					}
					else
					{
						m_ButtonJumpToPage[buttonIndex].DrawButton();
					}
					ImGui::NextColumn();
				}
				// render the 'next' page button : >
				m_ButtonNext.DrawButton();
				ImGui::NextColumn();
				// render the '+X' page button : >>
				m_ButtonNextNext.DrawButton();
				ImGui::NextColumn();

				ImGui::Columns(1);
			}
		}
	}
	ImGui::EndChild();
}

void GenericDataGrid::SetHeaderData(uint32_t col, const char* data)
{
	if (data == NULL)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceDataGrids, 0, 0,
			"DataGrid:Trying to set header col %d to NULL.", col);
		return;
	}
	if (col >= m_dCols + m_dColsHiddenData)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceDataGrids, 0, 0,
			"DataGrid:Trying to set header col %d (greater than max %d) to %s.", col, m_dCols, data);
		return;
	}

	// set the value of the header
	m_HeaderStrings[col] = data;
}

void GenericDataGrid::SetData(uint32_t col, uint32_t row, const char* data, const size_t dataSize)
{
	if (data == NULL)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceDataGrids, 0, 0,
			"DataGrid:Trying to set data col %d to NULL.", col);
		return;
	}
	if (col >= (m_dCols+ m_dColsHiddenData))
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceDataGrids, 0, 0,
			"DataGrid:Trying to set data col %d (greater than max %d) to %s.", col, m_dCols + m_dColsHiddenData, data);
		return;
	}
	if (row >= m_dRows)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceDataGrids, 0, 0,
			"DataGrid:Trying to set data row %d (greater than max %d) to %s.", row, m_dRows, data);
		return;
	}

	// not yet initialized
	if (m_DataRows == NULL)
	{
		InternalNewA(m_DataRows, DataGridRow, m_dRows);
		for (size_t i = 0; i < m_dRows; i++)
		{
			m_DataRows[i].Init((int)i, m_dCols, m_dColsHiddenData);
		}
	}
	if (m_DataRows == NULL)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceDataGrids, 0, 0,
			"DataGrid:Failed to allocate grid data store.");
		return;
	}

	// set the value of the header
	m_DataRows[row].GetCell(col)->Set(data, dataSize, m_uCellDataLimit);
}

void GenericDataGrid::AsyncTask_DeleteRow(void* deleteParams)
{
	GridRowDeleteAsyncParams* params = typecheck_castL(GridRowDeleteAsyncParams, deleteParams);
	if (params->row >= params->grid->m_dRows)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceDataGrids, 0, 0,
			"DataGrid:Trying to delete a row %u greater than max %u.", params->row, params->grid->m_dRows);
		return;
	}

	// would be strange to ever hit this
	if (params->grid->m_DataRows == NULL)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceDataGrids, 0, 0,
			"DataGrid:Trying to delete a row %u but there is no data yet.", params->row);
		return;
	}

	// would be strange to ever hit this
	if (params->grid->m_bIsLoading == true)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceDataGrids, 0, 0,
			"DataGrid:Trying to delete a row %u but some thread is already refreshing data. Would be out of sync", params->row);
		return;
	}

	params->grid->SetLoadingState(true);

	// move row to the end of the array
	if (params->grid->m_dRows > 1 && params->row != params->grid->m_dRows - 1)
	{
		params->grid->m_DataRows[params->row].SwapWith(&params->grid->m_DataRows[params->grid->m_dRows - 1]);
	}

	// start ignoring last row as if it would exist
	params->grid->m_dRows--;

	params->grid->SetLoadingState(false);

	// ditch the temp allocated memory
	InternalFree(params);
}

void GenericDataGrid::SetData(uint32_t col, uint32_t row, const char* data)
{
	SetData(col, row, data, strlen(data) + 1);
}

void GenericDataGrid::SetData(uint32_t col, uint32_t row, const uint64_t data)
{
	SetData(col, row, (const char*)&data, sizeof(uint64_t));
}

void GenericDataGrid::SetGridName(const char* newName)
{
	if (newName == NULL)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceDataGrids, 0, 0,
			"DataGrid:Can't set grid name to NULL.");
		return;
	}
	strcpy_s(m_sGridName, newName);
}

void GenericDataGrid::SetVisualSize(float newWidth, float newHeight) 
{ 
	if (newWidth < 0 || newHeight < 0)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceDataGrids, 0, 0,
			"DataGrid:Can't have negative visual size.");
		return;
	}
	m_fWidthPixels = newWidth; 
	m_fHeightPixels = newHeight; 
}

void GenericDataGrid::SortByCol(uint32_t col)
{
	if (m_dSortByCol != col)
	{
		m_dSortOrder = DataGridSortTypes::Ascending;
	}
	else
	{
		m_dSortOrder = (DataGridSortTypes)(m_dSortOrder + 1);
		if (m_dSortOrder >= DataGridSortTypes::MAX_VALID_VALUE)
		{
			m_dSortOrder = DataGridSortTypes::NotSorted;
		}
	}
	m_dSortByCol = col;
	m_Filter->UpdateFilter(NULL, true);
}

void GenericDataGrid::OnCellRender(uint32_t page, uint32_t col, uint32_t visualRow, uint32_t dataRow, const char* data)
{
	page; col; visualRow; dataRow;
	if (data != NULL)
	{
		ImGui::Text(data);
	}
	else
	{
//		ImGui::Text("");
	}
}

void GenericDataGrid::OnHeaderCellRender(uint32_t page, uint32_t col, const char* data)
{
	page;
	if (data != NULL)
	{
		ImGui::Text(data);
		if (m_dSortByCol == col)
		{
			if (m_dSortOrder == DataGridSortTypes::Ascending)
			{
				ImGui::SameLine();
//				ImGui::Text("(Asc)");

				ImDrawList* drawList = ImGui::GetWindowDrawList();
				ImVec2 topleft = ImGui::GetCursorPos();
				ImVec2 imageSize((float)sImageManager.GetImageWidth(ImageIds::II_LocationsGridAscOrder),
					(float)sImageManager.GetImageHeight(ImageIds::II_LocationsGridAscOrder));

				topleft.x += imageSize.x / 2;
				float extraYOffset = (ImGui::GetTextLineHeight() - imageSize.y) / 2;
				topleft.y += extraYOffset;

				drawList->AddImage(sImageManager.GetImage(ImageIds::II_LocationsGridAscOrder),
					ImVec2(topleft.x, topleft.y),
					ImVec2(topleft.x + imageSize.x, topleft.y + imageSize.y));
			}
			else if (m_dSortOrder == DataGridSortTypes::Descending)
			{
				ImGui::SameLine();
//				ImGui::Text("(Desc)");
				ImDrawList* drawList = ImGui::GetWindowDrawList();
				ImVec2 topleft = ImGui::GetCursorPos();
				ImVec2 imageSize((float)sImageManager.GetImageWidth(ImageIds::II_LocationsGridDescOrder),
					(float)sImageManager.GetImageHeight(ImageIds::II_LocationsGridDescOrder));

				topleft.x += imageSize.x / 2;
				float extraYOffset = (ImGui::GetTextLineHeight()- imageSize.y) / 2;
				topleft.y += extraYOffset;

				drawList->AddImage(sImageManager.GetImage(ImageIds::II_LocationsGridDescOrder),
					ImVec2(topleft.x, topleft.y),
					ImVec2(topleft.x + imageSize.x, topleft.y + imageSize.y));
			}
		}
	}
	else
	{
		//		ImGui::Text("");
	}
}

void GenericDataGrid::SetVisualColWidth(uint32_t col, float newVal)
{
	if (col > m_dCols)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceDataGrids, 0, 0,
			"DataGrid:Setting more than max col width.");
		return;
	}
	m_fColWidths[col] = newVal;

	if (newVal != 0)
	{
		m_bHasColumnWidthsSet = true;
	}
}

void GenericDataGrid::FillRemainingColWidths()
{
	float totalWidth = 0;
	int valuesFound = 0;
	for (size_t i = 0; i < m_dCols; i++)
	{
		if (m_fColWidths[i] != 0.0f)
		{
			totalWidth += m_fColWidths[i];
			valuesFound++;
		}
	}
	if ((int)m_dCols - valuesFound > 0)
	{
		float colSize = (m_fWidthPixels - totalWidth) / (m_dCols - valuesFound);
		for (size_t i = 0; i < m_dCols; i++)
		{
			if (m_fColWidths[i] == 0.0f)
			{
				m_fColWidths[i] = colSize;
			}
		}
	}
	if (m_fHeaderHeight == 0.0f)
	{
		m_fHeaderHeight = ImGui::GetTextLineHeight() + ImGui::GetStyle().FramePadding.y;
	}
	if (m_fRowHeight == 0.0f)
	{
		m_fRowHeight = ImGui::GetTextLineHeight() + ImGui::GetStyle().FramePadding.y;
	}
}

float GenericDataGrid::GetVisualColWidth(uint32_t col)
{
	if (col > m_dCols || m_bHasColumnWidthsSet == false)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceDataGrids, 0, 0,
			"DataGrid:Setting more than max col width.");
		return 0.0f;
	}
	return m_fColWidths[col];
}

void GenericDataGrid::SetSize(uint32_t col, uint32_t row)
{
	if (col != 0 && m_dCols != 0 && col != m_dCols)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceDataGrids, 0, 0,
			"DataGrid:col reinitialization not possible. Destroy old data first!");
		return;
	}
	if (row != 0 && m_dRows != 0 && row != m_dRows)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceDataGrids, 0, 0,
			"DataGrid:row reinitialization not possible. Destroy old data first!");
		return;
	}
	if (row > 0)
	{
//		m_dRows = m_dRowsFiltered = row;
		m_dRows = row;
//		OnActivePageChanged(); // update pagination button texts
	}
	if (col > 0)
	{
		m_dCols = col;
	}
}

void GenericDataGrid::OnActivePageChanged()
{
	int32_t numberOfPages = GetViewablePageCount((int)GetViewableRowCount(), m_dRowsPerPage);

	if (m_dCurPage < 0)
	{
		m_dCurPage = 0;
	}
	if (m_dCurPage >= numberOfPages)
	{
		m_dCurPage = numberOfPages - 1;
	}
	if (m_dCurPage > 1)
	{
		m_ButtonPrevPrev.SetDisabled(false);
	}
	else
	{
		m_ButtonPrevPrev.SetDisabled(true);
	}
	if (m_dCurPage > 0)
	{
		m_ButtonPrev.SetDisabled(false);
	}
	else
	{
		m_ButtonPrev.SetDisabled(true);
	}
	if (m_dCurPage + 1 < numberOfPages)
	{
		m_ButtonNext.SetDisabled(false);
	}
	else
	{
		m_ButtonNext.SetDisabled(true);
	}
	if (m_dCurPage + 2 < numberOfPages)
	{
		m_ButtonNextNext.SetDisabled(false);
	}
	else
	{
		m_ButtonNextNext.SetDisabled(true);
	}

	for (int32_t i = 0; i < _countof(m_ButtonJumpToPage); i++)
	{
		m_ButtonJumpToPage[i].SetText(m_dPaginationFirstPage + i + 1);
		if (m_dPaginationFirstPage + i == m_dCurPage)
		{
			m_ButtonJumpToPage[i].SetDisabled(true);
		}
		else
		{
			m_ButtonJumpToPage[i].SetDisabled(false);
		}
	}
}

void GenericDataGrid::OnPaginationButtonClick(GenericButton* btn, void* pParent)
{
	if (btn == NULL)
	{
		return;
	}
	GenericDataGrid* gdg = typecheck_castL(GenericDataGrid, pParent);

	bool bPageChanged = false;
	if (btn->GetId() == ButtonIds::BI_DATAGRID_PREVPREV)
	{
		if (gdg->m_dCurPage < PAGES_JUMP_LARGE)
		{
			gdg->m_dCurPage = 0;
		}
		else
		{
			gdg->m_dCurPage -= PAGES_JUMP_LARGE;
		}
		bPageChanged = true;
	}
	else if (btn->GetId() == ButtonIds::BI_DATAGRID_PREV)
	{
		if (gdg->m_dCurPage >= 1)
		{
			gdg->m_dCurPage -= 1;
		}
		bPageChanged = true;
	}
	else if (btn->GetId() == ButtonIds::BI_DATAGRID_NEXT)
	{
		gdg->m_dCurPage += 1;
		int viewablePages = GetViewablePageCount((int)gdg->GetViewableRowCount(), gdg->m_dRowsPerPage);
		if ((int)gdg->m_dCurPage >= viewablePages)
		{
			gdg->m_dCurPage = viewablePages - 1;
		}
		bPageChanged = true;
	}
	else if (btn->GetId() == ButtonIds::BI_DATAGRID_NEXTNEXT)
	{
		gdg->m_dCurPage += PAGES_JUMP_LARGE;
		int viewablePages = GetViewablePageCount((int)gdg->GetViewableRowCount(), gdg->m_dRowsPerPage);
		if ((int)gdg->m_dCurPage >= viewablePages)
		{
			gdg->m_dCurPage = viewablePages - 1;
		}
		bPageChanged = true;
	}

	if (btn->GetId() >= ButtonIds::BI_DATAGRID_PAGINATION_START &&
		btn->GetId() <= ButtonIds::BI_DATAGRID_PAGINATION_END)
	{
		int jumpToPage = btn->GetTextInt() - 1;
		if (jumpToPage < 0)
		{
			jumpToPage = 0;
		}
		gdg->m_dCurPage = jumpToPage;
		int viewablePages = GetViewablePageCount((int)gdg->GetViewableRowCount(), gdg->m_dRowsPerPage);
		if ((int)gdg->m_dCurPage >= viewablePages)
		{
			gdg->m_dCurPage = viewablePages - 1;
		}
		bPageChanged = true;
	}

	//if we need to set the start page index
	if (gdg->m_dCurPage < gdg->m_dPaginationFirstPage)
	{
		gdg->m_dPaginationFirstPage = gdg->m_dCurPage;
	}
	else if (gdg->m_dCurPage >= gdg->m_dPaginationFirstPage + PAGES_JUMP_LARGE)
	{
		gdg->m_dPaginationFirstPage = gdg->m_dCurPage - PAGES_JUMP_LARGE + 1;
	}

	if (bPageChanged)
	{
		gdg->OnActivePageChanged();
	}
}

void GenericDataGrid::SetCelDataSizeLimit(int newLimit)
{
	m_uCellDataLimit = newLimit;
}

void GenericDataGrid::SetDBColName(uint32_t col, const char* name) 
{ 
	if (col >= _countof(m_sGridDBColName))
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceDataGrids, 0, 0,
			"DataGrid:Trying to access out of bounds column!");
		return;
	}
	m_sGridDBColName[col] = name; 
}

const char* GenericDataGrid::GetDBColName(uint32_t col) 
{ 
	if (col >= _countof(m_sGridDBColName))
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceDataGrids, 0, 0,
			"DataGrid:Trying to access out of bounds column!");
		return NULL;
	}
	return m_sGridDBColName[col].c_str();
}

void GenericDataGrid::SetExtraHiddenColumns(uint32_t newVal) 
{ 
	if (m_DataRows != NULL)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceDataGrids, 0, 0,
			"DataGrid:Can't add extra columns after data has been loaded");
		return;
	}
	m_dColsHiddenData = newVal;
}

const char* GenericDataGrid::GetCellData(uint32_t row, uint32_t col)
{
	if (m_DataRows == NULL)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceDataGrids, 0, 0,
			"DataGrid:Has not been initialized yet");
		return NULL;
	}
	if (row >= m_dRows)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceDataGrids, 0, 0,
			"DataGrid:Requested row is out of bounds");
		return NULL;
	}
	if (col >= (m_dCols + m_dColsHiddenData))
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceDataGrids, 0, 0,
			"DataGrid:Requested col is out of bounds");
		return NULL;
	}

	return m_DataRows[row].GetCell(col)->GetFull();
}

void GenericDataGrid::SetActivePageColor(ImU32 newVal)
{
	m_uColorActivePage = newVal;
	for (int i = 0; i < _countof(m_ButtonJumpToPage); i++)
	{
		m_ButtonJumpToPage[i].SetColor(GenericButtonColors::GBCT_DisabledText, m_uColorActivePage);
	}
}

size_t GenericDataGrid::GetViewableRowCount()
{
	return m_Filter->GetSelectedRows().size();
}

void GenericDataGrid::SetLoadingState(bool newVal)
{
	if (newVal == false)
	{
		if (m_bIsLoading == true)
		{
			m_Filter->UpdateFilter("", true);
		}
	}
	else
	{
		// maybe should use a mutex / signals. Initially application was almost completly static
		while (m_bIsLoading == true || m_bIsRendering == true)
		{
			Sleep(1);
		}
	}
	m_bIsLoading = newVal;
}

void GenericDataGrid::DebugRenderCells()
{
	ImVec2 topleft = ImGui::GetCursorPos();

	ImVec2 botRight = { topleft.x + m_fWidthPixels, topleft.y + m_fHeightPixels };
	ImVec2 botRightHeader = { topleft.x + m_fWidthPixels, topleft.y + m_fHeaderHeight };

	{
		ImVec2 topLeftCell = topleft;
		ImVec2 botRightCell = { topLeftCell.x + m_fColWidths[0], topLeftCell.y + m_fHeaderHeight };
		// which cell
		for (size_t col = 0; col < m_dCols; col++)
		{
			ImGui::GetWindowDrawList()->AddRect(topLeftCell, botRightCell, IM_COL32(255, 0, 0, 255));
			topLeftCell.x += m_fColWidths[col];
			botRightCell.x += m_fColWidths[col + 1];
		}
	}

	// are data rows clicked ?
	ImVec2 topLeftData = { topleft.x, topleft.y + m_fHeaderHeight };
	ImVec2 botRightData = { topleft.x + m_fWidthPixels, topleft.y + m_fHeightPixels };
	for (size_t row = 0; row < m_dRowsPerPage; row++)
	{
		ImVec2 topLeftRow = { topLeftData.x, topLeftData.y + m_fRowHeight * row };
		ImVec2 botRightRow = { topleft.x + m_fWidthPixels, topleft.y + m_fRowHeight * (row + 1) };
		ImVec2 topLeftCell = topLeftRow;
		ImVec2 botRightCell = { topLeftCell.x + m_fColWidths[0], topLeftCell.y + m_fRowHeight };
		for (size_t col = 0; col < m_dCols; col++)
		{
			ImGui::GetWindowDrawList()->AddRect(topLeftCell, botRightCell, IM_COL32(255, 255, 0, 255));
			topLeftCell.x += m_fColWidths[col];
			botRightCell.x += m_fColWidths[col + 1];
		}
	}

	float totalWidth = 0;
	for (size_t i = 0; i < m_dCols; i++)
	{
		totalWidth += m_fColWidths[i];
	}
	if (totalWidth > m_fWidthPixels)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceDataGrids, 0, 0,
			"DataGrid:Sum of column widths is greater than total grid width. Will cause bad rendering.");
	}
	if(topleft.x + m_fWidthPixels > 1920 )
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceDataGrids, 0, 0,
			"DataGrid:Last column is rendered outside the screeen.");
	}
	if (topleft.y + m_fHeightPixels > 1050)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceDataGrids, 0, 0,
			"DataGrid:Last row is rendered outside the screeen.");
	}
}

void GenericDataGrid::DrawDataGrid2()
{
	ImVec2 topleft = ImGui::GetCursorPos();
	size_t rowsRendered = 0;

#ifdef _DEBUG
	DebugRenderCells();
#endif

	if (ImGui::IsMouseClicked(0))
	{
		ImVec2 botRight = { topleft.x + m_fWidthPixels, topleft.y + m_fHeightPixels };
		const ImGuiIO& io = ImGui::GetIO();
		const ImVec2 mousePos = io.MousePos;
		// is table clicked ?
		if (isPosInBox(mousePos, topleft, botRight))
		{
			// is header clicked
			ImVec2 botRightHeader = { topleft.x + m_fWidthPixels, topleft.y + m_fHeaderHeight };
			if (isPosInBox(mousePos, topleft, botRightHeader))
			{
				ImVec2 topLeftCell = topleft;
				ImVec2 botRightCell = { topLeftCell.x + m_fColWidths[0], topLeftCell.y + m_fHeaderHeight};
				// which cell
				for (size_t col = 0; col < m_dCols; col++)
				{
					if (isPosInBox(mousePos, topLeftCell, botRightCell))
					{
						OnHeaderCellClick((uint32_t)col);
						break;
					}
					topLeftCell.x += m_fColWidths[col];
					botRightCell.x += m_fColWidths[col + 1];
				}
			}

			// are data rows clicked ?
			ImVec2 topLeftData = { topleft.x, topleft.y + m_fHeaderHeight };
			ImVec2 botRightData = { topleft.x + m_fWidthPixels, topleft.y + m_fHeightPixels };
			if (isPosInBox(mousePos, topLeftData, botRightData))
			{
				for (size_t row = 0; row < m_dRowsPerPage; row++)
				{
					ImVec2 topLeftRow = { topLeftData.x, topLeftData.y + m_fRowHeight * row };
					ImVec2 botRightRow = { topleft.x + m_fWidthPixels, topleft.y + m_fRowHeight * ( row + 1)};
					if (isPosInBox(mousePos, topLeftRow, botRightRow))
					{
						ImVec2 topLeftCell = topLeftRow;
						ImVec2 botRightCell = { topLeftCell.x + m_fColWidths[0], topLeftCell.y + m_fRowHeight };
						for (size_t col = 0; col < m_dCols; col++)
						{
							if (isPosInBox(mousePos, topLeftCell, botRightCell))
							{
								OnCellClick(m_dCurPage, (uint32_t)col, (uint32_t)row);
								break;
							}
							topLeftCell.x += m_fColWidths[col];
							botRightCell.x += m_fColWidths[col + 1];
						}
					}
				}
			}
		}
	}

	// if desired, render the header
	if (m_bShowHeader)
	{
		ImVec2 topLeftCell = topleft;
		topLeftCell.x += m_fHeaderXOffset;
		topLeftCell.y += m_fHeaderYOffset;
		for (uint32_t col = 0; col < m_dCols; col++)
		{
			ImGui::SetCursorPos(topLeftCell); // grid X offset
			OnHeaderCellRender(m_dCurPage, col, m_HeaderStrings[col].c_str());
			topLeftCell.x += m_fColWidths[col];
		}
		rowsRendered++;
	}

	if (m_bIsLoading == false && m_DataRows != NULL)
	{
		m_bIsRendering = true;

		std::vector<unsigned short> &selectedRows = m_Filter->GetSelectedRows();

		ImVec2 topLeftCell = topleft;
		topLeftCell.y += m_fHeaderHeight;
		topLeftCell.y += m_fRowYOffset;
		topLeftCell.x += m_fRowXOffset;

		// render specific page of data
		size_t pageStart = m_dCurPage * m_dRowsPerPage;
		size_t pageEnd = (m_dCurPage + 1) * m_dRowsPerPage;
		if (pageEnd > GetViewableRowCount())
		{
			pageEnd = GetViewableRowCount();
		}
		for (size_t row = pageStart; row < pageEnd; row++)
		{
			uint32_t unfilteredRowNumber = selectedRows[row];
			for (uint32_t col = 0; col < m_dCols; col++)
			{
				// data is within the grid, but it's not shown
				if (m_cHideColumn[col])
				{
					continue;
				}

				ImGui::SetCursorPos(topLeftCell); // grid X offset
				topLeftCell.x += m_fColWidths[col];
				char* TxtToRender = NULL;
				if (m_DataRows[unfilteredRowNumber].GetCell(col))
				{
					TxtToRender = m_DataRows[unfilteredRowNumber].GetCell(col)->GetVisual();
				}
				OnCellRender(m_dCurPage, col, (uint32_t)row, unfilteredRowNumber, TxtToRender);
			}

			// add a line at the bottom of the row
			ImGui::GetWindowDrawList()->AddLine(
				ImVec2(topleft.x, topLeftCell.y- m_fRowYOffset + m_fRowHeight),
				ImVec2(topLeftCell.x, topLeftCell.y - m_fRowYOffset + m_fRowHeight),
				ImColor(23, 23, 23, 255), 3);

			topLeftCell.x = topleft.x + m_fRowXOffset;
			topLeftCell.y += m_fRowHeight;
			rowsRendered++;
		}

		m_bIsRendering = false;
	}

	// handle pagination
	uint32_t numberOfPages = GetViewablePageCount((int)GetViewableRowCount(), m_dRowsPerPage);
	if (numberOfPages > 1 && m_bEnablePagination == true) // only show pagination if there are more than 1 page
	{
		// render -X,+X page jumps
		int32_t minStart = (int)m_dPaginationFirstPage;

		const float singlePaginationButtonWidth = m_ButtonPrev.GetMinWidth();

		float totalExpectedWidth = (1 + numberOfPages + 1) * singlePaginationButtonWidth;
		ImVec2 nextButtonPos = { topleft.x + m_fWidthPixels / 2 - totalExpectedWidth / 2,
			topleft.y + m_fHeightPixels - ImGui::GetTextLineHeight() - ImGui::GetStyle().FramePadding.y - 10.0f };

		// render the jump back 1 button : <
		ImGui::SetCursorPos(nextButtonPos); // grid X offset
		m_ButtonPrev.DrawButtonAsTextKnownSize();
		nextButtonPos.x += singlePaginationButtonWidth;

		for (int32_t buttonIndex = 0; buttonIndex < _countof(m_ButtonJumpToPage); buttonIndex++)
		{
			int32_t pageIndex = buttonIndex + minStart;
			// empty cell if this page does not exist
			if (pageIndex < 0 || pageIndex >= (int)numberOfPages)
			{
			}
			else
			{
				ImGui::SetCursorPos(nextButtonPos); // grid X offset
				m_ButtonJumpToPage[buttonIndex].DrawButtonAsTextKnownSize();
				nextButtonPos.x += singlePaginationButtonWidth;
			}
		}
		// render the 'next' page button : >
		ImGui::SetCursorPos(nextButtonPos); // grid X offset
		m_ButtonNext.DrawButtonAsTextKnownSize();
	}
}

void GenericDataGrid::SetFilterString(const char* filterString)
{
	m_Filter->UpdateFilter(filterString, false);
}

uint32_t GenericDataGrid::GetVisualRowDataRow(uint32_t visualRow)
{
	if (visualRow >= m_dRowsPerPage)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceDataGrids, 0, 0,
			"DataGrid:Unexpected large visual row value. Got %u, max %u.", visualRow, m_dRowsPerPage);
		return 0xFFFF;
	}
	// thread concurrency making less value to be availbale than the one clicked ?
	size_t visualRowNr = m_dRowsPerPage * m_dCurPage + visualRow;
	if (visualRowNr >= m_Filter->GetSelectedRows().size())
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceDataGrids, 0, 0,
			"DataGrid:Unexpected large visual row value. Got %u, max %u.", visualRowNr, m_Filter->GetSelectedRows().size());
		return 0xFFFF;
	}
	return m_Filter->GetSelectedRows()[visualRowNr];
}