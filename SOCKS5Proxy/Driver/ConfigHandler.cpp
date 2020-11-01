#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "nxjson.h"
#include "RulesManager.h"
#include "Logger.h"
#include "proxy.h"
#include "Utils.h"
#include "ConfigHandler.h"

class ConfigStore
{
public:
	ConfigStore()
	{
		ThreadsRunning = 0;
		ProxyPort = 5557; // this is my local proxy that should convert any connection to socks5 connection
		ProxyIP = 0x7F000001;
		EssentialsLoadedFromFile = 0;
		CommandsPort = 5558;
		AlwaysRedirectPackets = 0;
		RedirectionEnabled = 1;

		//get the path where the exe is
		int bytes = GetModuleFileName(NULL, CurrentPath, sizeof(CurrentPath));
		//eat up until first slash
		for (int i = bytes - 1; i > 0; i--)
			if (CurrentPath[i] == '\\')
			{
				CurrentPath[i + 1] = 0;
				break;
			}
	}
	int ThreadsRunning;
	unsigned short ProxyPort;
	unsigned long ProxyIP;
	unsigned short CommandsPort;
	int EssentialsLoadedFromFile;
	int AlwaysRedirectPackets;
	int RedirectionEnabled; // can pause redirecting packets with external commands
	char CurrentPath[MAX_PATH];
};

//should be a singleton. We do not expose this object, in theory no need to add fireworks to make it singleton
ConfigStore cfg_store;

unsigned short GetOurProxyPort()
{
	return cfg_store.ProxyPort;
}

unsigned short GetOurCommandsPort()
{
	return cfg_store.ProxyPort;
}

int ProgramIsRunning = 1;
int IsWaitingForUserExitProgram()
{
	return ProgramIsRunning;
}

void SetProgramTerminated()
{
	ProgramIsRunning = 0;
	ShutdownProxyThread();
}

static HANDLE ConfigExecutionLock = NULL;
void LoadConfigValues(const char *Path)
{
	LockFunctionExecution Locker(&ConfigExecutionLock);
	//since we reload config file, we presume the old settings are invalid
	cfg_store.EssentialsLoadedFromFile = 0;
	
	//remove old rules
	ClearFilterRules();

	int FSize = GetFileSize2(Path);
	if (FSize == 0)
	{
		sLog.Log(LL_ERROR, __FILE__, __LINE__, "Config file missing at: %s", Path);
		fprintf(stderr,"Error: Config file content is missing\n");
		return;
	}
	//read the content of the cfg file into a string
	FILE* f;
	errno_t er = fopen_s(&f, Path, "rt");
	if (f == NULL)
	{
		sLog.Log(LL_ERROR, __FILE__, __LINE__, "Config file missing at: %s", Path);
		return;
	}
	char* code = (char*)malloc(FSize+32);
	if (code == NULL)
	{
		sLog.Log(LL_ERROR, __FILE__, __LINE__, "Failed to allocate memeory");
		return;
	}
	size_t ReadCount = fread(code, 1, FSize, f);
	fclose(f);
	int ConfigValuesFound = 0;
	const nx_json* json = nx_json_parse_utf8(code);
	if (json == NULL)
	{
		sLog.Log(LL_ERROR, __FILE__, __LINE__, "Could not parse json file at: %s", Path);
		fprintf(stderr, "Error: Config file could not be parsed\n");
		return;
	}
	const nx_json* ConfigSection = nx_json_get(json, "configs");
	if (ConfigSection != NULL)
	{
		const nx_json* item = nx_json_get(ConfigSection, "InternalCommandsPort");
		if (item != NULL)
		{
			cfg_store.CommandsPort = (unsigned short)item->num.s_value;
			ConfigValuesFound++;
		}
/*		item = nx_json_get(ConfigSection, "InternalBridgeIP");
		if (item != NULL)
		{
			unsigned int b[4]; //4 bytes for ipv4, but what if format is wrong ?
			sscanf_s(item->text_value, "%d.%d.%d.%d", b + 0, b + 1, b + 2, b + 3);
			cfg_store.ProxyIP = 0;
			for(int i=0;i<4;i++)
				cfg_store.ProxyIP = (cfg_store.ProxyIP << 8) + b[i];
			ConfigValuesFound++;
		}*/
		item = nx_json_get(ConfigSection, "InternalBridgePort");
		if (item != NULL)
		{
			cfg_store.ProxyPort = (unsigned short)item->num.s_value;
			ConfigValuesFound++;
		}
		item = nx_json_get(ConfigSection, "SwallowPacketsOnDeadTunnel");
		if (item != NULL)
		{
			cfg_store.AlwaysRedirectPackets = (unsigned short)item->num.s_value;
			ConfigValuesFound++;
		}		
		item = nx_json_get(ConfigSection, "LogLevelFlags");
		if (item != NULL)
			sLog.SetLogLevelFlags((int)item->num.s_value);
		item = nx_json_get(ConfigSection, "LogOutputFlags");
		if (item != NULL)
			sLog.SetLogOutputFlags((int)item->num.s_value);
		item = nx_json_get(ConfigSection, "LogFilePath");
		if (item != NULL)
			sLog.SetLogFile(item->text_value);
		item = nx_json_get(ConfigSection, "RedirectPacketsOnStartup");
		if (item != NULL)
			SetPacketRedirectionStatus((int)item->num.s_value);
/*		item = nx_json_get(ConfigSection, "SOCKS5TunnelIP");
		if (item != NULL)
		{
			unsigned int b[4]; //4 bytes for ipv4, but what if format is wrong ?
			sscanf_s(item->text_value, "%d.%d.%d.%d", b + 0, b + 1, b + 2, b + 3);
			cfg_store.TunnelIP = 0;
			for (int i = 0; i < 4; i++)
				cfg_store.TunnelIP = (cfg_store.TunnelIP << 8) + b[i];
			ConfigValuesFound++;
		}
		item = nx_json_get(ConfigSection, "SOCKS5TunnelPort");
		if (item != NULL)
		{
			cfg_store.TunnelPort = (unsigned short)item->num.s_value;
			ConfigValuesFound++;
		}*/
	}
	//load redirection rules
	const nx_json* RulesSection = nx_json_get(json, "rules");
	if (RulesSection != NULL)
	{
		const nx_json* itr = RulesSection->children.first;
		while (itr != NULL)
		{
			const nx_json* item;
			int IsWhitelistRule = 0;
			item = nx_json_get(itr, "type");
			if (item != NULL && strcmp(item->text_value, "direct") == 0)
				IsWhitelistRule = 1;

			item = nx_json_get(itr, "enabled");
			if (item != NULL && item->num.s_value == 0)
			{
				itr = itr->next;
				continue;
			}
			//create a name for our group. Should be unique or it's bad
			char GroupName[500];
			item = nx_json_get(itr, "name");
			GroupName[0] = 0;
			if (item != NULL)
				strcpy_s(GroupName, sizeof(GroupName), item->text_value);
			if (GroupName[0] == 0)
			{
				static int RuleGroupUID = 1;
				do{
					sprintf_s(GroupName, sizeof(GroupName), "%d", RuleGroupUID);
					RuleGroupUID++;
				} while (IsRuleNameTaken(GroupName));
			}

			//parse list of programs
			item = nx_json_get(itr, "programs");
			if (item != NULL)
			{
				const nx_json* itr2 = item->children.first;
				while (itr2)
				{
					FilterAddRule(GroupName, itr2->text_value, NULL, NULL, IsWhitelistRule);
					//advance iterator
					itr2 = itr2->next;
				}
			}

			//parse list of IPs
			item = nx_json_get(itr, "ips");
			if (item != NULL)
			{
				const nx_json* itr2 = item->children.first;
				while (itr2)
				{
					FilterAddRule(GroupName, NULL, itr2->text_value, NULL, IsWhitelistRule);
					//advance iterator
					itr2 = itr2->next;
				}
			}

			//get destination IP
			item = nx_json_get(itr, "destination");
			if(item != NULL)
				FilterAddRule(GroupName,NULL,NULL, item->text_value, IsWhitelistRule);

			//advance iterator
			itr = itr->next;
		}
	}
	nx_json_free(json);
	json = NULL;

	//try to optimize lookup table
	CompileFilterRules();

	//if we have enough important values in the config file, we consider it satisfying
	if(ConfigValuesFound==3)
		cfg_store.EssentialsLoadedFromFile = 1;

	//no longer needed
	free(code);

	//let user note if something went wrong with the config file
	if(GetRuleCount()==0)
		fprintf(stderr, "Error: Loaded 0 rules from config file\n");
}

int ConfigFileContainedSetupValues()
{
	return cfg_store.EssentialsLoadedFromFile == 1;
}

void IncreaseThreadsRunningCount()
{
	cfg_store.ThreadsRunning++;
}

void DecreaseThreadsRunningCount()
{
	cfg_store.ThreadsRunning--;
}

int GetRunningThreadCount()
{
	return cfg_store.ThreadsRunning;
}

int GetWatchdogTunnelSleepMs()
{
	return 100;
}

int RedirectPacketsToDeadTunnels()
{
	return cfg_store.AlwaysRedirectPackets;
}

unsigned short GetCommandsPort()
{
	return cfg_store.CommandsPort;
}

int IsRedirectionEnabled()
{
	return cfg_store.RedirectionEnabled;
}

void SetPacketRedirectionStatus(int EnableRedirection)
{
	cfg_store.RedirectionEnabled = (EnableRedirection != 0);
}

char* GetFullPath()
{
	return cfg_store.CurrentPath;
}