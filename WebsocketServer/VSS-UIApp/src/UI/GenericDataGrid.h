#pragma once

#define MAX_GRID_STRING_LEN		512
#define PAGES_JUMP_LARGE		10 // jump 5 pages of data when clicked on '<<' or '>>' buttons
#define MAX_GRIDDATA_COLUMNS	100 // used for sanity checks
#define MAX_GRIDDATE_ROWS		200 * 25 // more like a guide than a hard limit

// Sorting is able to have these values
// In case you need custom sorting types, you need to override default sorting function
enum DataGridSortTypes
{
	NotSorted = 0,
	Ascending,
	Descending,
	MAX_VALID_VALUE
};

#include "Util/ObjDescriptor.h"

/// <summary>
/// Interface class to force generic implementation over data grids
/// Data grids should be derived from this class to make code more readable
/// First column is always rowID. By default is invisible
/// </summary>
class DataGridRow;
class LocationsGrid;
class GenericDataGrid;

class GenericDataGrid
{
public:
	REFLECT_TYPE(GenericDataGrid);
	GenericDataGrid();
	~GenericDataGrid();

	/// <summary>
	///  When the user clicks on one of the cells trigger this callback function
	/// </summary>
	/// <param name="col"></param>
	/// <param name="row"></param>
	virtual void OnCellClick(uint32_t page, uint32_t col, uint32_t row) { page; col; row; };

	/// <summary>
	/// In case we want to support ordering based on columns
	/// </summary>
	virtual void OnHeaderCellClick(uint32_t col) { SortByCol(col); }

	/// <summary>
	/// When user hovers the mouse over a cell, to be able to play animation
	/// </summary>
	/// <param name="col"></param>
	/// <param name="row"></param>
//	virtual void OnCellHoverStart(int page, int col, int row) { page; col; row; };
//	virtual void OnCellHoverEnd(int page, int col, int row) { page; col; row; };

	/// <summary>
	/// Instance of the DataGrid should know how to render a specific column of the data
	/// </summary>
	/// <param name="col">Data type probably depends on column number</param>
	/// <param name="row">In case the background color depends on row count</param>
	/// <param name="data">String to be rendered</param>
	virtual void OnCellRender(uint32_t page, uint32_t col, uint32_t visualRow, uint32_t dataRow, const char* data);

	/// <summary>
	/// Function to customize header rendering of a data grid
	/// </summary>
	/// <param name="col"></param>
	/// <param name="data"></param>
	virtual void OnHeaderCellRender(uint32_t page, uint32_t col, const char* data);

	/// <summary>
	/// Generic grid rendering
	/// </summary>
	virtual void DrawDataGrid();
	virtual void DrawDataGrid2();

	/// <summary>
	/// Reorder data rows
	/// </summary>
	virtual void SortByCol(uint32_t col);

	/// <summary>
	/// Set the string shown in a specific column header
	/// </summary>
	void SetHeaderData(uint32_t col, const char* data);

	/// <summary>
	/// Set the data shown for a specific column and row
	/// </summary>
	void SetData(uint32_t col, uint32_t row, const char* data, const size_t dataSize);
	void SetData(uint32_t col, uint32_t row, const char* data);
	void SetData(uint32_t col, uint32_t row, const uint64_t data);

	/// <summary>
	/// Toggle if header should be shown or not
	/// </summary>
	void SetShowHeader(bool newVal) { m_bShowHeader = newVal; }

	/// <summary>
	/// Needed for ImGUI to identify an object
	/// </summary>
	void SetGridName(const char* newName);

	/// <summary>
	/// Show borders for the table ?
	/// </summary>
	void SetShowBorder(bool newVal) { m_bShowBroder = newVal; }

	/// <summary>
	/// Restrict the size of the table to this size
	/// </summary>
	void SetVisualSize(float newWidth, float newHeight);

	/// <summary>
	/// Spacing between row data
	/// </summary>
	void SetVisualRowDistance(uint32_t newVal) { m_dRowDistance = newVal; }

	/// <summary>
	/// Number of data rows show per page. Header is not considered a data row
	/// </summary>
	void SetRowsPerPage(uint32_t newVal) { m_dRowsPerPage = newVal; }

	/// <summary>
	/// Set the width of a specific column
	/// </summary>
	void SetVisualColWidth(uint32_t col, float newVal);
	float GetVisualColWidth(uint32_t col);
	inline float GetVisualRowHeight() const { return m_fRowHeight; }
	void FillRemainingColWidths(); // based on max size, divide the remaining size between columns

	/// <summary>
	/// Reserve space for data to be stored. Should come before storing data into row/col
	/// </summary>
	void SetSize(uint32_t col, uint32_t row);

	/// <summary>
	/// When we intend to refresh grid data only. Used when user relogs
	/// </summary>
	void ResetData();

	/// <summary>
	/// Limit the number of characters cells will show
	/// </summary>
	void SetCelDataSizeLimit(int newLimit);

	/// <summary>
	/// Check if enough initializations have been done.
	/// </summary>
	bool HasBeenInitialized() { return m_DataRows != NULL; }

	/// <summary>
	/// Check if we could shut down so that no memory is leaked. This is because managers are singletons
	/// </summary>
	virtual void DestructorCheckMemLeaks();

	/// <summary>
	/// When we query DB for values, data comming from API call would have this name
	/// </summary>
	void SetDBColName(uint32_t col, const char* name);

	/// <summary>
	/// Based on the order of columns, load data from API reply
	/// </summary>
	const char* GetDBColName(uint32_t col);

	/// <summary>
	/// Number of columns this grid contains
	/// </summary>
	uint32_t GetCols(bool IncludeExtra = false) 
	{ 
		if (IncludeExtra)
		{
			return m_dCols + m_dColsHiddenData;
		}
		return m_dCols; 
	}
	uint32_t GetRows() { return m_dRows; }

	/// <summary>
	/// Extra columns are not shown, but they can be queried for data
	/// </summary>
	void SetExtraHiddenColumns(uint32_t newVal);

	/// <summary>
	/// Full string data stored from the API call that populated this grid
	/// </summary>
	const char* GetCellData(uint32_t row, uint32_t col);

	/// <summary>
	/// Probably should use a mutex here
	/// </summary>
	void SetLoadingState(bool newVal);

	/// <summary>
	/// used to snap the grid to the bottom of a window
	/// </summary>
	float GetVisualHeight() { return m_fHeightPixels; }

	/// <summary>
	/// Some grids might want to hide pagination
	/// </summary>
	void SetShowPaginationButtons(bool newVal) { m_bEnablePagination = newVal; }

	/// <summary>
	/// Set the text color of the actively selected page
	/// </summary>
	void SetActivePageColor(ImU32 newVal);

	/// <summary>
	/// After filtering, the viewable row count might be smaller than max possible row count
	/// </summary>
	size_t GetViewableRowCount(); 

	/// <summary>
	/// Set filter string. This will reapply filtering on the existing data rows
	/// </summary>
	void SetFilterString(const char* filterString);
	
	/// <summary>
	/// Adjust rendered header text positioning
	/// </summary>
	void SetHeaderRenderingSizes(float HeaderYOffset, float HeaderXOffset, float HeaderHeight)
	{
		m_fHeaderYOffset = HeaderYOffset;
		m_fHeaderXOffset = HeaderXOffset;
		m_fHeaderHeight = HeaderHeight;
	}

	/// <summary>
	/// Adjust rendered data text positioning
	/// </summary>
	void SetDataRowRenderingSizes(float DataRowYOffset, float DataRowXOffset, float DataRowHeight)
	{
		m_fRowYOffset = DataRowYOffset;
		m_fRowXOffset = DataRowXOffset;
		m_fRowHeight = DataRowHeight;
	}

	/// <summary>
	/// Due to sorting and filtering, the row we see on screen will match a different data row
	/// </summary>
	uint32_t GetVisualRowDataRow(uint32_t visualRow);

	/// <summary>
	/// Delete a specific row. Dangerous :). Needs to be called outside the rendering loop or else it deadlocks
	/// </summary>
	struct GridRowDeleteAsyncParams
	{
		REFLECT_TYPE(GridRowDeleteAsyncParams);
		GenericDataGrid* grid;
		uint32_t row;
	};
	static void AsyncTask_DeleteRow(void *deleteParams);
protected:
	// Generic event when the whole window is clicked. Need to calculate the specific cell
	void OnMouseClick();
	void OnActivePageChanged(); // update pagination button texts
	static void OnPaginationButtonClick(GenericButton* pBtn, void* pParent);
	friend void OnGridButtonClick(GenericButton* pBtn, void* pParent);
	class GenericDataGridFilter;
	void DebugRenderCells();

	char m_sGridName[MAX_GRID_STRING_LEN]; // used by ImGUI and debugging
	std::string m_sGridDBColName[MAX_GRIDDATA_COLUMNS]; // mostly for debugging, but also for easier dat loading
	bool m_bShowBroder;
	float m_fWidthPixels; // if you want to specify width in pixels for this grid
	float m_fHeightPixels; //if you want to specify height in pixels for this grid. Might spawn scrollbar
	uint32_t m_dRows; // number of rows of total data
//	uint32_t m_dRowsFiltered; // number of rows of data that is actually going to be shown
	uint32_t m_dCols; // number of columns of total data
	uint32_t m_dColsHiddenData; // Should be able to fetch data from these columns, but they are not rendered
	uint32_t m_dRowDistance; // number of pixels between 2 rows
	uint32_t m_dRowsPerPage; // support for pagination of data
	int32_t m_dCurPage; // right now the user is viewing page X from N total
	int32_t m_dPaginationFirstPage;
	DataGridRow* m_DataRows; // matrix of strings of size : m_dRows * m_dCols. Extra column contains a numerical value to hide or not the data row
	bool m_bShowHeader; // should a header be rendered for this data grid?
	std::string m_HeaderStrings[MAX_GRIDDATA_COLUMNS]; // array of strings to be shown on header
	bool m_bShowFooter; // in case you want to perform operation on a whole column of data ?
	uint32_t m_dSortByCol; // reorder all data rows by this column
	DataGridSortTypes m_dSortOrder;
	float m_fColWidths[MAX_GRIDDATA_COLUMNS];
	char m_cHideColumn[MAX_GRIDDATA_COLUMNS];
	bool m_bHasColumnWidthsSet;
	bool m_bShowFilterRow;
	FlatButton m_ButtonPrevPrev;
	FlatButton m_ButtonPrev;
	FlatButton m_ButtonJumpToPage[PAGES_JUMP_LARGE];
	FlatButton m_ButtonNext;
	FlatButton m_ButtonNextNext;
	uint32_t m_uCellDataLimit;
	bool m_bIsLoading;
	bool m_bIsRendering;
	bool m_bEnablePagination;
	ImU32 m_uColorActivePage;
	GenericDataGridFilter *m_Filter;
	
	// when rednering header data, start rendering text with these pixel offsets
	float m_fHeaderYOffset;
	float m_fHeaderXOffset; // per cell value
	float m_fHeaderHeight; // used to jump to row rendering data

	// when rednering row data, start rendering text with these pixel offsets
	float m_fRowYOffset;
	float m_fRowXOffset; // per cell value
	float m_fRowHeight; // used to jump to the next row rendered data
};

/// <summary>
/// Takes a generic data grid as source. Will mark certain rows as non rendered
/// </summary>
class GenericDataGrid::GenericDataGridFilter
{
public:
	REFLECT_TYPE(GenericDataGridFilter);
	GenericDataGridFilter(GenericDataGrid* parent);
	void UpdateFilter(const char *newFilter, bool reasonDataChange);
	std::vector<unsigned short>& GetSelectedRows() { return *m_pRowsSelected; }
protected:
	static void AsyncTask_FilterOrSort(void* params);
	friend class GenericDataGrid;
	std::string m_sFilterString;
	std::vector<unsigned short> m_RowsSelected1;
	std::vector<unsigned short> m_RowsSelected2;
	std::vector<unsigned short> *m_pRowsSelected;
	GenericDataGrid* m_Parent;
	enum FilterRefreshStates
	{
		FilterIdle,
		FilterQueued,
		FilterProcessing,
	};
	FilterRefreshStates m_FilterState;
	uint64_t m_dFilterCounter;
};
