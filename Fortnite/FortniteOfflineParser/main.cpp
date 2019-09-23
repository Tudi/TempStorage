#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <assert.h>
#include "FortniteDefines.h"
#include "StreamReader.h"
#include "FortniteParsers.h"

int main()
{
	// Our parser that needs to support resumed parsing
	StreamParser sp;

	//open the replay for parsing
	FILE *f;
	errno_t opener = fopen_s(&f, "UnsavedReplay-2019.09.18-18.53.22.replay", "rb");
	if (f == NULL)
	{
		printf("Could not open input file\n");
		return 1;
	}
	//read a chunk and feed it to our parser
	unsigned char ReadBuff[65535];
	size_t BytesRead = fread(ReadBuff, 1, sizeof(ReadBuff), f);
	while (BytesRead > 0)
	{
		sp.AddInput(ReadBuff, BytesRead);
		BytesRead = fread(ReadBuff, 1, sizeof(ReadBuff), f);
	}

	int VerboseParsing = 0;
#ifdef _DEBUG
	VerboseParsing = 1;
#endif

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
			int EventParseError = ForniteEventParser::ParseEvent(&sp, chunk.chunk_size, VerboseParsing);
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