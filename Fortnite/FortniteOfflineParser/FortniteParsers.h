#pragma once

#define DebugPrint0(p,what) if(p==1)printf(what);
#define DebugPrint1(p,what,p1) if(p==1)printf(what,p1);
#define DebugPrint2(p,what,p1,p2) if(p==1)printf(what,p1,p2);

class StreamParser;

class ForniteMetaParser
{
public:
	static int ParseMeta(StreamParser *sp, int PrintValues = 0);
private:
};

class FortniteReadChuck
{
public:
	int ParseChunk(StreamParser *sp, int PrintValues = 0);
	unsigned int chunk_type;
	unsigned int chunk_size;
};

class ForniteHeaderParser
{
public:
	static int ParseHeader(StreamParser *sp, int ChunkSize, int PrintValues = 0);
private:
};


class ForniteEventParser
{
public:
	static int ParseEvent(StreamParser *sp, int ChunkSize, int PrintValues = 0);
private:
};