#include <stdio.h>
#include <conio.h>

#define LIBRARY_API __declspec(dllimport)
#include "../LicenseDLL/SharedMem/SharedMem.h"

#ifdef X64
	#ifdef _DEBUG
		#pragma comment(lib, "../x64/Debug/LicenseDLL.lib")
	#else
		#pragma comment(lib, "../x64/Release/LicenseDLL.lib")
	#endif
#endif

void main()
{
	int InstanceCounter;
	if (SharedMemGetValue("TestSharedMem", "InstanceCount", (char*)&InstanceCounter, sizeof(InstanceCounter)) != 0)
	{
		//first time init ( create variable )
		InstanceCounter = 0;
		SharedMemSetValue("TestSharedMem", "InstanceCount", (char*)&InstanceCounter, sizeof(InstanceCounter));
	}
	else
		printf("Shared memory is resident\n");

	//increase instance counter
	InstanceCounter++;
	SharedMemSetValue("TestSharedMem", "InstanceCount", (char*)&InstanceCounter, sizeof(InstanceCounter));

	//show it to user
	printf("Number of instances found without shared memory reset : %d\n", InstanceCounter);

	_getch();
}