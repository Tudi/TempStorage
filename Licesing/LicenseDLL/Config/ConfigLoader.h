#pragma once

#ifndef LIBRARY_API
	#include "../stdafx.h"
#endif

extern "C"
{
	/*
		If you have time, rewrite this to be something fast and cached...
	*/
	LIBRARY_API int GetIntConfig(const char *Filename, const char *ConfName, int *Store);
	LIBRARY_API int GetStrConfig(const char *Filename, const char *ConfName, char *Store, int MaxBytes);
	LIBRARY_API int GetFloatConfig(const char *Filename, const char *ConfName, float *Store);
}