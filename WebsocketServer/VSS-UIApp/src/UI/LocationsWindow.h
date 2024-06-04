#pragma once

#define LOCATIONS_ROW_PER_PAGE	9

#include "Util/ObjDescriptor.h"

class LocationsGrid : public GenericDataGrid
{
public:
	REFLECT_TYPE(LocationsGrid);
	LocationsGrid();
	void ResetState();
	void OnCellRender(uint32_t page, uint32_t col, uint32_t visualRow, uint32_t dataRow, const char* data);
	void DestructorCheckMemLeaks();
private:
	static void CB_AsyncDataArived(int CurlErr, char* response, void* userData);
	static void CB_AsyncLocationDeleted(int CurlErr, char* response, void* userData);
	static void OnGridButtonClick(GenericButton* pBtn, void* pParent);
	static void GetRowIndexForButton(void* pBtn, LocationsGrid *grid, size_t&out_row, size_t&out_type);
	static void AsyncTask_Init(void *userData); // because we need to wait for images to get loaded

#if defined(VER1_RENDERING)
	FlatButton m_ButtonJumpToPage[LOCATIONS_ROW_PER_PAGE]; // could / should have made it dynamic ?
#endif
	MultiStateButton m_ButtonViewLocation[LOCATIONS_ROW_PER_PAGE];
	MultiStateButton m_ButtonTrashcanLocation[LOCATIONS_ROW_PER_PAGE];
	MultiStateButton m_ButtonDeleteLocation[LOCATIONS_ROW_PER_PAGE];
	MultiStateButton m_ButtonCancelDeleteLocation[LOCATIONS_ROW_PER_PAGE];

	uint64_t m_PrevValuesCRC;
};

class LocationsWindow : public GenericWindow
{
public:
	LocationsWindow();
	int DrawWindow();
	void ResetState();
	void DestructorCheckMemLeaks();
	void OnUserLoggedIn();
private:
	static void OnWindowButtonClick(GenericButton* pBtn, void* pParent);
	MultiStateButton m_ButtonAddLocation;

	InputTextData m_SearchFilter;

	LocationsGrid m_GridLocations;
};