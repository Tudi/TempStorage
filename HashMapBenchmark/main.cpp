#include <Windows.h>
#include <map>
#include <conio.h>

template <typename T>
class SmallLookupMap
{
public:
	SmallLookupMap(int Max)
	{
		MaxKey = Max;
		LookupTable = (T*)malloc(Max * sizeof(T));
	}
	~SmallLookupMap()
	{
		free(LookupTable);
		LookupTable = NULL;
	}
	const inline void Set(int index, T val)
	{
		LookupTable[index] = val;
	}
	const inline T Get(int index)
	{
		return LookupTable[index];
	}
private:
	int		MaxKey;
	T		*LookupTable;
};

int RunHashTest(int MaxCount)
{
	int result = 0;	//needs to exist to avoid optimisations from compiler

	std::map<int, int> m;

	//fill test
	for (int i = 0; i < MaxCount; i++)
		m[i] = i;

	//search test
	for (int i = 0; i < MaxCount; i++)
		result += m[i];

	//anti optimisation dummy return
	return result;

}
void BenchmarkHash(int MaxCount)
{
	int junk = 0;
	LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
	LARGE_INTEGER Frequency;
	QueryPerformanceFrequency(&Frequency);
	QueryPerformanceCounter(&StartingTime);

	junk = RunHashTest(MaxCount);

	QueryPerformanceCounter(&EndingTime);
	ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
	ElapsedMicroseconds.QuadPart *= 1000000;
	ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;

	printf("Time spent in %s : %f\n", __FUNCTION__, (float)ElapsedMicroseconds.QuadPart);
}

int RunLookupTableTest(int MaxCount)
{
	int result = 0;	//needs to exist to avoid optimisations from compiler

	SmallLookupMap<int> m(MaxCount);

	//fill test
	for (int i = 0; i < MaxCount; i++)
		m.Set(i, i);

	//search test
	for (int i = 0; i < MaxCount; i++)
		result += m.Get(i);

	//anti optimisation dummy return
	return result;
}
void BenchmarkLookuptable(int MaxCount)
{
	int junk = 0;
	LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
	LARGE_INTEGER Frequency;
	QueryPerformanceFrequency(&Frequency);
	QueryPerformanceCounter(&StartingTime);

	junk = RunLookupTableTest(MaxCount);

	QueryPerformanceCounter(&EndingTime);
	ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
	ElapsedMicroseconds.QuadPart *= 1000000;
	ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;

	printf("Time spent in %s : %f\n", __FUNCTION__, (float)ElapsedMicroseconds.QuadPart);
}

void main()
{
	for (int i = 0; i < 10; i++)
	{
		BenchmarkHash(65535);
		BenchmarkLookuptable(65535);
	}
	printf("Press any key to exit");
	_getch();
}