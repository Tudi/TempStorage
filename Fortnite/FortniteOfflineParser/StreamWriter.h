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
		FlushesMade = 0;
	}
	~StreamWriter();
	void AddToStream(char *Buff, int len);
	void Flush();
	void SetOutputType(StreamWriterOutputTypes FlagVal, int EnableOutput);
	void SetOutputFileName(const char *NewFileName);
	void AddJsonKill(const unsigned char *Killer, const unsigned char *Killed, int knocked, int gun);
	void AddJsonMatchStats(float accuracy, unsigned int assists, unsigned int eliminations, unsigned int weapon_damage, unsigned int other_damage, unsigned int revives, unsigned int damage_taken, unsigned int damage_structures, unsigned int materials_gathered, unsigned int materials_used, unsigned int total_traveled);
private:
	StreamWriterOutputTypes OutType;
	char *OutBuff;
	int BuffSize;
	char *OutputFileName;
	int FlushesMade;
};

extern StreamWriter sw;
StreamWriter &GetWriter();