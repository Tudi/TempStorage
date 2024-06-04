#pragma once

// instead of spreading out API URls everywhere, let's summarize them here

enum WebApiErrorCodes : int
{
	WAE_ValueNotInitialized = 0,
	WAE_NoError,
	WAE_CouldNotReachEndpoint,
	WAE_IncompatibleClientVersion,
	WAE_InvalidUserName,
	WAE_InvalidPassword,
	WAE_UnknowLoginError,
	WAE_UnknowLogError,
	WAE_LogClientBanned,
	WAE_RawLatencyInvalidParam,
	WAE_InvalidSession,
	WAE_InvalidUser,
	WAE_InactiveUser,
	WAE_SessionExpired,
	WAE_EmptyResponse,
	WAE_MFATokenRequired,
	WAE_MFATokenInvalid,
};

/// <summary>
/// Shared between UI app and backend app. Make sure to update values everywhere !
/// </summary>
enum SharedErrorCodesWithBackend : int
{
	EC_NO_ERROR = 0,
	EC_BAD_USERNAME = -1,
	EC_BAD_PASSWORD = -2,
	EC_MISSING_STAMP = -3,
	EC_MISSING_HWKEY = -4,
	EC_BANNED = -5,
	EC_MISSING_EMAIL_FIELD = -6,
	EC_INVALID_RECOVERY_EMAIL = -7,
	EC_FAILED_TO_CREATE_DB_ENTRY = -8,
	EC_FAILED_TO_SEND_EMAIL = -9,
	EC_INVALID_SESSION = -10,
	EC_MISSING_PARAMETERS = -11,
	EC_INVALID_USER = -12,
	EC_INACTIVE_USER = -13,
	EC_NO_API_ACCESS = -14,
	EC_SESSION_EXPIRED = -15,
	EC_SESSION_SALT_INVALID = -16,
	EC_REQUIRE_RIGHTS = -17,
	EC_BAD_MFATOKEN = -20,

	EC_MAX_ERROR_CODE = 18 // this is positive while all codes are negative
};

// Get the string representation of a shared error code
const char* GetSharedErrorCodeString(SharedErrorCodesWithBackend ec);

/// <summary>
/// Ask the backend if this specific client should be using some other than defauls backend server
/// Used for load balancing or special clients ( edge servers )
/// </summary>
/// <returns></returns>
WebApiErrorCodes WebApi_PerformAPIRedirect();

/// <summary>
/// Obtain SessionId and salt to be used for API queries
/// </summary>
/// <param name="username"></param>
/// <param name="password"></param>
/// <returns></returns>
WebApiErrorCodes WebApi_LoginUser(const char* username, const char* password, const bool isMFAToken);

/// <summary>
/// Transfer Backend dedicated log messages from this App to backend
/// Crashlogs ?
/// </summary>
/// <param name="stamp"></param>
/// <param name="msg"></param>
/// <returns></returns>
WebApiErrorCodes WebApi_SendLogMessage(unsigned __int64 stamp, const char* msg);

/// <summary>
/// Raw latency is the roundtrip between the edge server and the client application
/// </summary>
/// <param name="out_latency"></param>
/// <returns></returns>
WebApiErrorCodes WebApi_GetRawLatency(int *out_latency);

/// <summary>
/// API latency includes a query to the DB. Measures how "real" time is the Module data
/// </summary>
/// <param name="out_latency"></param>
/// <returns></returns>
WebApiErrorCodes WebApi_GetAPILatency(int* out_latency);

/// <summary>
/// Reset a password request. Backend will mail the recovery email. 
/// If user clicks the password reset link, only then the password will be changed
/// </summary>
/// <param name="email"></param>
/// <returns></returns>
WebApiErrorCodes WebApi_ResetPassReq(const char *email);

/// <summary>
/// Used by UI window to show user related info
/// </summary>
/// <param name="UserId"></param>
/// <param name="cb"></param>
/// <param name="UserData"></param>
/// <returns></returns>
WebApiErrorCodes WebApi_GetUserInfoAsync(int UserId, CURL_AsyncCallback cb, void *UserData);

/// <summary>
/// Update user related info on the backend side. Active user needs to have the rights to update a user
/// </summary>
/// <param name="UserId"></param>
/// <param name="firstName"></param>
/// <param name="lastName"></param>
/// <param name="email"></param>
/// <param name="cb"></param>
/// <param name="UserData"></param>
/// <returns></returns>
WebApiErrorCodes WebApi_SetUserInfoAsync(int UserId,
	const char* firstName,
	const char* lastName,
	const char* email, CURL_AsyncCallback cb, void* UserData);

/// <summary>
/// Change the password of a specific user
/// </summary>
/// <param name="password"></param>
/// <returns></returns>
WebApiErrorCodes WebApi_PasswChangeAsync(const int UserId, const char* password);

/// <summary>
/// Used by UI window to show user related info
/// </summary>
/// <param name="cb"></param>
/// <returns></returns>
WebApiErrorCodes WebApi_GetServerDefinesAsync(CURL_AsyncCallback cb);

/// <summary>
/// Used by UI window to show user accesible locations
/// </summary>
/// <param name="cb"></param>
/// <returns></returns>
WebApiErrorCodes WebApi_GetLocationsAsync(int LocationId, int colFormat, CURL_AsyncCallback cb, void* UserData);

/// <summary>
/// Update location related info on the backend side. Active user needs to have the rights to update a location
/// </summary>
/// <returns></returns>
WebApiErrorCodes WebApi_UpdateLocationAsync(int Id, const char* Name, const char* Description,
	const char* Addr1, const char* Addr2, const char* City, const char* State, const char* CountryRegion,
	const char* Country, const char* Latitude, const char* Longitude, const char* Elevation,
	CURL_AsyncCallback cb, void* UserData);

/// <summary>
/// Delete location
/// </summary>
/// <returns></returns>
WebApiErrorCodes WebApi_DeleteLocationsAsync(int LocationId, CURL_AsyncCallback cb, void* UserData);

/// <summary>
/// Used by UI window to show alerts accesible to user/organization/Location
/// </summary>
/// <param name="cb"></param>
/// <returns></returns>
WebApiErrorCodes WebApi_GetAlertsAsync(int id, int locationId, int limit, int NewerId, CURL_AsyncCallback cb, void* UserData);

/// <summary>
/// Used by UI window to show alerts accesible to user/organization/Location
/// </summary>
/// <param name="cb"></param>
/// <returns></returns>
WebApiErrorCodes WebApi_GetActivityLogsAsync(int UserId, int RowCountLimit, CURL_AsyncCallback cb, void* UserData);

/// <summary>
/// Used by UI window to show modules accesible to organization
/// </summary>
/// <param name="cb"></param>
/// <returns></returns>
WebApiErrorCodes WebApi_GetOrganizationModulesAsync(int OrganizationId, int RowCountLimit, CURL_AsyncCallback cb, void* UserData);
WebApiErrorCodes WebApi_GetModuleInstancesAsync(int RowCountLimit, CURL_AsyncCallback cb, void* UserData);

/// <summary>
/// When the user changes the location ID of an OrganizationModule
/// </summary>
/// <param name="cb"></param>
/// <returns></returns>
WebApiErrorCodes WebApi_SetModuleLocationAsync(int ModuleInstanceId, int LocationId, CURL_AsyncCallback cb, void* UserData);

/// <summary>
/// Admin adds the module to a user account
/// </summary>
/// <param name="cb"></param>
/// <returns></returns>
WebApiErrorCodes WebApi_CreateOrgModuleAsync(int OrganizationId, int ModuleInstanceID, CURL_AsyncCallback cb, void* UserData);

/// <summary>
/// Admin adds the module to a user account
/// </summary>
/// <param name="cb"></param>
/// <returns></returns>
WebApiErrorCodes WebApi_DeleteOrgModuleAsync(int OrganizationId, int ModuleInstanceID, CURL_AsyncCallback cb, void* UserData);

/// <summary>
/// KPI data is computer specific user experience
/// </summary>
/// <param name="cb"></param>
/// <returns></returns>
WebApiErrorCodes WebApi_SaveKPIAsync(const char *KPIJson, CURL_AsyncCallback cb, void* UserData);

/// <summary>
/// KPI data is computer specific user experience
/// </summary>
/// <param name="cb"></param>
/// <returns></returns>
WebApiErrorCodes WebApi_SaveCrashLogAsync(const char* CallStack, CURL_AsyncCallback cb, void* UserData);

/// <summary>
/// KPI data is computer specific user experience
/// </summary>
/// <param name="cb"></param>
/// <returns></returns>
WebApiErrorCodes WebApi_GetDPSListAsync(int OrganizationId, CURL_AsyncCallback cb, void* UserData);

/// <summary>
/// TempApiCall to create a new Alert instance DB side
/// </summary>
/// <param name="cb"></param>
/// <returns></returns>
WebApiErrorCodes WebApi_CreateAlert(int OrganizationId, int UserId, int ModuleInstanceId, CURL_AsyncCallback cb, void* UserData);
WebApiErrorCodes WebApi_CreateSMSAlert(int AlertInstanceId, CURL_AsyncCallback cb, void* UserData);
WebApiErrorCodes WebApi_CreateEmailAlert(int AlertInstanceId, CURL_AsyncCallback cb, void* UserData);
WebApiErrorCodes WebApi_GetDemoRadarDataAsync(CURL_AsyncCallback cb, void* UserData);
