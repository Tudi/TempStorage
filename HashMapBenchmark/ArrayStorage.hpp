#pragma once

// !!! this presumes input is sorted by key !!!
// was curious about logarithmic search speed

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
