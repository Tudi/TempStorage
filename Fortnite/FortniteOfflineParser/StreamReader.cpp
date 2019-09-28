#include "StreamReader.h"
#include <memory>
#include <string>
#include <assert.h>

StreamParser::StreamParser()
{
	BytesRead = 0;
	Data = NULL;
}

StreamParser::~StreamParser()
{
	Destroy();
}

void StreamParser::Destroy()
{
	if (Data)
	{
		free(Data);
		Data = NULL;
	}
	BytesInData = 0;
	BytesRead = 0;
}

void StreamParser::AddInput(unsigned char *NewData, int Size)
{
	if (Data == NULL)
	{
		Data = (unsigned char	*)malloc(Size);
		memcpy(Data, NewData, Size);
		BytesRead = 0;
		BytesInData = Size;
	}
	else
	{
		int DataRemainingToBeParsed = BytesInData - BytesRead;
		unsigned char	*Data2 = (unsigned char	*)malloc(Size + DataRemainingToBeParsed);
		memcpy(Data2, &Data[BytesRead], DataRemainingToBeParsed);
		memcpy(&Data2[DataRemainingToBeParsed], NewData, Size);
		BytesRead = 0;
		BytesInData = DataRemainingToBeParsed + Size;
		free(Data);
		Data = Data2;
	}
}

unsigned __int64 StreamParser::ReadUInt64()
{
	unsigned __int64 ret = *(unsigned __int64 *)&Data[BytesRead];
	BytesRead += sizeof(unsigned __int64);
	return ret;
}
unsigned int StreamParser::ReadUInt32()
{
	unsigned int ret = *(unsigned int *)&Data[BytesRead];
	BytesRead += sizeof(unsigned int);
	return ret;
}

int StreamParser::ReadInt32()
{
	int ret = *(int *)&Data[BytesRead];
	BytesRead += sizeof(int);
	return ret;
}

unsigned short StreamParser::ReadUInt16()
{
	unsigned short ret = *(unsigned short *)&Data[BytesRead];
	BytesRead += sizeof(unsigned short);
	return ret;
}

unsigned char StreamParser::ReadUInt8()
{
	unsigned char ret = *(unsigned char *)&Data[BytesRead];
	BytesRead += sizeof(unsigned char);
	return ret;
}

float StreamParser::ReadFloat()
{
	float ret = *(float *)&Data[BytesRead];
	BytesRead += sizeof(float);
	return ret;
}

unsigned char *StreamParser::ReadString(int SizePresent)
{
	unsigned char *ret;
	if (SizePresent == 1)
	{
		int StringLength = ReadInt32();
		assert(StringLength > 0); // if this is negative, it could be a wide string
		assert(Data[BytesRead + StringLength - 1] == 0);
		ret = &Data[BytesRead];
		BytesRead += StringLength;
	}
	else
	{
		ret = &Data[BytesRead];
		while (BytesRead < BytesInData && Data[BytesRead] != 0)
			BytesRead++;
		BytesRead++;// also read the terminator
	}

	return ret;
}

unsigned char *StreamParser::ReadWideString()
{
	int StringLength = ReadInt32();
	unsigned char *ret = &Data[BytesRead];

#ifdef _DEBUG
	assert(StringLength < 0);
	assert(Data[BytesRead - StringLength * 2] == 0);
#endif

	BytesRead -= StringLength * 2;

	return ret;
}

unsigned char *StreamParser::ReadWideString(unsigned char **OutBuffer)
{
	unsigned char *ret = &Data[BytesRead];
	int t = BytesRead;
	int Size = 0;
	while (t < BytesInData && *(unsigned short*)&Data[t] != 0)
	{
		t += 2;
		Size++;
	}
	*OutBuffer = (unsigned char *)malloc(Size + 1);
	t = 0;
	while (BytesRead < BytesInData && *(unsigned short*)&Data[BytesRead] != 0)
	{
		(*OutBuffer)[t] = (unsigned char)*(unsigned short*)&Data[BytesRead];
		t++;
		BytesRead += 2;
	}
	(*OutBuffer)[t] = 0;
	BytesRead += 2; // also read the terminator

	return ret;
}

void StreamParser::SkipBytes(int Count)
{
	BytesRead += Count;
}

void StreamParser::GuessNextValue()
{
	unsigned __int64 i64 = *(unsigned __int64 *)&Data[BytesRead];
	unsigned int i = *(unsigned int *)&Data[BytesRead];
	unsigned short s = *(unsigned short *)&Data[BytesRead];
	float f = *(float *)&Data[BytesRead];
	double d = *(double *)&Data[BytesRead];
	printf("Value at %d : c=%c,s=%d,i=%d,u=%08X,f=%.02ff,%lld\n", BytesRead, Data[BytesRead], s, i, i, f, i64);
}

void StreamParser::GuessNext4Values()
{
	GuessNextValue();
	BytesRead++;  printf("\t"); GuessNextValue();
	BytesRead++;  printf("\t\t"); GuessNextValue();
	BytesRead++;  printf("\t\t\t"); GuessNextValue();
	BytesRead -= 3;
}

int StreamParser::EndReached()
{
	return BytesRead >= BytesInData;
}

int StreamParser::GetOffset()
{
	return BytesRead;
}

void StreamParser::SkipToChunkEnd(int ChunkStart, int ChunkSize)
{
	BytesRead = ChunkStart + ChunkSize;
}

int StreamParser::GetByteAt(int Offset)
{
	return Data[Offset];
}

int StreamParser::GetMaxByteIndex()
{
	return BytesInData;
}

void StreamParser::JumpTo(int NewIndex)
{
	BytesRead = NewIndex;
}

int StreamParser::GetBytesRemain()
{
	return BytesInData - BytesRead;
}