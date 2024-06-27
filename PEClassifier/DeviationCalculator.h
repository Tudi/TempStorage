#pragma once
#include <inttypes.h>

/*
* Calculate deviation of data. Used to determine if data is compressed or not
*/ 

class DeviationCalculator
{
public:
	/// <summary>
	/// Constructor
	/// </summary>
	DeviationCalculator() {
		m_fVariance = 0;
		m_fSectionCounter = 0;
	}
	/// <summary>
	/// Measure the deviation on 8 bit symbols
	/// The variance will be avereged between multiple buffers
	/// </summary>
	/// <param name="pBytes"></param>
	/// <param name="lldByteCount"></param>
	void IngestData(const char* pBytes, const size_t lldByteCount);
	/// <summary>
	/// Get the variance so far
	/// </summary>
	/// <returns></returns>
	double GetDeviation();
private:
	double m_fVariance; // get updated all the time
	double m_fSectionCounter;
};