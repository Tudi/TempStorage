#include <osrng.h>
#include <aes.h>
#include <modes.h>
#include <filters.h>

using namespace CryptoPP;

void Encrypt1Block(byte *key)
{
	unsigned char buffer[] = "This is the message to be encrypted.";
	size_t bufferSize = sizeof(buffer);

	byte iv[CryptoPP::AES::BLOCKSIZE];
	AutoSeededRandomPool rng;
	rng.GenerateBlock(iv, sizeof(iv));

	CryptoPP::AES::Encryption aesEncryption(key, CryptoPP::AES::MAX_KEYLENGTH);
	CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, iv);

	CryptoPP::StreamTransformationFilter filter(cbcEncryption);
	filter.Put(buffer, bufferSize);
	filter.MessageEnd();
}

void Decrypt1Block_CBC_AES_128_NOAUTH(const byte* iv, const byte* key, const byte* data, int dataLen, byte* out_data)
{
	// Create the decryption object
	CBC_Mode<AES>::Decryption decryptor;
	decryptor.SetKeyWithIV(key, CryptoPP::AES::DEFAULT_KEYLENGTH, iv);

	decryptor.ProcessData(out_data, data, dataLen);
}

void Decrypt1Block_CBC_AES_256_NOAUTH(const byte*iv, const byte*key, const byte*data, int dataLen, byte* out_data)
{
	// Create the decryption object
	CBC_Mode<AES>::Decryption decryptor;
	decryptor.SetKeyWithIV(key, CryptoPP::AES::MAX_KEYLENGTH, iv);

	decryptor.ProcessData(out_data, data, dataLen);
}

void LoadIV_Data_Hash_From(const char* fname, byte* iv, byte*data, int dataSize, byte* hash)
{
	FILE* f;
	errno_t openErr = fopen_s(&f, fname, "rb");
	if (f)
	{
		fseek(f, 8, SEEK_SET);
		fread(iv, 1, CryptoPP::AES::BLOCKSIZE, f);
		fread(data, 1, dataSize, f);
		fread(hash, 1, CryptoPP::SHA256::DIGESTSIZE, f);
		fclose(f);
	}
	else
	{
		printf("Failed to open input file %s\n", fname);
	}
}

void GetDataBlockHash(byte *iv, byte *data, int dataSize, byte *hash)
{
	CryptoPP::SHA256 sha256;
	sha256.Update(iv, CryptoPP::AES::BLOCKSIZE); // Hash the first block of data
	sha256.Update(data, dataSize); // Hash the second block of data
	sha256.Final(hash); // Finalize the hash
}

size_t getFileSize(FILE *f)
{
	fseek(f, 0, SEEK_END);
	size_t ret = ftell(f);
	fseek(f, 0, SEEK_SET);
	return ret;
}

#pragma pack(push,1)
typedef struct Point2DF
{
	float x, y;
}Point2DF;

typedef struct DB1Layout
{
	char fileName[8];
	char unk64[64];
	int val1;
	int val2;
	int val3;
	int val4;
	Point2DF points[8];
	unsigned short flags1;
	char footer6[6];
	char footer10[16];
}DB1Layout;
#pragma pack(pop)

void TrySpecificLocations()
{
	byte key[CryptoPP::AES::MAX_KEYLENGTH];
	byte iv1[CryptoPP::AES::BLOCKSIZE];
	byte iv2[CryptoPP::AES::BLOCKSIZE];
	byte iv3[CryptoPP::AES::BLOCKSIZE];
	byte hash1[CryptoPP::SHA256::DIGESTSIZE];
	byte hash2[CryptoPP::SHA256::DIGESTSIZE];
	byte hash3[CryptoPP::SHA256::DIGESTSIZE];

	const int dataSize = 176;
	byte data1[dataSize];
	byte data2[dataSize];
	byte data3[dataSize];
	byte data_res1[dataSize];
	byte data_res2[dataSize];
	byte data_res3[dataSize];
	byte hash_res1[CryptoPP::SHA256::DIGESTSIZE];
	byte hash_res2[CryptoPP::SHA256::DIGESTSIZE];
	byte hash_res3[CryptoPP::SHA256::DIGESTSIZE];

	LoadIV_Data_Hash_From("1.saf", iv1, data1, dataSize, hash1);
	LoadIV_Data_Hash_From("2.saf", iv2, data2, dataSize, hash2);
	LoadIV_Data_Hash_From("3.saf", iv3, data3, dataSize, hash3);

	FILE* fdump;
	errno_t errOpen = fopen_s(&fdump, "../allocs.dmp", "rb");
	if (fdump == NULL)
	{
		return;
	}

	size_t locations[] = { 292198472,292198760,292200344,292200488,292200776,292201352,292201640,292202072,292202648,292202792,292203224,292203368,292204520,292205096,317255624,317255768,317255912,317256344,317256488,317256632,317256776,317257064,317257496,317257640,317258072,317258648,317258936,317259224,317259800,317259944,317260232,317260520,317260664,317260808,317260952,317261528,317261672,317262248,317262536,317263544,317263976,317264264,318340808,0 };
	for (size_t locInd = 0; locations[locInd] != 0; locInd++)
	{
		fseek(fdump, (long)locations[locInd], SEEK_SET);
		fread(key, 1, CryptoPP::AES::MAX_KEYLENGTH, fdump);
		Decrypt1Block_CBC_AES_256_NOAUTH(iv1, key, data1, dataSize, data_res1);
		Decrypt1Block_CBC_AES_256_NOAUTH(iv2, key, data2, dataSize, data_res2);
		Decrypt1Block_CBC_AES_256_NOAUTH(iv3, key, data3, dataSize, data_res3);

		if (memcmp(data_res1, data_res2, dataSize) == 0
			&& memcmp(data_res1, data_res3, dataSize) == 0)
		{
			DB1Layout* db1 = (DB1Layout *)data_res1;
			GetDataBlockHash(iv1, data_res1, dataSize, hash_res1);
			GetDataBlockHash(iv2, data_res2, dataSize, hash_res2);
			GetDataBlockHash(iv3, data_res3, dataSize, hash_res3);
			printf("%zd)Result block matches. key maybe found at %zd\n", locInd, locations[locInd]);
			for (size_t ind = 0; ind < CryptoPP::AES::MAX_KEYLENGTH; ind++)
			{
				printf("0x%02X,", *(unsigned char*)&key[ind]);
			}
			printf("\n");
		}
	}

	fclose(fdump);
}

void TryDecryptDB3()
{
	byte key[CryptoPP::AES::MAX_KEYLENGTH] = { 0xAE,0xEC,0x5E,0x77,0x92,0xFF,0x0D,0x37,0xE0,0xA3,0x82,0x77,0x20,0x5A,0xC4,0xCC,0xE7,0x83,0xD8,0xE9,0x49,0x8D,0x87,0x38,0x00,0xB5,0x9B,0xE9,0x5C,0x1B,0xE5,0x7F };
	byte iv1[CryptoPP::AES::BLOCKSIZE];
	byte iv2[CryptoPP::AES::BLOCKSIZE];
	byte hash1[CryptoPP::SHA256::DIGESTSIZE];
	byte hash2[CryptoPP::SHA256::DIGESTSIZE];

	const int dataSize = 176;
	byte data1[dataSize];
	byte data2[dataSize];
	byte data_res1[dataSize];
	byte data_res2[dataSize];
	byte hash_res1[CryptoPP::SHA256::DIGESTSIZE];
	byte hash_res2[CryptoPP::SHA256::DIGESTSIZE];

	LoadIV_Data_Hash_From("1.saf", iv1, data1, dataSize, hash1);
	LoadIV_Data_Hash_From("2.saf", iv2, data2, dataSize, hash2);

	Decrypt1Block_CBC_AES_256_NOAUTH(iv1, key, data1, dataSize, data_res1);
	Decrypt1Block_CBC_AES_256_NOAUTH(iv2, key, data2, dataSize, data_res2);

	if (memcmp(data_res1, data_res2, dataSize) == 0)
	{
		GetDataBlockHash(iv1, data_res1, dataSize, hash_res1);
		GetDataBlockHash(iv2, data_res2, dataSize, hash_res2);
	}
}

int main()
{
	TrySpecificLocations(); return 0;

	byte key[CryptoPP::AES::MAX_KEYLENGTH];
	byte iv1[CryptoPP::AES::BLOCKSIZE];
	byte iv2[CryptoPP::AES::BLOCKSIZE];
	byte iv3[CryptoPP::AES::BLOCKSIZE];
	byte hash1[CryptoPP::SHA256::DIGESTSIZE];
	byte hash2[CryptoPP::SHA256::DIGESTSIZE];
	byte hash3[CryptoPP::SHA256::DIGESTSIZE];

	const int dataSize = 176;
	byte data1[dataSize];
	byte data2[dataSize];
	byte data3[dataSize];
	byte data_res1[dataSize];
	byte data_res2[dataSize];
	byte data_res3[dataSize];
	byte hash_res1[CryptoPP::SHA256::DIGESTSIZE];
	byte hash_res2[CryptoPP::SHA256::DIGESTSIZE];
	byte hash_res3[CryptoPP::SHA256::DIGESTSIZE];

	LoadIV_Data_Hash_From("1.saf", iv1, data1, dataSize, hash1);
	LoadIV_Data_Hash_From("2.saf", iv2, data2, dataSize, hash2);
	LoadIV_Data_Hash_From("3.saf", iv3, data3, dataSize, hash3);

	FILE* fdump;
//	errno_t errOpen = fopen_s(&fdump, "../SignAddress (2).DMP", "rb");
	errno_t errOpen = fopen_s(&fdump, "../allocs.dmp", "rb");
	size_t bytesToProcess = getFileSize(fdump);
	size_t pos = 0;
	int rowPrints = 0;
	if (fdump)
	{
		size_t readCount = 32;
		readCount = fread(key, 1, CryptoPP::AES::MAX_KEYLENGTH, fdump);
		while (readCount >= 1)
		{
			static size_t printedProgress = 0;
			size_t curProgress = pos * 100 / bytesToProcess;
			if (printedProgress < curProgress)
			{
				printedProgress = curProgress;
				printf("processed %zd %%     \r", curProgress);
			}

			Decrypt1Block_CBC_AES_256_NOAUTH(iv1, key, data1, dataSize, data_res1);
			Decrypt1Block_CBC_AES_256_NOAUTH(iv2, key, data2, dataSize, data_res2);

			if (memcmp(data_res1, data_res2, dataSize) == 0)
			{
				printf("Result block matches. key maybe found at %zd\n", pos);

				Decrypt1Block_CBC_AES_256_NOAUTH(iv3, key, data3, dataSize, data_res3);
				if (memcmp(data_res1, data_res3, dataSize) == 0)
				{
					printf("%d)Result block matches2. key maybe found at %zd\n", rowPrints++, pos);
					for (size_t ind = 0; ind < CryptoPP::AES::MAX_KEYLENGTH; ind++)
					{
						printf("%02x ", *(unsigned char*)&key[ind]);
					}
					printf("\n");
				}

				GetDataBlockHash(iv1, data_res1, dataSize, hash_res1);
				if (memcmp(hash1, hash_res1, CryptoPP::SHA256::DIGESTSIZE) == 0)
				{
					printf("Result block hash1 matches. key maybe found at %zd\n", pos);
					GetDataBlockHash(iv2, data_res2, dataSize, hash_res2);
					GetDataBlockHash(iv3, data_res3, dataSize, hash_res3);
					if (memcmp(hash2, hash_res2, CryptoPP::SHA256::DIGESTSIZE) == 0)
					{
						printf("Result block hash2 matches. key maybe found at %zd\n", pos);
					}
				}
			}


			pos++;
			memcpy(key, &key[1], CryptoPP::AES::MAX_KEYLENGTH - 1);
			readCount = fread(&key[31], 1, 1, fdump);
		}

		fclose(fdump);
	}
}