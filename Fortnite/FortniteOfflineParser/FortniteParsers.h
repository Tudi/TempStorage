#pragma once

#define DebugPrint0(p,what) if(p==1)printf(what);
#define DebugPrint1(p,what,p1) if(p==1)printf(what,p1);
#define DebugPrint2(p,what,p1,p2) if(p==1)printf(what,p1,p2);

#define FORTNITE_GUID_LEN	16

class StreamParser;

class ForniteMetaParser
{
public:
	static int ParseMetaRewind(StreamParser *sp, int PrintValues = 0);
	static int ParseMeta(StreamParser *sp, int PrintValues = 0);
private:
};

class FortniteReadChuck
{
public:
	FortniteReadChuck()
	{
		HasData = 0;
	}
	int ParseChunkRewind(StreamParser *sp, int PrintValues = 0);
	int ParseChunk(StreamParser *sp, int PrintValues = 0);
	void ClearData()
	{
		HasData = 0;
	}
	int			 HasData;
	unsigned int chunk_type;
	unsigned int chunk_size;
};

class ForniteHeaderParser
{
public:
	static int ParseHeaderRewind(StreamParser *sp, int ChunkSize, int PrintValues = 0);
	static int ParseHeader(StreamParser *sp, int ChunkSize, int PrintValues = 0);
private:
};


class ForniteEventParser
{
public:
	static int ParseEventRewind(StreamParser *sp, int ChunkSize, const unsigned char *FilterOutputByGUID, int PrintValues = 0);
	static int ParseEvent(StreamParser *sp, int ChunkSize, const unsigned char *FilterOutputByGUID, int PrintValues = 0);
private:
};

unsigned char *strGUIDToHexGUID(const unsigned char *StrGUID);