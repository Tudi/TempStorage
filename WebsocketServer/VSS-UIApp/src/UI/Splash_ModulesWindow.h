#pragma once

#define MAX_LOGIN_STRING_LENGTH	255

class SplashModulesWindow : public GenericWindow
{
public:
	SplashModulesWindow();
	int DrawWindow();
	void ResetState();
	void DestructorCheckMemLeaks();
private:
	static void OnWindowButtonClick(GenericButton* pBtn, void* pParent);
	FlatButton m_ButtonModules;
};