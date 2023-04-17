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
#include "SigFileReader.h"
#include "LineAntiDistorsion.h"
#include "tests.h"


#ifndef MIN
	#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef MAX
	#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#define Bytespp	3
#define ColorChannelCount 3 // RGB
#define INT_PRECISION_DIGITS	10000

// should mvoe all these into some logger module
void LogMessage(const char* file, int line, const char* msg);
#define SOFT_ASSERT(x,msg) if(!(x))LogMessage(__FILE__,__LINE__, msg)

#define FreeAndNULL(x) { free(x); x = NULL; }

