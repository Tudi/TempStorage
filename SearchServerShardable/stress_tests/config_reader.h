#ifndef _CONFIG_READER_H_
#define _CONFIG_READER_H_

#include <stddef.h>

// will search in the input JSON and replace the id with this amount of spaces
#define SPACES_FOR_ID_VAL 10
#define MAX_WORKER_THREADS 10000

// config options that will be available for different modules of the application
typedef struct AppConfig
{
	char* serverIP;
	int serverPort;
	char* inputFileName;
	int requestType;
	int entryIdStart;
	int entryIdIncreaseThread;
	int entryIdIncrease;
	int workerThreadCount;
	char* JSONContent;
	size_t JSONSize;
	size_t idStartPos;
}AppConfig;

// convert text file into internal format
int parseConfigFile(char* fileName);
// cleanup
void freeAppConf();
// parse the JSON we will be spam sending to the server
int prepareEntryContent();

// global configs
extern AppConfig appConf;
#endif