
#include <iostream>

#define LIBRARY_API __declspec(dllimport)
#include "../LicenseDLL/SSLClient/RemoteInfo_API.h"
#include "../LicenseDLL/config/ConfigLoader.h"
#include "../LicenseDLL/ComputerFingerprint.h"

#ifndef X64
	#ifdef _DEBUG
		#pragma comment(lib, "../Debug/LicenseDLL.lib")
	#else
		#pragma comment(lib, "../Release/LicenseDLL.lib")
	#endif
#else
	#ifdef _DEBUG
		#pragma comment(lib, "../x64/Debug/LicenseDLL.lib")
	#else
		#pragma comment(lib, "../x64/Release/LicenseDLL.lib")
	#endif
#endif

int main(int argc, char *argv[])
{

	/*
	Expected config file content is something like this :
	SSLCertificatePath=Certificates/
	QueryServicePort=8084
	QueryServiceIP=10.50.160.84
	*/
	char	ConfigIP[20];
	int		ConfigPort;
	if (GetStrConfig("config.txt", "QueryServiceIP", ConfigIP, sizeof(ConfigIP)))
		return 1;
	if (GetIntConfig("config.txt", "QueryServicePort", &ConfigPort))
		return 1;

	for (int i = 0; i < 2; i++)
	{
		char RemoteUUID[16];
		int err = GetRemoteUUID(ConfigIP, ConfigPort, RemoteUUID, sizeof(RemoteUUID));

		//we managed to get the response. Return the value
		std::cout << "Got uuid : ";
		for (int i = 0; i < 16; i++)
			std::cout << RemoteUUID[i] << " ";
		std::cout << std::endl;
		std::cout << std::endl;
	}

	//should be enough 
	ComputerFingerprint cf;
	if (cf.GenerateFingerprint() != 0)
	{
		printf("An error ocured while generating the license seed\n");
		return 1;
	}

	return 0;
}
