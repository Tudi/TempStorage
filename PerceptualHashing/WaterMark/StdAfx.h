#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <list>
#include <assert.h>
#include <math.h>
#include "Logger.h"
#include "FreeImage.h"
#include "ImageHandler.h"
#include "BitWriter.h"
#include "WaterMark.h"


#ifndef MIN
	#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef MAX
	#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#ifdef _WIN32
	#define strdup2 _strdup
#else
	#define strdup2 strdup
#endif

#define Bytespp	3
#define ColorChannelCount 3 // RGB
#define INT_PRECISION_DIGITS	10000
