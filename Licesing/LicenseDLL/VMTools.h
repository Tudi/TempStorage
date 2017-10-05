#pragma once

#ifndef LIBRARY_API
	#include "stdafx.h"
#endif

extern "C"
{
	LIBRARY_API int Detect_VMware();
	LIBRARY_API int Detect_VM();
}