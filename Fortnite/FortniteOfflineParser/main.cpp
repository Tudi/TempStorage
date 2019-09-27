#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <assert.h>
#include <io.h>
#include <time.h>
#include "FortniteDefines.h"
#include "StreamReader.h"
#include "FortniteParsers.h"
#include "StreamWriter.h"

char *GetLastModifiedFileName(const char *Directory)
{
	long long LastModifiedStamp = 0;
	char *ret = NULL;
	struct _finddata_t file_info;

	memset(&file_info, 0, sizeof(file_info));
	char SearchFilter[65535];
	char FileName[65535];
	FileName[0] = 0;

	if (Directory == NULL)
		sprintf(SearchFilter, ".\\*.replay"); // make sure to update path to poin to replay files folder !!
	else
		sprintf(SearchFilter, "%s*.replay", Directory);

	intptr_t handle = _findfirst(SearchFilter, &file_info); 
	if (handle == -1)
		return NULL;
	do
	{
		if (file_info.time_write > LastModifiedStamp)
		{
			// is this file opened by someone ? We are expecting fortnite to be writing to this file. That means nobody else is able to write to it.
			LastModifiedStamp = file_info.time_write;
			strcpy(FileName, file_info.name);
		}
	} while (_findnext(handle, &file_info) != -1);
	_findclose(handle);

	//could not find any replay files in that directory
	if (FileName[0] == 0)
		return NULL;

	if(Directory != NULL)
		sprintf(SearchFilter, "%s%s", Directory, FileName);

	return _strdup(SearchFilter);
}

char *ParseLastLogFile(const unsigned char *FilterByGUID, const char *Directory)
{
	const char *JSonFileName = "FortniteLogParsed.json";
	char *FileName = GetLastModifiedFileName(Directory);
	unsigned char *FilterByGUIDHex = strGUIDToHexGUID(FilterByGUID);
	//there are no log files in the log directory
	if (FileName == NULL)
	{
		printf("No replay files were found in the replay directory\n");
		return NULL;
	}

	// Our parser that needs to support resumed parsing
	StreamParser sp;
	//open the replay for parsing
	FILE *ReplayFile = fopen(FileName, "rb");
	if (ReplayFile == NULL)
	{
		printf("Could not open input file\n");
		return NULL;
	}

	GetWriter().SetOutputFileName(JSonFileName);
#ifdef _DEBUG
	GetWriter().SetOutputType(SWOT_CONSOLE_FLAG, 1);
	GetWriter().SetOutputType(SWOT_FILE_FLAG, 1);
#endif

	int VerboseParsing = 0;
	int MetaParsed = 0;
#define READ_TIMEOUT_SECONDS	3		// x seconds and the loop will break on no more new data
	time_t ReadTimeoutStart = 0;
	FortniteReadChuck chunk;

	//read a chunk and feed it to our parser
	unsigned char ReadBuff[65563];
	size_t BytesRead = fread(ReadBuff, 1, sizeof(ReadBuff), ReplayFile);
	while (1)
	{
		//we manage to read a few bytes from the file. Add it to our stream of bytes that we can use to seek and resume reading
		sp.AddInput(ReadBuff, BytesRead);

		//Only need to parse the meta once
		if (MetaParsed == 0)
		{
			//was there enough data to parse this block ? If not, rewind
			int MetaParseError = ForniteMetaParser::ParseMetaRewind(&sp, VerboseParsing);
			MetaParsed = MetaParseError == 0;
		}

		//try to read a whole chunk and than parse it
		while (sp.EndReached() == 0 && MetaParsed == 1)
		{
			if (chunk.HasData == 0)
			{
				int Err = chunk.ParseChunkRewind(&sp, VerboseParsing);
				if (Err != 0) // right this can only happen if we do not have enough data in the stream reader
					break;
			}

			//check if we have enough data for this chunk to be parsed
			if ((int)chunk.chunk_size > sp.GetBytesRemain())
				break;

			//if we do not have enough data to parse this chunk than pause pause the parsing
			int ChunkStart = sp.GetOffset();

			if (chunk.chunk_type == FN_CHUNK_HEADER)
			{
				int HeaderParseError = ForniteHeaderParser::ParseHeaderRewind(&sp, chunk.chunk_size, VerboseParsing);
			}
			else if (chunk.chunk_type == FN_CHUNK_CHECKPOINT)
			{
				//skipping checkpoint chunk
			}
			else if (chunk.chunk_type == FN_CHUNK_REPLAY)
			{
				//skipping replay chunk
			}
			else if (chunk.chunk_type == FN_CHUNK_EVENT)
			{
				int EventParseError = ForniteEventParser::ParseEventRewind(&sp, chunk.chunk_size, FilterByGUIDHex, VerboseParsing);
//				GetWriter().Flush();
			}
			else
			{
				printf("Unknown chunk type found : %d\n", chunk.chunk_type);
				/*		for (int i = 0; i < 40; i++)
				{
					sp->GuessNext4Values();
					sp->SkipBytes(4);
				}*/
			}

			//just in case we parsed the structure badly, next structure can still be parsed correctly
			sp.SkipToChunkEnd(ChunkStart, chunk.chunk_size);

			//so we can resume parsing later as we get more data
			chunk.ClearData();
		}

		//try to fetch even more data
		BytesRead = fread(ReadBuff, 1, sizeof(ReadBuff), ReplayFile);

		//if no more events come, consider this file as closed
		if (BytesRead == 0)
		{
			if (ReadTimeoutStart == 0)
				ReadTimeoutStart = time(NULL);
			//if too much time passed and still no new data, it's time to break the parsing
			if (time(NULL) - ReadTimeoutStart > READ_TIMEOUT_SECONDS)
				break;
		}
		else
			ReadTimeoutStart = 0; // reset read timeout, maybe nothing happened intgame for a few seconds
	}
//	GetWriter().Flush();
	fclose(ReplayFile);
	free(FileName);
	return GetWriter().GetStringBuffer();
}

int ParseLogFromConsoleParams(int argc, char **argv)
{
	// Our parser that needs to support resumed parsing
	StreamParser sp;

	const char *FileName = "UnsavedReplay-2019.09.18-18.53.22.replay";
	const char *JSonFileName = "FortniteLogParsed.json";
	for (int i = 0; i < argc; i++)
	{
		if (strcmp(argv[i], "-i") == 0 && i + 1 <= argc)
		{
			FileName = argv[i + 1];
			printf("Will try to parse input file : %s\n", FileName);
		}
		if (strcmp(argv[i], "-o") == 0 && i + 1 <= argc)
		{
			JSonFileName = argv[i + 1];
			printf("Will try to write output to file : %s\n", JSonFileName);
		}
	}

	//open the replay for parsing
	FILE *ReplayFile = fopen(FileName, "rb");
	if (ReplayFile == NULL)
	{
		printf("Could not open input file\n");
		return 1;
	}
	//read a chunk and feed it to our parser
	unsigned char ReadBuff[65535];
	size_t BytesRead = fread(ReadBuff, 1, sizeof(ReadBuff), ReplayFile);
	while (BytesRead > 0)
	{
		sp.AddInput(ReadBuff, BytesRead);
		BytesRead = fread(ReadBuff, 1, sizeof(ReadBuff), ReplayFile);
	}

	int VerboseParsing = 0;
#ifdef _DEBUG
	//	VerboseParsing = 1;
#endif
	GetWriter().SetOutputFileName(JSonFileName);
	GetWriter().SetOutputType(SWOT_CONSOLE_FLAG, 1);
	GetWriter().SetOutputType(SWOT_FILE_FLAG, 1);

	//Parse in a separate loop to be able to debug more properly
	int MetaParseError = ForniteMetaParser::ParseMeta(&sp, VerboseParsing);
	while (sp.EndReached() == 0)
	{
		FortniteReadChuck chunk;
		chunk.ParseChunk(&sp, VerboseParsing);
		int ChunkStart = sp.GetOffset();

		if (chunk.chunk_type == FN_CHUNK_HEADER)
		{
			int HeaderParseError = ForniteHeaderParser::ParseHeader(&sp, chunk.chunk_size, VerboseParsing);
		}
		else if (chunk.chunk_type == FN_CHUNK_CHECKPOINT)
		{
			//skipping checkpoint chunk
		}
		else if (chunk.chunk_type == FN_CHUNK_REPLAY)
		{
			//skipping replay chunk
		}
		else if (chunk.chunk_type == FN_CHUNK_EVENT)
		{
			int EventParseError = ForniteEventParser::ParseEvent(&sp, chunk.chunk_size, NULL, VerboseParsing);
		}
		else
		{
			printf("Unknown chunk type found : %d\n", chunk.chunk_type);
			/*		for (int i = 0; i < 40; i++)
			{
				sp->GuessNext4Values();
				sp->SkipBytes(4);
			}*/
		}

		sp.SkipToChunkEnd(ChunkStart, chunk.chunk_size);
	}
	return 0;
}

int main(int argc, char **argv)
{
	//if you wish to see all guids ( for testing ), you can set the value to NULL : ParseLastLogFile(NULL)
	char *JsonContent = ParseLastLogFile((const unsigned char*)"24987a8324f4498093928b902519c594","C:\\Users\\A687258\\AppData\\Local\\FortniteGame\\Saved\\Demos\\");
	if(JsonContent != NULL)
		printf("%s", JsonContent);

	return 0;
}