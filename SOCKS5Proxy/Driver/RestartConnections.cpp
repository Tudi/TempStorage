#include <cstdlib>
#include <list>
#include <string>
#include "ConfigHandler.h"
#include "Utils.h"

std::list<std::string> GetListEnabledNetworkCards()
{
	std::list<std::string> ret;
	//get the list of interfaces. Put the list in a temp file
	system("netsh interface show interface > Interfaces.txt");

	//parse the temp file
	FILE* f;
	errno_t er = fopen_s(&f, "Interfaces.txt", "rt");
	if (f == NULL)
		return ret;
/*
Admin State    State          Type             Interface Name
-------------------------------------------------------------------------
Enabled        Connected      Dedicated        Wi-Fi
Enabled        Disconnected   Dedicated        Ethernet
Enabled        Connected      Dedicated        vEthernet (Default Switch)*/
	//read lines until we see the dotted line
	char line[500];
	int count = 0;
	int ListStarted = 0;
	do {
		if (ListStarted == 1)
		{
			char AdminState[500];
			char State[500];
			count = fscanf_s(f, "%s%s%s%s", AdminState, (int)sizeof(AdminState), State, (int)sizeof(State), line, (int)sizeof(line), line, (int)sizeof(line));
			if (strcmp(State, "Connected") == 0)
				ret.push_back(line);
		}
		else
		{
			count = fscanf_s(f, "%s\n", line, (int)sizeof(line));
			if (line[0] == '-')
				ListStarted = 1;
		}
	}while(count > 0);
	//we are done with the file
	fclose(f);

	//delete the temp file
	remove("Interfaces.txt");

	return ret;
}

static void *RestartExecutionLock = NULL;
void RestartNetworkConnections(int StartRedirection)
{
	LockFunctionExecution Locker(&RestartExecutionLock);
	//get the list of interfaces that are "connected"
	std::list<std::string> CardList = GetListEnabledNetworkCards();
	//construct list of commands
	char CommandsDisable[8000];
	char CommandsEnable[8000];
	int WriteIndexDisable = 0;
	int WriteIndexEnable = 0;
	for (auto itr = CardList.begin(); itr != CardList.end(); itr++)
	{
		WriteIndexDisable += sprintf_s(&CommandsDisable[WriteIndexDisable], sizeof(CommandsDisable) - WriteIndexDisable, "netsh interface set interface \"%s\" DISABLED \n; ",(*itr).c_str());
		WriteIndexEnable += sprintf_s(&CommandsEnable[WriteIndexEnable], sizeof(CommandsEnable) - WriteIndexEnable, "netsh interface set interface \"%s\" ENABLED \n; ", (*itr).c_str());
	}
/*	if (WriteIndexDisable > 0)
	{
		CommandsDisable[WriteIndexDisable - 2] = 0;
		WriteIndexDisable -= 2;
	}
	WriteIndexDisable += sprintf_s(&CommandsDisable[WriteIndexDisable], sizeof(CommandsDisable) - WriteIndexDisable, " > output.txt ");*/
	system(CommandsDisable);
	if (StartRedirection)
		SetPacketRedirectionStatus(1);
	system(CommandsEnable);
}