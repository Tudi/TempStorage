#include "StdAfx.h"

// very basic test to see order of magnitude differences between containers that use different number of indirections to get value based on key

// anti optimization where direct lookup map has an advantage of stream erading memory
size_t* g_IndexSetOrder = NULL;
size_t* g_IndexSetOrdered = NULL;
size_t* g_IndexGetOrder = NULL;

const char* g_RunnedFunctionNames[15] = {};
size_t g_RunningTestIndex = 0;

// Custom hash function for uint32_t
struct CustomHash {
	std::size_t operator()(size_t key) const {
		return key;
	}
};

#ifndef _DEBUG
#define maxKeyValue 2000000
const size_t REPEAT_TESTS_COUNT = 60; // if a test takes less than a second, that is unmeasurable
#else
const size_t maxKeyValue = 500000;
const size_t REPEAT_TESTS_COUNT = 1; // if a test takes less than a second, that is unmeasurable
#endif

DirectLookupMap<TestStorageWithStruct>* g_DirectLookupMap;
Indirect1LayerLookupMap<TestStorageWithStruct>* g_Indirect1LayerLookupMap;
SQLResultCache<128, maxKeyValue, 0xFFFF, TestStorageWithStruct>* g_Tree32;
std::map<size_t, TestStorageWithStruct>* g_StdMap;
std::unordered_map<size_t, TestStorageWithStruct, CustomHash>* g_StdUnorderedMap;
ArrayStorage<TestStorageWithStruct, 0xFFFF, offsetof(TestStorageWithStruct, myipv4rowkey), sizeof(TestStorageWithStruct::myipv4rowkey)>* g_ArrayStorage;

TestStorageWithStruct* g_useThisForStorageTest;

template<bool bTestInit, bool bTestSet, bool bTestGet>
__declspec(noinline) TestStorageWithStruct RunHashTest(size_t MaxCount)
{
	TestStorageWithStruct result = {};	//needs to exist to avoid optimisations from compiler

	if (bTestInit)
	{
		g_StdMap->clear();
	}

	if (bTestSet)
	{
		//fill test
		for (size_t i = 0; i < MaxCount; i++)
		{
			g_useThisForStorageTest->mystate = i;
			(*g_StdMap)[g_IndexSetOrder[i]] = *g_useThisForStorageTest;
		}
	}

	if (bTestGet)
	{
		//search test
		for (size_t i = 0; i < MaxCount; i++)
		{
			const size_t getValForKey = g_IndexGetOrder[i];
			auto itr = g_StdMap->find(getValForKey);
			if (itr != g_StdMap->end())
			{
				result.AppendState(itr->second);
			}
		}
	}

	//anti optimisation dummy return
	return result;

}

template<bool bTestInit, bool bTestSet, bool bTestGet>
__declspec(noinline) TestStorageWithStruct RunUnorderedMapTest(size_t MaxCount)
{
	TestStorageWithStruct result = {};	//needs to exist to avoid optimisations from compiler

	if (bTestInit)
	{
		g_StdUnorderedMap->clear();
	}

	//fill test
	if (bTestSet)
	{
		for (size_t i = 0; i < MaxCount; i++)
		{
			g_useThisForStorageTest->mystate = i;
			(*g_StdUnorderedMap)[g_IndexSetOrder[i]] = *g_useThisForStorageTest;
		}
	}

	//search test
	if (bTestGet)
	{
		for (size_t i = 0; i < MaxCount; i++)
		{
			const size_t getValForKey = g_IndexGetOrder[i];
			auto itr = g_StdUnorderedMap->find(getValForKey);
			if (itr != g_StdUnorderedMap->end())
			{
				result.AppendState(itr->second);
			}
		}
	}

	//anti optimisation dummy return
	return result;

}

template<bool bTestInit, bool bTestSet, bool bTestGet>
__declspec(noinline) TestStorageWithStruct RunLookupTableTest(size_t MaxCount)
{
	TestStorageWithStruct result = {};	//needs to exist to avoid optimisations from compiler

	if (bTestInit)
	{
		g_DirectLookupMap->Init(MaxCount);
	}

	if (bTestSet)
	{
		//fill test
		for (size_t i = 0; i < MaxCount; i++)
		{
			g_useThisForStorageTest->mystate = i;
			g_DirectLookupMap->Set(g_IndexSetOrder[i], *g_useThisForStorageTest);
		}
	}

	if (bTestGet)
	{
		//search test
		for (size_t i = 0; i < MaxCount; i++)
		{
			const size_t getValForKey = g_IndexGetOrder[i];
			const TestStorageWithStruct* val = g_DirectLookupMap->Get(getValForKey);
			if (val)
			{
				result.AppendState(*val);
			}
		}
	}

	//anti optimisation dummy return
	return result;
}

template<bool bTestInit, bool bTestSet, bool bTestGet>
__declspec(noinline) TestStorageWithStruct RunLookupTableTest1Indirection(size_t MaxCount)
{
	TestStorageWithStruct result = {};	//needs to exist to avoid optimisations from compiler

	if (bTestInit)
	{
		g_Indirect1LayerLookupMap->Init(MaxCount);
	}

	//fill test
	if (bTestSet)
	{
		for (size_t i = 0; i < MaxCount; i++)
		{
			g_useThisForStorageTest->mystate = i;
			g_Indirect1LayerLookupMap->Set(g_IndexSetOrder[i], *g_useThisForStorageTest);
		}
	}

	//search test
	if (bTestGet)
	{
		for (size_t i = 0; i < MaxCount; i++)
		{
			const size_t getValForKey = g_IndexGetOrder[i];
			const TestStorageWithStruct* val = g_Indirect1LayerLookupMap->Get(getValForKey);
			if (val)
			{
				result.AppendState(*val);
			}
		}
	}

	//anti optimisation dummy return
	return result;
}


template<bool bTestInit, bool bTestSet, bool bTestGet>
__declspec(noinline) TestStorageWithStruct RunTreeLookupTable32Test(size_t MaxCount)
{
	TestStorageWithStruct result = {};	//needs to exist to avoid optimisations from compiler

	if (bTestInit)
	{
		g_Tree32->init();
	}

	//fill test
	if (bTestSet)
	{
		for (size_t i = 0; i < MaxCount; i++)
		{
			g_useThisForStorageTest->mystate = i;
			g_Tree32->Set(g_IndexSetOrder[i], *g_useThisForStorageTest);
		}
	}

	//search test
	if (bTestGet)
	{
		for (size_t i = 0; i < MaxCount; i++)
		{
			const size_t getValForKey = g_IndexGetOrder[i];
			const TestStorageWithStruct* val = g_Tree32->Get(getValForKey);
			if (val)
			{
				result.AppendState(*val);
			}
		}
	}

	//anti optimisation dummy return
	return result;
}

template<bool bTestInit, bool bTestSet, bool bTestGet>
__declspec(noinline) TestStorageWithStruct RunArrayStorageTest(size_t MaxCount)
{
	TestStorageWithStruct result = {};	//needs to exist to avoid optimisations from compiler

	if (bTestInit)
	{
		g_ArrayStorage->init();
	}

	//fill test
	if (bTestSet)
	{
		for (size_t i = 0; i < MaxCount; i++)
		{
			g_useThisForStorageTest->mystate = g_IndexSetOrdered[i];
			g_ArrayStorage->Set(g_IndexSetOrdered[i], *g_useThisForStorageTest);
		}
	}

	//search test
	if (bTestGet)
	{
		for (size_t i = 0; i < MaxCount; i++)
		{
			const size_t getValForKey = g_IndexGetOrder[i];
			const TestStorageWithStruct* val = g_ArrayStorage->Get(&getValForKey);
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
LONGLONG BenchmarkGenericTest(size_t MaxCount, bool bPrintRes)
{
	TestStorageWithStruct junk = {};
	LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
	LARGE_INTEGER Frequency;
	QueryPerformanceFrequency(&Frequency);
	QueryPerformanceCounter(&StartingTime);

	const char* TestFuncName = "";
	if (g_RunningTestIndex == 0)
	{
		TestFuncName = "std::HashMap";
		junk = RunHashTest<bTestInit, bTestSet, bTestGet>(MaxCount);
	}
	else if (g_RunningTestIndex == 1)
	{
		TestFuncName = "BinSearchArray";
		junk = RunArrayStorageTest<bTestInit, bTestSet, bTestGet>(MaxCount);
	}
	else if (g_RunningTestIndex == 2)
	{
		TestFuncName = "Tree";
		junk = RunTreeLookupTable32Test<bTestInit, bTestSet, bTestGet>(MaxCount);
	}
	else if (g_RunningTestIndex == 3)
	{
		TestFuncName = "std::unordered_map";
		junk = RunUnorderedMapTest<bTestInit, bTestSet, bTestGet>(MaxCount);
	}
	else if (g_RunningTestIndex == 4)
	{
		TestFuncName = "1LayerLookupMap";
		junk = RunLookupTableTest1Indirection<bTestInit, bTestSet, bTestGet>(MaxCount);
	}
	else if (g_RunningTestIndex == 5)
	{
		TestFuncName = "DirectLookupMap";
		junk = RunLookupTableTest<bTestInit, bTestSet, bTestGet>(MaxCount);
	}

	QueryPerformanceCounter(&EndingTime);
	ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
	ElapsedMicroseconds.QuadPart *= 1000000;
	ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;

	if (bPrintRes)
	{
		printf("Time spent in % 33s : %f. Junk %llu\n", TestFuncName, (float)ElapsedMicroseconds.QuadPart, junk.mystate);
		g_RunnedFunctionNames[g_RunningTestIndex] = TestFuncName;
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
		for (g_RunningTestIndex = 0; g_RunningTestIndex < 6; g_RunningTestIndex++)
		{
			sumResultTimes[g_RunningTestIndex] += BenchmarkGenericTest<bTestInit, bTestSet, bTestGet>(maxKeyValue, i == (REPEAT_TESTS_COUNT - 1));
		}
	}

	for (size_t i = 0; i < _countof(sumResultTimes); i++)
	{
		if (sumResultTimes[i] == 0)
			continue;
		LONGLONG avgExecutionTime = sumResultTimes[i] / REPEAT_TESTS_COUNT;
		printf("Test % 33s single execution time %lld ns, total %lld\n", g_RunnedFunctionNames[i], avgExecutionTime, sumResultTimes[i]);
	}
}

int Run24BPKTests()
{
	g_useThisForStorageTest = new TestStorageWithStruct();

	g_IndexSetOrder = (size_t*)malloc(maxKeyValue * sizeof(size_t));
	g_IndexGetOrder = (size_t*)malloc(maxKeyValue * sizeof(size_t));
	g_IndexSetOrdered = (size_t*)malloc(maxKeyValue * sizeof(size_t));
	if (g_IndexSetOrder == NULL || g_IndexGetOrder == NULL)
	{
		return -1;
	}
	for (size_t i = 0; i < maxKeyValue; i++)
	{
		g_IndexSetOrder[i] = (i * 3 * 7) % maxKeyValue;
		g_IndexGetOrder[i] = (i * 7 * 11) % maxKeyValue;
		g_IndexSetOrdered[g_IndexSetOrder[i]] = i; // reverse order
	}
	// for the sake of bounds checking
	g_IndexSetOrder[0] = maxKeyValue - 1;
	g_IndexGetOrder[0] = maxKeyValue - 1;
	g_useThisForStorageTest->mystate = 1;

	size_t memSnapshotBefore, memsnashotafter;
	// warmup
	memSnapshotBefore = GetHeapMemoryUsage();
	g_StdMap = new std::map<size_t, TestStorageWithStruct>();
	RunHashTest<true, true, true>(maxKeyValue);
	memsnashotafter = GetHeapMemoryUsage();
	printf("KBytes allocated while running RunHashTest : %lld\n", (memsnashotafter - memSnapshotBefore) / 1024);

	memSnapshotBefore = GetHeapMemoryUsage();
	g_StdUnorderedMap = new std::unordered_map<size_t, TestStorageWithStruct, CustomHash>();
	RunUnorderedMapTest<true, true, true>(maxKeyValue);
	memsnashotafter = GetHeapMemoryUsage();
	printf("KBytes allocated while running RunUnorderedMapTest : %lld\n", (memsnashotafter - memSnapshotBefore) / 1024);

	memSnapshotBefore = GetHeapMemoryUsage();
	g_DirectLookupMap = new DirectLookupMap<TestStorageWithStruct>();
	RunLookupTableTest<true, true, true>(maxKeyValue); // this is simply as reference to best case
	memsnashotafter = GetHeapMemoryUsage();
	printf("KBytes allocated while running RunLookupTableTest : %lld\n", (memsnashotafter - memSnapshotBefore) / 1024);

	memSnapshotBefore = GetHeapMemoryUsage();
	g_Indirect1LayerLookupMap = new Indirect1LayerLookupMap<TestStorageWithStruct>();
	RunLookupTableTest1Indirection<true, true, true>(maxKeyValue); // this is simply as reference to best case
	memsnashotafter = GetHeapMemoryUsage();
	printf("KBytes allocated while running RunLookupTableTest1Indirection : %lld\n", (memsnashotafter - memSnapshotBefore) / 1024);

	memSnapshotBefore = GetHeapMemoryUsage();
	g_Tree32 = new SQLResultCache<128, maxKeyValue, 0xFFFF, TestStorageWithStruct>();
	RunTreeLookupTable32Test<true, true, true>(maxKeyValue);
	//	RunTreeLookupTable32Test<false, false, true>(maxKeyValue);
	memsnashotafter = GetHeapMemoryUsage();
	printf("KBytes allocated while running RunTreeLookupTable32Test : %lld\n", (memsnashotafter - memSnapshotBefore) / 1024);

	memSnapshotBefore = GetHeapMemoryUsage();
	g_ArrayStorage = new ArrayStorage<TestStorageWithStruct, 0xFFFF, offsetof(TestStorageWithStruct, myipv4rowkey), sizeof(TestStorageWithStruct::myipv4rowkey)>();
	RunArrayStorageTest<true, true, true>(maxKeyValue);
	memsnashotafter = GetHeapMemoryUsage();
	printf("KBytes allocated while running RunArrayStorageTest : %lld\n", (memsnashotafter - memSnapshotBefore) / 1024);

	// run the speed tests
	RunInitSetGetTests<true, true, false>();
	RunInitSetGetTests<false, false, true>();

	free(g_IndexSetOrder);
	free(g_IndexGetOrder);
	free(g_IndexSetOrdered);
	delete g_useThisForStorageTest;
	delete g_StdMap;
	delete g_ArrayStorage;
	delete g_Tree32;
	delete g_Indirect1LayerLookupMap;
	delete g_DirectLookupMap;
	delete g_StdUnorderedMap;

	return 0;
}