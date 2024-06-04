#pragma once

#define MAX_LOCAL_STRING_LEN    512
#define MAX_EXPECTED_IDSTRING_COUNT 1024 // max is like 100

// Numerical representation of the string keys from the INI file loaded
// these values will be accessed with 60 FPS. Cache them for fast access
enum LocalizationRssIds : int
{
    LRSS_NOT_INITIALIZED = 0,
    Login_Btn_Text,
    Login_ResetPassw_Btn_Text,
    Menu_Locations_ViewAll_Text,
    Menu_Locations_Add_Text,
    Menu_Settings_MyActivityLog_Text,
    Menu_Settings_MyAccount_Text,
    Menu_Settings_Modules_Text,
    RstPassw_Reset_Btn_Text,
    RstPassw_Cancel_Btn_Text,
    RstPassw_Success_Btn_Text,
    UserInfo_Save_Btn_Text,
    UserInfo_Cancel_Btn_Text,
    WelcomeLocations_More_Btn_Text,
    WelcomeAlerts_More_Btn_Text,
    WelcomeAlerts_View_Btn_Text,
    WelcomeSession_Location_Btn_Text,
    WelcomeModule_Add_Btn_Text,
    DataGrid_PrevPrev_Btn_Text,
    DataGrid_Prev_Btn_Text,
    DataGrid_Next_Btn_Text,
    DataGrid_NextNext_Btn_Text,
    Locations_Add_Btn_Text,
    LocationEdit_Save_Btn_Text,
    LocationEdit_SaveNew_Btn_Text,
    LocationEdit_Cancel_Btn_Text,
    LocationView_Edit_Btn_Text,
    LRSS_MAX_VAL
};

typedef struct IdStringPair
{
    int Id;
    char Val[MAX_LOCAL_STRING_LEN];
}IdStringPair;

/// <summary>
/// Object that will hold in memory string values to be accessed globaly within the application
/// This Singleton is expected to be loaded on startup and remain unchanged while app is running
/// </summary>
class LocalizationManager
{
public:
    inline static LocalizationManager& getInstance() {
#ifdef _DEBUG
        static LocalizationManager* instance = new LocalizationManager;
        return *instance;
#else
        static LocalizationManager instance;
        return instance;
#endif
    }
    /// <summary>
    /// Loads an 'ini' file content in memory
    /// </summary>
    /// <param name="fileName"></param>
    void LoadStrings(const char* fileName);
    /// <summary>
    /// Get the key-value pair's value
    /// </summary>
    /// <param name="id"></param>
    /// <returns></returns>
    const char *GetString(LocalizationRssIds id)
    {
        if (m_sValues[id] == NULL)
        {
            return "";
        }
        return m_sValues[id];
    }
    /// <summary>
    /// Only when checking if Application has memory leaks
    /// </summary>
    void DestructorCheckMemLeaks();

    /// <summary>
    /// Use API call to fetch id-string definitions of Roles, Rights...
    /// </summary>
    void FetchServerStringDefines();

    /// <summary>
    /// Fetch the whole array of role defines. 
    /// Used to be displayed in a dropdown
    /// </summary>
    /// <param name="count"></param>
    /// <param name="out_roles"></param>
    void GetRoleStringDefines(int& count, const IdStringPair** out_roles)
    {
        count = m_dRolesCount;
        *out_roles = m_Roles;
    }

    /// <summary>
    /// Fetch the whole array of right defines. 
    /// Used to be displayed in a checkbox group
    /// </summary>
    /// <param name="count"></param>
    /// <param name="out_roles"></param>
    void GetRrightStringDefines(int& count, const IdStringPair** out_roles)
    {
        count = m_dRightsCount;
        *out_roles = m_Rights;
    }

    /// <summary>
    /// Get the string representation of a role ID
    /// </summary>
    /// <param name="id"></param>
    /// <returns></returns>
    const char* GetRoleIdString(const int id)
    {
        return GetIdStringGeneric(id, m_dRolesCount, m_Roles);
    }

    /// <summary>
    /// Get the string representation of a Right ID
    /// </summary>
    /// <param name="id"></param>
    /// <returns></returns>
    const char* GetRightIdString(const int id)
    {
        return GetIdStringGeneric(id, m_dRightsCount, m_Rights);
    }

    /// <summary>
    /// Get the string representation of a Right ID
    /// </summary>
    /// <param name="id"></param>
    /// <returns></returns>
    const char* GetActiveStateIdString(const int id)
    {
        return GetIdStringGeneric(id, m_dActiveStateCount, m_ActiveStates);
    }

    /// <summary>
    /// Alert state type ids to strings
    /// </summary>
    const char* GetAlertStateIdString(const int id)
    {
        return GetIdStringGeneric(id, m_dAlertStateCount, m_AlertStates);
    }

    /// <summary>
    /// Alert type ids to strings
    /// </summary>
    const char* GetAlertTypeIdString(const int id)
    {
        return GetIdStringGeneric(id, m_dAlertTypeCount, m_AlertTypes);
    }

    /// <summary>
    /// Module status type ids to strings
    /// </summary>
    const char* GetModuleStatusTypeIdString(const int id)
    {
        return GetIdStringGeneric(id, m_dModuleStatusTypeCount, m_ModulesStatusTypes);
    }

    /// <summary>
    /// Module status type ids to strings
    /// </summary>
    const char* GetModuleTypeIdString(const int id)
    {
        return GetIdStringGeneric(id, m_dModuleTypeCount, m_ModulesTypes);
    }

    /// <summary>
    /// Module status type ids to strings
    /// </summary>
    const char* GetLocationIdString(const int id)
    {
        return GetIdStringGeneric(id, m_dLocationNamesCount, m_LocationNames);
    }
private:
    LocalizationManager();
    LocalizationManager(const LocalizationManager&) = delete;
    LocalizationManager& operator=(const LocalizationManager&) = delete;

    friend void CB_ServerStringDefinesArrived(int CurlErr, char* response, void* userData);

    // search in an array of id-string pairs to find the string representation of an ID
    // Never returns NULL
    const char* GetIdStringGeneric(const int id, const int count, const IdStringPair* pair);

    // confirg values stored in an id-string pair manner
    char *m_sValues[LocalizationRssIds::LRSS_MAX_VAL];

    // values fetched from Backend : role id-string pairs
    int m_dRolesCount;
    IdStringPair *m_Roles;

    // values fetched from Backend : right id-string pairs
    int m_dRightsCount;
    IdStringPair* m_Rights;

    // values fetched from Backend : right id-string pairs
    int m_dActiveStateCount;
    IdStringPair* m_ActiveStates;

    // values fetched from Backend : right id-string pairs
    int m_dAlertStateCount;
    IdStringPair* m_AlertStates;

    // values fetched from Backend : right id-string pairs
    int m_dAlertTypeCount;
    IdStringPair* m_AlertTypes;

    // values fetched from Backend : right id-string pairs
    int m_dModuleStatusTypeCount;
    IdStringPair* m_ModulesStatusTypes;

    // values fetched from Backend : right id-string pairs
    int m_dModuleTypeCount;
    IdStringPair* m_ModulesTypes;

    // values fetched from Backend : right id-string pairs
    int m_dLocationNamesCount;
    IdStringPair* m_LocationNames;
};

#define sLocalization LocalizationManager::getInstance()
