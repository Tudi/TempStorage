#include "StdAfx.h"

UserSession::UserSession()
{
    ReInit();
}

void UserSession::DestructorCheckMemLeaks()
{
#ifdef _DEBUG
    delete &sUserSession;
#endif
}

void UserSession::ReInit()
{
    m_dUserId = 0;
    m_dOrganizationId = 0;
    m_dRoleId = 0;
    m_sFirstName[0] = 0;
    m_sLastName[0] = 0;
    m_sEmail[0] = 0;
    m_sPhoto[0] = 0;
    memset(m_dRights, 0, sizeof(m_dRights));
}

void UserSession::SetFirstName(const char* newVal)
{
    if (newVal == NULL)
    {
        strcpy_s(m_sFirstName, "");
    }
    else
    {
        strcpy_s(m_sFirstName, newVal);
    }
}

void UserSession::SetLastName(const char* newVal)
{
    if (newVal == NULL)
    {
        strcpy_s(m_sLastName, "");
    }
    else
    {
        strcpy_s(m_sLastName, newVal);
    }
}

void UserSession::SetEmail(const char* newVal)
{
    if (newVal == NULL)
    {
        strcpy_s(m_sEmail, "");
    }
    else
    {
        strcpy_s(m_sEmail, newVal);
    }
}

void UserSession::SetJobRole(const char* newVal)
{
    if (newVal == NULL)
    {
        strcpy_s(m_sJobRole, "");
    }
    else
    {
        strcpy_s(m_sJobRole, newVal);
    }
}

void UserSession::SetPhoto(const char* newVal)
{
    if (newVal == NULL)
    {
        strcpy_s(m_sPhoto, "");
    }
    else
    {
        strcpy_s(m_sPhoto, newVal);
    }
}


void CB_UserDataArrived(int CurlErr, char* response, void* userData)
{
    userData;
    CURLcode err = (CURLcode)CurlErr;
    if (err != CURLE_OK || response == NULL)
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceUserManagement, 0, 0,
            "UserManagement:Failed to get user info. ");
        return;
    }

    //generic parser
    sUserSession.OnLoggedIn(response);
}

void UserSession::OnLoggedIn(const char* response)
{
    // in case we want to fetch data from the server
    if (response == NULL)
    {
        WebApi_GetUserInfoAsync(GetUserId(), CB_UserDataArrived, NULL);
        return;
    }
    // parse the response
    // empty response string ?
    if (response[0] == 0)
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceUserManagement, 0, 0,
            "UserManagement:Failed to get user info. ");
        return;
    }

    // parse response
    nlohmann::json jsonResponse = nlohmann::json::parse(response, nullptr, false);

    // check for errors
    if (jsonResponse.contains("ErrorId") && jsonResponse["ErrorId"].is_number_integer())
    {
        int errorCode = jsonResponse["ErrorId"];
        if (errorCode != SharedErrorCodesWithBackend::EC_NO_ERROR)
        {
            AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceUserManagement, 0, 0,
                "UserManagement:Failed to get user info. Error %d:%s", errorCode, GetSharedErrorCodeString((SharedErrorCodesWithBackend)errorCode));
            return;
        }
    }

    if (jsonResponse.contains("FirstName"))
    {
        sUserSession.SetFirstName(jsonResponse["FirstName"].get<std::string>().c_str());
    }
    if (jsonResponse.contains("LastName"))
    {
        sUserSession.SetLastName(jsonResponse["LastName"].get<std::string>().c_str());
    }
    if (jsonResponse.contains("Email"))
    {
        sUserSession.SetEmail(jsonResponse["Email"].get<std::string>().c_str());
    }
    if (jsonResponse.contains("Photo"))
    {
        sUserSession.SetPhoto(jsonResponse["Photo"].get<std::string>().c_str());
    }
    if (jsonResponse.contains("JobRole"))
    {
        sUserSession.SetJobRole(jsonResponse["JobRole"].get<std::string>().c_str());
    }
    if (jsonResponse.contains("RoleID") && jsonResponse["RoleID"].is_number_integer())
    {
        sUserSession.SetRoleId(jsonResponse["RoleID"]);
    }
    if (jsonResponse.contains("OrganizationID") && jsonResponse["OrganizationID"].is_number_integer())
    {
        sUserSession.SetOrganizationId(jsonResponse["OrganizationID"]);
    }
    if (jsonResponse.contains("UserId") && jsonResponse["UserId"].is_number_integer())
    {
        sUserSession.SetUserId(jsonResponse["UserId"]);
    }
    if (jsonResponse.contains("Rights") && jsonResponse["Rights"].is_array())
    {
        for (const auto& element : jsonResponse["Rights"])
        {
            // Check if the element is an integer
            if (element.is_number_integer())
            {
                // Extract and print the integer value
                int value = element;
                if (value < UserRightIds::UR_MAX_VALUE)
                {
                    m_dRights[value] = 1;
                }
                else
                {
                    AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceUserManagement, 0, 0,
                        "UserManagement:Unexpected right value %d", value);
                }
            }
        }
    }

    AddLogEntry(LogDestinationFlags::LDF_SERVER, LogSeverityValue::LogSeverityNormal, 
        LogSourceGroups::LogSourceUserManagement, 0, 0, "Logged in");

}

void UserSession::OnLoggedOut()
{
    ReInit();
}

const char* UserSession::GetRole()
{ 
    return sLocalization.GetRoleIdString(m_dRoleId); 
}