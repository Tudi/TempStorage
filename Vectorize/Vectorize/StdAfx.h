#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <list>
#include "FreeImage.h"
#include "ImageHandler.h"
#include "ImageFilters.h"
#include "ShapeStore.h"
#include "DetectSquare.h"
#include "DetectLine.h"
#include "ExtractionStatusStore.h"
#include "FileWriter.h"


#ifndef MIN
	#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef MAX
	#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#define SHP_FILE_VERSION_STRING "ver1"
#define Bytespp	3
