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
			size_t mid = low + (high - low) / 2;

			const auto& midValue = m_Values[mid];
			__int64 cmp;
			cmp = reverse_memcmp<sizeof(KeyT)>(&midValue.key, &key);

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
