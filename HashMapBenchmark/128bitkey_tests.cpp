#include "StdAfx.h"

// very basic test to see order of magnitude differences between containers that use different number of indirections to get value based on key

namespace Testing128bitKeys {
	const char* g_RunnedFunctionNames_128[15] = {};
	size_t g_RunningTestIndex_128 = 0;

	// Custom hash function for uint32_t
	struct CustomHash1 {
		std::size_t operator()(const ipv6key &key) const {
			return std::hash<size_t>()(key.bytes8[0]) ^ (std::hash<size_t>()(key.bytes8[1]) << 1);
		}
	};
	struct CustomHash2 {
		std::size_t operator()(const ipv6key& key) const {
			return key.bytes8[0] ^ (key.bytes8[1] << 1);
		}
	};
	struct CustomHash3 {
		std::size_t operator()(const ipv6key& key) const {
			return key.bytes8[0];
		}
	};
	struct ipv6EQTo {
		bool operator()(const ipv6key& lhs, const ipv6key& rhs) const {
			return ((lhs.bytes8[0] == rhs.bytes8[0]) && (lhs.bytes8[1] == rhs.bytes8[1]));
		}
	};

#ifndef _DEBUG
	#define maxKeyValue		0x7FFFFFFFFFFFFFFF
	#define maxValueCount	2000000
	const size_t REPEAT_TESTS_COUNT = 20; // if a test takes less than a second, that is unmeasurable
#else
	const size_t maxKeyValue = 0x7FFFFFFFFFFFFFFF;
	#define maxValueCount	20000
	const size_t REPEAT_TESTS_COUNT = 1; // if a test takes less than a second, that is unmeasurable
#endif

	// anti optimization where direct lookup map has an advantage of stream erading memory
	ipv6key* g_IndexSetOrder_128 = NULL;
	ipv6key* g_IndexGetOrder_128 = NULL;

	std::map<ipv6key, TestStorageWithStruct>* g_StdMap_128;
	std::unordered_map<ipv6key, TestStorageWithStruct, CustomHash1, ipv6EQTo>* g_StdUnorderedMap_128;
	std::unordered_map<ipv6key, TestStorageWithStruct, CustomHash2, ipv6EQTo>* g_StdUnorderedMapNoHash_128;
	std::unordered_multimap<ipv6key, TestStorageWithStruct, CustomHash3, ipv6EQTo>* g_StdUnorderedMultiMapNoHash_128;
	ArrayStorage<ipv6key, TestStorageWithStruct, 0xFFFF>* g_ArrayStorage_128;
	ArrayStorage_v2<ipv6key, TestStorageWithStruct, 0xFFFF>* g_ArrayStorage_v2_128;

	TestStorageWithStruct* g_useThisForStorageTest_128;

	template<bool bTestInit, bool bTestSet, bool bTestGet>
	__declspec(noinline) TestStorageWithStruct RunHashTest()
	{
		TestStorageWithStruct result = {};	//needs to exist to avoid optimisations from compiler

		if (bTestInit)
		{
			g_StdMap_128->clear();
		}

		if (bTestSet)
		{
			//fill test
			for (size_t i = 0; i < maxValueCount; i++)
			{
				g_useThisForStorageTest_128->mystate = i;
				(*g_StdMap_128)[g_IndexSetOrder_128[i]] = *g_useThisForStorageTest_128;
			}
		}

		if (bTestGet)
		{
			//search test
			for (size_t i = 0; i < maxValueCount; i++)
			{
				auto itr = g_StdMap_128->find(g_IndexGetOrder_128[i]);
				if (itr != g_StdMap_128->end())
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
			g_StdUnorderedMap_128->clear();
		}

		//fill test
		if (bTestSet)
		{
			for (size_t i = 0; i < maxValueCount; i++)
			{
				g_useThisForStorageTest_128->mystate = i;
				(*g_StdUnorderedMap_128)[g_IndexSetOrder_128[i]] = *g_useThisForStorageTest_128;
			}
		}

		//search test
		if (bTestGet)
		{
			for (size_t i = 0; i < maxValueCount; i++)
			{
				auto itr = g_StdUnorderedMap_128->find(g_IndexGetOrder_128[i]);
				if (itr != g_StdUnorderedMap_128->end())
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
			g_StdUnorderedMapNoHash_128->clear();
		}

		//fill test
		if (bTestSet)
		{
			for (size_t i = 0; i < maxValueCount; i++)
			{
				g_useThisForStorageTest_128->mystate = i;
				(*g_StdUnorderedMapNoHash_128)[g_IndexSetOrder_128[i]] = *g_useThisForStorageTest_128;
			}
		}

		//search test
		if (bTestGet)
		{
			for (size_t i = 0; i < maxValueCount; i++)
			{
				auto itr = g_StdUnorderedMapNoHash_128->find(g_IndexGetOrder_128[i]);
				if (itr != g_StdUnorderedMapNoHash_128->end())
				{
					result.AppendState(itr->second);
				}
			}
		}

		//anti optimisation dummy return
		return result;

	}

	template<bool bTestInit, bool bTestSet, bool bTestGet>
	__declspec(noinline) TestStorageWithStruct RunUnorderedMultiMapNoHashTest()
	{
		TestStorageWithStruct result = {};	//needs to exist to avoid optimisations from compiler

		if (bTestInit)
		{
			g_StdUnorderedMultiMapNoHash_128->clear();
		}

		//fill test
		if (bTestSet)
		{
			for (size_t i = 0; i < maxValueCount; i++)
			{
				g_useThisForStorageTest_128->mystate = i;
				g_StdUnorderedMultiMapNoHash_128->insert(std::make_pair(g_IndexSetOrder_128[i], *g_useThisForStorageTest_128));
			}
		}

		//search test
		if (bTestGet)
		{
			for (size_t i = 0; i < maxValueCount; i++)
			{
				auto itr = g_StdUnorderedMultiMapNoHash_128->find(g_IndexGetOrder_128[i]);
				if (itr != g_StdUnorderedMultiMapNoHash_128->end())
				{
					result.AppendState(itr->second);
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
			g_ArrayStorage_128->init();
		}

		//fill test
		if (bTestSet)
		{
			for (size_t i = 0; i < maxValueCount; i++)
			{
				g_useThisForStorageTest_128->mystate = i;
				g_ArrayStorage_128->Set(g_IndexSetOrder_128[i], *g_useThisForStorageTest_128);
			}
			ASSERT(g_ArrayStorage_128->size() == maxValueCount);
		}

		//search test
		if (bTestGet)
		{
			for (size_t i = 0; i < maxValueCount; i++)
			{
				const TestStorageWithStruct* val = g_ArrayStorage_128->Get(g_IndexGetOrder_128[i]);
#ifndef _DEBUG
				if (val != NULL)
					result.AppendState(*val);
#else
				auto itr = g_StdUnorderedMapNoHash_128->find(g_IndexGetOrder_128[i]);
				if (val != NULL)
				{
					if (itr == g_StdUnorderedMapNoHash_128->end())
					{
						i = i;
						val = g_ArrayStorage_128->Get(g_IndexGetOrder_128[i]);
					}
					result.AppendState(*val);
				}
				else
				{
					if (itr != g_StdUnorderedMapNoHash_128->end())
					{
						i = i;
						val = g_ArrayStorage_128->Get(g_IndexGetOrder_128[i]);
					}
				}
#endif
			}
		}

		//anti optimisation dummy return
		return result;
	}

	template<bool bTestInit, bool bTestSet, bool bTestGet>
	__declspec(noinline) TestStorageWithStruct RunArrayStorage_v2_Test()
	{
		TestStorageWithStruct result = {};	//needs to exist to avoid optimisations from compiler

		if (bTestInit)
		{
			g_ArrayStorage_v2_128->init();
		}

		//fill test
		if (bTestSet)
		{
			for (size_t i = 0; i < maxValueCount; i++)
			{
				g_useThisForStorageTest_128->mystate = i;
				g_ArrayStorage_v2_128->Set(g_IndexSetOrder_128[i], *g_useThisForStorageTest_128);
			}
//			g_ArrayStorage_v2_128->BuildLookupKTree();
			ASSERT(g_ArrayStorage_v2_128->size() == maxValueCount);
		}

		//search test
		if (bTestGet)
		{
			for (size_t i = 0; i < maxValueCount; i++)
			{
				const TestStorageWithStruct* val = g_ArrayStorage_v2_128->Get(g_IndexGetOrder_128[i]);
#ifndef _DEBUG
				if (val != NULL)
					result.AppendState(*val);
#else
				auto itr = g_StdUnorderedMapNoHash_128->find(g_IndexGetOrder_128[i]);
				if (val != NULL)
				{
					if (itr == g_StdUnorderedMapNoHash_128->end())
					{
						i = i;
						val = g_ArrayStorage_v2_128->Get(g_IndexGetOrder_128[i]);
					}
					result.AppendState(*val);
				}
				else
				{
					if (itr != g_StdUnorderedMapNoHash_128->end())
					{
						i = i;
						val = g_ArrayStorage_v2_128->Get(g_IndexGetOrder_128[i]);
					}
				}
#endif
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
		if (g_RunningTestIndex_128 == 0)
		{
			TestFuncName = "std::HashMap";
			junk = RunHashTest<bTestInit, bTestSet, bTestGet>();
		}
		else if (g_RunningTestIndex_128 == 1)
		{
			TestFuncName = "BinSearchArray";
			junk = RunArrayStorageTest<bTestInit, bTestSet, bTestGet>();
		}
		else if (g_RunningTestIndex_128 == 2)
		{
			TestFuncName = "BinSearchArray_v2";
			junk = RunArrayStorage_v2_Test<bTestInit, bTestSet, bTestGet>();
		}
		else if (g_RunningTestIndex_128 == 3)
		{
			TestFuncName = "std::unordered_map";
			junk = RunUnorderedMapTest<bTestInit, bTestSet, bTestGet>();
		}
		else if (g_RunningTestIndex_128 == 4)
		{
			TestFuncName = "std::unordered_map_noHash";
			junk = RunUnorderedMapNoHashTest<bTestInit, bTestSet, bTestGet>();
		}
		else if (g_RunningTestIndex_128 == 5)
		{
			TestFuncName = "std::unordered_multimap_noHash";
			junk = RunUnorderedMultiMapNoHashTest<bTestInit, bTestSet, bTestGet>();
		}

		QueryPerformanceCounter(&EndingTime);
		ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
		ElapsedMicroseconds.QuadPart *= 1000000;
		ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;

		if (bPrintRes)
		{
			printf("Time spent in % 33s : %f. Junk %llu\n", TestFuncName, (float)ElapsedMicroseconds.QuadPart, junk.mystate);
			g_RunnedFunctionNames_128[g_RunningTestIndex_128] = TestFuncName;
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
			for (g_RunningTestIndex_128 = 0; g_RunningTestIndex_128 < 6; g_RunningTestIndex_128++)
			{
				sumResultTimes[g_RunningTestIndex_128] += BenchmarkGenericTest<bTestInit, bTestSet, bTestGet>(i == (REPEAT_TESTS_COUNT - 1));
			}
		}

		for (size_t i = 0; i < _countof(sumResultTimes); i++)
		{
			if (sumResultTimes[i] == 0)
				continue;
			LONGLONG avgExecutionTime = sumResultTimes[i] / REPEAT_TESTS_COUNT;
			printf("Test % 33s single execution time %lld ns, total %lld\n", g_RunnedFunctionNames_128[i], avgExecutionTime, sumResultTimes[i]);
		}
	}
};

using namespace Testing128bitKeys;

void SetSomeBitsOnKey(ipv6key* key, size_t seed)
{
	size_t seed1 = (seed * (maxKeyValue / maxValueCount)) % maxKeyValue;
	size_t seed2 = (seed * (maxKeyValue / maxValueCount)) % maxKeyValue;
	assert((sizeof(ipv6key) % sizeof(size_t)) == 0);
	size_t* valuesToSet = (size_t*)key;
	valuesToSet[0] = seed1;
	valuesToSet[1] = seed2;
}

int Run128BPKTests()
{
	printf("Runnning 128 BKP tests \n\n");

	g_useThisForStorageTest_128 = new TestStorageWithStruct();

	g_IndexSetOrder_128 = (ipv6key*)malloc(maxValueCount * sizeof(ipv6key));
	g_IndexGetOrder_128 = (ipv6key*)malloc(maxValueCount * sizeof(ipv6key));
	if (g_IndexSetOrder_128 == NULL || g_IndexGetOrder_128 == NULL)
	{
		return -1;
	}
	for (size_t i = 0; i < maxValueCount; i++)
	{
		SetSomeBitsOnKey(&g_IndexSetOrder_128[i], i);
		g_IndexGetOrder_128[i] = g_IndexSetOrder_128[i]; // every get will be successfull
	}

	// for the sake of bounds checking
	g_useThisForStorageTest_128->mystate = 1;

	size_t memSnapshotBefore, memsnashotafter;
	// warmup
	memSnapshotBefore = GetHeapMemoryUsage();
	g_StdMap_128 = new std::map<ipv6key, TestStorageWithStruct>();
	RunHashTest<true, true, true>();
	memsnashotafter = GetHeapMemoryUsage();
	printf("KBytes allocated while running RunHashTest : %lld\n", (memsnashotafter - memSnapshotBefore) / 1024);

	memSnapshotBefore = GetHeapMemoryUsage();
	g_StdUnorderedMap_128 = new std::unordered_map<ipv6key, TestStorageWithStruct, CustomHash1, ipv6EQTo>();
	RunUnorderedMapTest<true, true, true>();
	memsnashotafter = GetHeapMemoryUsage();
	printf("KBytes allocated while running RunUnorderedMapTest : %lld\n", (memsnashotafter - memSnapshotBefore) / 1024);

	memSnapshotBefore = GetHeapMemoryUsage();
	g_StdUnorderedMapNoHash_128 = new std::unordered_map<ipv6key, TestStorageWithStruct, CustomHash2, ipv6EQTo>();
	RunUnorderedMapNoHashTest<true, true, true>();
	memsnashotafter = GetHeapMemoryUsage();
	printf("KBytes allocated while running RunUnorderedMapNoHashTest : %lld\n", (memsnashotafter - memSnapshotBefore) / 1024);

	memSnapshotBefore = GetHeapMemoryUsage();
	g_StdUnorderedMultiMapNoHash_128 = new std::unordered_multimap<ipv6key, TestStorageWithStruct, CustomHash3, ipv6EQTo>();
	RunUnorderedMultiMapNoHashTest<true, true, true>();
	memsnashotafter = GetHeapMemoryUsage();
	printf("KBytes allocated while running RunUnorderedMultiMapNoHashTest : %lld\n", (memsnashotafter - memSnapshotBefore) / 1024);

	memSnapshotBefore = GetHeapMemoryUsage();
	g_ArrayStorage_128 = new ArrayStorage<ipv6key, TestStorageWithStruct, 0xFFFF>();
	RunArrayStorageTest<true, true, true>();
	memsnashotafter = GetHeapMemoryUsage();
	printf("KBytes allocated while running RunArrayStorageTest : %lld\n", (memsnashotafter - memSnapshotBefore) / 1024);

	memSnapshotBefore = GetHeapMemoryUsage();
	g_ArrayStorage_v2_128 = new ArrayStorage_v2<ipv6key, TestStorageWithStruct, 0xFFFF>();
	RunArrayStorage_v2_Test<true, true, true>();
	memsnashotafter = GetHeapMemoryUsage();
	printf("KBytes allocated while running RunArrayStorage_v2_Test : %lld\n", (memsnashotafter - memSnapshotBefore) / 1024);

	// run the speed tests
	RunInitSetGetTests<true, true, false>();
	RunInitSetGetTests<false, false, true>();

	free(g_IndexSetOrder_128);
	free(g_IndexGetOrder_128);
	delete g_useThisForStorageTest_128;
	delete g_StdMap_128;
	delete g_ArrayStorage_128;
	delete g_ArrayStorage_v2_128;
	delete g_StdUnorderedMap_128;

	return 0;
}