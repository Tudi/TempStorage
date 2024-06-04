#pragma once

#include "../ResourceManagers/LocalizationManager.h"

class GenericWindow;
class GenericDataGrid;

// make this global in case we need to debug/log something specific to be able to ID it
enum ButtonIds
{
	BI_UNINITIALIZED_VALUE = 0,
	BI_LOGIN_BUTTON_ID,
	BI_RESET_PASSW_BUTTON_ID,
	BI_MENU_DASHBOARD,
	BI_MENU_LOCATION_VIEW_ALL,
	BI_MENU_ALERTS_VIEW_ALL,
	BI_MENU_SETTINGS_VIEW_ALL,
	BI_MENU_FILE,
	BI_MENU_LOGOUT,
	BI_MENU_LOCATION_ADD,
	BI_MENU_SETTINGS_MYACTIVITYLOG,
	BI_MENU_SETTINGS_MYACCOUNT,
	BI_MENU_SETTINGS_MODULES,
	BI_RSTPSSW_CANCEL,
	BI_RSTPSSW_RESET,
	BI_RSTPSSW_SUCCESS,
	BI_USERINFO_CANCEL,
	BI_USERINFO_SAVE,
	BI_SPLASHRECENT_LOCATIONS,
	BI_SPLASHRECENT_LOCATION_0,
	BI_SPLASHRECENT_LOCATION_1,
	BI_SPLASHRECENT_LOCATION_2,
	BI_SPLASHRECENT_ALERTS,
	BI_DashboardAlerts_ALERTS,
	BI_SPLASHSESSION_LOCATION,
	BI_SPLASHMODULES_MODULE,
	BI_DATAGRID_PREVPREV,
	BI_DATAGRID_PREV,
	BI_DATAGRID_NEXT,
	BI_DATAGRID_NEXTNEXT,
	BI_DATAGRID_PAGINATION_START,
	BI_DATAGRID_PAGINATION_END = BI_DATAGRID_PAGINATION_START + 100,
	BI_LOCATIONS_ADD_NEW,
	BI_LOCATIONEDIT_SAVE,
	BI_LOCATIONEDIT_CANCEL,
	BI_LOCATIONVIEW_EDIT,
	BI_MODULES_ADD,
	BI_DOPLERMODULE_TRIGGERALERT,
	BI_SETTINGS_MYACTIVITY,
	BI_SETTINGS_MYACCOUNT,
	BI_SETTINGS_MODULES,
	BI_DASHBOARD_VIEWMORE_ALERTS,
	BI_DASHBOARD_VIEWMORE_LOCATIONS,
	BI_DASHBOARD_VIEWMORE_MODULES,
};

enum GenericButtonColors
{
	GBCT_Background = 1,
	GBCT_Hover,
	GBCT_Active,
	GBCT_Text,
	GBCT_TextHover,
	GBCT_Underline,
	GBCT_UnderlineHover,
	GBCT_DisabledText,
};

typedef void (*ButtonCallback)(GenericButton *pBtn, void *pParent);

class GenericButton
{
public:
	GenericButton();
	GenericButton(void* owner, ButtonCallback cb, ButtonIds id, LocalizationRssIds txtId);
	~GenericButton();

	/// <summary>
	/// Event callback when the user pushes the button.
	/// The owner window will be notified about this event.
	/// !! You should not do a lot of processing in events. Create a worker thread to avoid blocking UI !!
	/// </summary>
	virtual void OnPush();

/*	/// <summary>
	/// Used to track the hover event. For example start playing an animation
	/// </summary>
	/// <param name="m_Resolution"></param>
	virtual void OnHoverStart()
	{
		if (m_callbackWindow)
		{
			m_callbackWindow->OnButtonHoverStart(this);
		}
	};

	/// <summary>
	/// Used to track the hover event. For example start playing an animation
	/// </summary>
	/// <param name="m_Resolution"></param>
	virtual void OnHoverEnd()
	{
		if (m_callbackWindow)
		{
			m_callbackWindow->OnButtonHoverEnd(this);
		}
	}; */

	/// <summary>
	/// Draw the actual button. Specific styles should implement this function in their own way
	/// </summary>
	virtual void DrawButton() { };

	/// <summary>
	/// Button IDs are used to track click events
	/// </summary>
	ButtonIds GetId() { return m_dId; }
	void SetId(ButtonIds newId) { m_dId = newId; }

	/// <summary>
	/// Set one of the color properties of this button instance
	/// </summary>
	void SetColor(GenericButtonColors colorIndex, ImU32 newColor);

	/// <summary>
	/// Set the text displayed on the button
	/// </summary>
	void SetText(const char* newText);
	const char* GetText() { return m_sButtonText; }
	void SetText(const int newText);
	void SetTextLocalized(LocalizationRssIds txtId);
	int GetTextInt() { return m_dButtonText; }

	/// <summary>
	/// Buttons can belong to data grids also. Not just windows
	/// </summary>
	void SetCallback(void *owner, ButtonCallback cb);

	/// <summary>
	/// Dynamically generated buttons triggering callbacks need to be identified
	/// Data will be copied from param pointer
	/// </summary>
	void SetUserData(const char *data);
	const char* GetUserData() { return m_sUserData; }

	/// <summary>
	/// When you want to manually set the width / height of the button
	/// </summary>
	void SetMinSize(float width, float height) { m_forcedMinSize = { width, height }; }
	inline float GetMinWidth() const { return m_forcedMinSize.x; }
	inline float GetMinHeight() const { return m_forcedMinSize.y; }

	/// <summary>
	/// Check if we could shut down so that no memory is leaked. This is because managers are singletons
	/// </summary>
	virtual void DestructorCheckMemLeaks();

	void SetDisabled(bool newVal) { m_bDisabled = newVal; }
protected:
	void Init();
	void* m_CallbackParent; // callback function will receive the instance of the owner
	ButtonCallback m_CallBack; // generic callback function. Needs to be class specific
	ButtonIds m_dId; // generic way to identify a button click callback
	LocalizationRssIds m_dNameId;
	ImU32 m_uBGColor;
	ImU32 m_uHoverColor;
	ImU32 m_uActiveColor;
	ImU32 m_uTextColor;
	ImU32 m_uTextHoverColor;
	ImU32 m_uTextDisabledColor;
	ImU32 m_uUnderlineColor;
	ImU32 m_uUnderlineHoverColor;
	char* m_sButtonText;
	int m_dButtonText; // dynamically generated buttons might be queried many times
	char* m_sUserData; // this uses shared memory. It should not be deallocated by the button
	ImVec2 m_forcedMinSize;
	bool m_bDisabled; // no clicks
};

inline bool isPosInBox(const ImVec2& pos, const ImVec2& topLeft, const ImVec2& botRight)
{
	// above or to the left
	if (topLeft.x > pos.x || topLeft.y > pos.y)
	{
		return false;
	}
	// right or lower
	if (botRight.x < pos.x || botRight.y < pos.y)
	{
		return false;
	}
	return true;
}

inline bool isMouseHoveredInArea_Size(const ImVec2& topLeft, const ImVec2& size)
{
	const ImGuiIO& io = ImGui::GetIO();
	const ImVec2 mousePos = io.MousePos;

	// Check if mouse position is within the rectangle area
	if ((mousePos.x >= topLeft.x) == false || (mousePos.y >= topLeft.y) == false)
	{
		return false;
	}
	if ((mousePos.x <= (topLeft.x + size.x)) == false || (mousePos.y <= (topLeft.y + size.y)) == false)
	{
		return false;
	}

	return true;
}

inline bool isMouseClickedInArea_Size(const ImVec2& topLeft, const ImVec2& size) 
{
	return isMouseHoveredInArea_Size(topLeft, size) && ImGui::IsMouseClicked(0); // Check if left mouse button is clicked
}


inline bool isMouseHoveredInArea(const ImVec2& topLeft, const ImVec2& botRight)
{
	const ImGuiIO& io = ImGui::GetIO();
	const ImVec2 mousePos = io.MousePos;

	// Check if mouse position is within the rectangle area
	if ((mousePos.x >= topLeft.x) == false || (mousePos.y >= topLeft.y) == false)
	{
		return false;
	}
	if ((mousePos.x <= botRight.x) == false || (mousePos.y <= botRight.y) == false)
	{
		return false;
	}

	return true;
}

inline bool isMouseClickedInArea(const ImVec2& topLeft, const ImVec2& botRight)
{
	return isMouseHoveredInArea_Size(topLeft, botRight) && ImGui::IsMouseClicked(0); // Check if left mouse button is clicked
}