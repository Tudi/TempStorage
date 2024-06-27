#include <math.h>
#include "DeviationCalculator.h"

void DeviationCalculator::IngestData(const char* pBytes, const size_t lldByteCount)
{
	// sanity checks 
	if (pBytes == NULL || lldByteCount == 0)
	{
		return;
	}

	// get the sum of values
	int64_t lldSumOfBytes = 0;
	for (size_t i = 0; i < lldByteCount; i++)
	{
		lldSumOfBytes += pBytes[i];
	}

	// get the mean of the values
	int64_t nMean = lldSumOfBytes / (int64_t)lldByteCount;

	// get the deviation on this buffer
	for (size_t chuck = 0; chuck <= (lldByteCount / 0xFFFFFFFF); chuck++)
	{
		size_t chuckStart = chuck * 0xFFFFFFFF;
		size_t chuckEnd = chuckStart + 0xFFFFFFFF;
		if (chuckEnd > lldByteCount)
		{
			chuckEnd = lldByteCount;
		}
		uint64_t chunkVariance = 0;
		for (size_t i = chuckStart; i < chuckEnd; i++)
		{
			int64_t diff = ((int64_t)pBytes[i] - nMean);
			uint64_t diffSQ = diff * diff;
			chunkVariance += diffSQ;
		}
		double fChunkVariance = sqrt((double)chunkVariance);
		fChunkVariance = fChunkVariance / (chuckEnd - chuckStart);
		if (m_fVariance == 0)
		{
			// first time init
			m_fVariance = fChunkVariance;
			m_fSectionCounter = 1;
		}
		else
		{
			// avg variance of the 2 sections
			m_fVariance += fChunkVariance;
			m_fSectionCounter++;
		}
	}
}

double DeviationCalculator::GetDeviation()
{
	if (m_fSectionCounter == 0)
	{
		return 0;
	}
	return m_fVariance / m_fSectionCounter;
}