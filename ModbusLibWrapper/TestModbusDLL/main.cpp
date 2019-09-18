#include <stdio.h>
#include <conio.h>

#ifndef X64
	#ifdef _DEBUG
		#pragma comment(lib, "../ModBusAPI/Debug/ModBusAPI.lib")
	#else
		#pragma comment(lib, "../ModBusAPI/Release/ModBusAPI.lib")
	#endif
#else
	#ifdef _DEBUG
		#pragma comment(lib, "../ModBusAPI/x64/Debug/modbus.lib")
	#else
		#pragma comment(lib, "../ModBusAPI/x64/Release/modbus.lib")
	#endif
#endif

extern "C"
{
	__declspec(dllimport) int GetRegisterValue(int Register);
}

int main()
{
	int regv1 = GetRegisterValue(0);
	int regv2 = GetRegisterValue(1);
	printf("Register value : %d %d\n", regv1, regv2);
	_getch();
	return 0;
}