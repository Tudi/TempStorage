#pragma once

#define MAX_DB_STRING_LENGTH	512

// function declaration that will check if new value is acceptable : empty ? Passw ? Email ? Date ?
typedef bool (*TxtICheckVal)(const char* newVal, const char* crossCheck, const char* oldVal);
// ImGUI specific text input field data storage
class InputTextData
{
public:
	InputTextData()
	{
		m_sDBVal[0] = 0;
		m_sInputVal[0] = 0;
		m_sBadValTxt[0] = 0;
		m_sEmptyValTxt[0] = 0;
		m_CheckValOkFunc = NULL;
		m_bWarningValueMissing = false;
		m_bHasFocus = false;
		m_bRefocusRequested = false;
		m_bForceImGUIRefreshBackBuffer = false;
	}
	// called before ( sometimes after ) a window gets closed
	// Initialize the internal variables
	void ResetState(const char* DBVal)
	{
		if (DBVal == NULL)
		{
			m_sDBVal[0] = 0;
			m_sInputVal[0] = 0;
		}
		else
		{
			sprintf_s(m_sDBVal, "%s", DBVal);
			sprintf_s(m_sInputVal, "%s", DBVal);
		}
		m_bWarningValueMissing = false;
	}
	// check if we need to save this value to the DB
	bool ValueChanged()
	{
		return strcmp(m_sDBVal, m_sInputVal) != 0;
	}
	// where the input field should store it's data
	char* GetInputBuff() 
	{ 
		// since IMGUI keeps a copy of the value of the buffer, m_sInputVal needs to be flushed on focus change
		if (m_bHasFocus == false && m_sInputVal[0] == 0 && m_sEmptyValTxt[0] != 0)
		{
			return m_sEmptyValTxt;
		}
		return m_sInputVal; 
	}
	char* GetInputVal() {return m_sInputVal;}
	// how much data can the input field store
	int GetInputSize() { return sizeof(m_sInputVal); }
	// this is set when you click the "save" button
	void SetWarningValue(bool newVal) { m_bWarningValueMissing = newVal; }
	// this is checked when drawing
	bool HasWarningValue() { return m_bWarningValueMissing; }
	// if value is not acceptable, show this error message ( as a red text )
	void SetBadValText(const char* newVal)
	{
		if (newVal == NULL)
		{
			m_sBadValTxt[0] = 0;
		}
		else
		{
			strcpy_s(m_sBadValTxt, newVal);
		}
	}
	// rendering is asking for the bad Txt
	const char* GetBadTxt() { return m_sBadValTxt; }
	// set the function that will check if a new value is ok
	void SetCheckValFunc(TxtICheckVal newVal) { m_CheckValOkFunc = newVal; }
	// called when pushing "save" button. Check if the value is acceptable. Set warning level else
	bool CheckNewValOk(const char* refValue)
	{
		if (m_CheckValOkFunc != NULL)
		{
			bool ret = m_CheckValOkFunc(m_sInputVal, refValue, m_sDBVal);
			SetWarningValue(!ret);
			return ret;
		}
		return true;
	}

	// simple check function that ensures string value has some data in it
	static bool CheckStrIsEmpty(const char* newVal, const char* crossCheck, const char* oldVal)
	{
		crossCheck; oldVal;
		if (strlen(newVal) == 0)
		{
			return false;
		}
		return true;
	}

	// simple check function that ensures string value has some data in it
	static bool CheckStrIsCharOnly(const char* newVal, const char* crossCheck, const char* oldVal)
	{
		crossCheck; oldVal;
		size_t len = strlen(newVal);
		if (len == 0)
		{
			return false;
		}
		for (size_t i = 0; i < len; i++)
		{
			if (isalpha(newVal[i]) == 0)
			{
				return false;
			}
		}
		return true;
	}

	// simple check function that ensures string value has some data in it
	static bool CheckStrIsEmail(const char* newVal, const char* crossCheck, const char* oldVal)
	{
		crossCheck; oldVal;
		if (strlen(newVal) == 0)
		{
			return false;
		}
		if (strchr(newVal, '@') == NULL)
		{
			return false;
		}
		if (strchr(newVal, '.') == NULL)
		{
			return false;
		}
		if (crossCheck != NULL && strcmp(newVal, crossCheck) != 0)
		{
			return false;
		}
		return true;
	}

	// simple check function that ensures string value has some data in it
	static bool CheckStrIsPassword(const char* newVal, const char* crossCheck, const char* oldVal)
	{
		crossCheck; oldVal;
		size_t len = strlen(newVal);
		if (len < 16)
		{
			return false;
		}
		// no more repeating than 2 times consecutively
		for (size_t i = 2; i < len; i++)
		{
			if (newVal[i] == newVal[i - 1] && newVal[i - 1] == newVal[i - 2])
			{
				return false;
			}
		}
		if (crossCheck != NULL && strcmp(newVal, crossCheck) != 0)
		{
			return false;
		}
		return true;
	}

	// simple check function that ensures string value has some data in it
	static bool CheckStrIsConfirm(const char* newVal, const char* crossCheck, const char* oldVal)
	{
		crossCheck; oldVal;
		if (crossCheck == NULL || strcmp(newVal, crossCheck) != 0)
		{
			return false;
		}
		return true;
	}

	// text shown if input does not have focus and input value is empty
	void SetEmptyValueText(const char* emptyValText)
	{
		if (emptyValText == NULL)
		{
			m_sEmptyValTxt[0] = 0;
		}
		else
		{
			strcpy_s(m_sEmptyValTxt, emptyValText);
		}
	}
	inline void OnSetFocus(bool bHasFocus)
	{	
		// lost focus, from now on we should show the m_sEmptyValTxt
		if (bHasFocus == false && m_bHasFocus == true)
		{
			m_bForceImGUIRefreshBackBuffer = true;
		}
		// gained focus, start showing DB val
		else if (bHasFocus == true && m_bHasFocus == false)
		{
			m_bForceImGUIRefreshBackBuffer = true;
		}
		m_bHasFocus = bHasFocus; 
	}
	// needs to be called
	inline void RefocusIfRequested()
	{
		if (m_bRefocusRequested && m_bForceImGUIRefreshBackBuffer == false)
		{
			m_bRefocusRequested = false;
//			ImGui::SetKeyboardFocusHere(-1);
//			ImGui::FocusItem();
			ImGui::ActivateItemByID(((ImGuiID)((unsigned __int64)(this))));
		}
	}
	inline void SetFocus() { m_bRefocusRequested = true; }
	inline bool IsUsingEmptyVal() { return (m_bHasFocus == false && m_sEmptyValTxt[0] != 0 && m_sInputVal[0] == 0); }
	// whenever we manually edit the m_sInputVal, we need to tell ImGUI to refresh interval state
	inline unsigned int ForceRefreshIMGUI()
	{
		if (m_bForceImGUIRefreshBackBuffer)
		{
			m_bForceImGUIRefreshBackBuffer = false;
			return ImGuiInputTextFlags_ReadOnly;
		}
		return 0;
	}
	// right now ImGUI will not call "onFocus" if it's by default focused
	inline void IsFocusedByDefault() { m_bHasFocus = true; }
private:
	char m_sDBVal[MAX_DB_STRING_LENGTH];
	char m_sInputVal[MAX_DB_STRING_LENGTH];
	char m_sBadValTxt[MAX_DB_STRING_LENGTH];
	char m_sEmptyValTxt[MAX_DB_STRING_LENGTH];
	bool m_bHasFocus;
	bool m_bRefocusRequested;
	bool m_bForceImGUIRefreshBackBuffer; // if we manually change the m_sInputVal, we need to ask IMGUI to reinitialize itself with the new data
	bool m_bWarningValueMissing;
	TxtICheckVal m_CheckValOkFunc;
};

#define FORM_EMPTYLINE_OR_ERROR(TxtF) { if(TxtF.HasWarningValue() == false) ImGui::SetCursorPosY(ImGui::GetCursorPos().y + TextRowSpacing); \
        else \
       { \
          ImGui::PushStyleColor(ImGuiCol_Text, sStyles.GetUIntValue(StyleIds::Wnd_Txt_Error_Color)); \
          ImGui::Text(TxtF.GetBadTxt()); \
          ImGui::PopStyleColor(1); \
       } }

#define FORM_FLAT_TXTI_FIELD_F(Txt, TxtF, Flags) ImGui::Text(Txt); \
        ImGui::SameLine(); \
        ImGui::SetCursorPosY(ImGui::GetCursorPos().y - RaiseInputField); \
        ImGui::SetNextItemWidth(inputTextboxWidth - ImGui::GetCursorPosX()); \
        ImGui::InputText(GetImGuiIDPointer(&TxtF), "", TxtF.GetInputBuff(), TxtF.GetInputSize(), Flags); \
        ImGUIUnderlinePrevItem(sStyles.GetFloatValue(StyleIds::Input_Underline_Distance), sStyles.GetFloatValue(StyleIds::Input_Underline_Width), sStyles.GetUIntValue(StyleIds::Input_Underline_Color)); \
        FORM_EMPTYLINE_OR_ERROR(TxtF);

#define FORM_FLAT_TXTI_FIELD(Txt, TxtF) FORM_FLAT_TXTI_FIELD_F(Txt, TxtF, 0)

#ifdef _DEBUG
	#define DRAW_BOUNDING_BOX(Width) { ImVec2 topleft = ImGui::GetCursorPos(); \
		ImVec2 botRight; \
		botRight.x = topleft.x + Width; botRight.y = topleft.y + 30; \
		ImGui::GetWindowDrawList()->AddRect(topleft, botRight, IM_COL32(255, 0, 0, 255)); \
		ImGui::SetCursorPos(topleft);}
#else
	#define DRAW_BOUNDING_BOX(Width) ;
#endif
#define FORM_INVISIBLE_TXTI_FIELD(TxtF, Width, Flags)  {DRAW_BOUNDING_BOX(Width); \
		if((TxtF)->IsUsingEmptyVal()) { \
	        ImGui::PushStyleColor(ImGuiCol_Text, sStyles.GetUIntValue(StyleIds::Input_Txt_ColorInactiveDefaultValue)); \
			ImGui::SetNextItemWidth(Width); \
			ImGui::InputText(GetImGuiIDPointer((TxtF)), "", (TxtF)->GetInputBuff(), (TxtF)->GetInputSize(), ImGuiInputTextFlags_ReadOnly); \
            ImGui::PopStyleColor(1); \
		}else{ \
			ImGui::SetNextItemWidth(Width); \
			ImGui::InputText(GetImGuiIDPointer((TxtF)), "", (TxtF)->GetInputBuff(), (TxtF)->GetInputSize(), \
					Flags | (TxtF)->ForceRefreshIMGUI()); \
		} \
		(TxtF)->RefocusIfRequested(); \
		(TxtF)->OnSetFocus(ImGui::GetActiveID_()==GetImGuiIDPointer((TxtF))); \
	}
		
