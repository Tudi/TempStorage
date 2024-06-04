#pragma once

#define MODULES_ROW_PER_PAGE	10
#define MAX_MODULES_SHOWN	1000 

#include "Util/ObjDescriptor.h"

class ModulesGrid : public GenericDataGrid
{
public:
	REFLECT_TYPE(ModulesGrid);
	ModulesGrid();
	void OnCellRender(uint32_t page, uint32_t col, uint32_t visualRow, uint32_t dataRow, const char* data);
	void DestructorCheckMemLeaks();
	void RefreshData();
private:
	static void CB_AsyncDataArivedModules(int CurlErr, char* response, void* userData);
	static void OnGridButtonClick(GenericButton* pBtn, void* pParent);
	FlatButton m_ButtonJumpToPage[MODULES_ROW_PER_PAGE]; // could / should have made it dynamic ?
	uint64_t m_PrevValuesCRCModules;
};

class ModulesWindow : public GenericWindow
{
public:
	ModulesWindow();
	int DrawWindow();
	void ResetState();
	void DestructorCheckMemLeaks();
	void OnUserLoggedIn();
private:
	static void OnButtonClick(GenericButton* pBtn, void* pParent);
	FlatButton m_AddModuleButton;
	ModulesGrid m_GridModules;
};