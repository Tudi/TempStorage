#pragma once

#define SAF_4CC_SIZE		8
#define SAF_4CC_VAL			0x01464153
#define SAF_INCH_MULTIPLIER	25.4f
#define SAF_16BYTE_ALLIGN(x) (((x+15)/16)*16)

#pragma pack(push,1)
typedef struct Point2DF
{
	float x, y;
}Point2DF;

typedef struct SAF_File_Info
{
	char LCDName[8]; // shown on the robot preview screen
	char alwaysemptystring[64];
	int transitionCount1;
	int val2;
	int transitionCount2;
	int val4;
	Point2DF points[8]; // maybe it's 2 rects ?
	unsigned short flags1;
	// needed for the size of the structure to be dividable by 16
	char padding6[6]; // always 6
	char padding16[16];// always 16
}SAF_File_Info;

typedef struct SAF_File_Info2
{
	char val0; // always 0xf
	int val1;
	unsigned char val2; // always 4 ?
	float val3[10];
	unsigned short flags; // can have value 2 or 3. Always seen 2
	// needed for the size of the structure to be dividable by 16
	char padding16[32]; // always 16
}SAF_File_Info2;

typedef struct SAF_TransitionInfo
{
	static size_t GetSize() { return sizeof(SAF_TransitionInfo); }
	int prevSectionStartOffset; // Seen it take the value of block end offset
	int sectionEndOffset;
	char always0_1[8]; // should always be 0
	char always0_2[64]; // should always be 0 
	// unsure about the start of the structure
	float minX, minY, width, height;
	float totalLineLen;
	int pointCount; // maybe point count
	int lineCount; // maybe line count
	// needed for the size of the structure to be dividable by 16
	char padding4[4]; // always 4
	char padding16[16]; // always 16
}SAF_TransitionInfo;

// should have size 15
class SAF_PolylinePoint
{
public:
	static size_t GetSize() { return sizeof(x) + sizeof(y); }
	void ReadFromRaw(const unsigned char* buff, size_t bufSize, size_t& index);
	void WriteToRaw(unsigned char* buff, size_t bufSize, size_t& index);
	float x, y;
	int pointFlags;
	char unk1; // only present if *(char *)(param_1 + 0x40) != '\0'
 	float unk2; // only present if *(char *)(param_1 + 0x41) != '\0'
};

class SAF_Polyline
{
public:
	~SAF_Polyline();
	void ParseFromRawBuffer(const unsigned char* buff, size_t bufSize, size_t& index);
	void WriteToRawBuffer(unsigned char* buff, size_t bufSize, size_t& index);
	size_t GetSize() { return sizeof(short) + points.size() * SAF_PolylinePoint::GetSize(); }
	std::list<SAF_PolylinePoint*> points;
};

class SAF_TransitionData
{
public:
	SAF_TransitionData();
	~SAF_TransitionData();
	static int getPointByteCount() { return 8; } // not yet implemented to support multiple point types
	SAF_TransitionInfo transitionInfo;
	std::list<SAF_Polyline*> lines;
};

class SAFFile
{
	friend class SAFFile;
public:
	SAFFile();
	~SAFFile();
	// read and verify content
	int ReadFile(const char* fileName);
	void PrintContent();
	// create and write content
	void SetDisplayName(const char* newName);
	void AddNewLine();
	void AddNewLine(float firstX, float firstY);
	void AppendToLine(float nextX, float nextY);
	void AppendTransition();
	int WriteFile(const char* fileName);
	// based on sections, update file info that needs to be written to file
	void UpdateFileInfo();
	void IsEqual(SAFFile* t);
//private:
	char Header4CC[SAF_4CC_SIZE];
	SAF_File_Info fileInfo;
	SAF_File_Info2 fileInfo2;
	std::list<SAF_TransitionData*> sections;
};
#pragma pack(pop)

