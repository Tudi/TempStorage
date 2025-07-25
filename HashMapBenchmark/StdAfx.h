#pragma once

#include <Windows.h>
#include <map>
#include <unordered_map>
#include <conio.h>
#include <assert.h>
#include <string>

#ifdef _DEBUG
	#define ASSERT assert
#else
	#define ASSERT(x)
#endif

#include "FlatLookupMap.hpp"
#include "ArrayStorage.hpp"
#include "ConfigurableTranslationMap.hpp"
#include "ConfigurableTranslationMap_v2.hpp"

// very basic test to see order of magnitude differences between containers that use different number of indirections to get value based on key

struct ipv4key
{
	uint32_t  bytes4[1];
};

typedef struct ipv6key
{
	uint64_t	bytes8[2];

	ipv6key() = default;
	ipv6key(const ipv6key&) = default;
	ipv6key& operator=(const ipv6key&) = default;

	inline bool operator<(const ipv6key& other) const {
		// Compare based on the first member, then the second
		if (bytes8[0] < other.bytes8[0]) {
			return true;
		}
		else if (bytes8[0] == other.bytes8[0]) {
			return bytes8[1] < other.bytes8[1];
		}
		else {
			return false;
		}
	}
}ipv6key;

#pragma pack(push, 1)
typedef struct FlowKey
{
	uint64_t	ip1[2]; // v4 or v6
	uint64_t	ip2[2]; // v4 or v6
	uint16_t	port1, port2;
	uint8_t		l4_proto, vlan;
	uint8_t		padding[2];
	inline bool operator<(const FlowKey& other) const {
		return std::memcmp(this, &other, sizeof(FlowKey)) < 0;
	}
}FlowKey;
#pragma pack(pop)
static_assert((sizeof(FlowKey) % 8) == 0, "must be multiple of 8");

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
int Run32BPKTests();
int Run128BPKTests();
int RunFlowKey304BPKTests();