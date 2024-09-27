#pragma once

/*
this was made to check how much worse it is compared to unordered_map

I needed a tree to look up IPV4-String values.
I needed the container to have the option to not use hashing
I needed the container to be able to change the allocator

!!!
This tree is NOT threadsafe
This tree does not have "unset" or "remove"
This tree wastes a LOT of memory. Ex : for 65k node size and 300k values, worst case might use 300k*65k*8=156000MB for keys
									  for 256 node size and 300k values, worst case might use 300k*256*8=614MB for keys
									  for 64 node size and 300k values, worst case might use 300k*64*8=153MB for keys
									  for 16 node size and 300k values, worst case might use 300k*16*8=38MB for keys
!!!

Obviously you want to guess your MaxKeyVal to be as small as possible
Obviously NodeSize should be as large as possible without allocating too much memory ( depends on your maxKeyVal )
ValCountReserve is a wild guess how many elements you might need
*/

#ifdef _DEBUG
#define ASSERT assert
#else
#define ASSERT(x)
#endif

// no idea why this can't become a normal constexpr
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

template <size_t NodeSize, size_t MaxKeyVal, size_t ValCountReserve, typename T>
class SQLResultCache
{
public:
	SQLResultCache()
	{
		// sanity checks
		ASSERT(NodeSize > 0 && NodeSize < 0xFFFF);
		ASSERT(ValCountReserve > 0 && ValCountReserve < 0x000FFFFF);
		LookupTable = NULL;
		ValuesTable = NULL;
		ValuesInserted = 1; // 0 is reserved for not inserted value
	}
	void clear(bool bReinit = true)
	{
		if (LookupTable)
		{
			RecursiveFreeNode(LookupTable, 0);
			free(LookupTable);
			LookupTable = NULL;
		}
		if (ValuesTable)
		{
			free(ValuesTable);
			ValuesTable = NULL;
		}
		if (bReinit)
		{
			init();
		}
	}

	void init()
	{
		clear(false);

#ifdef _DEBUG
		size_t m_MaxNodeLevel = GetMaxNodeLevel();
#endif

		m_ValuesTableReserve = ValCountReserve;

		LookupTable = (T**)calloc(1, NodeSize * sizeof(T*));

		m_ValuesTableSize = 0;
		ExtendValuesTable();
		ValuesInserted = 1; // 0 is reserved for not inserted value
	}
	~SQLResultCache()
	{
		clear(false);
	}

	template <size_t Level, size_t NodeSize>
	struct UnwindLoopSet {
		static inline void UnwindSet(size_t& keyvalRemaining, T**& LookupTableNow) {
			size_t keyvalThisLevel = keyvalRemaining % NodeSize;  // mask to get bits / lookuptable key
			keyvalRemaining = keyvalRemaining / NodeSize;

			// Extend the node branch if missing
			if (LookupTableNow[keyvalThisLevel] == NULL) {
				LookupTableNow[keyvalThisLevel] = (T*)calloc(1, NodeSize * sizeof(T*));
			}

			// Go down one level in the tree
			LookupTableNow = (T**)LookupTableNow[keyvalThisLevel];

			// Recursively unwind the next level
			UnwindLoopSet<Level - 1, NodeSize>::UnwindSet(keyvalRemaining, LookupTableNow);
		}
	};

	// Base case: stop recursion when Level == 0
	template <size_t NodeSize>
	struct UnwindLoopSet<0, NodeSize> {
		static inline void UnwindSet(size_t&, T**&) {
			// No more recursion, base case
		}
	};

	const inline void Set(const size_t keyval, const T& val)
	{
		if (keyval >= MaxKeyVal)
		{
			ASSERT(false);
			return;
		}
		if constexpr (NodeSize < MaxKeyVal)
		{
			size_t keyvalRemaining = keyval;
			T** LookupTableNow = LookupTable;

			constexpr size_t MaxLevel = GetMaxNodeLevel();
			ASSERT(GetMaxNodeLevel_() == MaxLevel);

			UnwindLoopSet<MaxLevel, NodeSize>::UnwindSet(keyvalRemaining, LookupTableNow);

			ASSERT(keyvalRemaining < NodeSize);

			// value has not yet been stored
			if (LookupTableNow[keyvalRemaining] == NULL)
			{
				LookupTableNow[keyvalRemaining] = reinterpret_cast<T*>(ValuesInserted);
				ValuesTable[ValuesInserted] = val;
				ValuesInserted++;
				ExtendValuesTable();
			}
			else
			{
				ValuesTable[reinterpret_cast<size_t>(LookupTableNow[keyvalRemaining])] = val;
			}
		}
		else
		{
			// value has not yet been stored
			if (LookupTable[keyval] == NULL)
			{
				ValuesTable[ValuesInserted] = val;
				LookupTable[keyval] = &ValuesTable[ValuesInserted];
				ValuesInserted++;
				ExtendValuesTable();
			}
			else
			{
				*LookupTable[keyval] = val;
			}
		}
	}

	const inline T* Get(const size_t keyval)
	{
		if (keyval >= MaxKeyVal)
		{
			ASSERT(false);
			return NULL;
		}
		if constexpr (NodeSize < MaxKeyVal)
		{
			size_t keyvalRemaining = keyval;
			T** LookupTableNow = LookupTable;
			constexpr size_t MaxLevel = GetMaxNodeLevel();
			ASSERT(GetMaxNodeLevel_() == MaxLevel);
			return UnwindRecursiveGet<MaxLevel>(keyvalRemaining, LookupTableNow);
		}
		else
		{
			return LookupTable[keyval];
		}
	}
private:


	template <size_t Level>
	constexpr const T* UnwindRecursiveGet(const size_t keyvalRemaining, T** LookupTableNow) const
	{
		if constexpr (Level > 0)
		{
			size_t keyvalThisLevel = keyvalRemaining % NodeSize;
			if (LookupTableNow[keyvalThisLevel] == NULL)
			{
				return NULL;
			}
			return UnwindRecursiveGet<Level - 1>(keyvalRemaining / NodeSize,
				(T**)LookupTableNow[keyvalThisLevel]);
		}
		else
		{
			return &ValuesTable[reinterpret_cast<size_t>(LookupTableNow[keyvalRemaining])];
		}
	}

	template <size_t MaxKeyValNow, size_t NodeSizeNow>
	constexpr const size_t GetMaxNodeLevel__() const
	{
		if constexpr (MaxKeyValNow > NodeSizeNow)
		{
			return 1 + GetMaxNodeLevel__<(MaxKeyValNow / NodeSizeNow), NodeSizeNow>();
		}
		else
		{
			return 0;
		}
	}

	size_t GetMaxNodeLevel_() const
	{
		size_t ret = 0;
		size_t tempMaxKeyVal = MaxKeyVal;
		while ((tempMaxKeyVal / NodeSize) > 0)
		{
			ret++;
			tempMaxKeyVal = tempMaxKeyVal / NodeSize;
		}
		return ret;
	}

	void RecursiveFreeNode(T** LookupTable, size_t recursionLevel)
	{
		// last level of lookuptable is the offset in values table
		if (recursionLevel == GetMaxNodeLevel())
		{
			return;
		}
		for (size_t i = 0; i < NodeSize; i++)
		{
			if (LookupTable[i] != NULL)
			{
				RecursiveFreeNode((T**)LookupTable[i], recursionLevel + 1);
			}
		}
	}
	inline void ExtendValuesTable()
	{
		if (ValuesInserted >= m_ValuesTableSize)
		{
			T* newValuesTable = (T*)realloc(ValuesTable, (m_ValuesTableSize + m_ValuesTableReserve) * sizeof(T));
			if (newValuesTable)
			{
				ValuesTable = newValuesTable;
				m_ValuesTableSize += m_ValuesTableReserve;
			}
		}
	}

	T** LookupTable;

	size_t		m_ValuesTableReserve; // number of values to increase our values table
	size_t		m_ValuesTableSize;
	T* ValuesTable;
	size_t		ValuesInserted;
};
