#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <osrng.h>
#include <aes.h>
#include <modes.h>
#include <filters.h>
#include <hmac.h>

void LogMessage(const char* file, int line, const char* msg);
#define SOFT_ASSERT(x,msg) if(!(x))LogMessage(__FILE__,__LINE__, msg)

#include "SAFCrypto.h"
#include "SAFCommon.h"
#include "SigFileReader.h"
#include "tests.h"

using namespace CryptoPP;
