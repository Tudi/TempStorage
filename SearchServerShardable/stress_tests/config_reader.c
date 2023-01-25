#include <config_reader.h>
#include <app_errors.h>
#include <logger.h>
#include <utils.h>
#include <strings_ext.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// making it global because this is a small app
AppConfig appConf;

// search for a keyword. If it's found, create a new string from '=' to the end of line
static char* strConfValue(const char* confName, const char* fileContent, size_t fileSize)
{
	char* conf = strstr(fileContent, confName);
	if (conf == NULL)
	{
		LOG_MESSAGE(INFO_LOG_MSG, "Error : Missing config value %s", confName);
		return NULL;
	}
	conf += strlen(confName);

	size_t ValueLen = 0;
	while (conf[ValueLen] != 0 && conf[ValueLen] != '\n' && conf[ValueLen] != '\r')
	{
		ValueLen++;
	}
	if (ValueLen == 0)
	{
		ValueLen = 1;
	}

	char* ret = malloc(ValueLen); // would be enough to allocate 1 line worth of memory, we are talking about less than 1k ..
	if(ret == NULL)
	{
		LOG_MESSAGE(INFO_LOG_MSG, "Error : Failed to allocate mem");
		return NULL;
	}
	// copy the value to the new buffer
	memcpy(ret, conf, ValueLen);
	ret[ValueLen] = 0;

	return ret;
}

// search for a keyword. If it's found, convert it to a number
static int intConfValue(const char* confName, const char* fileContent, size_t fileSize)
{
	char* conf = strstr(fileContent, confName);
	if (conf == NULL)
	{
		LOG_MESSAGE(INFO_LOG_MSG, "Error : Missing config value %s", confName);
		return 0;
	}
	conf += strlen(confName);

	// be "smart" and skip spaces that follow '='
	while (*conf == ' ')
	{
		conf++;
	}

	// fixed point numbers only
	return atoi(conf);
}

// convert the contents of the config file to internal representation
int parseConfigFile(char* fileName)
{
	memset(&appConf, 0, sizeof(appConf));

	FILE* configFile = fopen(fileName, "rt");
	if (configFile == NULL)
	{
		LOG_MESSAGE(INFO_LOG_MSG, "Error : Failed to open config file '%s'", fileName);
		return ERR_FAILED_TO_OPEN_CONFIG_FILE;
	}

	char* fileContent = NULL;
	size_t fileSize = 0;
	getFileContents(configFile, (unsigned char **)&fileContent, &fileSize);
	fclose(configFile);
	
	if (fileSize == 0)
	{
		LOG_MESSAGE(INFO_LOG_MSG, "Error : Config file %s content is empty", fileName);
		return ERR_CONFIG_FILE_EMPTY;
	}

	appConf.serverIP = strConfValue("ServerIP=", fileContent, fileSize);

	if(appConf.serverIP == NULL)
	{
		free(fileContent);
		return ERR_MISSING_CONF_VALUE;
	}

	appConf.serverPort = intConfValue("ServerPort=", fileContent, fileSize);

	appConf.inputFileName = strConfValue("InputFile=", fileContent, fileSize);

	if (appConf.inputFileName == NULL)
	{
		free(fileContent);
		return ERR_MISSING_CONF_VALUE;
	}

	appConf.requestType = intConfValue("RequestType=", fileContent, fileSize);

	appConf.entryIdStart = intConfValue("OverWriteEntryIdStart=", fileContent, fileSize);

	appConf.entryIdIncreaseThread = intConfValue("OverWriteEntryIdThreadIncrease=", fileContent, fileSize);

	appConf.entryIdIncrease = intConfValue("OverWriteEntryIdIncrease=", fileContent, fileSize);
	
	appConf.workerThreadCount = intConfValue("WorkerThreadCount=", fileContent, fileSize);

	if (appConf.workerThreadCount > MAX_WORKER_THREADS)
	{
		LOG_MESSAGE(INFO_LOG_MSG, "Warning : Worker thread count %d seems to high. Reducing it to %d", appConf.workerThreadCount, MAX_WORKER_THREADS);
		appConf.workerThreadCount = MAX_WORKER_THREADS;
	}

	free(fileContent);

	return ERR_NO_ERROR;
}

// cleanup
void freeAppConf()
{
	free(appConf.serverIP);
	free(appConf.inputFileName);
	free(appConf.JSONContent);
}

// replace the "id" value with lots of spaces so we can overwrite it later by worker threads
int prepareEntryContent()
{
	// try to load the file that contains the entry
	FILE *inputFile = fopen(appConf.inputFileName, "rt");
	if (inputFile == NULL)
	{
		LOG_MESSAGE(INFO_LOG_MSG, "Error : Failed to open input file '%s'", appConf.inputFileName);
		return ERR_FAILED_TO_OPEN_CONFIG_FILE;
	}
	getFileContents(inputFile, (unsigned char**)&appConf.JSONContent, &appConf.JSONSize);
	fclose(inputFile);

	// ! this presumes that there is only 1 'id'. Or at least it will be the first one
	const char* idStr = "\"id\"";
	char* idStrLoc = strstr(appConf.JSONContent, idStr);
	if (idStrLoc == NULL)
	{
		LOG_MESSAGE(INFO_LOG_MSG, "Warning : could not find '%s' in entry file", idStr);
		return ERR_ID_MISSING_FROM_ENTRY;
	}

	// find the first digit of the ID number
	while (*idStrLoc != 0 && isdigit(*idStrLoc) == 0)
		idStrLoc++;

	// allocate space so later we can write our own ID in it
	char* newContent = malloc(appConf.JSONSize + SPACES_FOR_ID_VAL);
	if (newContent == NULL)
	{
		LOG_MESSAGE(INFO_LOG_MSG, "Error : Failed to allocate mem");
		return ERR_FAILED_TO_ALLOCATE_MEMORY;
	}
	
	// copy source str until id start
	size_t beginningStrLen = idStrLoc - appConf.JSONContent;
	appConf.idStartPos = beginningStrLen;
	memcpy(newContent, appConf.JSONContent, beginningStrLen);

	// leave some space for ID overwrite
	for (size_t i = 0; i < SPACES_FOR_ID_VAL; i++)
	{
		newContent[beginningStrLen + i] = ' ';
	}

	// find the beggining of the next json data
	while (*idStrLoc != 0 && *idStrLoc != ',' && *idStrLoc != '}')
		idStrLoc++;

	size_t endStrLen = strlen(appConf.JSONContent) - (idStrLoc - appConf.JSONContent) + 1;
	memcpy(&newContent[beginningStrLen+ SPACES_FOR_ID_VAL], idStrLoc, endStrLen);

	free(appConf.JSONContent);
	appConf.JSONContent = newContent;
	appConf.JSONSize = strlen(newContent);

	return ERR_NO_ERROR;
}
