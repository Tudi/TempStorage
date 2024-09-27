#pragma once

#include <Windows.h>
#include <map>
#include <unordered_map>
#include <conio.h>
#include <assert.h>
#include <string>
#include "FlatLookupMap.hpp"
#include "ArrayStorage.hpp"
#include "ConfigurableTranslationMap.hpp"
#include "ConfigurableTranslationMap_v2.hpp"

// very basic test to see order of magnitude differences between containers that use different number of indirections to get value based on key

struct ipv4key
{
	char bytes[4];
};

struct ipv6key
{
	char bytes[16];
};

struct TestStorageWithStruct
{
	void AppendState(const TestStorageWithStruct& addme)
	{
		mystate += addme.mystate;
	}
	ipv4key myipv4rowkey;
	ipv6key myipv6rowkey;
	size_t mystate;
	int someblabla;
};

SIZE_T GetHeapMemoryUsage();
int Run24BPKTests();

