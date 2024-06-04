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
	WAE_UnknowError,
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
/// TempApiCall to create a new Alert instance DB side
/// </summary>
/// <param name="cb"></param>
/// <returns></returns>
WebApiErrorCodes WebApi_CreateSMSAlert(__int64 OrganizationId, __int64 UserId, __int64 AlertInstanceId);
WebApiErrorCodes WebApi_CreateEmailAlert(__int64 OrganizationId, __int64 UserId, __int64 AlertInstanceId);
