#include "StdAfx.h"

int main()
{
	// curiosity test if key values are small. Kinda useless case in real world
	// the main point of these tests is to compare unordered_map to best case scenarios
//	Run24BPKTests(); 
//	Run32BPKTests();
	Run128BPKTests();

	printf("Press any key to exit");
	int getchres = _getch();

	return 0;
}