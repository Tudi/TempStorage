#include "StdAfx.h"

void LogMessage(const char* file, int line, const char* msg)
{ 
	char remsg[5000];
	sprintf_s(remsg, sizeof(remsg), "%s:%d:%s\n", __FILE__, __LINE__, msg);
	printf(remsg);
}