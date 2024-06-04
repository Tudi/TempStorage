#pragma once

enum DropDownIds
{
	DI_UNINITIALIZED_VALUE = 0,
};

struct DropdownEntry
{
	char Text[MAX_DB_STRING_LENGTH];
	uint64_t CallbackId; // could be something simple as rowNr
};

class GenericDropdown;
typedef void (*DropDownCallback)(GenericDropdown* pBtn, void* pParent);

class GenericDropdown
{
public:	
	GenericDropdown();
	GenericDropdown(void* owner, DropDownCallback cb, DropDownIds id);
	~GenericDropdown();

	/// <summary>
	/// Event callback when the user pushes the DropDown.
	/// The owner window will be notified about this event.
	/// !! You should not do a lot of processing in events. Create a worker thread to avoid blocking UI !!
	/// </summary>
	virtual void OnPush();

	/// <summary>
	/// Draw the actual DropDown. Specific styles should implement this function in their own way
	/// </summary>
	virtual void DrawDropdown();

	/// <summary>
	/// DropDown IDs are used to track click events
	/// </summary>
	DropDownIds GetId() { return m_dId; }
	void SetId(DropDownIds newId) { m_dId = newId; }

	/// <summary>
	/// Set one of the color properties of this DropDown instance
	/// </summary>
	void SetColor(GenericButtonColors colorIndex, ImU32 newColor);

	/// <summary>
	/// DropDowns can belong to data grids also. Not just windows
	/// </summary>
	void SetCallback(void* owner, DropDownCallback cb);

	/// <summary>
	/// Dynamically generated DropDowns triggering callbacks need to be identified
	/// Data will be copied from param pointer
	/// </summary>
	void SetUserData(const char* data);
	const char* GetUserData() { return m_sUserData; }

	/// <summary>
	/// When you want to manually set the width / height of the DropDown
	/// </summary>
	void SetMinSize(float width, float height) { m_forcedMinSize = { width, height }; }

	/// <summary>
	/// Check if we could shut down so that no memory is leaked. This is because managers are singletons
	/// </summary>
	virtual void DestructorCheckMemLeaks();

	/// <summary>
	/// No Longer able to select values
	/// </summary>
	void SetDisabled(bool newVal) { m_bDisabled = newVal; }

	/// <summary>
	/// Set the maxium amount of entries
	/// </summary>
	void SetSize(uint32_t entryCount);

	/// <summary>
	/// Set entry data
	/// </summary>
	void SetEntryData(uint32_t rowNr, const char* newText, uint32_t callbackId);

	/// <summary>
	/// Set entry that will be shown as active
	/// </summary>
	void SetSelectedRow(uint32_t rowNr);
	const DropdownEntry* SetSelectedRowFind(uint64_t nCallbackId);

	/// <summary>
	/// Used when the "onChange" callback gets triggered
	/// </summary>
	const DropdownEntry* GetSelectedEntry();

	void SetVisualSize(float width, float height) { m_fVisualWidth = width; m_fVisualHeight = height; }
protected:
	void Init();
	void* m_CallbackParent; // callback function will receive the instance of the owner
	DropDownCallback m_CallBack; // generic callback function. Needs to be class specific
	DropDownIds m_dId; // generic way to identify a DropDown click callback
	char m_DropdownName[MAX_DB_STRING_LENGTH];
	ImU32 m_uBGColor;
	ImU32 m_uHoverColor;
	ImU32 m_uBorderColor;
	ImU32 m_uActiveColor;
	ImU32 m_uTextColor;
	ImU32 m_uTextHoverColor;
	ImU32 m_uTextDisabledColor;
	DropdownEntry* m_Entries;
	ImU32 m_EntryCount;
	char *m_SelectedEntry;
	char* m_sUserData; // this uses shared memory. It should not be deallocated by the DropDown
	ImVec2 m_forcedMinSize;
	bool m_bDisabled; // no clicks
	float m_fVisualWidth;
	float m_fVisualHeight;
};