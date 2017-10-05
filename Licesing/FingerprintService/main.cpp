
#include <stdio.h>

#define LIBRARY_API __declspec(dllimport)
#include "../LicenseDLL/SSLClient/RemoteInfo_API.h"
#include "../LicenseDLL/config/ConfigLoader.h"
#include "../LicenseDLL/VMTools.h"

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
	if (Detect_VM())
	{
		printf("This program can not be run under virtual machine");
		return 1;
	}

	int		ConfigPort;
	char	ConfigCertificatesPath[200];
	if (GetStrConfig("config.txt", "SSLCertificatePath", ConfigCertificatesPath, sizeof(ConfigCertificatesPath)))
	{
		printf("Could not load config value : SSLCertificatePath. Exiting\n");
		return 1;
	}
	if (GetIntConfig("config.txt", "QueryServicePort", &ConfigPort))
	{
		printf("Could not load config value : QueryServicePort. Exiting\n");
		return 1;
	}

	//this is blocking forever
	StartFingerprintService(ConfigPort, ConfigCertificatesPath);

	return 0;
}
