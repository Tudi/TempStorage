#define _CRT_SECURE_NO_WARNINGS

#include <memory>
#include "StreamWriter.h"

StreamWriter sw;
StreamWriter &GetWriter() { return sw; }

void StreamWriter::AddToStream(char *Buff, int len)
{
	OutBuff = (char *)realloc(OutBuff, BuffSize + len);
	memcpy(&OutBuff[BuffSize], Buff, len);
	BuffSize += len;
}

StreamWriter::~StreamWriter()
{
	if (OutputFileName != NULL)
	{
		free(OutputFileName);
		OutputFileName = NULL;
	}
}

void StreamWriter::Flush()
{
	if (OutBuff == NULL)
		return;
	if (OutType & SWOT_CONSOLE_FLAG)
	{
		printf("%s", OutBuff);
	}
	if (OutType & SWOT_CONSOLE_FLAG)
	{
		FILE *f = fopen(OutputFileName, "at");
		if (f)
		{
			fprintf(f, "%s", OutBuff);
			fclose(f);
			f = NULL;
		}
	}
	free(OutBuff);
	OutBuff = NULL;
	BuffSize = 0;
	KillStructsAdded = 0;
}

void StreamWriter::SetOutputType(StreamWriterOutputTypes FlagVal, int EnableOutput)
{
	if (EnableOutput != 0)
		OutType = (StreamWriterOutputTypes)(OutType | FlagVal);
	else
		OutType = (StreamWriterOutputTypes)(OutType & (~FlagVal));
}

void StreamWriter::SetOutputFileName(const char *NewFileName)
{
	if (NewFileName == NULL)
		return;
	OutputFileName = _strdup(NewFileName);
}

void StreamWriter::AddJsonKill(const unsigned char *Killer, const unsigned char *Killed, int knocked, int gun)
{
	char Buff[2048]; 
	char GUID1AsHexStr[16*2+1];
	for (int i = 0; i < 16; i++)
		sprintf(&GUID1AsHexStr[i], "%02x", Killer[i]);
	GUID1AsHexStr[16 * 2] = 0;
	char GUID2AsHexStr[16 * 2 + 1];
	for (int i = 0; i < 16; i++)
		sprintf(&GUID2AsHexStr[i], "%02x", Killed[i]);
	GUID2AsHexStr[16 * 2] = 0;
	sprintf(Buff, "\"KillStruct\": { \"Killer\" : \"%s\", \"Killed\" : \"%s\", \"Knocked\" : %d, \"Gun\" : %d }\n", GUID1AsHexStr, GUID2AsHexStr, knocked, gun);
	AddToStream(Buff,strlen(Buff) + 1);
	KillStructsAdded++;
}