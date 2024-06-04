#pragma once

#define USER_CONFIG_FILE "./Data/userconfigs.ini"
// every X ms check if user settings should be saved to file
#define CONFIG_FILE_UPDATE_SLEEP_PERIOD 10000

// Numeric representation of config keys
enum ConfigOptionIds : int
{
    CO_UNINITIALIZED_VALUE = 0,
    API_Endpoint_URL,
    DeleteOlderLogFiles,
    LogFileMessages,
    LogServerMessages,
    LogDestinationEnable,
    MenuTextFontFile,
    LargeTextFontFile,
    MediumTextFontFile,
    NormalTextFontFile,
    SmallTextFontFile,
    ButtonTextFontFile,
    MaxRenderFPS,
    StartFullScreen,
    Language,
    API_IgnoreCertificateIssuer,
    DPS_ReconnectInterval,
    DB_URL,
    DB_Port,
    DB_User,
    DB_Passw,
    DB_Database,
    DB_ConnectionsPooled,
    DPS_Id,
    DPS_WSListenPort,
    DPS_MaxNetworkThreads,
    DPS_UDPMaxListenThreads,
    DPS_UDPListenPort,
    DPS_MaxAlertProcessingThreads,
    AsyncTasksThreadPoolSize,
    CO_MAX_VALUE
};

#define USER_CONFIG_COMMON_SECTION std::string( "Common" )
#define USER_CONFIG_CUR_USER std::string( "ActiveUser" )

class WatchdogThreadData;

/// <summary>
/// Object that will hold in memory config values to be accessed globaly within the application
/// This Singleton is expected to be loaded on startup and remain unchanged while app is running
/// </summary>
class ConfigManager
{
public:
    inline static ConfigManager& getInstance() {
#ifdef _DEBUG
        static ConfigManager* instance = new ConfigManager;
        return *instance;
#else
        static ConfigManager instance;
        return instance;
#endif
    }
    /// <summary>
    /// Loads an 'ini' file content in memory
    /// </summary>
    /// <param name="fileName"></param>
    void LoadStrings(const char* fileName, bool bReload = false);
    /// <summary>
    /// Get the key-value pair's value
    /// </summary>
    /// <param name="id"></param>
    /// <returns></returns>
    inline const char* GetString(ConfigOptionIds id)
    {
        return m_sValues[id];
    }
    const char* GetString(ConfigOptionIds id, const char* defaultValue)
    {
        if (m_sValues[id] == NULL)
        {
            return defaultValue;
        }
        return m_sValues[id];
    }
    inline int GetInt(ConfigOptionIds id)
    {
        return std::atoi(m_sValues[id]);
    }
    int GetInt(ConfigOptionIds id, int defaultValue);

    /// <summary>
    /// In case some of the settings are changed while the application is running.
    /// These changes are not persistent and will be lost on shutdown
    /// </summary>
    /// <param name="id"></param>
    /// <param name="newVal"></param>
    void UpdateString(ConfigOptionIds id, const char *newVal);

    /// <summary>
    /// Only when checking if Application has memory leaks
    /// </summary>
    void DestructorCheckMemLeaks();

    /// <summary>
    /// Once the user logs in we will update active username
    /// </summary>
    /// <param name="sActiveUser"></param>
    void SetActiveUsername(const char* sActiveUser);

    /// <summary>
    /// Return the last successfull logged in user name
    /// </summary>
    /// <returns></returns>
    const char* GetLastActiveUser();

    /// <summary>
    /// Set a custom key to a custom value in user config file
    /// </summary>
    void SetUserConfig(const char* key, const char *val);
    const char* GetUserConfig(const char* key);
private:
    ConfigManager();
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    char* m_sValues[ConfigOptionIds::CO_MAX_VALUE];

    const char* GetUserConfig(const char *section, const char* key);
    void SetUserConfig(const char* section, const char* key, const char* val);
    friend void ConfigManager_AsyncExecuteThread(WatchdogThreadData* wtd);
    std::mutex m_FileUpdateLock;
    bool m_bEditedConfigs;
    std::string m_szActiveUser;
    std::string m_sLoadedIniFileName;
    IniFile m_UserIniFile; // store user settings, but also preserve other user settings to be able to edit the file
};

#define sConfigManager ConfigManager::getInstance()