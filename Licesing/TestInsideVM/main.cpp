#include <stdio.h>

#define LIBRARY_API __declspec(dllimport)
#include "../LicenseDLL/VMTools.h"

#ifdef X64
	#ifdef _DEBUG
		#pragma comment(lib, "../x64/Debug/LicenseDLL.lib")
	#else
		#pragma comment(lib, "../x64/Release/LicenseDLL.lib")
	#endif
#endif

void main()
{
	if (Detect_VMware())
		printf("Detected we are running inside VMWare\n");
	else if (Detect_VM())
		printf("Detected we are running inside VM\n");
	else
		printf("No VM detected\n");
}