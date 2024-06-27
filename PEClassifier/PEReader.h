#pragma once
#include <inttypes.h>

/*
* Minimalistic class that will load a file and will be able to fetch text sections for us
*/

/// <summary>
/// For proper error logging and treatment
/// </summary>
enum PEReaderErrorCodes
{
    NoErr = 0,
    MissingFileName,
    FailedToOpenFile,
    FailedToGetFileSize,
    FailedToAllocateBuffer,
    FileContentChanged,
    MissingInputParam,
    NotInitialized,
    NotProperPEFile,
    NoMoreSections,
};

class PEReader
{
public:
    /// <summary>
    /// Constructor
    /// </summary>
    PEReader();
    /// <summary>
    /// Destructor
    /// </summary>
    ~PEReader();
    /// <summary>
    /// Load a file into memory so we can parse it later
    /// </summary>
    /// <param name="szFileName"></param>
    /// <returns></returns>
    PEReaderErrorCodes LoadFile(const char* szFileName);
    /// <summary>
    /// Get the offeset of a loaded file so we may parse it a section
    /// </summary>
    /// <param name="out_nextSection"></param>
    /// <param name="out_NextSectionSize"></param>
    /// <returns></returns>
    PEReaderErrorCodes SeekNextSection(const char** out_nextSection, size_t& out_NextSectionSize);

private:
    /// <summary>
    /// Used when this class gets reused to load a new file
    /// </summary>
    void Reinit();
    /// <summary>
    /// Check if the loaded file is an acceptable PE file based on header inspection
    /// </summary>
    /// <returns></returns>
    int CheckProperPEFileHeader();

    // there is no need to load the file into memory. Did it anyway
    char* m_pFileContent;
    // so that we know when we should stop parsing
    size_t m_lldFileContentSize;
    // like an iterator
    uint64_t m_lldNextSectionIndex;
    // can iterate to max this section count
    uint64_t m_lldNumSections;
    // obtained by parsing a valid PE header
    uint64_t m_lldFirstSectionOffset, m_lldNextSectionOffset;
};