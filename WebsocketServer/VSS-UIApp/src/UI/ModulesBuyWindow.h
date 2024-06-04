#pragma once

#define MODULES_ROW_PER_PAGE	10
#define MAX_MODULES_SHOWN	1000 

#include "Util/ObjDescriptor.h"

class ModulesBuyGrid : public GenericDataGrid
{
public:
	REFLECT_TYPE(ModulesBuyGrid);

	ModulesBuyGrid();
	void OnCellRender(uint32_t page, uint32_t col, uint32_t visualRow, uint32_t dataRow, const char* data);
	void DestructorCheckMemLeaks();
	void RefreshData();
	void RefreshDataModuleBuy();
	void RefreshInstanceLocation();
private:
	static void CB_AsyncDataArivedLocations(int CurlErr, char* response, void* userData);
	static void CB_AsyncDataArivedOrganizationModules(int CurlErr, char* response, void* userData);
	static void CB_AsyncDataArivedModuleInstances(int CurlErr, char* response, void* userData);
	static void CB_InitDatagridToStoreLocations(size_t rowCount, ExtractDBColumnToBinary* colDef);
	static bool CB_OnLocationRowExtracted(int rowIndex, ExtractDBColumnToBinary* rowColDataArr);
	static void OnGridButtonClick(GenericButton* pBtn, void* pParent);
	static void OnGridDropdownClick(GenericDropdown* pBtn, void* pParent);
	void ReinitDropdownActiveSelections();
	FlatButton m_ButtonBuyModule[MODULES_ROW_PER_PAGE]; // could / should have made it dynamic ?
	GenericDropdown m_ddSelectLocation[MODULES_ROW_PER_PAGE]; // could / should have made it dynamic ?
	uint64_t m_PrevValuesCRCOrganizationModules;
	uint64_t m_PrevValuesCRCModuleInstances;
	uint64_t m_PrevValuesCRCLocations;
	bool m_bLocationsDataChangedSinceReinit;
	bool m_bOrganizationModuleDataChangedSinceReinit;
	bool m_bModuleInstanceDataChangedSinceReinit;
	std::set<int32_t> m_OrganizationModules; // actively logged in organization
	bool m_bLocationDataArrived;
	bool m_bOrganizationDataArrived;
	bool m_bModuleInstanceDataArrived;
};

class ModulesBuyWindow : public GenericWindow
{
public:
	ModulesBuyWindow();
	int DrawWindow();
	void ResetState();
	void RefreshDataModuleBuy();
	void RefreshInstanceLocation();
	void DestructorCheckMemLeaks();
	void OnUserLoggedIn();
private:

	ModulesBuyGrid m_GridModules;
};