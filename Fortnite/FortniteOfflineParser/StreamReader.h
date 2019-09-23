#pragma once

class StreamParser
{
public:
	StreamParser();
	void AddInput(unsigned char *NewData, int Size);
	unsigned __int64 ReadUInt64();
	unsigned int ReadUInt32();
	int ReadInt32();
	unsigned short ReadUInt16();
	unsigned char ReadUInt8();
	float ReadFloat();
	unsigned char *ReadString(int SizePresent = 1);
	unsigned char *ReadWideString();
	unsigned char *ReadWideString(unsigned char **OutBuffer);
	void SkipBytes(int Count);
	void GuessNextValue();
	void GuessNext4Values();
	int EndReached();
	int GetOffset();
	void SkipToChunkEnd(int ChunkStart, int ChunkSize);
	int GetByteAt(int Offset);
	unsigned char *GetByteArray() { return (unsigned char *)&Data[BytesRead]; }
private:
	int				BytesRead; // index in our data stream
	int				BytesInData;
	unsigned char	*Data;
};