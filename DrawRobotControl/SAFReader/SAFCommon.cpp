#include "stdafx.h"

int GetDB4Data(FILE* f, SAF_TransitionData* out_db4)
{
	if (f == NULL)
	{
		return -1;
	}

	size_t bytesNeededDB4 = out_db4->transitionInfo.lineCount * sizeof(short) 
		+ out_db4->transitionInfo.pointCount * SAF_TransitionData::getPointByteCount();
	bytesNeededDB4 = SAF_16BYTE_ALLIGN(bytesNeededDB4);
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
	// set actual values 
	*(int*)&Header4CC[0] = SAF_4CC_VAL;

	for (int i = 0; i < _countof(fileInfo.padding6); i++)
	{
		fileInfo.padding6[i] = 6;
	}

	for (int i = 0; i < _countof(fileInfo.padding16); i++)
	{
		fileInfo.padding16[i] = 16;
	}

	fileInfo2.val0 = 15;
	fileInfo2.val2 = 4;
	fileInfo2.flags = 2;
	fileInfo2.val3[0] = 0.0f;
	fileInfo2.val3[1] = 0.0f;
	fileInfo2.val3[2] = 0.5f;
	fileInfo2.val3[3] = 0.0f;
	fileInfo2.val3[4] = 2.0f;
	fileInfo2.val3[5] = 1.0f;
	fileInfo2.val3[6] = 1.0f;
	fileInfo2.val3[7] = 0.1f;
	fileInfo2.val3[8] = 0.0f;
	fileInfo2.val3[9] = 0.1f;

	for (int i = 0; i < _countof(fileInfo2.padding16); i++)
	{
		fileInfo2.padding16[i] = 16;
	}
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
	if (*(int*)Header4CC != SAF_4CC_VAL)
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
	printf("\t Display Name : %s\n", fileInfo.LCDName);
	printf("\t alwaysemptystring : ");
	for (size_t i2 = 0; i2 < _countof(fileInfo.alwaysemptystring); i2++)
	{
		printf("%02X", fileInfo.alwaysemptystring[i2]);
	}
	printf("\n");

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
	printf("(expected : 0.00, 0.00, 0.50, 0.00, 2.00, 1.00, 1.00, 0.10, 0.00, 0.10) \n");
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

		printf("\t always1 0 : ");
		for (size_t i2 = 0; i2 < _countof((*itr)->transitionInfo.always0_1); i2++)
		{
			printf("%02X", (*itr)->transitionInfo.always0_1[i2]);
		}
		printf("\n");

		printf("\t always2 0 : ");
		for (size_t i2 = 0; i2 < _countof((*itr)->transitionInfo.always0_2); i2++)
		{
			printf("%02X", (*itr)->transitionInfo.always0_2[i2]);
		}
		printf("\n");

		printf("\t minx=%f, miny=%f, width=%f, height=%f\n", (*itr)->transitionInfo.minX, (*itr)->transitionInfo.minY, (*itr)->transitionInfo.width, (*itr)->transitionInfo.height);
		printf("\t totalLineLen : %f (actually measured %f)\n", (*itr)->transitionInfo.totalLineLen, lineLen);
		printf("\t pointCount : %d\n", (*itr)->transitionInfo.pointCount);
		printf("\t lineCount : %d\n", (*itr)->transitionInfo.lineCount);
		int lineIndex = 1;
		for (auto itr2 = (*itr)->lines.begin(); itr2 != (*itr)->lines.end() && lineIndex <= 6; itr2++, lineIndex++)
		{
			printf("\t Some points from line %d :", lineIndex);
			int pointIndex = 1;
			for (auto itr3 = (*itr2)->points.begin(); itr3 != (*itr2)->points.end() && pointIndex <= 4; itr3++, pointIndex++)
			{
				printf("(%.02f,%.02f)", (*itr3)->x, (*itr3)->y);
			}
			printf(" .. \n");
		}
		printf("\t .. \n");
	}
}

void SAFFile::UpdateFileInfo()
{
	fileInfo.transitionCount1 = fileInfo.transitionCount2 = (int)sections.size();

	fileInfo2.flags = 2;
	if (fileInfo.transitionCount1 > 1)
		fileInfo2.flags |= 1;

	int i = 0;
	int prevSectionStartOffset = 0;
	int sectionStartOffset = (int)(sizeof(Header4CC) +
		GetIVSize() + sizeof(SAF_File_Info) + GetHashSize() +
		sizeof(SAF_File_Info2) + GetHashSize());

	for (auto itr = sections.begin(); itr != sections.end(); itr++, i++)
	{
		// get the total line len
		double lineLen = 0;
		double minX = 10000, minY = 10000, maxX = -10000, maxY = -10000;
		int thisSectionSize = 2 * GetIVSize() + sizeof(SAF_TransitionInfo) + 2 * GetHashSize();
		int pointCount = 0;
		for (auto itr2 = (*itr)->lines.begin(); itr2 != (*itr)->lines.end(); itr2++)
		{
			thisSectionSize += (int)(*itr2)->GetSize();
			pointCount += (int)(*itr2)->points.size();
			float prevX = (*(*itr2)->points.begin())->x;
			float prevY = (*(*itr2)->points.begin())->y;
			for (auto itr3 = (*itr2)->points.begin(); itr3 != (*itr2)->points.end(); itr3++)
			{
				double dx = prevX - (*itr3)->x;
				double dy = prevY - (*itr3)->y;
				lineLen += sqrt(dx * dx + dy * dy);

				prevX = (*itr3)->x;
				prevY = (*itr3)->y;

				if ((*itr3)->x < minX)
				{
					minX = (*itr3)->x;
				}
				if ((*itr3)->x > maxX)
				{
					maxX = (*itr3)->x;
				}

				if ((*itr3)->y < minY)
				{
					minY = (*itr3)->y;
				}
				if ((*itr3)->y > maxY)
				{
					maxY = (*itr3)->y;
				}
			}
		}
		thisSectionSize = SAF_16BYTE_ALLIGN(thisSectionSize);

		(*itr)->transitionInfo.prevSectionStartOffset = prevSectionStartOffset;
		(*itr)->transitionInfo.sectionEndOffset = sectionStartOffset + thisSectionSize;
		(*itr)->transitionInfo.totalLineLen = (float)lineLen;
		(*itr)->transitionInfo.minX = (float)minX;
		(*itr)->transitionInfo.minY = (float)minY;
		(*itr)->transitionInfo.width = (float)(maxX - minX);
		(*itr)->transitionInfo.height = (float)(maxY - minY);
		(*itr)->transitionInfo.lineCount = (int)(*itr)->lines.size();
		(*itr)->transitionInfo.pointCount = pointCount;

		prevSectionStartOffset = sectionStartOffset;
		sectionStartOffset += thisSectionSize;
	}
}

void SAFFile::IsEqual(SAFFile* t)
{
	if (memcmp(Header4CC, t->Header4CC, sizeof(Header4CC)) != 0)
	{
		printf("4CC mismatch\n");
	}
	if (memcmp(&fileInfo, &t->fileInfo, sizeof(fileInfo)) != 0)
	{
		printf("fileInfo mismatch\n");
	}
	if (memcmp(&fileInfo2, &t->fileInfo2, sizeof(fileInfo2)) != 0)
	{
		printf("fileInfo2 mismatch\n");
	}
}

void SAFFile::SetDisplayName(const char* newName)
{
	strncpy_s(fileInfo.LCDName, sizeof(fileInfo.LCDName), newName, sizeof(fileInfo.LCDName) - 1);
}

void SAFFile::AppendTransition()
{
	SAF_TransitionData* td = new SAF_TransitionData;
	sections.push_back(td);
}

void SAFFile::AddNewLine()
{
	// file does not have any sections, create the first one
	if (sections.empty())
	{
		AppendTransition();
	}
	auto itr = sections.end();
	itr--;
	SAF_TransitionData* td = (*itr);
	SAF_Polyline* pl = new SAF_Polyline;
	td->lines.push_back(pl);
}

void SAFFile::AddNewLine(float firstX, float firstY)
{
	// file does not have any sections, create the first one
	if (sections.empty())
	{
		AppendTransition();
	}
	auto itr = sections.end();
	itr--;
	SAF_TransitionData* td = (*itr);
	SAF_Polyline* pl = new SAF_Polyline;
	SAF_PolylinePoint* pp = new SAF_PolylinePoint;
	pp->x = firstX;
	pp->y = firstY;
	pl->points.push_back(pp);
	td->lines.push_back(pl);
}

void SAFFile::AppendToLine(float nextX, float nextY)
{
	// file does not have any sections, create the first one
	if (sections.empty())
	{
		AddNewLine(nextX, nextY);
		return;
	}
	auto itrSections = sections.end();
	itrSections--;
	SAF_TransitionData* td = (*itrSections);
	if (td->lines.empty())
	{
		AddNewLine(nextX, nextY);
		return;
	}
	auto itrLines = td->lines.end();
	itrLines--;
	SAF_Polyline* pl = (*itrLines);
	SAF_PolylinePoint* pp = new SAF_PolylinePoint;
	pp->x = nextX;
	pp->y = nextY;
	pl->points.push_back(pp);
}

int SAFFile::WriteFile(const char* fileName)
{
	// make sure sections are up to date
	UpdateFileInfo();

	FILE* f;
	errno_t openErr = fopen_s(&f, fileName, "wb");
	if (f == NULL)
	{
		printf("Failed to open output file\n");
		return -1;
	}

	size_t writeCount;

	writeCount = fwrite(Header4CC, 1, sizeof(Header4CC), f);
	if (writeCount != sizeof(Header4CC))
	{
		printf("Failed to write header 4CC\n");
		return -2;
	}

	WriteGenericEncryptedBlock(f, (byte*)&fileInfo, sizeof(fileInfo));
	WriteGenericBlock(f, (byte*)&fileInfo2, sizeof(fileInfo2));

	int transitionsParsed = 0;
	for (auto itr = sections.begin(); itr != sections.end(); itr++)
	{
		WriteGenericEncryptedBlock(f, (byte*)&((*itr)->transitionInfo), sizeof((*itr)->transitionInfo));
		size_t bytesRequired = (*itr)->transitionInfo.sectionEndOffset - (*itr)->transitionInfo.prevSectionStartOffset;
		byte* tempBuff = (byte*)malloc(bytesRequired);
		if (tempBuff == NULL)
		{
			printf("Critical malloc error. Failed to obtain %zd bytes\n", bytesRequired);
			break;
		}
		memset(tempBuff, 0, bytesRequired);
		size_t writeIndex = 0;
		for (auto itr2 = (*itr)->lines.begin(); itr2 != (*itr)->lines.end(); itr2++)
		{
			(*itr2)->WriteToRawBuffer(tempBuff, bytesRequired, writeIndex);
		}
//		printf("transition %d. db4 size %zd. write index %zd\n", transitionsParsed, bytesRequired, writeIndex);
		writeIndex = SAF_16BYTE_ALLIGN(writeIndex);
		WriteGenericEncryptedBlock(f, tempBuff, writeIndex);
		transitionsParsed++;
	}

	fclose(f);

	return 0;
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
	for (size_t i = 0; i < pointCount && index < bufSize; i++)
	{
		SAF_PolylinePoint* newPoint = new SAF_PolylinePoint();
		newPoint->ReadFromRaw(buf, bufSize, index);
		points.push_back(newPoint);
	}
}

void SAF_Polyline::WriteToRawBuffer(unsigned char* buff, size_t bufSize, size_t& index)
{
	*(short*)&buff[index] = (short)points.size();
	index += sizeof(short);
	for (auto itr3 = points.begin(); itr3 != points.end(); itr3++)
	{
		(*itr3)->WriteToRaw(buff, bufSize, index);
	}
}

void SAF_PolylinePoint::ReadFromRaw(const unsigned char* buff, size_t bufSize, size_t& index)
{
	x = *(float*)&buff[index];
	index += sizeof(float);
	y = *(float*)&buff[index];
	index += sizeof(float);
}

void SAF_PolylinePoint::WriteToRaw(unsigned char* buff, size_t bufSize, size_t& index)
{
	*(float*)&buff[index] = x;
	index += sizeof(float);
	*(float*)&buff[index] = y;
	index += sizeof(float);
}
