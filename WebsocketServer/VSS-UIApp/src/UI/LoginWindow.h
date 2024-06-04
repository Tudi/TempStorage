#pragma once

#include "Util/ObjDescriptor.h"

class LoginWindow : public GenericWindow
{
public:
	REFLECT_TYPE(LoginWindow);
	LoginWindow();
	int DrawWindow();
	void ResetState();
	void DestructorCheckMemLeaks();
private:
	static void OnWindowButtonClick(GenericButton* pBtn, void* pParent);
#if defined(VER1_RENDERING)
	FlatButton m_ButtonLogin;
	FlatButton m_ButtonResetPassw;
#endif
	MultiStateButton m_ButtonLogin2;
	MultiStateButton m_ButtonResetPassw2;
	InputTextData m_cUserName;
	InputTextData m_cPassword;
	bool m_bSetDefaultFocus;
	bool m_bTokenRequired;
};