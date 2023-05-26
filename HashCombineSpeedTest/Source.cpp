#include <nmmintrin.h>
#include <cstring>
#include <stdint.h>
#include <stdio.h>
#include <Windows.h>
#include "Profile.h"
#include "city.h"

#define RepeatTestCount 1000

#define CombineTwoHashes(hash1, hash2) (hash1^hash2)
//    hash2 += 0x9e3779b97f4a7c13ULL + (hash1 << 6) + (hash1 >> 2);
//    hash1 ^= hash2;
//    return hash1;

template <size_t NumHashes>
static uint64_t CombineHashes(const uint64_t(&hashes)[NumHashes]) {
    static_assert(NumHashes > 0, "At least one hash value is required.");

    if (NumHashes == 1)
    {
        return hashes[0];
    }
    else if (NumHashes == 2)
    {
        return CombineTwoHashes(hashes[0], hashes[1]);
    }
    else if (NumHashes == 4)
    {
        return CombineTwoHashes(CombineTwoHashes(hashes[0], hashes[1]), CombineTwoHashes(hashes[2], hashes[3]));
    }
    else if (NumHashes == 8)
    {
        uint64_t t1 = CombineTwoHashes(CombineTwoHashes(hashes[0], hashes[1]), CombineTwoHashes(hashes[2], hashes[3]));
        uint64_t t2 = CombineTwoHashes(CombineTwoHashes(hashes[4], hashes[5]), CombineTwoHashes(hashes[6], hashes[7]));
        return CombineTwoHashes(t1, t2);
    }

    uint64_t combinedHash = hashes[0];

    for (size_t i = 1; i < NumHashes; ++i) 
    {
        combinedHash = CombineTwoHashes(combinedHash, hashes[i]);
    }

    return combinedHash;
}

template <size_t Size>
static uint64_t HashWithCombine(void *buff) {
    constexpr size_t numChunks = (Size + 7) / 8;
    constexpr size_t remainder = Size % 8;

    const uint64_t* chunks = reinterpret_cast<const uint64_t*>(buff);
    uint64_t hashes[numChunks];

    // Hash complete 8-byte chunks
    for (size_t i = 0; i < numChunks; ++i) {
        hashes[i] = _mm_crc32_u64(0, chunks[i]);
    }

    // Hash the remaining bytes (if any)
    if (remainder > 0) {
        uint64_t remainderChunk = 0;
        const uint8_t* remainderBytes = reinterpret_cast<const uint8_t*>(&chunks[numChunks]);
        std::memcpy(&remainderChunk, remainderBytes, remainder);
        const uint64_t ret = CombineHashes<numChunks>(hashes);
        return _mm_crc32_u64(ret, remainderChunk);
    }

    // Combine all hash values
    return CombineHashes<numChunks>(hashes);
}

template <size_t Size>
static uint64_t NormalHash1Struct(uint64_t* inputBuffer)
{
    uint64_t hash_ret = 0;
    for (size_t j = 0; j < Size / 8; j++)
    {
        hash_ret = _mm_crc32_u64(hash_ret, inputBuffer[j]);
    }
    return hash_ret;
}

// wanted to check if I should replace function pointer with compiler optimization
template <size_t Size>
static uint64_t RunSpeedTestHashKnownStruct(char* inputBuffer, uint32_t buffSize, uint32_t jump)
{
    uint64_t hash_ret = 0;
    for (size_t repeatCnt = 0; repeatCnt < RepeatTestCount; repeatCnt++)
    {
        for (size_t i = 0; i < buffSize - jump; i += jump)
        {
            hash_ret += NormalHash1Struct<Size>((uint64_t*)&inputBuffer[i]);
        }
    }
    return hash_ret;
}

static uint64_t NormalHash1Block(uint64_t* inputBuffer, size_t blockSize)
{
    uint64_t hash_ret = 0;
    for (size_t j = 0; j < blockSize / 8; j++)
    {
        hash_ret = _mm_crc32_u64(hash_ret, inputBuffer[j]);
    }
    return hash_ret;
}

// because it was made with function pointers in one of the projects I worked on
static uint64_t RunSpeedTestDynamicBuff(char* inputBuffer, uint32_t buffSize, uint32_t jump, uint32_t structSize)
{
    uint64_t hash_ret = 0;
    for (size_t repeatCnt = 0; repeatCnt < RepeatTestCount; repeatCnt++)
    {
        for (size_t i = 0; i < buffSize - jump; i += jump)
        {
            hash_ret += NormalHash1Block((uint64_t*)&inputBuffer[i], structSize);
        }
    }
    return hash_ret;
}

// asked AI how to combine data. Curious how much worse this is
template <size_t Size>
static uint64_t RunSpeedTestHashKnownStructCombine(char* inputBuffer, uint32_t buffSize, uint32_t jump)
{
    uint64_t hash_ret = 0;
    for (size_t repeatCnt = 0; repeatCnt < RepeatTestCount; repeatCnt++)
    {
        for (size_t i = 0; i < buffSize - jump; i += jump)
        {
            hash_ret += HashWithCombine<Size>(&inputBuffer[i]);
        }
    }
    return hash_ret;
}

// because it was made with function pointers in one of the projects I worked on
static uint64_t RunSpeedTestCityDynamicBuff(char* inputBuffer, uint32_t buffSize, uint32_t jump, uint32_t structSize)
{
    uint64_t hash_ret = 0;
    for (size_t repeatCnt = 0; repeatCnt < RepeatTestCount; repeatCnt++)
    {
        for (size_t i = 0; i < buffSize - jump; i += jump)
        {
            hash_ret += CityHash64(&inputBuffer[i], structSize);
        }
    }
    return hash_ret;
}

template <size_t Size>
static uint64_t RunSpeedTestHashCityKnownStruct(char* inputBuffer, uint32_t buffSize, uint32_t jump)
{
    uint64_t hash_ret = 0;
    for (size_t repeatCnt = 0; repeatCnt < RepeatTestCount; repeatCnt++)
    {
        for (size_t i = 0; i < buffSize - jump; i += jump)
        {
            hash_ret += CityHash64_<Size>(&inputBuffer[i]);
        }
    }
    return hash_ret;
}

int main()
{
    uint32_t buffSize = 0;
    uint32_t buffCnt = 0;
    printf("memory block size (anti optimization. must write 1): \n");
    scanf_s("%u", &buffCnt);
    uint32_t jmpSize = 0;
    printf("block jump size (anti optimization. larger then 64): \n");
    scanf_s("%u", &jmpSize);

    buffSize = buffCnt * 1024 * 1024 * 100;
    char* inputBuffer = (char*)malloc(buffSize + 64);
    if (inputBuffer == NULL)
    {
        printf("Failed to allocate memory\n");
        return 1;
    }
    for (size_t i = 0; i < buffSize; i++)
    {
        inputBuffer[i] = (char)i;
    }

    uint64_t final_val;
    __int64 startStamp, startStamp2;

    printf("\n");
    startStamp = GetTick(); startStamp2 = GetTickCount64();
    final_val = RunSpeedTestHashKnownStruct<8>(inputBuffer, buffSize, jmpSize);
    printf("KnownSize 8. Res %llu. Time : %lf %lld\n", final_val, GetCounterDiff(startStamp), GetTickCount64() - startStamp2);

    startStamp = GetTick(); startStamp2 = GetTickCount64();
    final_val = RunSpeedTestHashKnownStruct<16>(inputBuffer, buffSize, jmpSize);
    printf("KnownSize 16. Res %llu. Time : %lf %lld\n", final_val, GetCounterDiff(startStamp), GetTickCount64() - startStamp2);

    startStamp = GetTick(); startStamp2 = GetTickCount64();
    final_val = RunSpeedTestHashKnownStruct<32>(inputBuffer, buffSize, jmpSize);
    printf("KnownSize 32. Res %llu. Time : %lf %lld\n", final_val, GetCounterDiff(startStamp), GetTickCount64() - startStamp2);

    startStamp = GetTick(); startStamp2 = GetTickCount64();
    final_val = RunSpeedTestHashKnownStruct<64>(inputBuffer, buffSize, jmpSize);
    printf("KnownSize 64. Res %llu. Time : %lf %lld\n", final_val, GetCounterDiff(startStamp), GetTickCount64() - startStamp2);

    printf("\n");
    startStamp = GetTick(); startStamp2 = GetTickCount64();
    final_val = RunSpeedTestDynamicBuff(inputBuffer, buffSize, jmpSize, 8 * buffCnt);
    printf("DynamicSize 8. Res %llu. Time : %lf %lld\n", final_val, GetCounterDiff(startStamp), GetTickCount64() - startStamp2);

    startStamp = GetTick(); startStamp2 = GetTickCount64();
    final_val = RunSpeedTestDynamicBuff(inputBuffer, buffSize, jmpSize, 16 * buffCnt);
    printf("DynamicSize 16. Res %llu. Time : %lf %lld\n", final_val, GetCounterDiff(startStamp), GetTickCount64() - startStamp2);

    startStamp = GetTick(); startStamp2 = GetTickCount64();
    final_val = RunSpeedTestDynamicBuff(inputBuffer, buffSize, jmpSize, 32 * buffCnt);
    printf("DynamicSize 32. Res %llu. Time : %lf %lld\n", final_val, GetCounterDiff(startStamp), GetTickCount64() - startStamp2);

    startStamp = GetTick(); startStamp2 = GetTickCount64();
    final_val = RunSpeedTestDynamicBuff(inputBuffer, buffSize, jmpSize, 64 * buffCnt);
    printf("DynamicSize 64. Res %llu. Time : %lf %lld\n", final_val, GetCounterDiff(startStamp), GetTickCount64() - startStamp2);

    printf("\n");
    startStamp = GetTick(); startStamp2 = GetTickCount64();
    final_val = RunSpeedTestHashKnownStructCombine<8>(inputBuffer, buffSize, jmpSize);
    printf("Combine 8. Res %llu. Time : %lf %lld\n", final_val, GetCounterDiff(startStamp), GetTickCount64() - startStamp2);

    startStamp = GetTick(); startStamp2 = GetTickCount64();
    final_val = RunSpeedTestHashKnownStructCombine<16>(inputBuffer, buffSize, jmpSize);
    printf("Combine 16. Res %llu. Time : %lf %lld\n", final_val, GetCounterDiff(startStamp), GetTickCount64() - startStamp2);

    startStamp = GetTick(); startStamp2 = GetTickCount64();
    final_val = RunSpeedTestHashKnownStructCombine<32>(inputBuffer, buffSize, jmpSize);
    printf("Combine 32. Res %llu. Time : %lf %lld\n", final_val, GetCounterDiff(startStamp), GetTickCount64() - startStamp2);

    startStamp = GetTick(); startStamp2 = GetTickCount64();
    final_val = RunSpeedTestHashKnownStructCombine<64>(inputBuffer, buffSize, jmpSize);
    printf("Combine 64. Res %llu. Time : %lf %lld\n", final_val, GetCounterDiff(startStamp), GetTickCount64() - startStamp2);

    printf("\n");
    startStamp = GetTick(); startStamp2 = GetTickCount64();
    final_val = RunSpeedTestCityDynamicBuff(inputBuffer, buffSize, jmpSize, 8 * buffCnt);
    printf("city DynamicSize 8. Res %llu. Time : %lf %lld\n", final_val, GetCounterDiff(startStamp), GetTickCount64() - startStamp2);

    startStamp = GetTick(); startStamp2 = GetTickCount64();
    final_val = RunSpeedTestCityDynamicBuff(inputBuffer, buffSize, jmpSize, 16 * buffCnt);
    printf("city DynamicSize 16. Res %llu. Time : %lf %lld\n", final_val, GetCounterDiff(startStamp), GetTickCount64() - startStamp2);

    startStamp = GetTick(); startStamp2 = GetTickCount64();
    final_val = RunSpeedTestCityDynamicBuff(inputBuffer, buffSize, jmpSize, 32 * buffCnt);
    printf("city DynamicSize 32. Res %llu. Time : %lf %lld\n", final_val, GetCounterDiff(startStamp), GetTickCount64() - startStamp2);

    startStamp = GetTick(); startStamp2 = GetTickCount64();
    final_val = RunSpeedTestCityDynamicBuff(inputBuffer, buffSize, jmpSize, 64 * buffCnt);
    printf("city DynamicSize 64. Res %llu. Time : %lf %lld\n", final_val, GetCounterDiff(startStamp), GetTickCount64() - startStamp2);

    printf("\n");
    startStamp = GetTick(); startStamp2 = GetTickCount64();
    final_val = RunSpeedTestHashCityKnownStruct<8>(inputBuffer, buffSize, jmpSize);
    printf("city fixed 8. Res %llu. Time : %lf %lld\n", final_val, GetCounterDiff(startStamp), GetTickCount64() - startStamp2);

    startStamp = GetTick(); startStamp2 = GetTickCount64();
    final_val = RunSpeedTestHashCityKnownStruct<16>(inputBuffer, buffSize, jmpSize);
    printf("city fixed 16. Res %llu. Time : %lf %lld\n", final_val, GetCounterDiff(startStamp), GetTickCount64() - startStamp2);

    startStamp = GetTick(); startStamp2 = GetTickCount64();
    final_val = RunSpeedTestHashCityKnownStruct<32>(inputBuffer, buffSize, jmpSize);
    printf("city fixed 32. Res %llu. Time : %lf %lld\n", final_val, GetCounterDiff(startStamp), GetTickCount64() - startStamp2);

    startStamp = GetTick(); startStamp2 = GetTickCount64();
    final_val = RunSpeedTestHashCityKnownStruct<64>(inputBuffer, buffSize, jmpSize);
    printf("city fixed 64. Res %llu. Time : %lf %lld\n", final_val, GetCounterDiff(startStamp), GetTickCount64() - startStamp2);

    return 0;
}