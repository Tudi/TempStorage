#pragma once

#ifndef _WIN32
	#include <unistd.h>
#endif
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <time.h>

extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libavutil/imgutils.h>
	#include <libavutil/avstring.h>
	#include <libavutil/time.h>
	#include <libavutil/opt.h>
	#include <libavformat/avformat.h>
	#include <libswscale/swscale.h>
	#include <libswresample/swresample.h>
	#include <SDL.h>
	#include <SDL_thread.h>
	#include <jpeglib.h>
#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX
	#include <Windows.h>
	#include <SDL_syswm.h>
#endif
}

#define FF_QUIT_EVENT (SDL_USEREVENT + 1)

#include "../DecoderCore/DecoderInterface.h"
#include "Profiling.h"
#include "InputHandler.h"
#include "SaveImage.h"
#include "SaveAudio.h"
#include "CommandLineParser.h"
#include "Player.h"
