#pragma once

#include "Util/ObjDescriptor.h"

class LocationEditWindow : public GenericWindow
{
public:
	REFLECT_TYPE(LocationEditWindow);
	LocationEditWindow();
	int DrawWindow();
	void ResetState() { ResetState(0); }
	void ResetState(int id);
	void DestructorCheckMemLeaks();
	// helper function. Can be used when called from locationView. Not a must to be used
	// helps to hide network latency
	void SetLocationData(const char* name, const char* desc, const char* addr1, const char* addr2,
		const char* city, const char* state, const char* countryRegion, const char* country);
private:
	static void OnWindowButtonClick(GenericButton* pBtn, void* pParent);
	static void CB_AsyncDataArived(int CurlErr, char* response, void* userData);
	FlatButton m_ButtonSave;
	FlatButton m_ButtonCancel;
	int m_uEditedID;
	InputTextData m_sName;
	InputTextData m_sDescription;
	InputTextData m_sAddress1;
	InputTextData m_sAddress2;
	InputTextData m_sCity;
	InputTextData m_sState;
	InputTextData m_sCountryRegion;
	InputTextData m_sCountry;
	InputTextData m_sLatitude;
	InputTextData m_sLongitude;
	InputTextData m_sElevation;
	bool m_bSetDefaultFocus;
};