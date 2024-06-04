#include <string>
#include "../../project/git_info.cpp"
#include "curl/include/curl/curl.h"
#include "CurlInterface.h"
#include "WebApiInterface.h"
#include "ResourceManager/LogManager.h"
#include "Util/InitFileHandler.h"
#include "ResourceManager/ConfigManager.h"
#include "Session/ApplicationSession.h"
#include "Util/Allocator.h"

const char* GetSharedErrorCodeString(SharedErrorCodesWithBackend ec)
{
	// one time intialization of the array of strings
	static const char* ErrorCodeStrings[EC_MAX_ERROR_CODE] = { 0 };
	if (ErrorCodeStrings[EC_NO_ERROR] == NULL)
	{
		ErrorCodeStrings[EC_NO_ERROR] = "No error";
		ErrorCodeStrings[-EC_BAD_USERNAME] = "Bad Username";
		ErrorCodeStrings[-EC_BAD_PASSWORD] = "Bad username";
		ErrorCodeStrings[-EC_MISSING_STAMP] = "Missing stamp field";
		ErrorCodeStrings[-EC_MISSING_HWKEY] = "Missing ClientEndpoint field";
		ErrorCodeStrings[-EC_BANNED] = "User is banned";
		ErrorCodeStrings[-EC_MISSING_EMAIL_FIELD] = "Missing email field";
		ErrorCodeStrings[-EC_INVALID_RECOVERY_EMAIL] = "Recovery email contains invalid value";
		ErrorCodeStrings[-EC_FAILED_TO_CREATE_DB_ENTRY] = "Failed to create DB entry";
		ErrorCodeStrings[-EC_FAILED_TO_SEND_EMAIL] = "Failed to send email";
		ErrorCodeStrings[-EC_INVALID_SESSION] = "Invalid session";
		ErrorCodeStrings[-EC_MISSING_PARAMETERS] = "Missing parameter(s)";
		ErrorCodeStrings[-EC_INVALID_USER] = "Invalid user";
		ErrorCodeStrings[-EC_INACTIVE_USER] = "Inactive user";
		ErrorCodeStrings[-EC_NO_API_ACCESS] = "No access";
		ErrorCodeStrings[-EC_SESSION_EXPIRED] = "Session expired";
		ErrorCodeStrings[-EC_SESSION_SALT_INVALID] = "Session salt invalid";
		ErrorCodeStrings[-EC_REQUIRE_RIGHTS] = "Not enough rights";
	}

	// fetch the string for the error code
	int index = -ec;
	if (index < EC_MAX_ERROR_CODE && ErrorCodeStrings[index] != NULL)
	{
		return ErrorCodeStrings[index];
	}

	// unexpected : We are missing string value for this error code
	AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityNormal, LogSourceGroups::LogSourceWebAPI, 0, 0,
		"API:Missing string representation for shared error code %d", ec);

	return "";
}

static void curl_easy_AddField(std::string &str, const char *field, CURL* handle, const char* string)
{
	// sanity checks
	if (field == NULL || string == NULL || handle == NULL)
	{
		return;
	}
	// default way of adding a curl field
	char* ret = curl_easy_escape(handle, string, (int)strlen(string));
	if (ret)
	{
		str += field;
		str += ret;
		curl_free(ret);
	}
	else
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceWebAPI, 0, 0,
			"API:Failed to CURL escape string %s", string);
	}
}

#define EXPAND(x) x

#define CREATE_GENERIC_CURL_POST_WITH_FIELDS const char* sHWKey = ""; \
	CURL* curl = curl_easy_init(); \
	std::string postFields; \
	curl_easy_AddField(postFields, "AppVer=", curl, APPLICATION_VERSION); \
	curl_easy_AddField(postFields, "&ClientEndpoint=", curl, sHWKey); \
	postFields += "&SessionKey=" + std::to_string(sAppSession.SessionIdGet()); \
	postFields += "&SessionSalt=" + std::to_string(sAppSession.SessionSaltGet());

#define CLEANUP_GENERIC_CURL_ASYNC_CALL(Endpoint, ErrStr, ...) 	curl_easy_cleanup(curl); \
	std::string apiEndpoint = sConfigManager.GetString(API_Endpoint_URL) + std::string(Endpoint); \
	AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceWebAPI, 0, 0, \
		ErrStr, EXPAND(__VA_ARGS__), apiEndpoint.c_str()); \
	GetURLResponseAsync(apiEndpoint.c_str(), postFields.c_str(), cb, UserData); \
	return WebApiErrorCodes::WAE_NoError; 

#define DO_GENERIC_CURL_CALL(Endpoint, ErrStr) \
	std::string apiEndpoint = sConfigManager.GetString(API_Endpoint_URL) + std::string(Endpoint); \
	AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceWebAPI, 0, 0, \
		ErrStr, apiEndpoint.c_str()); \
	CURLcode err; \
	char* resp = GetURLResponse(apiEndpoint.c_str(), postFields.c_str(), &err); \
	curl_easy_cleanup(curl); \
	if (err != CURLE_OK || resp == NULL) \
	{ \
		InternalFree(resp); \
		return WebApiErrorCodes::WAE_CouldNotReachEndpoint; \
	} \
	if (resp[0] == 0) \
	{ \
		InternalFree(resp); \
		return WebApiErrorCodes::WAE_EmptyResponse; \
	} 

WebApiErrorCodes WebApi_CreateSMSAlert(__int64 OrganizationId, __int64 UserId, __int64 AlertInstanceId)
{
	CREATE_GENERIC_CURL_POST_WITH_FIELDS;

	postFields += "&AlertID=" + std::to_string(AlertInstanceId);
	postFields += "&OrganizationID=" + std::to_string(OrganizationId);
	postFields += "&UserID=" + std::to_string(UserId);
	postFields += "&ThisIsADPSCall=" + std::to_string(AlertInstanceId);

	DO_GENERIC_CURL_CALL("CreateSMSNotification.php", "API:Created SMS Alert notification using %s");

	if (strstr(resp, "ErrorId") != NULL)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceWebAPI, 0, 0, 
			"API %s call returned %s", apiEndpoint.c_str(), resp);

		InternalFree(resp);
		return WebApiErrorCodes::WAE_UnknowError;
	}

	InternalFree(resp);
	return WebApiErrorCodes::WAE_NoError;
}

WebApiErrorCodes WebApi_CreateEmailAlert(__int64 OrganizationId, __int64 UserId, __int64 AlertInstanceId)
{
	CREATE_GENERIC_CURL_POST_WITH_FIELDS;

	postFields += "&AlertID=" + std::to_string(AlertInstanceId);
	postFields += "&OrganizationID=" + std::to_string(OrganizationId);
	postFields += "&UserID=" + std::to_string(UserId);
	postFields += "&ThisIsADPSCall=" + std::to_string(AlertInstanceId);

	DO_GENERIC_CURL_CALL("CreateEmailNotification.php", "API:Created Email Alert notification using %s");

	if (strstr(resp, "ErrorId") != NULL)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceWebAPI, 0, 0,
			"API %s call returned %s", apiEndpoint.c_str(), resp);

		InternalFree(resp);
		return WebApiErrorCodes::WAE_UnknowError;
	}

	InternalFree(resp);
	return WebApiErrorCodes::WAE_NoError;
}
