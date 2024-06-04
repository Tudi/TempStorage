#pragma once

#define ConsoleCommandThreadSleep 5000

class VSSConsoleCommandsManager
{
public:
	inline static VSSConsoleCommandsManager& getInstance()
	{
		static VSSConsoleCommandsManager instance;
		return instance;
	}
	~VSSConsoleCommandsManager();
	void DestructorCheckMemLeaks();
	void Init();
private:
	VSSConsoleCommandsManager();
	VSSConsoleCommandsManager(const VSSConsoleCommandsManager&) = delete;
	VSSConsoleCommandsManager& operator=(const VSSConsoleCommandsManager&) = delete;

};

#define sCCManager VSSConsoleCommandsManager::getInstance()