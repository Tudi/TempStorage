#include <stdio.h>
#include "PEReader.h"
#include "DeviationCalculator.h"
#include "EntropyCalculator.h"

void CategorizeFile(const char* szCurFileName)
{
	PEReader lPEReader;

	// load file content
	if (lPEReader.LoadFile(szCurFileName) != PEReaderErrorCodes::NoErr)
	{
		// should catch the error and print it ? Would mess up the required output formatting
		return;
	}

	// seek to next header
	const char* pNextSection;
	size_t lldNextSectionSize;
	DeviationCalculator deviationCalc;
	EntropyCalculator entropyCalc;
	while (lPEReader.SeekNextSection(&pNextSection, lldNextSectionSize) == PEReaderErrorCodes::NoErr)
	{
		// feed it to deviation calculator
		deviationCalc.IngestData(pNextSection, lldNextSectionSize);
		// feed it to entropy calculator
		entropyCalc.IngestData(pNextSection, lldNextSectionSize);
	}

	// clasify the file
	double fDeviation = deviationCalc.GetDeviation();
	double fEntropy1Bit, fEntropy8Bit;
	entropyCalc.GetEntropy(fEntropy1Bit, fEntropy8Bit);

	const char* szFileType;
	if (fEntropy1Bit > 0.7 && fEntropy8Bit > 7 && fDeviation < 50)
	{
		szFileType = "packed";
	}
	else
	{
		szFileType = "regular";
	}
	printf("%-8s %-8s\n", szCurFileName, szFileType);
}