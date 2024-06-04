#pragma once

#include "Util/ObjDescriptor.h"

class UserInfoWindow : public GenericWindow
{
public:
	REFLECT_TYPE(UserInfoWindow);
	UserInfoWindow();
	int DrawWindow();
	void ResetState();
	void DestructorCheckMemLeaks();
private:
	static void OnWindowButtonClick(GenericButton* pBtn, void* pParent);
	FlatButton m_buttonCancelForm;
	FlatButton m_buttonSave;
	int m_dUserRole;
	InputTextData m_sRole;
	InputTextData m_sFirstName;
	InputTextData m_sLastName;
	InputTextData m_sEmail;
	InputTextData m_sEmailConfirm;
	InputTextData m_sPassword;
	InputTextData m_sPasswordConfirm;
	bool m_bSetDefaultFocus;
};