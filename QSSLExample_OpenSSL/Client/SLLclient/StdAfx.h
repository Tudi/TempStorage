#pragma once

#include <iostream>
#include <Windows.h>
#include "PreciseClock.h"

#ifndef MAX
	#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#define MAX_BUFFER_TO_SEND	( 2 * 1024 * 1024 )
#define INITIAL_PACKET_SIZE MAX( 128, sizeof(PacketHeader) )
#define MIN_SEND_COUNT		100
#define MIN_SEND_TIME		( 4000 )			// bechmark for at least this amount of time before estimating processing speed. This is required to avoid rounding errors

//#define MIN_SEND_COUNT		1
//#define MIN_SEND_TIME		( 0 )			// bechmark for at least this amount of time before estimating processing speed. This is required to avoid rounding errors
