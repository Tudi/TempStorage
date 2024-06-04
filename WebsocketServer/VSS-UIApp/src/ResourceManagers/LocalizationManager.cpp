#include "StdAfx.h"

LocalizationManager::LocalizationManager()
{
	memset(m_sValues, 0, sizeof(m_sValues));

	m_dRolesCount = 0;
	m_Roles = NULL;

	m_dRightsCount = 0;
	m_Rights = NULL;

	m_dActiveStateCount = 0;
	m_ActiveStates = NULL;

	m_dAlertStateCount = 0;
	m_AlertStates = NULL;

	m_AlertTypes = NULL;
	m_dAlertTypeCount = 0;

	m_dModuleStatusTypeCount = 0;
	m_ModulesStatusTypes = NULL;

	m_dModuleTypeCount = 0;
	m_ModulesTypes = NULL;

	m_dLocationNamesCount = 0;
	m_LocationNames = NULL;
}

void LocalizationManager::DestructorCheckMemLeaks()
{ 
	for (int i = 0; i < LocalizationRssIds::LRSS_MAX_VAL; i++)
	{
		if (m_sValues[i] != NULL)
		{
			InternalFree(m_sValues[i]);
			m_sValues[i] = NULL;
		}
	}

	m_dRolesCount = 0;
	InternalFree(m_Roles);

	m_dRightsCount = 0;
	InternalFree(m_Rights);

	m_dActiveStateCount = 0;
	InternalFree(m_ActiveStates);

	m_dAlertStateCount = 0;
	InternalFree(m_AlertStates);

	m_dAlertTypeCount = 0;
	InternalFree(m_AlertTypes);

	m_dModuleStatusTypeCount = 0;
	InternalFree(m_ModulesStatusTypes);

	m_dModuleTypeCount = 0;
	InternalFree(m_ModulesTypes);

	m_dLocationNamesCount = 0;
	InternalFree(m_LocationNames);

#ifdef _DEBUG
	delete& sLocalization;
#endif

}

static LocalizationRssIds GetLocalizationId(const char* keyName)
{
	if (keyName == NULL)
	{
		return LocalizationRssIds::LRSS_NOT_INITIALIZED;
	}
	auto keyname = magic_enum::enum_cast<LocalizationRssIds>(keyName, magic_enum::case_insensitive);
	if (keyname.has_value() && LocalizationRssIds::LRSS_NOT_INITIALIZED < keyname && keyname < LocalizationRssIds::LRSS_MAX_VAL)
	{
		return keyname.value();
	}
	return LocalizationRssIds::LRSS_NOT_INITIALIZED;
}

void LocalizationManager::LoadStrings(const char* fileName)
{
	AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityNormal, LogSourceGroups::LogSourceConfig, 0, 0,
		"LocalizationManager:Started loading localization values from %s", fileName);

	IniFile ini = parseIniFile(fileName);
	for (auto sectionsItr = ini.sections.begin(); sectionsItr != ini.sections.end(); sectionsItr++)
	{
		for (auto keyval : sectionsItr->second)
		{
			LocalizationRssIds id = GetLocalizationId(keyval.first.c_str());
			if (id == LocalizationRssIds::LRSS_NOT_INITIALIZED)
			{
				continue;
			}

			AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceLocalStrings, 0, 0,
				"LocalizationManager:Assigned id-val %d = %s", id, keyval.second.c_str());

			m_sValues[id] = InternalStrDup(keyval.second.c_str());
		}
	}
}

void ParseGenericIdValArray(const char *arrayName, const char *idName, const char *ValName, 
	yyjson_doc* jsonResponse, int& elemCount, IdStringPair*& idValStore)
{
	yyjson_val* root = yyjson_doc_get_root(jsonResponse);
	yyjson_val* yyarrayName = yyjson_obj_get(root, arrayName);

	// parse role strings
	if (yyarrayName != NULL && yyjson_is_arr(yyarrayName))
	{
		elemCount = (int)yyjson_arr_size(yyarrayName);
		if (elemCount > MAX_EXPECTED_IDSTRING_COUNT)
		{
			AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceLocalStrings, 0, 0,
				"LocalizationManager:Unexpected large array %s count %d. Aborting", arrayName, elemCount);
			return;
		}
		idValStore = (IdStringPair*)InternalMalloc(elemCount * sizeof(IdStringPair));

		// should never happen
		if (idValStore == NULL)
		{
			return;
		}

		// init storage. Better safe than sorry
		memset(idValStore, 0, elemCount * sizeof(IdStringPair));

		size_t elementsAdded = 0;
		size_t idx, max;
		yyjson_val* yyRow;
		yyjson_arr_foreach(yyarrayName, idx, max, yyRow)
		{
			// should never happen
			if (elementsAdded >= elemCount)
			{
				AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceLocalStrings, 0, 0,
					"LocalizationManager:%s array mem corruption. Aborting", arrayName, elemCount);
				return;
			}

			// sanity check for expected element format
			yyjson_val* yyRowColId = yyjson_obj_get(yyRow, idName);
			if (yyRowColId == NULL || yyjson_is_str(yyRowColId) == false)
			{
				continue;
			}
			yyjson_val* yyRowColVal = yyjson_obj_get(yyRow, ValName);
			if (yyRowColVal == NULL || yyjson_is_str(yyRowColVal) == false)
			{
				continue;
			}

			// store element values
			// this can randomly come as string instead of number
			idValStore[elementsAdded].Id = atoi(yyjson_get_str(yyRowColId));

			// store the actual value for the id
			if (strcpy_s(idValStore[elementsAdded].Val, yyjson_get_str(yyRowColVal)))
			{
				AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceLocalStrings, 0, 0,
					"LocalizationManager:%s string too large. Aborting", arrayName, yyjson_get_str(yyRowColVal));
			}

			// increase store index
			elementsAdded++;
		}

		// should be the same as before, but better make sure we do not have larger count than max added
		elemCount = (int)elementsAdded;
	}
}

void CB_ServerStringDefinesArrived(int CurlErr, char* response, void* userData)
{
	userData;
	yyJSON(yydoc);

	if (ExtractDBColumnToBinary::DBH_APIResultValid(CurlErr, response, yydoc, LogSourceGroups::LogSourceLocalStrings, "CB_ServerStringDefinesArrived") != WebApiErrorCodes::WAE_NoError)
	{
		return;
	}

	ParseGenericIdValArray("Rights", "RightID", "RightName", yydoc, sLocalization.m_dRightsCount, sLocalization.m_Rights);
	ParseGenericIdValArray("Roles", "RoleID", "RoleName", yydoc, sLocalization.m_dRolesCount, sLocalization.m_Roles);
	ParseGenericIdValArray("ActiveStates", "UserActiveID", "UserActiveName", yydoc, sLocalization.m_dActiveStateCount, sLocalization.m_ActiveStates);
	ParseGenericIdValArray("AlertStatusType", "AlertStatusTypeId", "AlertStatusTypeName", yydoc, sLocalization.m_dAlertStateCount, sLocalization.m_AlertStates);
	ParseGenericIdValArray("AlertType", "AlertTypeId", "AlertTypeName", yydoc, sLocalization.m_dAlertTypeCount, sLocalization.m_AlertTypes);
	ParseGenericIdValArray("ModuleStatusTypes", "ModuleStatusID", "ModuleStatusName", yydoc, sLocalization.m_dModuleStatusTypeCount, sLocalization.m_ModulesStatusTypes);
	ParseGenericIdValArray("ModuleTypes", "ModuleTypeID", "ModuleTypeName", yydoc, sLocalization.m_dModuleTypeCount, sLocalization.m_ModulesTypes);
	ParseGenericIdValArray("LocationNames", "LocationID", "LocationName", yydoc, sLocalization.m_dLocationNamesCount, sLocalization.m_LocationNames);
}

void LocalizationManager::FetchServerStringDefines()
{
	// only call it once. No need to reinit after each login
	// when language changes, will need to flush old values ..
	if (m_dRolesCount != 0 || m_dRightsCount != 0 || m_dActiveStateCount != 0)
	{
		return;
	}
	WebApi_GetServerDefinesAsync(CB_ServerStringDefinesArrived);
}

const char* LocalizationManager::GetIdStringGeneric(const int id, const int count, const IdStringPair* pair)
{
	if (count == 0 || pair == NULL)
	{
		return "";
	}
	for (size_t i = 0; i < count; i++)
	{
		if (pair[i].Id == id)
		{
			return pair[i].Val;
		}
	}
	return "";
}