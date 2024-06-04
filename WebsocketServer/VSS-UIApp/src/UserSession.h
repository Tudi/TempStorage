#pragma once

#define MAX_DB_STRING_LENGTH	512

// these are stored in DB also. Make sure to copy new values
enum UserRightIds
{
    UR_CREATE_USER = 10,
    UR_VIEW_USER = 20,
    UR_VIEW_ANY_USER = 30,
    UR_EDIT_USER = 40,
    UR_EDIT_ANY_USER = 50,
    UR_EDIT_USER_ROLE_HIGHER = 60,
    UR_DELETE_USER = 70,

    UR_VIEW_USER_LOGS = 80,
    UR_VIEW_USER_ACTIVITY = 90,
    UR_VIEW_ORGANIZATION_LOGS = 100,
    UR_VIEW_ORGANIZATION_ACTIVITY = 110,

    UR_CREATE_ALERT = 120,
    UR_VIEW_ALERT = 130,
    UR_EDIT_ALERT = 140,
    UR_DELETE_ALERT = 150,

    UR_CREATE_REPORT = 160,
    UR_VIEW_REPORT = 170,
    UR_EDIT_REPORT = 180,
    UR_DELETE_REPORT = 190,

    UR_CREATE_RADAR = 200,
    UR_VIEW_RADAR = 210,
    UR_EDIT_RADAR = 220,
    UR_DELETE_RADAR = 230,

    UR_CREATE_ORGANIZATION = 240,
    UR_VIEW_ORGANIZATION = 250,
    UR_EDIT_ORGANIZATION = 260,
    UR_DELETE_ORGANIZATION = 270,

    UR_CREATE_LOCATION = 280,
    UR_VIEW_LOCATION = 290,
    UR_EDIT_LOCATION = 300,
    UR_DELETE_LOCATION = 310,

    UR_CREATE_MODULE = 320,
    UR_VIEW_MODULE = 330,
    UR_EDIT_MODULE = 340,
    UR_DELETE_MODULE = 350,

    UR_VIEW_OTHER_ORGANIZATION = 400,
    UR_UPDATE_OTHER_ORGANIZATION = 401,
    UR_VIEW_OTHER_USER = 450,
    UR_UPDATE_OTHER_USER = 451,

    UR_MAX_VALUE,
};

/// <summary>
/// Store the actively logged in user data to be accesable quickly
/// </summary>
class UserSession
{
public:
    inline static UserSession& getInstance() {
#ifdef _DEBUG
        static UserSession *instance = new UserSession;
        return *instance;
#else
        static UserSession instance;
        return instance;
#endif
    }

    /// <summary>
    /// When user logs in, we will initiate an async call to the server to fetch user data
    /// </summary>
    void OnLoggedIn(const char *response);
    /// <summary>
    /// Flush all active data from memory
    /// </summary>
    void OnLoggedOut();

    /// <summary>
    /// Returns the actively logged in UserId
    /// </summary>
    int GetUserId() { return m_dUserId; }
    void SetUserId(int newId) { m_dUserId = newId; }

    int GetOrganizationId() { return m_dOrganizationId; }
    void SetOrganizationId(int newId) { m_dOrganizationId = newId; }

    int GetRoleId() { return m_dRoleId; }
    void SetRoleId(int newId) { m_dRoleId = newId; }

    const char *GetFirstName() { return m_sFirstName; }
    void SetFirstName(const char* newVal);

    const char* GetLastName() { return m_sLastName; }
    void SetLastName(const char* newVal);

    const char* GetEmail() { return m_sEmail; }
    void SetEmail(const char* newVal);

    const char* GetRole();

    const char* GetJobRole() { return m_sJobRole; }
    void SetJobRole(const char* newVal);

    const char* GetPhoto() { return m_sPhoto; }
    void SetPhoto(const char* newVal);

    // check if we could shut down so that no memory is leaked. This is because managers are singletons
    void DestructorCheckMemLeaks();
private:
    UserSession();
    UserSession(const UserSession&) = delete;
    UserSession& operator=(const UserSession&) = delete;
    void ReInit();

    int m_dUserId;
    int m_dOrganizationId;
    int m_dRoleId;
    char m_sFirstName[MAX_DB_STRING_LENGTH];
    char m_sLastName[MAX_DB_STRING_LENGTH];
    char m_sEmail[MAX_DB_STRING_LENGTH];
    char m_sJobRole[MAX_DB_STRING_LENGTH];
    char m_sPhoto[MAX_DB_STRING_LENGTH];
    char m_dRights[UserRightIds::UR_MAX_VALUE];
};

#define sUserSession UserSession::getInstance()