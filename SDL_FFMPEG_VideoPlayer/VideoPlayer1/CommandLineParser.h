#pragma once

#define MAX_FILE_NAME_LEN   500

struct PlayerCreateParams
{
    int MaxFramesToDecode;
    char InputFileName[MAX_FILE_NAME_LEN];
    int DumpImagesToFile;
    char ImageFileName[MAX_FILE_NAME_LEN];
    int DumpAudioToFile;
    char AudioFileName[MAX_FILE_NAME_LEN];
    int ShowFrameInfo;
};

PlayerCreateParams *ParseCommandLineParams(int argc, char* argv[]);

/**
 * Methods declaration.
 */
void printHelpMenu(const char *ExeName);