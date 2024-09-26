#include <Windows.h>
#include <map>
#include <unordered_map>
#include <conio.h>
#include <assert.h>
#include <string>
#include "FlatLookupMap.hpp"

// very basic test to see order of magnitude differences between containers that use different number of indirections to get value based on key

struct TestStorageWithStruct
{
	void AppendState(const TestStorageWithStruct &addme)
	{
		mystate += addme.mystate;
	}
	char myrowkey[8]; // in my case this would be an IPV6 value
	int mystate;
	size_t someblabla;
};

// anti optimization where direct lookup map has an advantage of stream erading memory
size_t* g_IndexSetOrder = NULL;
size_t* g_IndexGetOrder = NULL;

char* g_RunnedFunctionNames[15] = {};
size_t g_RunningTestIndex = 0;

// kinda unrealistic situation where values can be stored in a simple array
template <typename T>
class DirectLookupMap
{
public:
	DirectLookupMap()
	{
		MaxKey = 0;
		LookupTable = NULL;
		LookupTableIfValueSet = NULL;
	}
	void Init(size_t Max)
	{
		if (LookupTable)
		{
			free(LookupTable);
		}
		if (LookupTableIfValueSet)
		{
			free(LookupTableIfValueSet);
		}
		MaxKey = Max;
		LookupTable = (T*)calloc(1, Max * sizeof(T));
		LookupTableIfValueSet = (bool*)calloc(1, Max * sizeof(bool));
	}
	~DirectLookupMap()
	{
		free(LookupTable);
		free(LookupTableIfValueSet);
	}
	const inline void Set(const size_t index, const T &val)
	{
		assert(index < MaxKey);
		LookupTable[index] = val;
		LookupTableIfValueSet[index] = true;
	}
	const inline T &Get(const size_t index, bool &bFoundKey)
	{
		assert(index < MaxKey);
		if (LookupTableIfValueSet[index] == true)
		{
			bFoundKey = true; // oops, need to store "set" state in a bit_vect or something
			return LookupTable[index];
		}
		bFoundKey = false;
		return LookupTable[0]; 
	}
private:
	size_t		MaxKey;
	T			*LookupTable;
	bool		*LookupTableIfValueSet;
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
	const inline void Set(const size_t index, const T &val)
	{
		assert(index < MaxKey);
		assert(LookupTable[index] < MaxKey);
		// value has not yet been stored
		if (LookupTable[index] == 0)
		{
			LookupTable[index] = (uint32_t)ValuesInserted;
			ValuesTable[ValuesInserted] = val;
			ValuesInserted++;
			assert(ValuesInserted <= MaxKey);
		}
		else
		{
			ValuesTable[LookupTable[index]] = val;
		}
	}
	const inline T &Get(const size_t index, bool &bFoundKey)
	{
		assert(index < MaxKey);
		assert(LookupTable[index] < MaxKey);
		if (LookupTable[index] != 0)
		{
			bFoundKey = true;
			return ValuesTable[LookupTable[index]];
		}
		bFoundKey = false;
		return ValuesTable[0];
	}
private:
	size_t			MaxKey;
	uint32_t	*LookupTable;
	T			*ValuesTable;
	size_t		ValuesInserted;
};

SIZE_T GetHeapMemoryUsage()
{
	HANDLE hHeap = GetProcessHeap();  // Get handle to the default heap
	if (hHeap == NULL) {
		return 0;
}

	PROCESS_HEAP_ENTRY entry;
	entry.lpData = NULL;

	SIZE_T totalAllocatedBytes = 0;

	// Walk through the heap to find allocated blocks
	while (HeapWalk(hHeap, &entry) != FALSE)
	{
		if ((entry.wFlags & PROCESS_HEAP_ENTRY_BUSY) != 0) // Memory is allocated
		{
			totalAllocatedBytes += entry.cbData;  // Accumulate the allocated size
		}
	}

	if (GetLastError() != ERROR_NO_MORE_ITEMS)
	{
		return 0;
	}

	return totalAllocatedBytes;
}

// Custom hash function for uint32_t
struct CustomHash {
	std::size_t operator()(size_t key) const {
		return key;
	}
};

#ifndef _DEBUG
	const size_t maxKeyValue = 500000;
	const size_t REPEAT_TESTS_COUNT = 10; // if a test takes less than a second, that is unmeasurable
#else
	const size_t maxKeyValue = 500000;
	const size_t REPEAT_TESTS_COUNT = 1; // if a test takes less than a second, that is unmeasurable
#endif

DirectLookupMap<TestStorageWithStruct> g_DirectLookupMap;
Indirect1LayerLookupMap<TestStorageWithStruct> g_Indirect1LayerLookupMap;
SQLResultCache<128, maxKeyValue, 0xFFFF, TestStorageWithStruct> g_Tree32;
std::map<size_t, TestStorageWithStruct> g_StdMap;
std::unordered_map<size_t, TestStorageWithStruct, CustomHash> g_StdUnorderedMap;
ArrayStorage<TestStorageWithStruct, 0xFFFF, offsetof(TestStorageWithStruct, myrowkey), sizeof(TestStorageWithStruct::myrowkey)> g_ArrayStorage;

TestStorageWithStruct g_useThisForStorageTest;

template<bool bTestInit, bool bTestSet, bool bTestGet>
TestStorageWithStruct RunHashTest(size_t MaxCount)
{
	TestStorageWithStruct result = {};	//needs to exist to avoid optimisations from compiler

	if (bTestInit)
	{
		g_StdMap.clear();
	}

	if (bTestSet)
	{
		//fill test
		for (size_t i = 0; i < MaxCount; i++)
			g_StdMap[g_IndexSetOrder[i]] = g_useThisForStorageTest;
	}

	if (bTestGet)
	{
		//search test
		for (size_t i = 0; i < MaxCount; i++)
		{
			const size_t getValForKey = g_IndexGetOrder[i];
			auto itr = g_StdMap.find(getValForKey);
			if (itr != g_StdMap.end())
			{
				result.AppendState(itr->second);
			}
		}
	}

	//anti optimisation dummy return
	return result;

}

template<bool bTestInit, bool bTestSet, bool bTestGet>
LONGLONG BenchmarkHash(size_t MaxCount, bool bPrintRes)
{
	TestStorageWithStruct junk = {};
	LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
	LARGE_INTEGER Frequency;
	QueryPerformanceFrequency(&Frequency);
	QueryPerformanceCounter(&StartingTime);

	junk = RunHashTest<bTestInit, bTestSet, bTestGet>(MaxCount);

	QueryPerformanceCounter(&EndingTime);
	ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
	ElapsedMicroseconds.QuadPart *= 1000000;
	ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;

	if (bPrintRes)
	{
		printf("Time spent in % 33s : %f. Junk %d\n", __FUNCTION__, (float)ElapsedMicroseconds.QuadPart, junk.mystate);
		g_RunnedFunctionNames[g_RunningTestIndex] = __FUNCTION__;
	}

	return ElapsedMicroseconds.QuadPart;
}

template<bool bTestInit, bool bTestSet, bool bTestGet>
TestStorageWithStruct RunUnorderedMapTest(size_t MaxCount)
{
	TestStorageWithStruct result = {};	//needs to exist to avoid optimisations from compiler

	if (bTestInit)
	{
		g_StdUnorderedMap.clear();
	}

	//fill test
	if (bTestSet)
	{
		for (size_t i = 0; i < MaxCount; i++)
			g_StdUnorderedMap[g_IndexSetOrder[i]] = g_useThisForStorageTest;
	}

	//search test
	if (bTestGet)
	{
		for (size_t i = 0; i < MaxCount; i++)
		{
			const size_t getValForKey = g_IndexGetOrder[i];
			auto itr = g_StdUnorderedMap.find(getValForKey);
			if (itr != g_StdUnorderedMap.end())
			{
				result.AppendState(itr->second);
			}
		}
	}

	//anti optimisation dummy return
	return result;

}

template<bool bTestInit, bool bTestSet, bool bTestGet>
LONGLONG BenchmarkUnorderedHash(size_t MaxCount, bool bPrintRes)
{
	TestStorageWithStruct junk = {};
	LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
	LARGE_INTEGER Frequency;
	QueryPerformanceFrequency(&Frequency);
	QueryPerformanceCounter(&StartingTime);

	junk = RunUnorderedMapTest<bTestInit, bTestSet, bTestGet>(MaxCount);

	QueryPerformanceCounter(&EndingTime);
	ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
	ElapsedMicroseconds.QuadPart *= 1000000;
	ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;

	if (bPrintRes)
	{
		printf("Time spent in % 33s : %f. Junk %d\n", __FUNCTION__, (float)ElapsedMicroseconds.QuadPart, junk.mystate);
		g_RunnedFunctionNames[g_RunningTestIndex] = __FUNCTION__;
	}
	return ElapsedMicroseconds.QuadPart;
}

template<bool bTestInit, bool bTestSet, bool bTestGet>
TestStorageWithStruct RunLookupTableTest(size_t MaxCount)
{
	TestStorageWithStruct result = {};	//needs to exist to avoid optimisations from compiler

	if (bTestInit)
	{
		g_DirectLookupMap.Init(MaxCount);
	}

	if (bTestSet)
	{
		//fill test
		for (size_t i = 0; i < MaxCount; i++)
			g_DirectLookupMap.Set(g_IndexSetOrder[i], g_useThisForStorageTest);
	}

	if (bTestGet)
	{
		//search test
		for (size_t i = 0; i < MaxCount; i++)
		{
			const size_t getValForKey = g_IndexGetOrder[i];
			bool bFoundKey;
			const TestStorageWithStruct& val = g_DirectLookupMap.Get(getValForKey, bFoundKey);
			if (bFoundKey)
			{
				result.AppendState(val);
			}
		}
	}

	//anti optimisation dummy return
	return result;
}

template<bool bTestInit, bool bTestSet, bool bTestGet>
LONGLONG BenchmarkLookuptable(size_t MaxCount, bool bPrintRes)
{
	TestStorageWithStruct junk = {};
	LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
	LARGE_INTEGER Frequency;
	QueryPerformanceFrequency(&Frequency);
	QueryPerformanceCounter(&StartingTime);

	junk = RunLookupTableTest<bTestInit, bTestSet, bTestGet>(MaxCount);

	QueryPerformanceCounter(&EndingTime);
	ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
	ElapsedMicroseconds.QuadPart *= 1000000;
	ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;

	if (bPrintRes)
	{
		printf("Time spent in % 33s : %f. Junk %d\n", __FUNCTION__, (float)ElapsedMicroseconds.QuadPart, junk.mystate);
		g_RunnedFunctionNames[g_RunningTestIndex] = __FUNCTION__;
	}

	return ElapsedMicroseconds.QuadPart;
}

template<bool bTestInit,bool bTestSet,bool bTestGet>
TestStorageWithStruct RunLookupTableTest1Indirection(size_t MaxCount)
{
	TestStorageWithStruct result = {};	//needs to exist to avoid optimisations from compiler

	if (bTestInit)
	{
		g_Indirect1LayerLookupMap.Init(MaxCount);
	}

	//fill test
	if (bTestSet)
	{
		for (size_t i = 0; i < MaxCount; i++)
			g_Indirect1LayerLookupMap.Set(g_IndexSetOrder[i], g_useThisForStorageTest);
	}

	//search test
	if (bTestGet)
	{
		for (size_t i = 0; i < MaxCount; i++)
		{
			const size_t getValForKey = g_IndexGetOrder[i];
			bool bFoundKey;
			const TestStorageWithStruct& val = g_Indirect1LayerLookupMap.Get(getValForKey, bFoundKey);
			if (bFoundKey)
			{
				result.AppendState(val);
			}
		}
	}

	//anti optimisation dummy return
	return result;
}

template<bool bTestInit, bool bTestSet, bool bTestGet>
LONGLONG BenchmarkLookuptable1Indirection(size_t MaxCount, bool bPrintRes)
{
	TestStorageWithStruct junk = {};
	LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
	LARGE_INTEGER Frequency;
	QueryPerformanceFrequency(&Frequency);
	QueryPerformanceCounter(&StartingTime);

	junk = RunLookupTableTest1Indirection<bTestInit, bTestSet, bTestGet>(MaxCount);

	QueryPerformanceCounter(&EndingTime);
	ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
	ElapsedMicroseconds.QuadPart *= 1000000;
	ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;

	if (bPrintRes)
	{
		printf("Time spent in % 33s : %f. Junk %d\n", __FUNCTION__, (float)ElapsedMicroseconds.QuadPart, junk.mystate);
		g_RunnedFunctionNames[g_RunningTestIndex] = __FUNCTION__;
	}

	return ElapsedMicroseconds.QuadPart;
}


template<bool bTestInit, bool bTestSet, bool bTestGet>
TestStorageWithStruct RunTreeLookupTable32Test(size_t MaxCount)
{
	TestStorageWithStruct result = {};	//needs to exist to avoid optimisations from compiler

	if (bTestInit)
	{
		g_Tree32.init();
	}

	//fill test
	if (bTestSet)
	{
		for (size_t i = 0; i < MaxCount; i++)
			g_Tree32.Set(g_IndexSetOrder[i], g_useThisForStorageTest);
	}

	//search test
	if (bTestGet)
	{
		for (size_t i = 0; i < MaxCount; i++)
		{
			const size_t getValForKey = g_IndexGetOrder[i];
			bool bFoundKey;
			const TestStorageWithStruct& val = g_Tree32.Get(getValForKey, bFoundKey);
			if (bFoundKey)
			{
				result.AppendState(val);
			}
		}
	}

	//anti optimisation dummy return
	return result;
}

template<bool bTestInit, bool bTestSet, bool bTestGet>
LONGLONG BenchmarkTreeLookuptable32(size_t MaxCount, bool bPrintRes)
{
	TestStorageWithStruct junk = {};
	LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
	LARGE_INTEGER Frequency;
	QueryPerformanceFrequency(&Frequency);
	QueryPerformanceCounter(&StartingTime);

	junk = RunTreeLookupTable32Test<bTestInit, bTestSet, bTestGet>(MaxCount);

	QueryPerformanceCounter(&EndingTime);
	ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
	ElapsedMicroseconds.QuadPart *= 1000000;
	ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;

	if (bPrintRes)
	{
		printf("Time spent in % 33s : %f. Junk %d\n", __FUNCTION__, (float)ElapsedMicroseconds.QuadPart, junk.mystate);
		g_RunnedFunctionNames[g_RunningTestIndex] = __FUNCTION__;
	}

	return ElapsedMicroseconds.QuadPart;
}

template<bool bTestInit, bool bTestSet, bool bTestGet>
TestStorageWithStruct RunArrayStorageTest(size_t MaxCount)
{
	TestStorageWithStruct result = {};	//needs to exist to avoid optimisations from compiler

	if (bTestInit)
	{
		g_ArrayStorage.init();
	}

	//fill test
	if (bTestSet)
	{
		for (size_t i = 0; i < MaxCount; i++)
			g_ArrayStorage.Set(g_IndexSetOrder[i], g_useThisForStorageTest);
	}

	//search test
	if (bTestGet)
	{
		for (size_t i = 0; i < MaxCount; i++)
		{
			const size_t getValForKey = g_IndexGetOrder[i];
			const TestStorageWithStruct *val = g_ArrayStorage.Get(&getValForKey);
			if (val != NULL)
			{
				result.AppendState(*val);
			}
		}
	}

	//anti optimisation dummy return
	return result;
}

template<bool bTestInit, bool bTestSet, bool bTestGet>
LONGLONG BenchmarkArrayStorage(size_t MaxCount, bool bPrintRes)
{
	TestStorageWithStruct junk = {};
	LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
	LARGE_INTEGER Frequency;
	QueryPerformanceFrequency(&Frequency);
	QueryPerformanceCounter(&StartingTime);

	junk = RunArrayStorageTest<bTestInit, bTestSet, bTestGet>(MaxCount);

	QueryPerformanceCounter(&EndingTime);
	ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
	ElapsedMicroseconds.QuadPart *= 1000000;
	ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;

	if (bPrintRes)
	{
		printf("Time spent in % 33s : %f. Junk %d\n", __FUNCTION__, (float)ElapsedMicroseconds.QuadPart, junk.mystate);
		g_RunnedFunctionNames[g_RunningTestIndex] = __FUNCTION__;
	}

	return ElapsedMicroseconds.QuadPart;
}

template<bool bTestInit, bool bTestSet, bool bTestGet>
void RunInitSetGetTests()
{
	if (bTestInit && bTestSet)
		printf("Running speed tests for Init + Set\n");
	else
	{
		if (bTestInit)
			printf("Running speed tests for Init\n");
		if (bTestSet)
			printf("Running speed tests for Set\n");
	}
	if (bTestGet)
		printf("Running speed tests for Get\n");


	LONGLONG sumResultTimes[15] = {};
	for (size_t i = 0; i < REPEAT_TESTS_COUNT; i++)
	{
		g_RunningTestIndex = 0;
		sumResultTimes[g_RunningTestIndex++] += BenchmarkHash<bTestInit, bTestSet, bTestGet>(maxKeyValue, i == (REPEAT_TESTS_COUNT - 1));
		sumResultTimes[g_RunningTestIndex++] += BenchmarkArrayStorage<bTestInit, bTestSet, bTestGet>(maxKeyValue, i == (REPEAT_TESTS_COUNT - 1)); // this is simply as reference to best case
		sumResultTimes[g_RunningTestIndex++] += BenchmarkTreeLookuptable32<bTestInit, bTestSet, bTestGet>(maxKeyValue, i == (REPEAT_TESTS_COUNT - 1)); // this is simply as reference to best case
		sumResultTimes[g_RunningTestIndex++] += BenchmarkUnorderedHash<bTestInit, bTestSet, bTestGet>(maxKeyValue, i == (REPEAT_TESTS_COUNT - 1));
		sumResultTimes[g_RunningTestIndex++] += BenchmarkLookuptable1Indirection<bTestInit, bTestSet, bTestGet>(maxKeyValue, i == (REPEAT_TESTS_COUNT - 1)); // this is simply as reference to best case
		sumResultTimes[g_RunningTestIndex++] += BenchmarkLookuptable<bTestInit, bTestSet, bTestGet>(maxKeyValue, i == (REPEAT_TESTS_COUNT - 1)); // this is simply as reference to best case
		assert(g_RunningTestIndex <= _countof(sumResultTimes));
	}

	for (size_t i = 0; i < _countof(sumResultTimes); i++)
	{
		if (sumResultTimes[i] == 0)
			continue;
		LONGLONG avgExecutionTime = sumResultTimes[i] / REPEAT_TESTS_COUNT;
		printf("Test % 33s single execution time %lld ns, total %lld\n", g_RunnedFunctionNames[i], avgExecutionTime, sumResultTimes[i]);
	}
}

void main()
{
	g_IndexSetOrder = (size_t*)malloc(maxKeyValue * sizeof(size_t));
	g_IndexGetOrder = (size_t*)malloc(maxKeyValue * sizeof(size_t));
	if (g_IndexSetOrder == NULL || g_IndexGetOrder == NULL)
	{
		return;
	}
	for (size_t i = 0; i < maxKeyValue; i++)
	{
		g_IndexSetOrder[i] = ( i * 3 * 7 ) % maxKeyValue;
		g_IndexGetOrder[i] = (i * 7 * 11) % maxKeyValue;
	}
	// for the sake of bounds checking
	g_IndexSetOrder[0] = maxKeyValue - 1;
	g_IndexGetOrder[0] = maxKeyValue - 1;
	g_useThisForStorageTest.mystate = 1;

	size_t memSnapshotBefore, memsnashotafter;
	// warmup
	memSnapshotBefore = GetHeapMemoryUsage();
	RunHashTest<true, true, true>(maxKeyValue);
	memsnashotafter = GetHeapMemoryUsage();
	printf("Bytes allocated while running RunHashTest : %lld\n", memsnashotafter - memSnapshotBefore);

	memSnapshotBefore = GetHeapMemoryUsage();
	RunUnorderedMapTest<true, true, true>(maxKeyValue);
	memsnashotafter = GetHeapMemoryUsage();
	printf("Bytes allocated while running RunUnorderedMapTest : %lld\n", memsnashotafter - memSnapshotBefore);

	memSnapshotBefore = GetHeapMemoryUsage();
	RunLookupTableTest<true, true, true>(maxKeyValue); // this is simply as reference to best case
	memsnashotafter = GetHeapMemoryUsage();
	printf("Bytes allocated while running RunLookupTableTest : %lld\n", memsnashotafter - memSnapshotBefore);

	memSnapshotBefore = GetHeapMemoryUsage();
	RunLookupTableTest1Indirection<true, true, true>(maxKeyValue); // this is simply as reference to best case
	memsnashotafter = GetHeapMemoryUsage();
	printf("Bytes allocated while running RunLookupTableTest1Indirection : %lld\n", memsnashotafter - memSnapshotBefore);

	memSnapshotBefore = GetHeapMemoryUsage();
	RunTreeLookupTable32Test<true, true, true>(maxKeyValue); // this is simply as reference to best case
	memsnashotafter = GetHeapMemoryUsage();
	printf("Bytes allocated while running RunTreeLookupTable32Test : %lld\n", memsnashotafter - memSnapshotBefore);

	memSnapshotBefore = GetHeapMemoryUsage();
	RunArrayStorageTest<true, true, true>(maxKeyValue); // this is simply as reference to best case
	memsnashotafter = GetHeapMemoryUsage();
	printf("Bytes allocated while running RunArrayStorageTest : %lld\n", memsnashotafter - memSnapshotBefore);

	// run the speed tests
	RunInitSetGetTests<true, true, false>();
	RunInitSetGetTests<false, false, true>();

	printf("Press any key to exit");
	int getchres = _getch();

	free(g_IndexSetOrder);
	free(g_IndexGetOrder);
}