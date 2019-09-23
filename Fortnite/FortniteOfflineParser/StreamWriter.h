#pragma once

enum StreamWriterOutputTypes
{
	SWOT_CONSOLE_FLAG	= 1,
	SWOT_FILE_FLAG		= 2,
};

class StreamWriter
{
public:
	StreamWriter()
	{
		OutBuff = NULL;
		OutputFileName = NULL;
		OutType = SWOT_CONSOLE_FLAG;
		BuffSize = 0;
		KillStructsAdded = 0;
	}
	~StreamWriter();
	void AddToStream(char *Buff, int len);
	void Flush();
	void SetOutputType(StreamWriterOutputTypes FlagVal, int EnableOutput);
	void SetOutputFileName(const char *NewFileName);
	void AddJsonKill(const unsigned char *Killer, const unsigned char *Killed, int knocked, int gun);
private:
	StreamWriterOutputTypes OutType;
	char *OutBuff;
	int BuffSize;
	char *OutputFileName;
	int KillStructsAdded;
};

extern StreamWriter sw;
StreamWriter &GetWriter();