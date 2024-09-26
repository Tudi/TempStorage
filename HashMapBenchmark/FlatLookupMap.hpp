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

template <size_t NodeSize, size_t MaxKeyVal, size_t ValCountReserve, typename T>
class SQLResultCache
{
public:
	SQLResultCache()
	{
		// sanity checks
		assert(NodeSize > 0 && NodeSize < 0xFFFF);
		assert(ValCountReserve > 0 && ValCountReserve < 0x000FFFFF);
		LookupTable = NULL;
		ValuesTable = NULL;
		ValuesInserted = 1; // 0 is reserved for not inserted value
	}
	void clear(bool bReinit=true)
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

		m_MaxNodeLevel = 0;
		size_t tempMaxKeyVal = MaxKeyVal;
		while (tempMaxKeyVal > 0)
		{
			m_MaxNodeLevel++;
			tempMaxKeyVal = tempMaxKeyVal / NodeSize;
		}
		m_MaxNodeLevel--; // last nodes are actually offsets in values table

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
	const inline void Set(const size_t keyval, const T& val)
	{
		if (keyval >= MaxKeyVal)
		{
			assert(false);
			return;
		}
		if (NodeSize < MaxKeyVal)
		{
			size_t keyvalRemaining = keyval;
			T** LookupTableNow = LookupTable;
			for (size_t i = 0; i < m_MaxNodeLevel; i++)
			{
				size_t keyvalThisLevel = keyvalRemaining % NodeSize; // mask it to get bits / lookuptable key
				keyvalRemaining = keyvalRemaining / NodeSize;
				// if this node branch is missing, extend it
				if (LookupTableNow[keyvalThisLevel] == NULL)
				{
					LookupTableNow[keyvalThisLevel] = (T*)calloc(1, NodeSize * sizeof(T*));
				}
				// go down on this branch
				LookupTableNow = (T**)LookupTableNow[keyvalThisLevel];
			}
			assert(keyvalRemaining < NodeSize);

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
	const inline T& Get(const size_t keyval, bool &bFoundKey)
	{
		if (keyval >= MaxKeyVal)
		{
			assert(false);
			return ValuesTable[0];
		}
		if (NodeSize < MaxKeyVal)
		{
			size_t keyvalRemaining = keyval;
			T** LookupTableNow = LookupTable;
			for (size_t i = 0; i < m_MaxNodeLevel; i++)
			{
				size_t keyvalThisLevel = keyvalRemaining % NodeSize; // mask it to get bits / lookuptable key
				if (LookupTableNow[keyvalThisLevel] == NULL)
				{
					bFoundKey = false;
					return ValuesTable[0];
				}
				keyvalRemaining = keyvalRemaining / NodeSize;
				LookupTableNow = (T**)LookupTableNow[keyvalThisLevel];
			}
			bFoundKey = true;
			return ValuesTable[reinterpret_cast<size_t>(LookupTableNow[keyvalRemaining])];
		}
		else
		{
			if (LookupTable[keyval] == NULL)
			{
				bFoundKey = false;
				return ValuesTable[0];
			}
			else
			{
				bFoundKey = true;
				return *LookupTable[keyval];
			}
		}
	}
private:
	void RecursiveFreeNode(T** LookupTable, size_t recursionLevel)
	{
		// last level of lookuptable is the offset in values table
		if (recursionLevel == m_MaxNodeLevel)
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

	size_t		m_MaxNodeLevel;
	T			**LookupTable;

	size_t		m_ValuesTableReserve; // number of values to increase our values table
	size_t		m_ValuesTableSize; 
	T			*ValuesTable;
	size_t		ValuesInserted;
};

// !!! this presumes input is sorted by key !!!
template <typename T, size_t extendSize, size_t keyOffset, size_t keySize>
class ArrayStorage {
public:
	ArrayStorage() {
		storage.reserve(extendSize); // Reserve initial space
	}

	void init()
	{
		storage.clear();
		storage.reserve(extendSize); // Reserve initial space
	}

	// Adds a value of type T at the end of the array
	const inline void Set(const size_t key, const T& value) {

		T* existingkey = Get(&key);
		if (existingkey != NULL)
		{
			*existingkey = value;
			return;
		}

		if (storage.size() == storage.capacity()) {
			storage.reserve(storage.capacity() + extendSize); // Extend capacity by extendSize
		}

		storage.push_back(value); // Add value at the end

		char* keyLocation = reinterpret_cast<char*>(&storage.back()) + keyOffset;

		if (sizeof(key) >= keySize) {
			memcpy(keyLocation, &key, keySize); // Copy only the required number of bytes
		}
		else {
			memcpy(keyLocation, &key, sizeof(key)); // Copy the size of the key
			memset(keyLocation + sizeof(key), 0, keySize - sizeof(key)); // Zero-fill remaining bytes
		}
	}

	// Inlined binary search for maximum speed
	inline T* Get(const void* key) {
		size_t low = 0;
		size_t high = storage.size();

		while (low < high) {
			size_t mid = low + (high - low) / 2;

			const T& midValue = storage[mid];
			__int64 cmp;
			if (keySize <= 8)
			{
				__int64 val1 = *(__int64*)(reinterpret_cast<const char*>(&midValue) + keyOffset);
				__int64 val2 = *(__int64*)(key);
				if (val1 < val2)
					cmp = -1;
				else if (val1 > val2)
					cmp = 1;
				else
					cmp = 0;
			}
			else 
			{ 
				cmp = memcmp(reinterpret_cast<const char*>(&midValue) + keyOffset, key, keySize);
			}

			if (cmp < 0) {
				low = mid + 1;  // Key is in the upper half
			}
			else if (cmp > 0) {
				high = mid;     // Key is in the lower half
			}
			else {
				return &storage[mid];  // Found the key
			}
		}

		return nullptr;  // Not found
	}

private:
	std::vector<T> storage; // Storage for elements
};
