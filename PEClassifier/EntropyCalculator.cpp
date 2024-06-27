#include <string.h>
#include <cmath>
#include "EntropyCalculator.h"

EntropyCalculator::EntropyCalculator()
{
	memset(m_byteCounter, 0, sizeof(m_byteCounter));
}

void EntropyCalculator::IngestData(const char* pBytes, const size_t lldByteCount)
{
	// sanity checks 
	if (pBytes == NULL || lldByteCount == 0)
	{
		return;
	}

	const unsigned char* puBytes = (const unsigned char*)pBytes;
	for (size_t i = 0; i < lldByteCount; i++)
	{
		m_byteCounter[puBytes[i]]++;
	}
}

// TODO:make this a constexpr
static inline int GetBitcount(unsigned char val)
{
	int count = 0;
	while (val) 
	{
		count += (val & 1);
		val >>= 1;
	}
	return count;
}

void EntropyCalculator::GetEntropy(double& out_fEntropy1Bit, double &out_fEntropy8Bit)
{
	// make sure to set values for output
	out_fEntropy1Bit = 0;
	out_fEntropy8Bit = 0;

	uint64_t lldOnes = 0;
	uint64_t lldValCount = 0;
	for (size_t i = 0; i < UINT8_MAX; i++)
	{
		lldValCount += m_byteCounter[i];
		lldOnes += GetBitcount((unsigned char)i) * m_byteCounter[i];
	}

	if (lldOnes != 0)
	{
		out_fEntropy1Bit = (double)lldOnes / (double)(lldValCount * 4);
	}

	// this might get slower the more symbols are inspected
	// if the 1 bit entropy is not decent enough, this step could be skipped
	for (size_t i = 0; i < UINT8_MAX; i++)
	{
		double symbolFreq = (double)(m_byteCounter[i]) / (double)lldValCount;
		if (symbolFreq != 0)
		{
			out_fEntropy8Bit -= symbolFreq * std::log2(symbolFreq);
		}
	}
}