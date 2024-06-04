#include "StdAfx.h"
#include "../../project/git_info.cpp"

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

void PerformAPIRedirectCallback(int CurlErr, char* response, void* userData)
{
	userData; // suppress warning
	CURLcode err = (CURLcode)CurlErr;
	if (err != CURLE_OK || response == NULL)
	{
		return;
	}

	AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityNormal, LogSourceGroups::LogSourceWebAPI, 0, 0,
		"API:API endpoint redirection value '%s'", response);

	// maybe we should check if this url is reachable ?
	if (std::string(response).find(std::string("http:/")) != std::string::npos)
	{
		// set our new API endpoint
		sConfigManager.UpdateString(API_Endpoint_URL, response);
	}
}

WebApiErrorCodes WebApi_PerformAPIRedirect()
{
	std::string apiEndpoint = sConfigManager.GetString(API_Endpoint_URL) + std::string("GetAPIRedirection.php");

	AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityNormal, LogSourceGroups::LogSourceWebAPI, 0, 0,
		"API:Fetching load balanced API endpoint from balancer : %s", apiEndpoint.c_str());

	sKPI.IncreaseAPICallCount();
	GetURLResponseAsync(apiEndpoint.c_str(), NULL, PerformAPIRedirectCallback, NULL);

	return WebApiErrorCodes::WAE_NoError;
}

WebApiErrorCodes WebApi_LoginUser(const char* username, const char* password, const bool isMFAToken)
{
	unsigned int saltInit = (int)time(NULL);

	// always send HW key
	const char* sHWKey = LicensingGetFingerprintString(true);
	char License[1] = { 0 };

	// pack data to be sent
	CURL* curl = curl_easy_init();
	std::string postFields;
	curl_easy_AddField(postFields, "AppVer=", curl, APPLICATION_VERSION);
	curl_easy_AddField(postFields, "&ClientEndpoint=", curl, sHWKey);
	curl_easy_AddField(postFields, "&uname=", curl, username);
	if (isMFAToken == false)
	{
		// obfuscate passw, just in case it gets logged by someone
		unsigned int passwHash1 = crc31_hash(0xFEED, password, strlen(password));
		unsigned int passwHash2 = crc31_hash(0xBABE, password, strlen(password));
		char passwHashed[50];
		sprintf_s(passwHashed, "%u%u", passwHash1, passwHash2);
		curl_easy_AddField(postFields, "&pword=", curl, passwHashed);
	}
	else
	{
		curl_easy_AddField(postFields, "&token=", curl, password);
	}
	curl_easy_AddField(postFields, "&lic=", curl, License);
	postFields += "&stamp=" + std::to_string(time(NULL));
	curl_easy_cleanup(curl);

	CURLcode err;
	std::string apiEndpoint = sConfigManager.GetString(API_Endpoint_URL);
	if (isMFAToken == false)
	{
		apiEndpoint += std::string("Login.php");
	}
	else
	{
		apiEndpoint += std::string("LoginMFA.php");
	}

	AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityNormal, LogSourceGroups::LogSourceWebAPI, 0, 0,
		"API:Trying to log in user %s at %s", username, apiEndpoint.c_str());

	sKPI.IncreaseAPICallCount();
	char *resp = GetURLResponse(apiEndpoint.c_str(), postFields.c_str(), &err);
	if (err != CURLE_OK || resp == NULL)
	{
		InternalFree(resp);
		return WebApiErrorCodes::WAE_CouldNotReachEndpoint;
	}
	// empty response string ? unexpected
	if (resp[0] == 0)
	{
		InternalFree(resp);
		return WebApiErrorCodes::WAE_UnknowLoginError;
	}

	// deobfuscate response

	// parse response
	nlohmann::json jsonResponse = nlohmann::json::parse(resp, nullptr, false, true);
	
	// check for errors
	if (jsonResponse.contains("ErrorId") && jsonResponse["ErrorId"].is_number_integer())
	{
		int errorCode = jsonResponse["ErrorId"];
		if (errorCode == SharedErrorCodesWithBackend::EC_BAD_USERNAME)
		{
			InternalFree(resp);
			return WebApiErrorCodes::WAE_InvalidUserName;
		}
		if (errorCode == SharedErrorCodesWithBackend::EC_BAD_PASSWORD)
		{
			InternalFree(resp);
			return WebApiErrorCodes::WAE_InvalidPassword;
		}
		if (errorCode == SharedErrorCodesWithBackend::EC_BAD_MFATOKEN)
		{
			InternalFree(resp);
			return WebApiErrorCodes::WAE_MFATokenInvalid;
		}
		if (errorCode != 0)
		{
			InternalFree(resp);
			return WebApiErrorCodes::WAE_UnknowLoginError;
		}
	}

	// this is the time difference between client and server stamp
	if (jsonResponse.contains("stamp") && jsonResponse["stamp"].is_number_integer())
	{
		__int64 lldSessionSalt = jsonResponse["stamp"];
		unsigned int saltEnd = (int)time(NULL);
		lldSessionSalt = (saltEnd - saltInit) / 2 + (lldSessionSalt - saltEnd);
		sAppSession.SessionSaltUpdate(lldSessionSalt);
	}

	// this means we will need a secondary password to be called
	if (jsonResponse.contains("MFAType"))
	{
		std::string szMFAType = jsonResponse["MFAType"];
		unsigned __int64 lldMfaType = atoll(szMFAType.c_str());
		if (lldMfaType == 2 || lldMfaType == 3)
		{
			InternalFree(resp);
			return WebApiErrorCodes::WAE_MFATokenRequired;
		}
	}

	if (jsonResponse.contains("SessionId"))
	{
		std::string szSessionId = jsonResponse["SessionId"];
		unsigned __int64 lldSessionId = atoll(szSessionId.c_str());
		sAppSession.SessionIdInit(lldSessionId);
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceWebAPI, 0, 0,
			"API:Obtained session id %lld", sAppSession.SessionIdGet());
	}
	else
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceWebAPI, 0, 0,
			"API:resp : %s", resp);
		InternalFree(resp);
		return WebApiErrorCodes::WAE_UnknowLoginError;
	}

	if (jsonResponse.contains("UserId") && jsonResponse["UserId"].is_number_integer())
	{
		sConfigManager.SetActiveUsername(username);
		sUserSession.OnLoggedIn(resp);
		sWindowManager.OnUserLoggedIn();
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceWebAPI, 0, 0,
			"API:Obtained user id %lld", sUserSession.GetUserId());
	}
	else
	{
		InternalFree(resp);
		return WebApiErrorCodes::WAE_UnknowLoginError;
	}

	// cleanup
	InternalFree(resp);

	// return all good
	return WebApiErrorCodes::WAE_NoError;
}

#define EXPAND(x) x

#define CREATE_GENERIC_CURL_POST_WITH_FIELDS const char* sHWKey = LicensingGetFingerprintString(true); \
	CURL* curl = curl_easy_init(); \
	std::string postFields; \
	curl_easy_AddField(postFields, "AppVer=", curl, APPLICATION_VERSION); \
	curl_easy_AddField(postFields, "&ClientEndpoint=", curl, sHWKey); \
	postFields += "&SessionKey=" + std::to_string(sAppSession.SessionIdGet()); \
	postFields += "&SessionSalt=" + std::to_string(sAppSession.SessionSaltGet());

#define CLEANUP_GENERIC_CURL_ASYNC_CALLS(Endpoint, ErrStr) 	curl_easy_cleanup(curl); \
	std::string apiEndpoint = sConfigManager.GetString(API_Endpoint_URL) + std::string(Endpoint); \
	AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceWebAPI, 0, 0, \
		ErrStr, apiEndpoint.c_str()); \
	sKPI.IncreaseAPICallCount(); \
	GetURLResponseAsync(apiEndpoint.c_str(), postFields.c_str(), cb, UserData); \
	return WebApiErrorCodes::WAE_NoError;

#define CLEANUP_GENERIC_CURL_ASYNC_CALLV(Endpoint, ErrStr, ...) 	curl_easy_cleanup(curl); \
	std::string apiEndpoint = sConfigManager.GetString(API_Endpoint_URL) + std::string(Endpoint); \
	AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceWebAPI, 0, 0, \
		ErrStr, __VA_ARGS__, apiEndpoint.c_str()); \
	sKPI.IncreaseAPICallCount(); \
	GetURLResponseAsync(apiEndpoint.c_str(), postFields.c_str(), cb, UserData); \
	return WebApiErrorCodes::WAE_NoError; 

WebApiErrorCodes WebApi_SendLogMessage(unsigned __int64 stamp, const char* msg)
{
	CREATE_GENERIC_CURL_POST_WITH_FIELDS;

	curl_easy_AddField(postFields, "&LogVer=", curl, LOG_FORMAT_VERSION_INFO);
	curl_easy_AddField(postFields, "&msg=", curl, msg);
	postFields += "&stamp=" + std::to_string(stamp);
	postFields += "&source=" + std::to_string(LogSourceTypeVSSUI);

	curl_easy_cleanup(curl);

	CURLcode err;
	std::string apiEndpoint = sConfigManager.GetString(API_Endpoint_URL) + std::string("AddLog.php");

	AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceWebAPI, 0, 0,
		"API:Trying to send log %s to %s", msg, apiEndpoint.c_str());

	sKPI.IncreaseAPICallCount();
	char* resp = GetURLResponse(apiEndpoint.c_str(), postFields.c_str(), &err);

	yyJSON(yydoc);
	WebApiErrorCodes errCheckRes = ExtractDBColumnToBinary::DBH_APIResultValid(err, resp, yydoc, LogSourceGroups::LogSourceWebAPI, __FUNCTION__);
	if (errCheckRes != WebApiErrorCodes::WAE_NoError && errCheckRes != WebApiErrorCodes::WAE_EmptyResponse)
	{
		InternalFree(resp);
		return WebApiErrorCodes::WAE_UnknowLogError;
	}

	// cleanup
	InternalFree(resp);

	// return all good
	return WebApiErrorCodes::WAE_NoError;
}

WebApiErrorCodes WebApi_GetRawLatency(int* out_latency)
{
	if (out_latency == NULL)
	{
		return WebApiErrorCodes::WAE_RawLatencyInvalidParam;
	}
	ULONGLONG startStamp = GetTickCount64();

	CURLcode err;
	std::string apiEndpoint = sConfigManager.GetString(API_Endpoint_URL) + std::string("GetLatencyRaw.php");

	AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceWebAPI, 0, 0,
		"API:Trying to measure raw latency at %s", apiEndpoint.c_str());

	sKPI.IncreaseAPICallCount();
	char* resp = GetURLResponse(apiEndpoint.c_str(), NULL, &err);
	if (err != CURLE_OK || resp == NULL)
	{
		InternalFree(resp);
		return WebApiErrorCodes::WAE_CouldNotReachEndpoint;
	}

	ULONGLONG endStamp = GetTickCount64();
	*out_latency = (int)(endStamp - startStamp);

	// cleanup
	InternalFree(resp);

	// return all good
	return WebApiErrorCodes::WAE_NoError;
}

WebApiErrorCodes WebApi_GetAPILatency(int* out_latency)
{
	if (out_latency == NULL)
	{
		return WebApiErrorCodes::WAE_RawLatencyInvalidParam;
	}
	ULONGLONG startStamp = GetTickCount64();

	CREATE_GENERIC_CURL_POST_WITH_FIELDS;

	postFields += "&stamp=" + std::to_string(time(NULL));

	curl_easy_cleanup(curl);

	CURLcode err;
	std::string apiEndpoint = sConfigManager.GetString(API_Endpoint_URL) + std::string("GetLatencyAPI.php");

	AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceWebAPI, 0, 0,
		"API:Trying to measure API latency at %s", apiEndpoint.c_str());

	sKPI.IncreaseAPICallCount();
	char* resp = GetURLResponse(apiEndpoint.c_str(), postFields.c_str(), &err);

	yyJSON(yydoc);
	if (ExtractDBColumnToBinary::DBH_APIResultValid(err, resp, yydoc, LogSourceGroups::LogSourceWebAPI, __FUNCTION__) != WebApiErrorCodes::WAE_NoError)
	{
		InternalFree(resp);
		return WebApiErrorCodes::WAE_UnknowLogError;
	}

	ULONGLONG endStamp = GetTickCount64();
	*out_latency = (int)(endStamp - startStamp);

	// cleanup
	InternalFree(resp);

	// return all good
	return WebApiErrorCodes::WAE_NoError;
}

WebApiErrorCodes WebApi_ResetPassReq(const char* email)
{
	CURL_AsyncCallback cb = NULL;
	void* UserData = NULL;
	CREATE_GENERIC_CURL_POST_WITH_FIELDS;

	curl_easy_AddField(postFields, "AppVer=", curl, APPLICATION_VERSION);
	curl_easy_AddField(postFields, "&ClientEndpoint=", curl, sHWKey);
	curl_easy_AddField(postFields, "&email=", curl, email);

	CLEANUP_GENERIC_CURL_ASYNC_CALLS("RstPasswReq.php", "API:Passw reset req at %s");
}

WebApiErrorCodes WebApi_GetUserInfoAsync(int UserId, CURL_AsyncCallback cb, void* UserData)
{
	CREATE_GENERIC_CURL_POST_WITH_FIELDS;

	postFields += "&TargetUserId=" + std::to_string(UserId);

	CLEANUP_GENERIC_CURL_ASYNC_CALLS("GetUserInfo.php", "API:Get user info at %s");
}

WebApiErrorCodes WebApi_SetUserInfoAsync(int UserId, 
	const char * FirstName,
	const char * LastName,
	const char * Email, CURL_AsyncCallback cb, void* UserData)
{
	CREATE_GENERIC_CURL_POST_WITH_FIELDS;

	postFields += "&TargetUserId=" + std::to_string(UserId);
	curl_easy_AddField(postFields, "&FirstName=", curl, FirstName);
	curl_easy_AddField(postFields, "&LastName=", curl, LastName);
	curl_easy_AddField(postFields, "&Email=", curl, Email);

	CLEANUP_GENERIC_CURL_ASYNC_CALLS("SetUserInfo.php", "API:Set user info at %s");
}

WebApiErrorCodes WebApi_PasswChangeAsync(const int UserId, const char* password)
{
	// obfuscate passw, just in case it gets logged by someone
	unsigned int passwHash1 = crc31_hash(0xFEED, password, strlen(password));
	unsigned int passwHash2 = crc31_hash(0xBABE, password, strlen(password));
	char passwHashed[50];
	sprintf_s(passwHashed, "%u%u", passwHash1, passwHash2);
	CURL_AsyncCallback cb = NULL;
	void* UserData = NULL;

	CREATE_GENERIC_CURL_POST_WITH_FIELDS;

	postFields += "&TargetUserId=" + std::to_string(UserId);
	curl_easy_AddField(postFields, "&NewPssw=", curl, passwHashed);

	CLEANUP_GENERIC_CURL_ASYNC_CALLV("SetPassw.php", "Changed user %d passw to %s using %s", UserId, password);
}

WebApiErrorCodes WebApi_GetServerDefinesAsync(CURL_AsyncCallback cb)
{
	void* UserData = NULL;
	const char* sLang = sConfigManager.GetString(ConfigOptionIds::Language);

	CREATE_GENERIC_CURL_POST_WITH_FIELDS;

	curl_easy_AddField(postFields, "&Lang=", curl, sLang);

	CLEANUP_GENERIC_CURL_ASYNC_CALLS("GetStringDefines.php", "Fetching string defines from %s");
}

WebApiErrorCodes WebApi_GetLocationsAsync(int LocationId, int colFormat, CURL_AsyncCallback cb, void* UserData)
{
	CREATE_GENERIC_CURL_POST_WITH_FIELDS;

	postFields += "&TargetId=" + std::to_string(LocationId);
	postFields += "&ResultFormat=" + std::to_string(colFormat);

	CLEANUP_GENERIC_CURL_ASYNC_CALLS("GetLocations.php", "Fetching locations from %s");
}

WebApiErrorCodes WebApi_UpdateLocationAsync(int Id, const char* Name, const char* Description,
	const char* Addr1, const char* Addr2, const char* City, const char* State, const char* CountryRegion,
	const char* Country, const char* Latitude, const char* Longitude, const char* Elevation,
	CURL_AsyncCallback cb, void* UserData)
{
	CREATE_GENERIC_CURL_POST_WITH_FIELDS;

	postFields += "&LocationID=" + std::to_string(Id);
	curl_easy_AddField(postFields, "&LocationName=", curl, Name);
	curl_easy_AddField(postFields, "&LocationDescription=", curl, Description);
	curl_easy_AddField(postFields, "&LocationAddressLine1=", curl, Addr1);
	curl_easy_AddField(postFields, "&LocationAddressLine2=", curl, Addr2);
	curl_easy_AddField(postFields, "&LocationCity=", curl, City);
	curl_easy_AddField(postFields, "&LocationState=", curl, State);
	curl_easy_AddField(postFields, "&LocationCountyOrRegion=", curl, CountryRegion);
	curl_easy_AddField(postFields, "&LocationCountry=", curl, Country);
	curl_easy_AddField(postFields, "&LocationX=", curl, Latitude);
	curl_easy_AddField(postFields, "&LocationY=", curl, Longitude);
	curl_easy_AddField(postFields, "&LocationZ=", curl, Elevation);
	postFields += "&OrganizationID=" + std::to_string(sUserSession.GetOrganizationId());

	CLEANUP_GENERIC_CURL_ASYNC_CALLS("SetLocation.php", "Update location info at %s");
}

WebApiErrorCodes WebApi_DeleteLocationsAsync(int LocationId, CURL_AsyncCallback cb, void* UserData)
{
	CREATE_GENERIC_CURL_POST_WITH_FIELDS;

	postFields += "&TargetId=" + std::to_string(LocationId);

	CLEANUP_GENERIC_CURL_ASYNC_CALLS("DeleteLocations.php", "Delete location using %s");
}

WebApiErrorCodes WebApi_GetAlertsAsync(int id, int locationId, int limit, int NewerId
	, CURL_AsyncCallback cb, void* UserData)
{
	CREATE_GENERIC_CURL_POST_WITH_FIELDS;

	postFields += "&AlertId=" + std::to_string(id);
	postFields += "&LocationId=" + std::to_string(locationId);
	postFields += "&RowLimit=" + std::to_string(limit);
	postFields += "&NewerId=" + std::to_string(NewerId);

	CLEANUP_GENERIC_CURL_ASYNC_CALLS("GetAlerts.php", "API:Fetching alerts from %s");
}

WebApiErrorCodes WebApi_GetActivityLogsAsync(int UserId, int RowCountLimit, CURL_AsyncCallback cb, void* UserData)
{
	CREATE_GENERIC_CURL_POST_WITH_FIELDS;

	postFields += "&UserId=" + std::to_string(UserId);
	postFields += "&RowLimit=" + std::to_string(RowCountLimit);

	CLEANUP_GENERIC_CURL_ASYNC_CALLS("GetActivity.php", "API:Fetching activity logs from %s");
}

WebApiErrorCodes WebApi_GetOrganizationModulesAsync(int OrganizationId, int RowCountLimit, CURL_AsyncCallback cb, void* UserData)
{
	CREATE_GENERIC_CURL_POST_WITH_FIELDS;

	postFields += "&OrganizationId=" + std::to_string(OrganizationId);
	postFields += "&RowLimit=" + std::to_string(RowCountLimit);

	CLEANUP_GENERIC_CURL_ASYNC_CALLS("GetOrganizationModules.php", "API:Fetching Organization Modules from %s");
}

WebApiErrorCodes WebApi_GetModuleInstancesAsync(int RowCountLimit, CURL_AsyncCallback cb, void* UserData)
{
	CREATE_GENERIC_CURL_POST_WITH_FIELDS;

	postFields += "&RowLimit=" + std::to_string(RowCountLimit);

	CLEANUP_GENERIC_CURL_ASYNC_CALLS("GetModuleInstances.php", "API:Fetching Module Instances from %s");
}

WebApiErrorCodes WebApi_SetModuleLocationAsync(int ModuleInstanceId, int LocationId, CURL_AsyncCallback cb, void* UserData)
{
	CREATE_GENERIC_CURL_POST_WITH_FIELDS;

	postFields += "&ModuleInstanceID=" + std::to_string(ModuleInstanceId);
	postFields += "&LocationID=" + std::to_string(LocationId);

	CLEANUP_GENERIC_CURL_ASYNC_CALLS("SetModuleInstanceLocation.php", "API:Set module location using %s");
}

WebApiErrorCodes WebApi_CreateOrgModuleAsync(int OrganizationId, int ModuleInstanceID, CURL_AsyncCallback cb, void* UserData)
{
	CREATE_GENERIC_CURL_POST_WITH_FIELDS;

	postFields += "&OrganizationID=" + std::to_string(OrganizationId);
	postFields += "&ModuleInstanceID=" + std::to_string(ModuleInstanceID);

	CLEANUP_GENERIC_CURL_ASYNC_CALLS("CreateOrganizationModule.php", "API:Created organization module using %s");
}

WebApiErrorCodes WebApi_DeleteOrgModuleAsync(int OrganizationId, int ModuleInstanceID, CURL_AsyncCallback cb, void* UserData)
{
	CREATE_GENERIC_CURL_POST_WITH_FIELDS;

	postFields += "&OrganizationID=" + std::to_string(OrganizationId);
	postFields += "&ModuleInstanceID=" + std::to_string(ModuleInstanceID);

	CLEANUP_GENERIC_CURL_ASYNC_CALLS("DeleteOrganizationModule.php", "API:Deleted OrganizationModule using %s");
}

WebApiErrorCodes WebApi_CreateAlert(int OrganizationId, int UserId, int ModuleInstanceId, CURL_AsyncCallback cb, void* UserData)
{
	CREATE_GENERIC_CURL_POST_WITH_FIELDS;

	postFields += "&OrganizationID=" + std::to_string(OrganizationId);
	postFields += "&UserID=" + std::to_string(UserId);
	postFields += "&ModuleInstanceID=" + std::to_string(ModuleInstanceId);

	CLEANUP_GENERIC_CURL_ASYNC_CALLS("CreateAlert.php", "API:Created Alert using %s");
}

WebApiErrorCodes WebApi_CreateSMSAlert(int AlertInstanceId, CURL_AsyncCallback cb, void* UserData)
{
	CREATE_GENERIC_CURL_POST_WITH_FIELDS;

	postFields += "&AlertID=" + std::to_string(AlertInstanceId);
	postFields += "&OrganizationID=" + std::to_string(sUserSession.GetOrganizationId());
	postFields += "&UserID=" + std::to_string(sUserSession.GetUserId());

	CLEANUP_GENERIC_CURL_ASYNC_CALLS("CreateSMSNotification.php", "API:Created SMS Alert notification using %s");
}

WebApiErrorCodes WebApi_CreateEmailAlert(int AlertInstanceId, CURL_AsyncCallback cb, void* UserData)
{
	CREATE_GENERIC_CURL_POST_WITH_FIELDS;

	postFields += "&AlertID=" + std::to_string(AlertInstanceId);
	postFields += "&OrganizationID=" + std::to_string(sUserSession.GetOrganizationId());
	postFields += "&UserID=" + std::to_string(sUserSession.GetUserId());

	CLEANUP_GENERIC_CURL_ASYNC_CALLS("CreateEmailNotification.php", "API:Created Email Alert notification using %s");
}

WebApiErrorCodes WebApi_GetDemoRadarDataAsync(CURL_AsyncCallback cb, void* UserData)
{
	CREATE_GENERIC_CURL_POST_WITH_FIELDS;
	CLEANUP_GENERIC_CURL_ASYNC_CALLS("GetDemoRadarFeed.php", "API:Fetching demo radar data from %s");
}

WebApiErrorCodes WebApi_SaveKPIAsync(const char* KPIJson, CURL_AsyncCallback cb, void* UserData)
{
	CREATE_GENERIC_CURL_POST_WITH_FIELDS;

	curl_easy_AddField(postFields, "&KPIData=", curl, KPIJson);

	CLEANUP_GENERIC_CURL_ASYNC_CALLS("SetKPI.php", "API:Saved KPI data at %s");
}

WebApiErrorCodes WebApi_SaveCrashLogAsync(const char* CallStack, CURL_AsyncCallback cb, void* UserData)
{
	CREATE_GENERIC_CURL_POST_WITH_FIELDS;

	curl_easy_AddField(postFields, "&CallStack=", curl, CallStack);
	curl_easy_AddField(postFields, "&Rev=", curl, GIT_HASH);

	CLEANUP_GENERIC_CURL_ASYNC_CALLS("SetCrashLog.php", "API:Saved crash callstack at %s");
}

WebApiErrorCodes WebApi_GetDPSListAsync(int OrganizationId, CURL_AsyncCallback cb, void* UserData)
{
	CREATE_GENERIC_CURL_POST_WITH_FIELDS;

	postFields += "&OrganizationId=" + std::to_string(OrganizationId);

	CLEANUP_GENERIC_CURL_ASYNC_CALLS("GetRadarDataSources.php", "API:Fetched DPS list from %s");
}