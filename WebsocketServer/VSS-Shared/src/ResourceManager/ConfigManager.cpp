#include <string.h>
#include <thread>
#include <mutex>
#include <list>
#include <fstream>
#include <filesystem>
#include "Util/InitFileHandler.h"
#include "ResourceManager/ConfigManager.h"
#include "Util/Allocator.h"
#include "ResourceManager/LogManager.h"
#include "Session/ApplicationSession.h"
#include "magic_enum/magic_enum_all.hpp"

void ConfigManager_AsyncExecuteThread(WatchdogThreadData* wtd);

const char* NeverNullString = "";

ConfigManager::ConfigManager()
{
	for (int i = 0; i < ConfigOptionIds::CO_MAX_VALUE; i++)
	{
		m_sValues[i] = (char*)NeverNullString;
	}
	m_bEditedConfigs = false;
	m_UserIniFile = parseIniFile(USER_CONFIG_FILE);
	m_sLoadedIniFileName = "";
}

void ConfigManager::DestructorCheckMemLeaks()
{ 
	for (int i = 0; i < ConfigOptionIds::CO_MAX_VALUE; i++)
	{
		if (m_sValues[i] == NeverNullString)
		{
			m_sValues[i] = NULL;
		}
		else if (m_sValues[i] != NULL)
		{
			InternalFree(m_sValues[i]);
			m_sValues[i] = NULL;
		}
	}
	if (m_bEditedConfigs == true)
	{
		m_bEditedConfigs = false;
		writeIniFile(m_UserIniFile, std::string(USER_CONFIG_FILE));
		m_szActiveUser.clear();
		m_UserIniFile.sections.clear();
	}

#ifdef _DEBUG
	delete& sConfigManager;
#endif
}

static ConfigOptionIds GetConfigId(const char* keyName)
{
	if (keyName == NULL)
	{
		return ConfigOptionIds::CO_UNINITIALIZED_VALUE;
	}
	auto keyname = magic_enum::enum_cast<ConfigOptionIds>(keyName, magic_enum::case_insensitive);
	if (keyname.has_value() && ConfigOptionIds::CO_UNINITIALIZED_VALUE < keyname && keyname < ConfigOptionIds::CO_MAX_VALUE)
	{
		return keyname.value();
	}
	return ConfigOptionIds::CO_UNINITIALIZED_VALUE;
}

void ConfigManager::LoadStrings(const char* fileName, bool bReload)
{
	if (bReload == false)
	{
		if (m_sLoadedIniFileName.length() != 0)
		{
			AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityNormal, LogSourceGroups::LogSourceConfig, 0, 0,
				"ConfigManager:Code was made to load a single ini file. Previous file %s, new file", m_sLoadedIniFileName.c_str(), fileName);
		}
		m_sLoadedIniFileName = fileName;
	}

	AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityNormal, LogSourceGroups::LogSourceConfig, 0, 0,
		"ConfigManager:Started loading configs from %s", fileName);

	IniFile ini = parseIniFile(fileName);
	for (auto sectionsItr = ini.sections.begin(); sectionsItr != ini.sections.end(); sectionsItr++)
	{
		for (auto keyval : sectionsItr->second)
		{
			ConfigOptionIds id = GetConfigId(keyval.first.c_str());
			if (id == ConfigOptionIds::CO_UNINITIALIZED_VALUE)
			{
				continue;
			}

			AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceConfig, 0, 0,
				"ConfigManager:Assigned id-name-val %d = %s = %s", id, keyval.first.c_str(), keyval.second.c_str());

			UpdateString(id, keyval.second.c_str());
		}
	}

	static bool bFirstTimeInit = true;
	if (bFirstTimeInit == true)
	{
		bFirstTimeInit = false;
		sAppSession.CreateWorkerThread(ConfigManager_AsyncExecuteThread, "ConfigManager", CONFIG_FILE_UPDATE_SLEEP_PERIOD);
	}
}

int ConfigManager::GetInt(ConfigOptionIds id, int defaultValue)
{
	if (m_sValues[id] == NeverNullString || m_sValues[id] == NULL)
	{
		return defaultValue;
	}
	return std::atoi(m_sValues[id]);
}

void ConfigManager::UpdateString(ConfigOptionIds id, const char* newVal)
{
	if (m_sValues[id] == NeverNullString)
	{
		// do nothing : either replace it or leave it as non null
	}
	else if (m_sValues[id] != NULL)
	{
		InternalFree(m_sValues[id]);
		m_sValues[id] = NULL;
	}
	if (newVal != NULL)
	{
		m_sValues[id] = InternalStrDup(newVal);
	}
}

void ConfigManager::SetActiveUsername(const char* sActiveUser)
{
	if (sActiveUser == NULL)
	{
		m_szActiveUser = "Unknown";
	}
	else
	{
		m_szActiveUser = sActiveUser;
	}
	if (GetLastActiveUser() != m_szActiveUser)
	{
		m_bEditedConfigs = true;
		m_UserIniFile.sections[std::string(USER_CONFIG_COMMON_SECTION)][USER_CONFIG_CUR_USER] = m_szActiveUser;
	}
}

void ConfigManager::SetUserConfig(const char* section, const char* key, const char* val)
{
	if (strcmp(val, GetUserConfig(section, key)) != 0)
	{
		m_UserIniFile.sections[section][key] = val;
		m_bEditedConfigs = true;
	}
}

const char* ConfigManager::GetUserConfig(const char* section, const char* key)
{
	auto sectionIter = m_UserIniFile.sections.find(section);
	if (sectionIter != m_UserIniFile.sections.end()) {
		const std::map<std::string, std::string>& sectionMap = sectionIter->second;
		auto keyIter = sectionMap.find(key);
		if (keyIter != sectionMap.end()) {
			// The key exists in the specified section.
			return keyIter->second.c_str();
		}
	}
	return "";
}

const char* ConfigManager::GetUserConfig(const char* key)
{
	return GetUserConfig(m_szActiveUser.c_str(), key);
}

void ConfigManager::SetUserConfig(const char* key, const char *val)
{
	return SetUserConfig(m_szActiveUser.c_str(), key, val);
}

const char* ConfigManager::GetLastActiveUser()
{
	return GetUserConfig(USER_CONFIG_COMMON_SECTION.c_str(), USER_CONFIG_CUR_USER.c_str());
}

void ConfigManager_AsyncExecuteThread(WatchdogThreadData* wtd)
{
	static bool OnlyOneInstace = false;
	if (OnlyOneInstace == true)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceConfig, 0, 0,
			"ConfigManager:Async:Trying to start already running worker thread. Unexpected behavior");
		return;
	}
	OnlyOneInstace = true;

	std::filesystem::path filePath(sConfigManager.m_sLoadedIniFileName.c_str());
	std::filesystem::file_time_type lastWriteTime = std::filesystem::last_write_time(filePath);

	// wait for the user to log in
	while (sAppSession.IsApplicationRunning() && wtd->ShouldShutDown() == false)
	{
		// let watchdog know this thread is functioning as expected
		wtd->SignalHeartbeat();

		if (sConfigManager.m_bEditedConfigs == true)
		{
			std::unique_lock<std::mutex> lock(sConfigManager.m_FileUpdateLock);
			sConfigManager.m_bEditedConfigs = false; // first set the flag so in case a new config arives while we save, we resave
			writeIniFile(sConfigManager.m_UserIniFile, USER_CONFIG_FILE);
			lock.unlock();
		}

		// need to reload the ini file ?
		if (lastWriteTime != std::filesystem::last_write_time(filePath)) 
		{
			sConfigManager.LoadStrings(sConfigManager.m_sLoadedIniFileName.c_str(), true);
			lastWriteTime = std::filesystem::last_write_time(filePath); // Update last write time
		}

		wtd->BlockThreadUntilNextCycle();
	}

	// Let watchdog know we exited
	wtd->MarkDead();
}
