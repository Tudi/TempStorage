#include "StdAfx.h"
#include "uthash.h"

// very basic test to see order of magnitude differences between containers that use different number of indirections to get value based on key
#pragma pack(push, 1)
typedef struct FlowKeyUTHash
{
	uint64_t	ip1[2]; // v4 or v6
	uint64_t	ip2[2]; // v4 or v6
	uint16_t	port1, port2;
	uint8_t		l4_proto, vlan;
	uint8_t		padding[10];
}FlowKeyUTHash;
#pragma pack(pop)
static_assert(((sizeof(FlowKeyUTHash)) % 12) == 0, "must be multiple of 8");
typedef struct FlowKeyUTHashEntry
{
	FlowKey entryKey;
	TestStorageWithStruct entryContent;
	UT_hash_handle hh; // stores prev/next hash ..
}FlowKeyUTHashEntry;

namespace Testing304bitKeys {
	const char* g_RunnedFunctionNames_304[15] = {};
	size_t g_RunningTestIndex_304 = 0;

	// Custom hash function for uint32_t
	struct CustomHash1 {
		std::size_t operator()(const FlowKey& key) const {
			const uint8_t* data = reinterpret_cast<const uint8_t*>(&key);
			size_t hash = 0xcbf29ce484222325; // FNV-1a 64-bit offset basis
			for (size_t i = 0; i < sizeof(FlowKey); ++i) {
				hash ^= data[i];
				hash *= 0x100000001b3; // FNV-1a 64-bit prime
			}
			return hash;
		}
	};
	struct CustomHash2 {
		std::size_t operator()(const FlowKey& key) const {
			const size_t* uint64values = (size_t*)&key;
			size_t ret = ~0;
			for (size_t i = 0; i < sizeof(FlowKey) / 8; i++) {
				ret ^= uint64values[i];
			}
			return ret;
		}
	};
	struct FlowKeyEQTo {
		bool operator()(const FlowKey& lhs, const FlowKey& rhs) const {
			return memcmp(&lhs, &rhs, sizeof(FlowKey)) == 0;
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
	FlowKey* g_IndexSetOrder_304 = NULL;
	FlowKey* g_IndexGetOrder_304 = NULL;

	std::map<FlowKey, TestStorageWithStruct>* g_StdMap_304;
	std::unordered_map<FlowKey, TestStorageWithStruct, CustomHash1, FlowKeyEQTo>* g_StdUnorderedMap_304;
	std::unordered_map<FlowKey, TestStorageWithStruct, CustomHash2, FlowKeyEQTo>* g_StdUnorderedMapNoHash_304;
	FlowKeyUTHashEntry* g_utHash_304;

	TestStorageWithStruct* g_useThisForStorageTest_304;

	template<bool bTestInit, bool bTestSet, bool bTestGet>
	__declspec(noinline) TestStorageWithStruct RunHashTest()
	{
		TestStorageWithStruct result = {};	//needs to exist to avoid optimisations from compiler

		if (bTestInit)
		{
			g_StdMap_304->clear();
		}

		if (bTestSet)
		{
			//fill test
			for (size_t i = 0; i < maxValueCount; i++)
			{
				g_useThisForStorageTest_304->mystate = i;
				(*g_StdMap_304)[g_IndexSetOrder_304[i]] = *g_useThisForStorageTest_304;
			}
		}

		if (bTestGet)
		{
			//search test
			for (size_t i = 0; i < maxValueCount; i++)
			{
				auto itr = g_StdMap_304->find(g_IndexGetOrder_304[i]);
				if (itr != g_StdMap_304->end())
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
			g_StdUnorderedMap_304->clear();
		}

		//fill test
		if (bTestSet)
		{
			for (size_t i = 0; i < maxValueCount; i++)
			{
				g_useThisForStorageTest_304->mystate = i;
				(*g_StdUnorderedMap_304)[g_IndexSetOrder_304[i]] = *g_useThisForStorageTest_304;
			}
		}

		//search test
		if (bTestGet)
		{
			for (size_t i = 0; i < maxValueCount; i++)
			{
				auto itr = g_StdUnorderedMap_304->find(g_IndexGetOrder_304[i]);
				if (itr != g_StdUnorderedMap_304->end())
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
			g_StdUnorderedMapNoHash_304->clear();
		}

		//fill test
		if (bTestSet)
		{
			for (size_t i = 0; i < maxValueCount; i++)
			{
				g_useThisForStorageTest_304->mystate = i;
				(*g_StdUnorderedMapNoHash_304)[g_IndexSetOrder_304[i]] = *g_useThisForStorageTest_304;
			}
		}

		//search test
		if (bTestGet)
		{
			for (size_t i = 0; i < maxValueCount; i++)
			{
				auto itr = g_StdUnorderedMapNoHash_304->find(g_IndexGetOrder_304[i]);
				if (itr != g_StdUnorderedMapNoHash_304->end())
				{
					result.AppendState(itr->second);
				}
			}
		}

		//anti optimisation dummy return
		return result;
	}

	template<bool bTestInit, bool bTestSet, bool bTestGet>
	__declspec(noinline) TestStorageWithStruct RunUTHashTest()
	{
		TestStorageWithStruct result = {};	//needs to exist to avoid optimisations from compiler

		if (bTestInit)
		{
			FlowKeyUTHashEntry* entry = NULL, * tmp = NULL;
			HASH_ITER(hh, g_utHash_304, entry, tmp) {
				HASH_DEL(g_utHash_304, entry);
				free(entry);
			}
		}

		//fill test
		if (bTestSet)
		{
			for (size_t i = 0; i < maxValueCount; i++)
			{
				g_useThisForStorageTest_304->mystate = i;

				FlowKeyUTHashEntry* new_entry = (FlowKeyUTHashEntry*)malloc(sizeof(FlowKeyUTHashEntry));
				new_entry->entryContent = *g_useThisForStorageTest_304;
				new_entry->entryKey = g_IndexSetOrder_304[i];
				HASH_ADD_KEYPTR(hh, g_utHash_304, &new_entry->entryKey, sizeof(new_entry->entryKey), new_entry);
			}
		}

		//search test
		if (bTestGet)
		{
			for (size_t i = 0; i < maxValueCount; i++)
			{
				FlowKeyUTHashEntry* existing_entry;
				HASH_FIND(hh, g_utHash_304, &g_IndexGetOrder_304[i], sizeof(g_IndexSetOrder_304[0]), existing_entry);
				if (existing_entry) {
					result.AppendState(existing_entry->entryContent);
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
		if (g_RunningTestIndex_304 == 0)
		{
			TestFuncName = "std::HashMap";
			junk = RunHashTest<bTestInit, bTestSet, bTestGet>();
		}
		else if (g_RunningTestIndex_304 == 2)
		{
			TestFuncName = "uthash";
			junk = RunUTHashTest<bTestInit, bTestSet, bTestGet>();
		}
		else if (g_RunningTestIndex_304 == 3)
		{
			TestFuncName = "std::unordered_map";
			junk = RunUnorderedMapTest<bTestInit, bTestSet, bTestGet>();
		}
		else if (g_RunningTestIndex_304 == 4)
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
			g_RunnedFunctionNames_304[g_RunningTestIndex_304] = TestFuncName;
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
			for (g_RunningTestIndex_304 = 0; g_RunningTestIndex_304 < 6; g_RunningTestIndex_304++)
			{
				sumResultTimes[g_RunningTestIndex_304] += BenchmarkGenericTest<bTestInit, bTestSet, bTestGet>(i == (REPEAT_TESTS_COUNT - 1));
			}
		}

		for (size_t i = 0; i < _countof(sumResultTimes); i++)
		{
			if (sumResultTimes[i] == 0)
				continue;
			LONGLONG avgExecutionTime = sumResultTimes[i] / REPEAT_TESTS_COUNT;
			printf("Test % 33s single execution time %lld ns, total %lld\n", g_RunnedFunctionNames_304[i], avgExecutionTime, sumResultTimes[i]);
		}
	}
};

using namespace Testing304bitKeys;

void SetSomeBitsOnKey(FlowKey* key, size_t seed)
{
	size_t seed1 = (seed * (maxKeyValue / maxValueCount)) % maxKeyValue;
	size_t seed2 = (seed * (maxKeyValue / maxValueCount)) % maxKeyValue;
	assert((sizeof(FlowKey) % sizeof(size_t)) == 0);
	size_t* valuesToSet = (size_t*)key;
	valuesToSet[0] = seed1;
	valuesToSet[1] = seed2;
	valuesToSet[2] = seed1;
	valuesToSet[3] = seed2;
	valuesToSet[4] = seed1;
}

int RunFlowKey304BPKTests()
{
	printf("Runnning 304 BKP tests \n\n");

	g_useThisForStorageTest_304 = new TestStorageWithStruct();

	g_IndexSetOrder_304 = (FlowKey*)malloc(maxValueCount * sizeof(FlowKey));
	g_IndexGetOrder_304 = (FlowKey*)malloc(maxValueCount * sizeof(FlowKey));
	if (g_IndexSetOrder_304 == NULL || g_IndexGetOrder_304 == NULL)
	{
		return -1;
	}
	for (size_t i = 0; i < maxValueCount; i++)
	{
		SetSomeBitsOnKey(&g_IndexSetOrder_304[i], i);
		g_IndexGetOrder_304[i] = g_IndexSetOrder_304[i]; // every get will be successfull
	}

	// for the sake of bounds checking
	g_useThisForStorageTest_304->mystate = 1;

	size_t memSnapshotBefore, memsnashotafter;
	// warmup
	memSnapshotBefore = GetHeapMemoryUsage();
	g_StdMap_304 = new std::map<FlowKey, TestStorageWithStruct>();
	RunHashTest<true, true, true>();
	memsnashotafter = GetHeapMemoryUsage();
	printf("KBytes allocated while running RunHashTest : %lld\n", (memsnashotafter - memSnapshotBefore) / 1024);

	memSnapshotBefore = GetHeapMemoryUsage();
	g_StdUnorderedMap_304 = new std::unordered_map<FlowKey, TestStorageWithStruct, CustomHash1, FlowKeyEQTo>();
	RunUnorderedMapTest<true, true, true>();
	memsnashotafter = GetHeapMemoryUsage();
	printf("KBytes allocated while running RunUnorderedMapTest : %lld\n", (memsnashotafter - memSnapshotBefore) / 1024);

	memSnapshotBefore = GetHeapMemoryUsage();
	g_StdUnorderedMapNoHash_304 = new std::unordered_map<FlowKey, TestStorageWithStruct, CustomHash2, FlowKeyEQTo>();
	RunUnorderedMapNoHashTest<true, true, true>();
	memsnashotafter = GetHeapMemoryUsage();
	printf("KBytes allocated while running RunUnorderedMapNoHashTest : %lld\n", (memsnashotafter - memSnapshotBefore) / 1024);

	memSnapshotBefore = GetHeapMemoryUsage();
//	g_utHash_304 = (FlowKeyUTHashEntry*)malloc(sizeof(FlowKeyUTHashEntry));
//	memset(g_utHash_304, 0, sizeof(FlowKeyUTHashEntry));
	g_utHash_304 = NULL;
	RunUTHashTest<true, true, true>();
	memsnashotafter = GetHeapMemoryUsage();
	printf("KBytes allocated while running RunUTHashTest : %lld\n", (memsnashotafter - memSnapshotBefore) / 1024);

	// run the speed tests
	RunInitSetGetTests<true, true, false>();
	RunInitSetGetTests<false, false, true>();

	free(g_IndexSetOrder_304);
	free(g_IndexGetOrder_304);
	delete g_useThisForStorageTest_304;
	delete g_StdMap_304;
	free(g_utHash_304);

	return 0;
}