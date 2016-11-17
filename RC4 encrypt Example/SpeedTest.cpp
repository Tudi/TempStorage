#include "Src\WDRCrypt.hxx"
#include "Tools.h"
#include <string.h>
#include <stdio.h>
#include <conio.h>
#include <Windows.h>
#include <mutex>

#define SPEED_TEST_DURATION_SEC			5
#define OVERHEAD_REDUCE_LOOP_UNROLL		10
#define NR_PARALEL_THREADS_FOR_TEST		4

__int64 SingleEncoderBytesPerSec;
void TestSingleThreadedSpeed()
{
	// should be loaded from config file or generated per session...
	char	StaticSalt[] = "Session1HelmutUnsecurePassw";
	size_t	SaltSize = strlen(StaticSalt);

	//should be added to WDR
	WDRCrypt Endpoint1, Endpoint2;
	Endpoint1.Init(StaticSalt, SaltSize);
	Endpoint2.Init(StaticSalt, SaltSize);

	//testing functionality
	char	TestString[] = "Almost super secret message with a reasonable amount of length to bump the 64 byte cache barrier. The more barriers we break the worse results we should get";
	size_t	TestBuffSize = strlen(TestString) + 1;
	char	*EncryptedBuffer = new char[TestBuffSize];
	char	*DecryptedBuffer = new char[TestBuffSize];

	//initialize our clock
	StartCounter();
	int EncodeDecodeCount;
	double Start;
	__int64 BytesPerSecond;

	printf("================================================================\n");

	//test 1 - simple encrypt
	printf("Testing simple encrypt speed for %d seconds\n", SPEED_TEST_DURATION_SEC);
	EncodeDecodeCount = 0;
	Start = GetCounter();
	while (GetCounter() - Start < SPEED_TEST_DURATION_SEC * 1000.0 )
	{
		for (int i = 0; i < OVERHEAD_REDUCE_LOOP_UNROLL;i++)
			Endpoint1.Process(TestString, TestBuffSize, EncryptedBuffer, CRYPT_CHANNEL_SERVER_SEND);
		EncodeDecodeCount += OVERHEAD_REDUCE_LOOP_UNROLL;
	}
	BytesPerSecond = TestBuffSize * EncodeDecodeCount / SPEED_TEST_DURATION_SEC;
	printf("One Encrypter can process %d bytes = %dkb/s = %d MB/s per second\n", BytesPerSecond, BytesPerSecond / 1024, BytesPerSecond / 1024 / 1024);
	printf("================================================================\n");

	// test 2 - encrypt / decrypt
	// due to smaller overhead, the speed of this test can be greater than half of the simple test
	printf("Testing encrypt / descrypt speed for %d seconds\n", SPEED_TEST_DURATION_SEC);
	EncodeDecodeCount = 0;
	Start = GetCounter();
	while (GetCounter() - Start < SPEED_TEST_DURATION_SEC * 1000.0)
	{
		for (int i = 0; i < OVERHEAD_REDUCE_LOOP_UNROLL; i++)
		{
			char *NewMessage = new char[TestBuffSize];
			memcpy(NewMessage, TestString, TestBuffSize);
			Endpoint1.Process(NewMessage, TestBuffSize, EncryptedBuffer, CRYPT_CHANNEL_SERVER_SEND);
			Endpoint1.Process(EncryptedBuffer, TestBuffSize, DecryptedBuffer, CRYPT_CHANNEL_CLIENT_RECV); // this uses some different function with different salt buffers... should have slightly less than half speed
			delete[]NewMessage;
		}
		EncodeDecodeCount += OVERHEAD_REDUCE_LOOP_UNROLL;
	}
	BytesPerSecond = TestBuffSize * EncodeDecodeCount / SPEED_TEST_DURATION_SEC;
	printf("encrypt / descrypt can process %d bytes = %dkb/s = %d MB/s per second\n", BytesPerSecond, BytesPerSecond / 1024, BytesPerSecond / 1024 / 1024);
	printf("================================================================\n");

}

std::mutex WriteScreenLock;
__int64 TotalThreadedEncodeBytesPerSec;
int ThreadsExited = 0;
DWORD WINAPI EncryptMessage(void *param)
{
	char	StaticSalt[] = "Session1HelmutUnsecurePassw";
	size_t	SaltSize = strlen(StaticSalt);

	//should be added to WDR
	WDRCrypt Endpoint1, Endpoint2;
	Endpoint1.Init(StaticSalt, SaltSize);

	//testing functionality
	char	TestString[] = "Almost super secret message with a reasonable amount of length to bump the 64 byte cache barrier. The more barriers we break the worse results we should get";
	size_t	TestBuffSize = strlen(TestString) + 1;
	char	*EncryptedBuffer = new char[TestBuffSize];
	char	*DecryptedBuffer = new char[TestBuffSize];

	//initialize our clock
	StartCounter();
	int EncodeDecodeCount;
	double Start;
	__int64 BytesPerSecond;

	EncodeDecodeCount = 0;
	Start = GetCounter();
	while (GetCounter() - Start < SPEED_TEST_DURATION_SEC * 1000.0)
	{
		for (int i = 0; i < OVERHEAD_REDUCE_LOOP_UNROLL; i++)
		{
			//allocate a new buffer. A bit more realistical test than using a constant string...
			char *NewMessage = new char[TestBuffSize];
			memcpy(NewMessage, TestString, TestBuffSize);
			Endpoint1.Process(TestString, TestBuffSize, EncryptedBuffer, CRYPT_CHANNEL_SERVER_SEND);
			delete[]NewMessage;
		}
		EncodeDecodeCount += OVERHEAD_REDUCE_LOOP_UNROLL;
	}
	BytesPerSecond = TestBuffSize * EncodeDecodeCount / SPEED_TEST_DURATION_SEC;
	TotalThreadedEncodeBytesPerSec += BytesPerSecond;

	WriteScreenLock.lock();
	printf("Thread %d Encrypter can process %d bytes = %dkb/s = %d MB/s per second\n", GetCurrentThreadId(), BytesPerSecond, BytesPerSecond / 1024, BytesPerSecond / 1024 / 1024);
	ThreadsExited++;
	WriteScreenLock.unlock();

	return 0;
}

void TestMultiThreadedSpeed()
{
	printf("Test %d threads encoding a message in paralel\n", NR_PARALEL_THREADS_FOR_TEST);
	TotalThreadedEncodeBytesPerSec = 0;
	DWORD ThreadIDs[NR_PARALEL_THREADS_FOR_TEST];
	for (int i = 0; i < NR_PARALEL_THREADS_FOR_TEST;i++)
		CreateThread(NULL, 0, EncryptMessage, 0, 0, &ThreadIDs[i]);

	//wait for threads to finish processing
	while (ThreadsExited < NR_PARALEL_THREADS_FOR_TEST)
		Sleep(100);

	printf("Overall parallel encrypters can process %d bytes = %dkb/s = %d MB/s per second\n", TotalThreadedEncodeBytesPerSec, TotalThreadedEncodeBytesPerSec / 1024, TotalThreadedEncodeBytesPerSec / 1024 / 1024);
	printf("================================================================\n");
}