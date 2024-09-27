#pragma once


// kinda unrealistic situation where values can be stored in a simple array
template <typename T>
class DirectLookupMap
{
private:
	struct InternalElementStore
	{
		char flags; // 0 if not set
		T val;
	};
public:
	DirectLookupMap()
	{
		MaxKey = 0;
		LookupTable = NULL;
	}
	void Init(size_t Max)
	{
		if (LookupTable)
		{
			free(LookupTable);
		}
		MaxKey = Max;
		LookupTable = (InternalElementStore*)calloc(1, Max * sizeof(InternalElementStore));
	}
	~DirectLookupMap()
	{
		free(LookupTable);
	}
	const inline void Set(const size_t index, const T& val)
	{
		assert(index < MaxKey);
		if (index < MaxKey)
		{
			LookupTable[index].flags = 1;
			LookupTable[index].val = val;
		}
	}
	const inline T* Get(const size_t index)
	{
		assert(index < MaxKey);
		if (index < MaxKey && LookupTable[index].flags == 1)
		{
			return &LookupTable[index].val;
		}
		return NULL;
	}
private:
	size_t					MaxKey;
	InternalElementStore	*LookupTable;
};

// semi unrealistic situation when we can translate key to values without collision
// this sacrifices speed in favor of saving memory ex : lookuptable[12323454] = valindex; values[valindex]=mystruct;
template <typename T>
class Indirect1LayerLookupMap
{
public:
	Indirect1LayerLookupMap()
	{
		MaxKey = 0;
		LookupTable = NULL;
		ValuesTable = NULL;
		ValuesInserted = 1; // 0 is reserved for not inserted value
	}
	void Init(size_t Max)
	{
		if (LookupTable)
		{
			free(LookupTable);
		}
		if (ValuesTable)
		{
			free(ValuesTable);
		}
		MaxKey = Max + 5; // have to find out how on earth this gets larger than max
		LookupTable = (uint32_t*)calloc(1, (Max + 1) * sizeof(uint32_t));
		ValuesTable = (T*)calloc(1, (Max + 1) * sizeof(T));
		ValuesInserted = 1; // 0 is reserved for not inserted value
	}
	~Indirect1LayerLookupMap()
	{
		free(LookupTable);
		LookupTable = NULL;
		free(ValuesTable);
		ValuesTable = NULL;
	}
	const inline void Set(const size_t index, const T& val)
	{
		assert(index < MaxKey);
		assert(LookupTable[index] < MaxKey);
		// value has not yet been stored
		if (LookupTable[index] == 0)
		{
			if (ValuesInserted < MaxKey)
			{
				LookupTable[index] = (uint32_t)ValuesInserted;
				ValuesInserted++;
			}
			else
			{
				return;
			}
			assert(ValuesInserted <= MaxKey);
		}
		ValuesTable[LookupTable[index]] = val;
	}
	const inline T* Get(const size_t index)
	{
		assert(index < MaxKey);
		assert(LookupTable[index] < MaxKey);
		if (LookupTable[index] != 0)
		{
			return &ValuesTable[LookupTable[index]];
		}
		return NULL;
	}
private:
	size_t			MaxKey;
	uint32_t		* LookupTable;
	T				* ValuesTable;
	size_t			ValuesInserted;
};
