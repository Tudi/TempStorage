#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <list>
#include <assert.h>
#include <math.h>
#include "FreeImage.h"
#include "ImageHandler.h"
#include "BinFileCommon.h"
#include "BinFileReader.h"
#include "BinFileWriter.h"
#include "BinFileLineDraw.h"


#ifndef MIN
	#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef MAX
	#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#define Bytespp	3
#define ColorChannelCount 3 // RGB
#define INT_PRECISION_DIGITS	10000
#define SOFT_ASSERT(x,msg) do{ if(!(x)) { \
	char remsg[5000];\
	sprintf_s(remsg, sizeof(remsg), "%s:%s:%s", __FILE__,__LINE__,msg);\
	printf(remsg);}}while(0);