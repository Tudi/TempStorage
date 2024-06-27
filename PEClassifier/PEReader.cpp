#include "PEReader.h"
#include <stdlib.h>
#include <stdio.h>

// PE header format based on + cheat sheet
// https://learn.microsoft.com/en-us/windows/win32/debug/pe-format
const uint32_t m_ui32PEHeaderOffset = 0x3C;
#pragma pack(push,1)
typedef struct 
{
	uint32_t ui32_Signature;
	uint16_t ui16_MachineType;
	uint16_t ui16_NumberOfSections;
	uint32_t ui32_TimeStamp;
	uint32_t ui32_SymbolTableOffset;
	uint32_t ui32_NumberOfSymbols;
	uint16_t ui16_OptionalHeaderSize;
	uint16_t ui16_Flags;
} PEHeader;
typedef struct 
{
	char     u8_Name[8];
	uint32_t ui32_VirtualSize;
	uint32_t ui32_VirtualAddr;
	uint32_t ui32_RawDataSize;
	int32_t  i32_RawDataPointer;
	int32_t  i32_RelocPointer;
	int32_t  i32_llnumPointer;
	uint16_t ui16_NumRelocs;
	uint16_t ui16_Numlnnums;
	int32_t  ui32_Flags;
} PESectionHeader;
#pragma pack(pop)

PEReader::PEReader()
{
	m_pFileContent = NULL;
	m_lldFileContentSize = 0;
	m_lldNextSectionIndex = 0;
	m_lldNumSections = 0;
	m_lldFirstSectionOffset = 0;
	m_lldNextSectionOffset = 0;
}

PEReader::~PEReader()
{
	Reinit();
}

void PEReader::Reinit()
{
	if (m_pFileContent)
	{
		free(m_pFileContent);
		m_pFileContent = NULL;
	}
	m_lldFileContentSize = 0;
	m_lldNextSectionIndex = 0;
	m_lldNumSections = 0;
	m_lldFirstSectionOffset = 0;
	m_lldNextSectionOffset = 0;
}

static int getFileSize(FILE* f, size_t &size)
{
	// make sure it's always initialized
	size = 0;

	// sanity check
	if (f == NULL)
	{
		return 1;
	}

	// jump to the end
	if (fseek(f, 0, SEEK_END) != 0)
	{
		return 1;
	}

	// get the offset
	size = ftell(f);

	// jump back
	if (fseek(f, 0, SEEK_SET) != 0)
	{
		return 1;
	}

	return 0;
}

PEReaderErrorCodes PEReader::LoadFile(const char* szFileName)
{
	FILE* pFile;

	// sanity checks
	if (szFileName == NULL)
	{
		return PEReaderErrorCodes::MissingFileName;
	}

	// open the file
	errno_t openErr = fopen_s(&pFile, szFileName, "rb");
	if (pFile == NULL)
	{
		return PEReaderErrorCodes::FailedToOpenFile;
	}

	// clean up old content if there was one
	Reinit();

	// get number of bytes required to load the file
	if (getFileSize(pFile, m_lldFileContentSize) != 0 || m_lldFileContentSize == 0)
	{
		return PEReaderErrorCodes::FailedToGetFileSize;
	}

	// allocate file content store
	m_pFileContent = (char*)malloc(m_lldFileContentSize);
	if (m_pFileContent == NULL)
	{
		fclose(pFile);
		return PEReaderErrorCodes::FailedToAllocateBuffer;
	}

	// load file content
	size_t lldBytesReadNow;
	lldBytesReadNow = fread(m_pFileContent, 1, m_lldFileContentSize, pFile);
	if (lldBytesReadNow != m_lldFileContentSize)
	{
		fclose(pFile);
		Reinit();
		return PEReaderErrorCodes::FileContentChanged;
	}

	// first time handling this file
	if (CheckProperPEFileHeader() != 0)
	{
		fclose(pFile);
		Reinit();
		return PEReaderErrorCodes::NotProperPEFile;
	}

	fclose(pFile);

	return PEReaderErrorCodes::NoErr;
}

// TODO : add error codes in case logging will be supported
int PEReader::CheckProperPEFileHeader()
{
	// sanity checks
	if (m_pFileContent == NULL || 
		m_lldFileContentSize < sizeof(PEHeader) ||
		m_lldFileContentSize < m_ui32PEHeaderOffset)
	{
		return 1;
	}

	const uint32_t ui32_peOffset = *(uint32_t*)&m_pFileContent[m_ui32PEHeaderOffset];

	// file is too small
	if (ui32_peOffset + sizeof(PEHeader) >= m_lldFileContentSize)
	{
		return 1;
	}

	// map the header
	const PEHeader* pPH = (PEHeader*)&m_pFileContent[ui32_peOffset];
	
	// check if signiture is properly detected
	if (pPH->ui32_Signature != 0x4550) // PE00
	{
		return 1;
	}

	// copy number of sections
	m_lldNumSections = pPH->ui16_NumberOfSections;

	//`first_section_header_offset = PE_header_offset + sizeof(regular_header) + optional_header_size`
	m_lldFirstSectionOffset = ui32_peOffset + sizeof(PEHeader) + pPH->ui16_OptionalHeaderSize;
	if (m_lldFirstSectionOffset > m_lldFileContentSize)
	{
		m_lldFirstSectionOffset = 0;
		m_lldNumSections = 0;
		return 1;
	}
	m_lldNextSectionOffset = m_lldFirstSectionOffset;

	// all good. Looks like a PE file format
	return 0;
}

PEReaderErrorCodes PEReader::SeekNextSection(const char** out_nextSection, size_t& out_NextSectionSize)
{
	// sanity checks
	if (out_nextSection == NULL)
	{
		return PEReaderErrorCodes::MissingInputParam;
	}

	// init output even if we set it later ( in case we early exit )
	*out_nextSection = NULL;
	out_NextSectionSize = 0;

	if (m_pFileContent == NULL)
	{
		return PEReaderErrorCodes::NotInitialized;
	}

	// iterator reached it's end
	if (m_lldNextSectionIndex >= m_lldNumSections)
	{
		return PEReaderErrorCodes::NoMoreSections;
	}

	const PESectionHeader* pSH = (PESectionHeader*)&m_pFileContent[m_lldNextSectionOffset];
	const uint64_t dataStart = m_lldFirstSectionOffset + m_lldNumSections * sizeof(PESectionHeader);
	if (pSH->i32_RawDataPointer < dataStart)
	{
		return PEReaderErrorCodes::NoMoreSections; // bad data ?
	}
	if (pSH->i32_RawDataPointer + pSH->ui32_RawDataSize > m_lldFileContentSize)
	{
		return PEReaderErrorCodes::NoMoreSections; // bad data ?
	}

	// values to be returned
	*out_nextSection = &m_pFileContent[pSH->i32_RawDataPointer];
	out_NextSectionSize = pSH->ui32_RawDataSize;

	// point to next element
	m_lldNextSectionIndex++;
	m_lldNextSectionOffset += sizeof(PESectionHeader);

	// all good, there is a new section
	return PEReaderErrorCodes::NoErr;
}