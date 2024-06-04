#pragma once

#define MAX_LOGIN_STRING_LENGTH	255

#include "Util/ObjDescriptor.h"

class ResetPasswWindow : public GenericWindow
{
public:
	REFLECT_TYPE(ResetPasswWindow);
	ResetPasswWindow();
	int DrawWindow();
	void ResetState();
	void SetDrawAsEmbeded() { m_bDrawAsEmbeded = true; }
	void DestructorCheckMemLeaks();
private:
	static void OnWindowButtonClick(GenericButton* pBtn, void* pParent);
	static void AsyncTask_Init(void* params);
#if defined(VER1_RENDERING)
	FlatButton m_ButtonResetPassw;
	FlatButton m_CancelForm;
	FlatButton m_Success;
#endif
	MultiStateButton m_ButtonResetPassw2;
	MultiStateButton m_CancelForm2;
	MultiStateButton m_Success2;
	InputTextData m_UserEmail;
	bool m_bBadEmail;
	bool m_bUnknownError;
	bool m_bShowSuccess;
	bool m_bDrawAsEmbeded; // does not have it's own window
	bool m_bSetDefaultFocus;
};