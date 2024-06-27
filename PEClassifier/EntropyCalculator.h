#pragma once
#include <inttypes.h>

/*
* Calculate deviation of data. Used to determine if data is compressed or not
*/

class EntropyCalculator
{
public:
	// constructor
	EntropyCalculator();
	/// <summary>
	/// Generate symbol statistics on the next buffer
	/// </summary>
	/// <param name="pBytes"></param>
	/// <param name="lldByteCount"></param>
	void IngestData(const char* pBytes, const size_t lldByteCount);
	/// <summary>
	/// Entropy 1 means there is perfect balance between 0 and 1 symbols
	/// Perfect entropy would mean to inspect all symbol sizes to make sure the distribution is even
	/// </summary>
	/// <param name="out_fEntropy1Bit"></param>
	/// <param name="out_fEntropy8Bit"></param>
	void GetEntropy(double &out_fEntropy1Bit, double &out_fEntropy8Bit);
private:
	// right now only generates statistics on 8bit symbols. Would be great to extend it
	uint64_t m_byteCounter[UINT8_MAX+1];
}; 
