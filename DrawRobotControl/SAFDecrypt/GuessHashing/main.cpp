#include <iostream>
#include <iomanip>
#include <sha.h>
#include <md4.h>
#include <md5.h>
#include <sha3.h>
#include <openssl/sha.h>

void GetFileContent(unsigned char *bf1, int buffSize)
{
	FILE* f1;
	errno_t open_err;
	open_err = fopen_s(&f1, "./Very Small Line.saf", "rb");
	if (f1 == NULL)
	{
		printf("Failed to open input file !\n");
		return;
	}

	fread(bf1, 1, buffSize, f1);

	fclose(f1);
}

enum TryHashingAlgos
{
	sha1 = 1,
	sha2,
	sha224,
	sha256,
	sha384,
	sha512,
	MD4,
	MD5,
	sha3_224,
	sha3_256,
	sha3_384,
	sha3_512,
};

void TryHash(unsigned char* bf1, int startAt, int dataBlockSize, int algo)
{
	unsigned char *input = &bf1[startAt];
	unsigned char hash[CryptoPP::SHA512::DIGESTSIZE];
	int outputHashSize = 0;

	if (algo == sha1)
	{
		CryptoPP::SHA1 sha1;
		sha1.CalculateDigest(hash, input, dataBlockSize);
		outputHashSize = sha1.DIGESTSIZE;
	}
	else if (algo == sha224)
	{
		CryptoPP::SHA224 sha1;
		sha1.CalculateDigest(hash, input, dataBlockSize);
		outputHashSize = sha1.DIGESTSIZE;
	}
	else if (algo == sha256)
	{
		CryptoPP::SHA256 sha1;
		sha1.CalculateDigest(hash, input, dataBlockSize);
		outputHashSize = sha1.DIGESTSIZE;
		// same thing but from openssl
		unsigned char hash2[CryptoPP::SHA512::DIGESTSIZE];
		SHA256(input, dataBlockSize, hash2);
		if (memcmp(hash, hash2, outputHashSize) != 0)
		{
			printf("Openssl Has256 produced different result than CryptoPP hash\n");
		}
	}
	else if (algo == sha384)
	{
		CryptoPP::SHA384 sha1;
		sha1.CalculateDigest(hash, input, dataBlockSize);
		outputHashSize = sha1.DIGESTSIZE;
	}
	else if (algo == sha512)
	{
		CryptoPP::SHA512 sha1;
		sha1.CalculateDigest(hash, input, dataBlockSize);
		outputHashSize = sha1.DIGESTSIZE;
	}
	else if (algo == MD4)
	{
		CryptoPP::MD4 sha1;
		sha1.CalculateDigest(hash, input, dataBlockSize);
		outputHashSize = sha1.DIGESTSIZE;
	}
	else if (algo == MD5)
	{
		CryptoPP::MD5 sha1;
		sha1.CalculateDigest(hash, input, dataBlockSize);
		outputHashSize = sha1.DIGESTSIZE;
	}
	else if (algo == sha3_224)
	{
		CryptoPP::SHA3_224 sha1;
		sha1.CalculateDigest(hash, input, dataBlockSize);
		outputHashSize = sha1.DIGESTSIZE;
	}
	else if (algo == sha3_256)
	{
		CryptoPP::SHA3_256 sha1;
		sha1.CalculateDigest(hash, input, dataBlockSize);
		outputHashSize = sha1.DIGESTSIZE;
	}
	else if (algo == sha3_384)
	{
		CryptoPP::SHA3_384 sha1;
		sha1.CalculateDigest(hash, input, dataBlockSize);
		outputHashSize = sha1.DIGESTSIZE;
	}
	else if (algo == sha3_512)
	{
		CryptoPP::SHA3_512 sha1;
		sha1.CalculateDigest(hash, input, dataBlockSize);
		outputHashSize = sha1.DIGESTSIZE;
	}
	else
	{
		printf("Unknown hashing algo\n");
		return;
	}

	if (outputHashSize < 16)
	{
		printf("Unexpectedly small hash size\n");
	}

	//check if hash matches the one found in the file
	const int checkBytesMatch = 32;
	for (int badAllignment = -600; badAllignment <= 600; badAllignment++)
	{
		int isMatch = 1;
		for (int i = startAt + dataBlockSize, hashInd = 0; i < startAt + dataBlockSize + checkBytesMatch; i++, hashInd++)
		{
			if (i + badAllignment < 0)
			{
				isMatch = 0;
				break;
			}
			if (bf1[i + badAllignment] != hash[hashInd])
			{
				isMatch = 0;
				break;
			}
			else
			{
				isMatch = isMatch;
			}
		}
		if (isMatch)
		{
			printf("1 StartAt %X-%X. DB size %d, Found a hash result match using algo %d. Bytes match %d\n", startAt, startAt + dataBlockSize, dataBlockSize, algo, checkBytesMatch);
		}
		// try reverse hash ?
		isMatch = 1;
		for (int i = startAt + dataBlockSize, hashInd = 0; i < startAt + dataBlockSize + checkBytesMatch; i++, hashInd++)
		{
			if (i + badAllignment < 0)
			{
				isMatch = 0;
				break;
			}
			if (bf1[i + badAllignment] != hash[outputHashSize - hashInd])
			{
				isMatch = 0;
				break;
			}
			else
			{
				isMatch = isMatch;
			}
		}
		if (isMatch)
		{
			printf("2 StartAt %X-%X. DB size %d, Found a hash result match using algo %d. Bytes match %d\n", startAt, startAt + dataBlockSize, dataBlockSize, algo, checkBytesMatch);
		}
	}
}

int main()
{
	unsigned char bf1[1000];
	GetFileContent(bf1, sizeof(bf1));

//	for (int dbSize = 120 - 32; dbSize <= 120 + 32; dbSize++)
	for (int dbSize = 32; dbSize <= 250; dbSize++)
	{
//		for (int i = 0xE8 - 0xE0; i <= 0xE8 + 32; i++)
		for (int i = 0x08; i <= 0x250; i++)
		{
//			printf("Try : startat %x , DBsize %d\n", i, dbSize);
			TryHash(bf1, i, dbSize, sha1);
			TryHash(bf1, i, dbSize, sha224);
			TryHash(bf1, i, dbSize, sha256);
			TryHash(bf1, i, dbSize, sha384);
			TryHash(bf1, i, dbSize, sha512);
			TryHash(bf1, i, dbSize, MD4);
			TryHash(bf1, i, dbSize, MD5);
			TryHash(bf1, i, dbSize, sha3_224);
			TryHash(bf1, i, dbSize, sha3_256);
			TryHash(bf1, i, dbSize, sha3_384);
			TryHash(bf1, i, dbSize, sha3_512);
		}
	}

    return 0;
}
