#include <string.h>
#include <stdio.h>
#include <conio.h>

#include "Src\WDRCrypt.hxx"
#include "SpeedTest.h"

void main()
{
	// should be loaded from config file or generated per session...
	char	StaticSalt[] = "Session1HelmutUnsecurePassw";
	size_t	SaltSize = strlen(StaticSalt);

	//should be added to WDR
	WDRCrypt Endpoint1, Endpoint2;
	Endpoint1.Init(StaticSalt, SaltSize);
	Endpoint2.Init(StaticSalt, SaltSize);

	//testing functionality
	char	TestString[] = "Almost super secret message";
	size_t	TestBuffSize = strlen(TestString) + 1;
	char	*EncryptedBuffer = new char[TestBuffSize];
	char	*DecryptedBuffer = new char[TestBuffSize];

	// testing channel 1
	Endpoint1.Process(TestString, TestBuffSize, EncryptedBuffer, CRYPT_CHANNEL_SERVER_SEND);
	Endpoint2.Process(EncryptedBuffer, TestBuffSize, DecryptedBuffer, CRYPT_CHANNEL_CLIENT_RECV);

	if (memcmp(TestString, DecryptedBuffer, TestBuffSize) != 0)
		printf("Could not decode message 1\n");
	else
		printf("Properly encoded/decoded message 1\n");

	// testing channel 2
	Endpoint2.Process(TestString, TestBuffSize, EncryptedBuffer, CRYPT_CHANNEL_CLIENT_SEND);
	Endpoint1.Process(EncryptedBuffer, TestBuffSize, DecryptedBuffer, CRYPT_CHANNEL_SERVER_RECV);

	if (memcmp(TestString, DecryptedBuffer, TestBuffSize) != 0)
		printf("Could not decode message 2\n");
	else
		printf("Properly encoded/decoded message 2\n");

	//perform some speed tests that could be used as reference in case we inspect other Encryption methods
	TestSingleThreadedSpeed();

	//perform speed test in case we manage to run multiple network treads in parallel
	TestMultiThreadedSpeed();

	printf("all done\n");
	_getch();
}