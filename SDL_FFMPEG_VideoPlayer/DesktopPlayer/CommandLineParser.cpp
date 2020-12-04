#include "StdAfx.h"

const char* GetParamValue(const char* param, const char* key)
{
	int i = 0;
	for (; key[i] != '=' && key[i] != 0; i++)
		if (param[i] != key[i])
			return NULL;
	//seems like there is a key match here
	if (param[i] == key[i])
		return &param[i + 1];
	return NULL;
}

int CopyVal(const char* From, char* To)
{
	if (From == NULL)
		return -1;
	int i = 0;
	for (;From[i] != 0; i++)
		To[i] = From[i];
	//add terminator
	To[i] = 0;
	//all went ok
	return i;
}

PlayerCreateParams *ParseCommandLineParams(int argc, char* argv[])
{
	//in case tried to start player without knowing what it can do
	if (argc == 1)
	{
		printHelpMenu(argv[0]);
		return NULL;
	}
	PlayerCreateParams* ret = (PlayerCreateParams*)malloc(sizeof(PlayerCreateParams));
	//should never happen
	if (ret == NULL)
		return NULL;
	// init values to not initialized
	memset(ret, 0, sizeof(PlayerCreateParams));
	//parse all input params
	for (int i = 0; i < argc; i++)
	{
		char* param = argv[i];
		char ParamVal[500];
		if (CopyVal(GetParamValue(param, "SaveImageStream="), ParamVal) > 0)
		{
			strcpy_s(ret->ImageFileName, sizeof(ret->ImageFileName), ParamVal);
			ret->DumpImagesToFile = 1;
		}
		else if (CopyVal(GetParamValue(param, "SaveAudioStream="), ParamVal) > 0)
		{
			strcpy_s(ret->AudioFileName, sizeof(ret->AudioFileName), ParamVal);
			ret->DumpAudioToFile = 1;
		}
		else if (CopyVal(GetParamValue(param, "in="), ParamVal) > 0)
			strcpy_s(ret->InputFileName, sizeof(ret->InputFileName), ParamVal);
		else if (CopyVal(GetParamValue(param, "max_frames_to_decode="), ParamVal) > 0)
		{
			char* pEnd;
			ret->MaxFramesToDecode = strtol(ParamVal, &pEnd, 10);
		}
		else if (CopyVal(GetParamValue(param, "ShowFrameInfo="), ParamVal) > 0)
		{
			char* pEnd;
			ret->ShowFrameInfo = strtol(ParamVal, &pEnd, 10);
		}
	}

	//if no value specified, decode whole movie
	if (ret->MaxFramesToDecode == 0)
		ret->MaxFramesToDecode = 0x0FFFFFFF;

	return ret;
}

/**
 * Print help menu containing usage information.
 */
void printHelpMenu(const char* ExeName)
{
	printf("Invalid arguments.\n\n");
	printf("Usage: ./%s in=filename [max_frames_to_decode=<val>] [SaveImageStream=<val>] [SaveAudioStream=<val>] [ShowFrameInfo=1]\n\n", ExeName);
	printf("e.g: ./%s in=video.mp4 max_frames_to_decode=10 SaveImageStream=Frames SaveAudioStream=Audio.wav ShowFrameInfo=1\n", ExeName);
}
