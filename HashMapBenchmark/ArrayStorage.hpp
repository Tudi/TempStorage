#pragma once

// !!! this presumes input is sorted by key !!!
// was curious about logarithmic search speed

template <typename KeyT, typename ValT, size_t extendSize = 1000>
class ArrayStorage {
private:
	struct ElementStoreStruct
	{
		KeyT key;
		ValT val;
	};
public:
	ArrayStorage() {
		m_Values.reserve(extendSize); // Reserve initial space
	}

	void init()
	{
		m_Values.clear();
		m_Values.reserve(extendSize); // Reserve initial space
	}

	// Adds a value of type ValT at the end of the array
	const inline void Set(const KeyT &key, const ValT& value) {

		ValT* existingkey = Get(key);
		if (existingkey != NULL)
		{
			*existingkey = value;
			return;
		}

		if (m_Values.size() == m_Values.capacity()) {
			m_Values.reserve(m_Values.capacity() + extendSize); // Extend capacity by extendSize
		}

		m_Values.emplace_back();
		m_Values.back().val = value; 
		m_Values.back().key = key;

		ASSERT(m_Values.size() < 2 || reverse_memcmp<sizeof(KeyT)>(&m_Values[m_Values.size() - 1].key, &m_Values[m_Values.size() - 2].key) == 1);
	}

	// Inlined binary search for maximum speed
	inline ValT* Get(const KeyT &key) {
		size_t low = 0;
		size_t high = m_Values.size();

		while (low < high) {
			size_t mid = (high + low) / 2; // same as : low + (high - low) / 2;
			__int64 cmp = reverse_memcmp<sizeof(KeyT)>(&m_Values[mid].key, &key);

			if (cmp < 0) {
				low = mid + 1;  // Key is in the upper half
			}
			else if (cmp > 0) {
				high = mid;     // Key is in the lower half
			}
			else {
				return &m_Values[mid].val;  // Found the key
			}
		}

		return nullptr;  // Not found
	}

	inline size_t size() { return m_Values.size(); }

private:
	template <size_t n>
	inline int reverse_memcmp(const void* s1, const void* s2) {
		if constexpr (n == 1)
		{
			const uint8_t* p1 = static_cast<const uint8_t*>(s1);
			const uint8_t* p2 = static_cast<const uint8_t*>(s2);
			if (p1[0] < p2[0])
				return -1;
			if (p1[0] > p2[0])
				return 1;
			return 0;
		}
		else if constexpr (n == 2)
		{
			const uint16_t* p1 = static_cast<const uint16_t*>(s1);
			const uint16_t* p2 = static_cast<const uint16_t*>(s2);
			if (p1[0] < p2[0])
				return -1;
			if (p1[0] > p2[0])
				return 1;
			return 0;
		}
		else if constexpr (n == 4)
		{
			const uint32_t* p1 = static_cast<const uint32_t*>(s1);
			const uint32_t* p2 = static_cast<const uint32_t*>(s2);
			if (p1[0] < p2[0])
				return -1;
			if (p1[0] > p2[0])
				return 1;
			return 0;
		}
		else if constexpr (n == 8)
		{
			const uint64_t* p1 = static_cast<const uint64_t*>(s1);
			const uint64_t* p2 = static_cast<const uint64_t*>(s2);
			if (p1[0] < p2[0])
				return -1;
			if (p1[0] > p2[0])
				return 1;
			return 0;
		}
		else if constexpr (n == 16)
		{
			const uint64_t* p1 = static_cast<const uint64_t*>(s1);
			const uint64_t* p2 = static_cast<const uint64_t*>(s2);
			if (p1[1] < p2[1])
				return -1;
			if (p1[1] > p2[1])
				return 1;
			if (p1[0] < p2[0])
				return -1;
			if (p1[0] > p2[0])
				return 1;
			return 0;
		}
		else
		{
			const uint8_t* p1 = static_cast<const uint8_t*>(s1);
			const uint8_t* p2 = static_cast<const uint8_t*>(s2);
			for (int64_t i = n - 1; i >= 0; --i) {
				if (p1[i] != p2[i]) {
					return (p1[i] < p2[i]) ? -1 : 1;
				}
			}

			return 0; // Memory blocks are equal
		}
	}

	std::vector<ElementStoreStruct> m_Values; // Storage for elements
};


// !!! this presumes input is sorted by key !!!
// was curious about logarithmic search speed

template <typename KeyT, typename ValT, size_t extendSize = 1000>
class ArrayStorage_v2 {
private:
public:
	ArrayStorage_v2() {
		m_Values.reserve(extendSize); // Reserve initial space
		m_Keys.reserve(extendSize); // Reserve initial space
	}

	void init()
	{
		m_Values.clear();
		m_Values.reserve(extendSize); // Reserve initial space
		m_Keys.clear();
		m_Keys.reserve(extendSize); // Reserve initial space
	}

	// Adds a value of type ValT at the end of the array
	const inline void Set(const KeyT& key, const ValT& value) {

		ValT* existingkey = Get(key);
		if (existingkey != NULL)
		{
			*existingkey = value;
			return;
		}

		if (m_Values.size() == m_Values.capacity()) {
			m_Values.reserve(m_Values.capacity() + extendSize); // Extend capacity by extendSize
			m_Keys.reserve(m_Keys.capacity() + extendSize); // Extend capacity by extendSize
		}

		m_Values.emplace_back(value);
		m_Keys.emplace_back(key);

		ASSERT(m_Values.size() < 2 || reverse_memcmp<sizeof(KeyT)>(&m_Keys[m_Keys.size() - 1], &m_Keys[m_Keys.size() - 2]) == 1);
	}

#if 0
	template<size_t needle_len>
	char* strnstr_unrolled(const char* haystack, size_t haystack_len, const char* needle) {
		if (needle_len > haystack_len) {
			return NULL;
		}

		size_t search_limit = haystack_len - needle_len + 1;

		// Handle different needle sizes with manual unrolling
		for (size_t i = 0; i < search_limit; i++) {
			switch (needle_len) {
			case 1:
				if (haystack[i] == needle[0]) return (char*)&haystack[i];
				break;
			case 2:
				if (memcmp(&haystack[i], needle, 2) == 0) return (char*)&haystack[i];
				break;
			case 3:
				if (memcmp(&haystack[i], needle, 3) == 0) return (char*)&haystack[i];
				break;
			case 4:
				if (memcmp(&haystack[i], needle, 4) == 0) return (char*)&haystack[i];
				break;
			case 8:
				if (memcmp(&haystack[i], needle, 8) == 0) return (char*)&haystack[i];
				break;
			case 16:
				if (memcmp(&haystack[i], needle, 16) == 0) return (char*)&haystack[i];
				break;
			default:
				if (memcmp(&haystack[i], needle, needle_len) == 0) return (char*)&haystack[i];
			}
		}

		return NULL;
	}
#endif

	// Inlined binary search for maximum speed
	inline ValT* Get(const KeyT& key) {
#if 1
		size_t low = 0;
		size_t high = m_Keys.size();

		while (low < high) {
			size_t mid = (high + low) / 2; // same as : low + (high - low) / 2;
			__int64 cmp = reverse_memcmp<sizeof(KeyT)>(&m_Keys[mid], &key);

			if (cmp < 0) {
				low = mid + 1;  // Key is in the upper half
			}
			else if (cmp > 0) {
				high = mid;     // Key is in the lower half
			}
			else {
				return &m_Values[mid];  // Found the key
			}
		}

		return nullptr;  // Not found
#else
		const char* keys = (const char*)&m_Keys[0];
		char* foundat = strnstr_unrolled<sizeof(KeyT)>(keys, m_Keys.size() * sizeof(KeyT), (char*)&key);
		if (foundat)
		{
			size_t index = (foundat - keys) / sizeof(KeyT);
			return &m_Values[index];
		}
		return NULL;
#endif
	}

	inline size_t size() { return m_Values.size(); }

private:
	template <size_t n>
	inline int reverse_memcmp(const void* s1, const void* s2) {
		if constexpr (n == 1)
		{
			const uint8_t* p1 = static_cast<const uint8_t*>(s1);
			const uint8_t* p2 = static_cast<const uint8_t*>(s2);
			if (p1[0] < p2[0])
				return -1;
			if (p1[0] > p2[0])
				return 1;
			return 0;
		}
		else if constexpr (n == 2)
		{
			const uint16_t* p1 = static_cast<const uint16_t*>(s1);
			const uint16_t* p2 = static_cast<const uint16_t*>(s2);
			if (p1[0] < p2[0])
				return -1;
			if (p1[0] > p2[0])
				return 1;
			return 0;
		}
		else if constexpr (n == 4)
		{
			const uint32_t* p1 = static_cast<const uint32_t*>(s1);
			const uint32_t* p2 = static_cast<const uint32_t*>(s2);
			if (p1[0] < p2[0])
				return -1;
			if (p1[0] > p2[0])
				return 1;
			return 0;
		}
		else if constexpr (n == 8)
		{
			const uint64_t* p1 = static_cast<const uint64_t*>(s1);
			const uint64_t* p2 = static_cast<const uint64_t*>(s2);
			if (p1[0] < p2[0])
				return -1;
			if (p1[0] > p2[0])
				return 1;
			return 0;
		}
		else if constexpr (n == 16)
		{
			const uint64_t* p1 = static_cast<const uint64_t*>(s1);
			const uint64_t* p2 = static_cast<const uint64_t*>(s2);
			if (p1[1] < p2[1])
				return -1;
			if (p1[1] > p2[1])
				return 1;
			if (p1[0] < p2[0])
				return -1;
			if (p1[0] > p2[0])
				return 1;
			return 0;
		}
		else
		{
			const uint8_t* p1 = static_cast<const uint8_t*>(s1);
			const uint8_t* p2 = static_cast<const uint8_t*>(s2);
			for (int64_t i = n - 1; i >= 0; --i) {
				if (p1[i] != p2[i]) {
					return (p1[i] < p2[i]) ? -1 : 1;
				}
			}

			return 0; // Memory blocks are equal
		}
	}

	std::vector<KeyT> m_Keys; 
	std::vector<ValT> m_Values;
};


// probably worse than previosu version. Not even going to bother debugging it

template <typename KeyT, typename ValT, size_t extendSize = 1000>
class ArrayStorage_v3 {
private:
	struct ElementStoreStruct
	{
		KeyT key;
		ValT val;
	};
	#define KeysPerCacheLine (64 / sizeof(KeyT))
	#define KeysPerKTreeNode (KeysPerCacheLine * 2)

	struct KTreeNode
	{
		KeyT keys[KeysPerKTreeNode]; // fetch keys from the sorted array at ex : 0%, 25%, 50%, 100%
		uint32_t index[KeysPerKTreeNode];
		uint32_t index_prev[KeysPerKTreeNode]; // lasy coding
	};
public:
	ArrayStorage_v3() {
		m_Values.reserve(extendSize); // Reserve initial space
		memset(&node0, 0, sizeof(node0));
		memset(&node1, 0, sizeof(node1));
	}

	void init()
	{
		m_Values.clear();
		m_Values.reserve(extendSize); // Reserve initial space
		memset(&node0, 0, sizeof(node0));
		memset(&node1, 0, sizeof(node1));
	}

	// Adds a value of type ValT at the end of the array
	const inline void Set(const KeyT& key, const ValT& value) {

		ValT* existingkey = Get(key);
		if (existingkey != NULL)
		{
			*existingkey = value;
			return;
		}

		if (m_Values.size() == m_Values.capacity()) {
			m_Values.reserve(m_Values.capacity() + extendSize); // Extend capacity by extendSize
		}

		m_Values.emplace_back();
		m_Values.back().val = value;
		m_Values.back().key = key;

		ASSERT(m_Values.size() < 2 || reverse_memcmp<sizeof(KeyT)>(&m_Values[m_Values.size() - 1].key, &m_Values[m_Values.size() - 2].key) == 1);

		// so we can actually "get" key-values
		BuildLookupKTree();
	}

	void InitLevelTreeNodes(KTreeNode *node, size_t startIndex, size_t endIndex, size_t level)
	{
		if (level > 1)
		{
			return;
		}

		// if 2 keys at indexes : 50%, 100%
		// if 3 keys at indexes : 0%, 50%, 100%
		const double segment_size = (double)(endIndex - startIndex) / (double)(KeysPerKTreeNode);

		for (size_t i = 0; i < KeysPerKTreeNode; i++)
		{
			size_t atIndex = (size_t)(startIndex + (i + 1) * segment_size);
			node->keys[i] = m_Values[atIndex].key;
			node->index[i] = (uint32_t)(atIndex);

			size_t atIndex_prev = (size_t)(startIndex + (i + 0) * segment_size);
			node->index_prev[i] = (uint32_t)(atIndex_prev);

			if (level == 0)
			{
				InitLevelTreeNodes(&node1[i], atIndex_prev, atIndex, level + 1);
			}
		}
	}
	// try to boost that logarithmic search speed using a ktree
	void BuildLookupKTree()
	{
		// in case we fail to fill them up
		memset(&node0, 0, sizeof(node0));
		memset(&node1, 0, sizeof(node1));

		if (m_Values.empty() == false)
		{
			InitLevelTreeNodes(&node0, 0, m_Values.size() - 1, 0);
		}
	}

	inline ValT* Get(const KeyT& key) {
		size_t low;
		size_t high;

//		double segment_size0 = (double)(m_Values.size() - 0) / (double)(KeysPerKTreeNode);

		for (size_t i = 0; i < KeysPerKTreeNode; i++)
		{
			__int64 cmp0 = reverse_memcmp<sizeof(KeyT)>(&node0.keys[i], &key);

//			if (cmp0 == 0 && (i + 1) * segment_size0 < m_Values.size())
			if (cmp0 == 0 && node0.index[i] < m_Values.size())
			{
//				return &m_Values[(size_t)((i + 1) * segment_size0)].val;
				return &m_Values[node0.index[i]].val;
			}

			// this key is already too large to search, result should be below us
			if (cmp0 > 0)
			{
//				low = (size_t)((i + 0) * segment_size0);
//				high = (size_t)((i + 1) * segment_size0);
//				goto got_range;

//				low = node0.index_prev[i];
//				high = node0.index[i];
//				goto got_range;

//				low = node0_Index * segment_size0;
//				high = low + segment_size0;
//				size_t segment_size1 = segment_size0 / (KeysPerKTreeNode + 1);
				for (size_t j = 0; j < KeysPerKTreeNode; j++)
				{
					__int64 cmp1 = reverse_memcmp<sizeof(KeyT)>(&node1[i].keys[j], &key);
					if (cmp1 == 0 && node1[i].index[j] < m_Values.size()) // what are the odds
					{
						return &m_Values[node1[i].index[j]].val;
					}
					if (cmp1 > 0)
					{
						low = node1[i].index_prev[j];
						high = node1[i].index[j];
						goto got_range;
					}
				}
			}
		}
		// without a range this key is probably larger than our max
		return nullptr;
got_range:
		while (low < high) {
			size_t mid = (high + low) / 2; // same as : low + (high - low) / 2;
			__int64 cmp = reverse_memcmp<sizeof(KeyT)>(&m_Values[mid].key, &key);

			if (cmp < 0) {
				low = mid + 1;  // Key is in the upper half
			}
			else if (cmp > 0) {
				high = mid;     // Key is in the lower half
			}
			else {
				return &m_Values[mid].val;  // Found the key
			}
		}

		return nullptr;  // Not found
	}

	inline size_t size() { return m_Values.size(); }

private:
	template <size_t n>
	inline int reverse_memcmp(const void* s1, const void* s2) {
		if constexpr (n == 1)
		{
			const uint8_t* p1 = static_cast<const uint8_t*>(s1);
			const uint8_t* p2 = static_cast<const uint8_t*>(s2);
			if (p1[0] < p2[0])
				return -1;
			if (p1[0] > p2[0])
				return 1;
			return 0;
		}
		else if constexpr (n == 2)
		{
			const uint16_t* p1 = static_cast<const uint16_t*>(s1);
			const uint16_t* p2 = static_cast<const uint16_t*>(s2);
			if (p1[0] < p2[0])
				return -1;
			if (p1[0] > p2[0])
				return 1;
			return 0;
		}
		else if constexpr (n == 4)
		{
			const uint32_t* p1 = static_cast<const uint32_t*>(s1);
			const uint32_t* p2 = static_cast<const uint32_t*>(s2);
			if (p1[0] < p2[0])
				return -1;
			if (p1[0] > p2[0])
				return 1;
			return 0;
		}
		else if constexpr (n == 8)
		{
			const uint64_t* p1 = static_cast<const uint64_t*>(s1);
			const uint64_t* p2 = static_cast<const uint64_t*>(s2);
			if (p1[0] < p2[0])
				return -1;
			if (p1[0] > p2[0])
				return 1;
			return 0;
		}
		else if constexpr (n == 16)
		{
			const uint64_t* p1 = static_cast<const uint64_t*>(s1);
			const uint64_t* p2 = static_cast<const uint64_t*>(s2);
			if (p1[1] < p2[1])
				return -1;
			if (p1[1] > p2[1])
				return 1;
			if (p1[0] < p2[0])
				return -1;
			if (p1[0] > p2[0])
				return 1;
			return 0;
		}
		else
		{
			const uint8_t* p1 = static_cast<const uint8_t*>(s1);
			const uint8_t* p2 = static_cast<const uint8_t*>(s2);
			for (int64_t i = n - 1; i >= 0; --i) {
				if (p1[i] != p2[i]) {
					return (p1[i] < p2[i]) ? -1 : 1;
				}
			}

			return 0; // Memory blocks are equal
		}
	}

	std::vector<ElementStoreStruct> m_Values; // Storage for elements
	KTreeNode node0;
	KTreeNode node1[KeysPerKTreeNode];
};
