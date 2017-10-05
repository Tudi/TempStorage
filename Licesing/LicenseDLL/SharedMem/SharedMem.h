#pragma once

#ifndef LIBRARY_API
	#include "../stdafx.h"
#endif

extern "C"
{
	/*
	Shared memeory is something that is available as long as at least one instance of a process is still using it
	*/
	LIBRARY_API int SharedMemGetValue(const char *SessionName, const char *VarName, void *Store, int Size);
	LIBRARY_API int SharedMemSetValue(const char *SessionName, const char *VarName, void *Store, int Size);
}