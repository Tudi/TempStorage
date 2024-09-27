#pragma once

/*
*/

#ifdef _DEBUG
	#define ASSERT assert
#else
	#define ASSERT(x)
#endif

// no idea why this can'ValT become a normal constexpr
#ifndef GetMaxNodeLevel
#define GetMaxNodeLevel() ( (MaxKeyVal/NodeSize) == 0 ? 0 : \
							(MaxKeyVal/NodeSize/NodeSize) == 0 ? 1 : \
							(MaxKeyVal/NodeSize/NodeSize/NodeSize) == 0 ? 2 : \
							(MaxKeyVal/NodeSize/NodeSize/NodeSize/NodeSize) == 0 ? 3 : \
							(MaxKeyVal/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize) == 0 ? 4 : \
							(MaxKeyVal/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize) == 0 ? 5 : \
							(MaxKeyVal/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize) == 0 ? 6 : \
							(MaxKeyVal/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize) == 0 ? 7 : \
							(MaxKeyVal/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize) == 0 ? 8 : \
							(MaxKeyVal/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize) == 0 ? 9 : \
							(MaxKeyVal/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize) == 0 ? 10 : \
							(MaxKeyVal/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize) == 0 ? 11 : \
							(MaxKeyVal/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize) == 0 ? 12 : \
							(MaxKeyVal/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize) == 0 ? 13 : \
							(MaxKeyVal/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize) == 0 ? 14 : \
							(MaxKeyVal/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize/NodeSize) == 0 ? 15 : 16	)
#endif

template <typename KeyT, typename ValT>
class SQLResultCache_v3
{
private:
	struct ElementStore
	{
		KeyT key;
		ValT val;
	};
	struct LookupTable
	{
		size_t keyShiftLeft;
		size_t keyMask;
		bool pointsToValue;
		size_t valueSize; // based on the key obtained we jump in the values to an offset 
		size_t valueMask; // 1 byte, 2 byte, 3 bytes
		uint8_t* values; // valueIndex = values[ partialkey * valueSize ] & valueMask;
	};
public:
	SQLResultCache_v3()
	{
	}
	void clear(bool bReinit = true)
	{
	}
	void init()
	{
	}
	~SQLResultCache_v3()
	{
	}

	const inline push_back(const KeyT &key, const ValT& val)
	{
		if (m_Values.size() == m_Values.capacity()) 
		{
			reserveSize = m_Values.capacity() + 10000;
			m_Values.reserve(reserveSize);
		}
		ElementStore elem(key, val);
		m_Values.push_back(elem);
	}

	// count the number of values
	void hashContent()
	{
		size_t nValueCount = m_Values.size();

		size_t nBytesNeededForIndexing = 1;
		while ((1 << (8 * nBytesNeededForIndexing)) <= nValueCount)
		{
			nBytesNeededForIndexing++;
		}

		KeyT nLargestKey = m_Values[0].key;
		for (size_t i = 1; i < nValueCount; i++)
		{
			if (m_Values[i].key > nLargestKey)
			{
				nLargestKey = m_Values[i].key;
			}
		}
		// get bytes needed per lookup table
	}

private:
	std::vector<ElementStore> m_Values;
};
