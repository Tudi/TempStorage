#define LIBRARY_API __declspec(dllimport)

#include "../LicenseDLL/src/ComputerFingerprint.h"
#include <stdio.h>
#include <conio.h>

#pragma comment(lib, "LicenseDLL.lib")

int main()
{
	ComputerFingerprint *ClientSeed;

	//create a new store
//	ClientSeed = new ComputerFingerprint;
	ClientSeed = CreateComputerFingerprint();

	//test generate API
	ClientSeed->GenerateFingerprint();

	//test debugprint
	ClientSeed->Print();

	//test save
	ClientSeed->SaveFingerprint("ClientSeed.dat");

	//test destroy
	DestroyComputerFingerprint(ClientSeed);

	///////////////////////////////////////////////////////////////////////////

	//create a new store
	ClientSeed = CreateComputerFingerprint();

	//test load
	ClientSeed->LoadFingerprint("ClientSeed.dat");

	//test if load went well
	printf("\n\nShould write same as above :\n\n");
	ClientSeed->Print();

	//destroy
	//delete ClientSeed;
	DestroyComputerFingerprint(ClientSeed);
		
	//wait for keypress
	_getch();
	return 0;
}