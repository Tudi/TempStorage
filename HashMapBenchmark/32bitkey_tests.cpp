#include "StdAfx.h"

// very basic test to see order of magnitude differences between containers that use different number of indirections to get value based on key

namespace Testing32bitKeys {
	const char* g_RunnedFunctionNames_32[15] = {};
	size_t g_RunningTestIndex_32 = 0;

	// Custom hash function for uint32_t
	struct CustomHash {
		std::size_t operator()(size_t key) const {
			return key;
		}
	};

#ifndef _DEBUG
	#define maxKeyValue		0x7FFFFFFF
	#define maxValueCount	200000
	const size_t REPEAT_TESTS_COUNT = 60; // if a test takes less than a second, that is unmeasurable
#else
	const size_t maxKeyValue = 0x7FFFFFFF;
	#define maxValueCount	200000
	const size_t REPEAT_TESTS_COUNT = 1; // if a test takes less than a second, that is unmeasurable
#endif

	// anti optimization where direct lookup map has an advantage of stream erading memory
	size_t* g_IndexSetOrder_32 = NULL;
	size_t* g_IndexGetOrder_32 = NULL;

	SQLResultCache<32, maxKeyValue, 0xFFFF, TestStorageWithStruct>* g_Tree32_32;
	std::map<size_t, TestStorageWithStruct>* g_StdMap_32;
	std::unordered_map<size_t, TestStorageWithStruct>* g_StdUnorderedMap_32;
	std::unordered_map<size_t, TestStorageWithStruct, CustomHash>* g_StdUnorderedMapNoHash_32;
	ArrayStorage<TestStorageWithStruct, 0xFFFF, offsetof(TestStorageWithStruct, myipv4rowkey), sizeof(TestStorageWithStruct::myipv4rowkey)>* g_ArrayStorage_32;

	TestStorageWithStruct* g_useThisForStorageTest_32;

	template<bool bTestInit, bool bTestSet, bool bTestGet>
	__declspec(noinline) TestStorageWithStruct RunHashTest()
	{
		TestStorageWithStruct result = {};	//needs to exist to avoid optimisations from compiler

		if (bTestInit)
		{
			g_StdMap_32->clear();
		}

		if (bTestSet)
		{
			//fill test
			for (size_t i = 0; i < maxValueCount; i++)
			{
				g_useThisForStorageTest_32->mystate = i;
				(*g_StdMap_32)[g_IndexSetOrder_32[i]] = *g_useThisForStorageTest_32;
			}
		}

		if (bTestGet)
		{
			//search test
			for (size_t i = 0; i < maxValueCount; i++)
			{
				const size_t getValForKey = g_IndexGetOrder_32[i];
				auto itr = g_StdMap_32->find(getValForKey);
				if (itr != g_StdMap_32->end())
				{
					result.AppendState(itr->second);
				}
			}
		}

		//anti optimisation dummy return
		return result;

	}

	template<bool bTestInit, bool bTestSet, bool bTestGet>
	__declspec(noinline) TestStorageWithStruct RunUnorderedMapTest()
	{
		TestStorageWithStruct result = {};	//needs to exist to avoid optimisations from compiler

		if (bTestInit)
		{
			g_StdUnorderedMap_32->clear();
		}

		//fill test
		if (bTestSet)
		{
			for (size_t i = 0; i < maxValueCount; i++)
			{
				g_useThisForStorageTest_32->mystate = i;
				(*g_StdUnorderedMap_32)[g_IndexSetOrder_32[i]] = *g_useThisForStorageTest_32;
			}
		}

		//search test
		if (bTestGet)
		{
			for (size_t i = 0; i < maxValueCount; i++)
			{
				const size_t getValForKey = g_IndexGetOrder_32[i];
				auto itr = g_StdUnorderedMap_32->find(getValForKey);
				if (itr != g_StdUnorderedMap_32->end())
				{
					result.AppendState(itr->second);
				}
			}
		}

		//anti optimisation dummy return
		return result;

	}

	template<bool bTestInit, bool bTestSet, bool bTestGet>
	__declspec(noinline) TestStorageWithStruct RunUnorderedMapNoHashTest()
	{
		TestStorageWithStruct result = {};	//needs to exist to avoid optimisations from compiler

		if (bTestInit)
		{
			g_StdUnorderedMapNoHash_32->clear();
		}

		//fill test
		if (bTestSet)
		{
			for (size_t i = 0; i < maxValueCount; i++)
			{
				g_useThisForStorageTest_32->mystate = i;
				(*g_StdUnorderedMapNoHash_32)[g_IndexSetOrder_32[i]] = *g_useThisForStorageTest_32;
			}
		}

		//search test
		if (bTestGet)
		{
			for (size_t i = 0; i < maxValueCount; i++)
			{
				const size_t getValForKey = g_IndexGetOrder_32[i];
				auto itr = g_StdUnorderedMapNoHash_32->find(getValForKey);
				if (itr != g_StdUnorderedMapNoHash_32->end())
				{
					result.AppendState(itr->second);
				}
			}
		}

		//anti optimisation dummy return
		return result;

	}

	template<bool bTestInit, bool bTestSet, bool bTestGet>
	__declspec(noinline) TestStorageWithStruct RunTreeLookupTable32Test()
	{
		TestStorageWithStruct result = {};	//needs to exist to avoid optimisations from compiler

		if (bTestInit)
		{
			g_Tree32_32->init();
		}

		//fill test
		if (bTestSet)
		{
			for (size_t i = 0; i < maxValueCount; i++)
			{
				g_useThisForStorageTest_32->mystate = i;
				g_Tree32_32->Set(g_IndexSetOrder_32[i], *g_useThisForStorageTest_32);
			}
		}

		//search test
		if (bTestGet)
		{
			for (size_t i = 0; i < maxValueCount; i++)
			{
				const size_t getValForKey = g_IndexGetOrder_32[i];
				const TestStorageWithStruct* val = g_Tree32_32->Get(getValForKey);
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
	__declspec(noinline) TestStorageWithStruct RunArrayStorageTest()
	{
		TestStorageWithStruct result = {};	//needs to exist to avoid optimisations from compiler

		if (bTestInit)
		{
			g_ArrayStorage_32->init();
		}

		//fill test
		if (bTestSet)
		{
			for (size_t i = 0; i < maxValueCount; i++)
			{
				g_useThisForStorageTest_32->mystate = i;
				g_ArrayStorage_32->Set(i, *g_useThisForStorageTest_32);
			}
		}

		//search test
		if (bTestGet)
		{
			for (size_t i = 0; i < maxValueCount; i++)
			{
				const size_t getValForKey = g_IndexGetOrder_32[i];
				const TestStorageWithStruct* val = g_ArrayStorage_32->Get(&getValForKey);
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
	LONGLONG BenchmarkGenericTest(bool bPrintRes)
	{
		TestStorageWithStruct junk = {};
		LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
		LARGE_INTEGER Frequency;
		QueryPerformanceFrequency(&Frequency);
		QueryPerformanceCounter(&StartingTime);

		const char* TestFuncName = "";
		if (g_RunningTestIndex_32 == 0)
		{
			TestFuncName = "std::HashMap";
			junk = RunHashTest<bTestInit, bTestSet, bTestGet>();
		}
		else if (g_RunningTestIndex_32 == 1)
		{
			TestFuncName = "BinSearchArray";
			junk = RunArrayStorageTest<bTestInit, bTestSet, bTestGet>();
		}
		else if (g_RunningTestIndex_32 == 2)
		{
			TestFuncName = "Tree";
			junk = RunTreeLookupTable32Test<bTestInit, bTestSet, bTestGet>();
		}
		else if (g_RunningTestIndex_32 == 3)
		{
			TestFuncName = "std::unordered_map";
			junk = RunUnorderedMapTest<bTestInit, bTestSet, bTestGet>();
		}
		else if (g_RunningTestIndex_32 == 4)
		{
			TestFuncName = "std::unordered_map_noHash";
			junk = RunUnorderedMapNoHashTest<bTestInit, bTestSet, bTestGet>();
		}

		QueryPerformanceCounter(&EndingTime);
		ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
		ElapsedMicroseconds.QuadPart *= 1000000;
		ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;

		if (bPrintRes)
		{
			printf("Time spent in % 33s : %f. Junk %llu\n", TestFuncName, (float)ElapsedMicroseconds.QuadPart, junk.mystate);
			g_RunnedFunctionNames_32[g_RunningTestIndex_32] = TestFuncName;
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
			for (g_RunningTestIndex_32 = 0; g_RunningTestIndex_32 < 5; g_RunningTestIndex_32++)
			{
				sumResultTimes[g_RunningTestIndex_32] += BenchmarkGenericTest<bTestInit, bTestSet, bTestGet>(i == (REPEAT_TESTS_COUNT - 1));
			}
		}

		for (size_t i = 0; i < _countof(sumResultTimes); i++)
		{
			if (sumResultTimes[i] == 0)
				continue;
			LONGLONG avgExecutionTime = sumResultTimes[i] / REPEAT_TESTS_COUNT;
			printf("Test % 33s single execution time %lld ns, total %lld\n", g_RunnedFunctionNames_32[i], avgExecutionTime, sumResultTimes[i]);
		}
	}
};

using namespace Testing32bitKeys;

int Run32BPKTests()
{
	g_useThisForStorageTest_32 = new TestStorageWithStruct();

	g_IndexSetOrder_32 = (size_t*)malloc(maxValueCount * sizeof(size_t));
	g_IndexGetOrder_32 = (size_t*)malloc(maxValueCount * sizeof(size_t));
	if (g_IndexSetOrder_32 == NULL || g_IndexGetOrder_32 == NULL)
	{
		return -1;
	}
	for (size_t i = 0; i < maxValueCount; i++)
	{
		g_IndexSetOrder_32[i] = (i * 3 * 7 * 11 * 13 * 17) % maxKeyValue;
		g_IndexGetOrder_32[i] = (i * 7 * 11 * 13 * 17) % maxKeyValue;
	}
	// for the sake of bounds checking
	g_IndexSetOrder_32[0] = maxKeyValue - 1;
	g_IndexGetOrder_32[0] = maxKeyValue - 1;
	g_useThisForStorageTest_32->mystate = 1;

	size_t memSnapshotBefore, memsnashotafter;
	// warmup
	memSnapshotBefore = GetHeapMemoryUsage();
	g_StdMap_32 = new std::map<size_t, TestStorageWithStruct>();
	RunHashTest<true, true, true>();
	memsnashotafter = GetHeapMemoryUsage();
	printf("KBytes allocated while running RunHashTest : %lld\n", (memsnashotafter - memSnapshotBefore) / 1024);

	memSnapshotBefore = GetHeapMemoryUsage();
	g_StdUnorderedMap_32 = new std::unordered_map<size_t, TestStorageWithStruct>();
	RunUnorderedMapTest<true, true, true>();
	memsnashotafter = GetHeapMemoryUsage();
	printf("KBytes allocated while running RunUnorderedMapTest : %lld\n", (memsnashotafter - memSnapshotBefore) / 1024);

	memSnapshotBefore = GetHeapMemoryUsage();
	g_StdUnorderedMapNoHash_32 = new std::unordered_map<size_t, TestStorageWithStruct, CustomHash>();
	RunUnorderedMapNoHashTest<true, true, true>();
	memsnashotafter = GetHeapMemoryUsage();
	printf("KBytes allocated while running RunUnorderedMapNoHashTest : %lld\n", (memsnashotafter - memSnapshotBefore) / 1024);

	memSnapshotBefore = GetHeapMemoryUsage();
	g_Tree32_32 = new SQLResultCache<32, maxKeyValue, 0xFFFF, TestStorageWithStruct>();
	RunTreeLookupTable32Test<true, true, true>();
	//	RunTreeLookupTable32Test<false, false, true>(maxKeyValue);
	memsnashotafter = GetHeapMemoryUsage();
	printf("KBytes allocated while running RunTreeLookupTable32Test : %lld\n", (memsnashotafter - memSnapshotBefore) / 1024);

	memSnapshotBefore = GetHeapMemoryUsage();
	g_ArrayStorage_32 = new ArrayStorage<TestStorageWithStruct, 0xFFFF, offsetof(TestStorageWithStruct, myipv4rowkey), sizeof(TestStorageWithStruct::myipv4rowkey)>();
	RunArrayStorageTest<true, true, true>();
	memsnashotafter = GetHeapMemoryUsage();
	printf("KBytes allocated while running RunArrayStorageTest : %lld\n", (memsnashotafter - memSnapshotBefore) / 1024);

	// run the speed tests
	RunInitSetGetTests<true, true, false>();
	RunInitSetGetTests<false, false, true>();

	free(g_IndexSetOrder_32);
	free(g_IndexGetOrder_32);
	delete g_useThisForStorageTest_32;
	delete g_StdMap_32;
	delete g_ArrayStorage_32;
	delete g_Tree32_32;
	delete g_StdUnorderedMap_32;

	return 0;
}