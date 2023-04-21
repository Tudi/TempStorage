#include "stdafx.h"

int GetDB4Data(FILE* f, SAF_TransitionData* out_db4)
{
	if (f == NULL)
	{
		return -1;
	}

	size_t bytesNeededDB4 = out_db4->transitionInfo.lineCount * sizeof(short) 
		+ out_db4->transitionInfo.pointCount * SAF_TransitionData::getPointByteCount();
	bytesNeededDB4 = ((bytesNeededDB4 + 31) / 32) * 32;
	byte* DB4Decrypted = (byte*)malloc(bytesNeededDB4);

	if (DB4Decrypted == NULL)
	{
		printf("Critical allocation error \n");
		return -1;
	}

	ReadGenericEncryptedBlock(f, bytesNeededDB4, (byte*)DB4Decrypted);

	size_t readIndex = 0;
	int lineIndex = 0;
	while (readIndex < bytesNeededDB4 && lineIndex < out_db4->transitionInfo.lineCount)
	{
		SAF_Polyline* line = new SAF_Polyline;
		line->ParseFromRawBuffer(DB4Decrypted, bytesNeededDB4, readIndex);
		out_db4->lines.push_back(line);
		lineIndex++;
	}

	free(DB4Decrypted);

	return 0;
}

SAFFile::SAFFile()
{
	memset(Header4CC, 0, sizeof(Header4CC));
	memset(&fileInfo, 0, sizeof(fileInfo));
	memset(&fileInfo2, 0, sizeof(fileInfo2));
}

SAFFile::~SAFFile()
{
	memset(Header4CC, 0, sizeof(Header4CC));
	memset(&fileInfo, 0, sizeof(fileInfo));
	memset(&fileInfo2, 0, sizeof(fileInfo2));
	for (auto itr = sections.begin(); itr != sections.end(); itr++)
	{
		delete* itr;
	}
	sections.clear();
}

int SAFFile::ReadFile(const char* fileName)
{
	FILE* f;
	errno_t openErr = fopen_s(&f, fileName, "rb");
	if (f == NULL)
	{
		printf("Failed to open input file\n");
		return -1;
	}
	fseek(f, 0, SEEK_END);
	size_t fileSize = ftell(f);
	fseek(f, 0, SEEK_SET);

	size_t readCount;

	readCount = fread(Header4CC, 1, sizeof(Header4CC), f);
	if (readCount != sizeof(Header4CC))
	{
		printf("Failed to read header 4CC\n");
		return -2;
	}
	unsigned __int64 expected4CC = 0x01464153;
	if (*(int*)Header4CC != expected4CC)
	{
		printf("File does not seem to be a SAF file\n");
		return -3;
	}

	ReadGenericEncryptedBlock(f, sizeof(fileInfo), (byte*)&fileInfo);
	ReadGenericBlock(f, sizeof(fileInfo2), (byte*)&fileInfo2);

	int transitionsParsed = 0;
	while(!feof(f) && ftell(f) + 31 < fileSize)
	{
		SAF_TransitionData* td = new SAF_TransitionData;
		ReadGenericEncryptedBlock(f, sizeof(td->transitionInfo), (byte*)&td->transitionInfo);
		GetDB4Data(f, td);
		sections.push_back(td);

		transitionsParsed++;
	}

	fclose(f);

	if (fileInfo.transitionCount1 != transitionsParsed)
	{
		printf("Parse mismatch. Was expecting %d transition, but read %d\n", fileInfo.transitionCount1, transitionsParsed);
	}

	return 0;
}

void SAFFile::PrintContent()
{
	printf("File info section :\n");
	printf("\t sigFileName : %s\n", fileInfo.sigFileName);
	printf("\t alwaysemptystring : %s\n", fileInfo.alwaysemptystring);
	printf("\t transitionCount1 : %d\n", fileInfo.transitionCount1);
	printf("\t val2 : %d (always 0)\n", fileInfo.val2);
	printf("\t transitionCount2 : %d\n", fileInfo.transitionCount2);
	printf("\t val4 : %d (always 0)\n", fileInfo.val4);
	printf("\t points : ");
	for (size_t i = 0; i < _countof(fileInfo.points); i++)
	{
		printf("%.02f/%.02f, ", fileInfo.points[i].x, fileInfo.points[i].y);
	}
	printf("\t \n");
	printf("\t flags1 : %d\n", fileInfo.flags1);

	printf("File info 2 section :\n");
	printf("\t val0 : %d (always 15)\n", fileInfo2.val0);
	printf("\t val1 : %d (always 0)\n", fileInfo2.val1);
	printf("\t val2 : %d (always 4)\n", fileInfo2.val2);
	printf("\t points : ");
	for (size_t i = 0; i < _countof(fileInfo2.val3); i++)
	{
		printf("%.02f, ", fileInfo2.val3[i]);
	}
	printf("\t \n");
	printf("\t flags1 : %d (val=3 => file has transitions, else flags=2)\n", fileInfo2.flags);

	int i = 0;
	for (auto itr = sections.begin(); itr != sections.end(); itr++, i++)
	{
		// get the total line len
		double lineLen = 0;
		for (auto itr2 = (*itr)->lines.begin(); itr2 != (*itr)->lines.end(); itr2++)
		{
			float prevX = (*(*itr2)->points.begin())->x;
			float prevY = (*(*itr2)->points.begin())->y;
			for (auto itr3 = (*itr2)->points.begin(); itr3 != (*itr2)->points.end(); itr3++)
			{
				double dx = prevX - (*itr3)->x;
				double dy = prevY - (*itr3)->y;
				lineLen += sqrt(dx * dx + dy * dy);

				prevX = (*itr3)->x;
				prevY = (*itr3)->y;
			}
		}

		printf("File transition section %d :\n", i);
		printf("\t prevSectionStartOffset : %d\n", (*itr)->transitionInfo.prevSectionStartOffset);
		printf("\t sectionEndOffset : %d\n", (*itr)->transitionInfo.sectionEndOffset);
		printf("\t somerect : %f, %f, %f, %f\n", (*itr)->transitionInfo.val5.top, (*itr)->transitionInfo.val5.left, (*itr)->transitionInfo.val5.right, (*itr)->transitionInfo.val5.bottom);
		printf("\t totalLineLen : %f (actually measured %f)\n", (*itr)->transitionInfo.totalLineLen, lineLen);
		printf("\t pointCount : %d\n", (*itr)->transitionInfo.pointCount);
		printf("\t lineCount : %d\n", (*itr)->transitionInfo.lineCount);
	}
}

SAF_TransitionData::SAF_TransitionData()
{
	memset(&transitionInfo, 0, sizeof(transitionInfo));
}

SAF_TransitionData::~SAF_TransitionData()
{
	memset(&transitionInfo, 0, sizeof(transitionInfo));
	for (auto itr = lines.begin(); itr != lines.end(); itr++)
	{
		delete* itr;
	}
	lines.clear();
}

SAF_Polyline::~SAF_Polyline()
{
	for (auto itr = points.begin(); itr != points.end(); itr++)
	{
		delete* itr;
	}
	points.clear();
}

void SAF_Polyline::ParseFromRawBuffer(const unsigned char* buf, size_t bufSize, size_t& index)
{
	short pointCount = *(short*)&buf[index];
	index += sizeof(short);
	if (pointCount == 0)
	{
		return;
	}
	for (size_t i = 0; i < pointCount && index < bufSize; i++)
	{
		SAF_PolylinePoint* newPoint = new SAF_PolylinePoint();
		newPoint->x = *(float*)&buf[index];
		index += sizeof(float);
		newPoint->y = *(float*)&buf[index];
		index += sizeof(float);
		points.push_back(newPoint);
	}
}