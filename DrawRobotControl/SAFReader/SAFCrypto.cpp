#include "stdafx.h"

const unsigned char* GetSAFEncryptionKey()
{
	static const unsigned char gEncryptionKey[32] = { 0xAE,0xEC,0x5E,0x77,0x92,0xFF,0x0D,0x37,0xE0,0xA3,0x82,0x77,0x20,0x5A,0xC4,0xCC,0xE7,0x83,0xD8,0xE9,0x49,0x8D,0x87,0x38,0x00,0xB5,0x9B,0xE9,0x5C,0x1B,0xE5,0x7F };
	return gEncryptionKey;
}

void Decrypt1Block_CBC_AES_256_NOAUTH(const byte* iv, const byte* data, size_t dataLen, byte* out_data)
{
	// Create the decryption object
	CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption decryptor;
	decryptor.SetKeyWithIV(GetSAFEncryptionKey(), CryptoPP::AES::MAX_KEYLENGTH, iv);
	decryptor.ProcessData(out_data, data, dataLen);
}

void GetDataBlockHash(byte* iv, byte* data, size_t dataSize, byte* hash)
{
	CryptoPP::HMAC<CryptoPP::SHA256> hmac(GetSAFEncryptionKey(), CryptoPP::AES::MAX_KEYLENGTH);
	hmac.Update(iv, CryptoPP::AES::BLOCKSIZE); // Hash the first block of data
	hmac.Update(data, dataSize); // Hash the second block of data
	hmac.Final(hash); // Finalize the hash
}

int ReadGenericEncryptedBlock(FILE* f, size_t blockSize, byte* out_dec)
{
	byte iv[CryptoPP::AES::BLOCKSIZE];
	byte hash[CryptoPP::SHA256::DIGESTSIZE];
	byte hash_check[CryptoPP::SHA256::DIGESTSIZE];

	size_t readCount;
	readCount = fread(iv, 1, CryptoPP::AES::BLOCKSIZE, f);
	if (readCount != CryptoPP::AES::BLOCKSIZE)
	{
		printf("ERROR : File does not contain enough data to read IV\n");
		return -1;
	}

	byte* tempEncData = (byte*)malloc(blockSize);
	if (tempEncData == NULL)
	{
		printf("ERROR : Failed to allocate %zd bytes of data\n", blockSize);
		return -2;
	}

	readCount = fread(tempEncData, 1, blockSize, f);
	if (readCount != blockSize)
	{
		printf("File does not contain enough data to read block data\n");
		free(tempEncData);
		return -2;
	}

	readCount = fread(hash, 1, CryptoPP::SHA256::DIGESTSIZE, f);
	if (readCount != CryptoPP::SHA256::DIGESTSIZE)
	{
		printf("File does not contain enough data to read hash data\n");
		free(tempEncData);
		return -2;
	}

	Decrypt1Block_CBC_AES_256_NOAUTH(iv, (byte*)tempEncData, (int)blockSize, (byte*)out_dec);

	GetDataBlockHash(iv, (byte*)tempEncData, (int)blockSize, hash_check);
	if (memcmp(hash, hash_check, CryptoPP::SHA256::DIGESTSIZE) != 0)
	{
		printf("Data authentication failed for block sized %zd\n", blockSize);
	}

	free(tempEncData);

	return 0;
}

int ReadGenericBlock(FILE* f, size_t blockSize, byte* out_dec)
{
	byte hash[CryptoPP::SHA256::DIGESTSIZE];
	byte hash_check[CryptoPP::SHA256::DIGESTSIZE];

	size_t readCount;

	readCount = fread(out_dec, 1, blockSize, f);
	if (readCount != blockSize)
	{
		printf("File does not contain enough data to read block data\n");
		return -2;
	}

	readCount = fread(hash, 1, CryptoPP::SHA256::DIGESTSIZE, f);
	if (readCount != CryptoPP::SHA256::DIGESTSIZE)
	{
		printf("File does not contain enough data to read hash data\n");
		return -2;
	}

	CryptoPP::SHA256 sha256;
	sha256.Update((byte*)out_dec, blockSize); // Hash the first block of data
	sha256.Final(hash_check); // Finalize the hash 

	if (memcmp(hash, hash_check, CryptoPP::SHA256::DIGESTSIZE) != 0)
	{
		printf("Data authentication failed for block sized %zd\n", blockSize);
	}

	return 0;
}

int GetIVSize()
{
	return CryptoPP::AES::BLOCKSIZE;
}

int GetHashSize()
{
	return CryptoPP::SHA256::DIGESTSIZE;
}

void WriteGenericEncryptedBlock(FILE* f, byte* buff, size_t bufferSize)
{
	byte iv[CryptoPP::AES::BLOCKSIZE];
	AutoSeededRandomPool rng;
	rng.GenerateBlock(iv, sizeof(iv));

	CryptoPP::AES::Encryption aesEncryption(GetSAFEncryptionKey(), CryptoPP::AES::MAX_KEYLENGTH);
	CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, iv);

	byte* ciphertext = new byte[bufferSize];
	CryptoPP::ArraySink arraySink(ciphertext, bufferSize); // write to a byte buffer

	CryptoPP::StreamTransformationFilter filter(cbcEncryption);
	filter.Attach(new CryptoPP::Redirector(arraySink)); // attach the ArraySink to write to the buffer
	filter.Put(buff, bufferSize);
	filter.MessageEnd();

	byte hash[CryptoPP::SHA256::DIGESTSIZE];
	GetDataBlockHash(iv, ciphertext, bufferSize, hash);

	fwrite(iv, 1, sizeof(iv), f);
	fwrite(ciphertext, 1, bufferSize, f);
	fwrite(hash, 1, sizeof(hash), f);

	delete[] ciphertext;
}

void WriteGenericBlock(FILE* f, byte* buff, size_t bufferSize)
{
	byte hash[CryptoPP::SHA256::DIGESTSIZE];
	CryptoPP::SHA256 sha256;
	sha256.Update(buff, bufferSize); // Hash the first block of data
	sha256.Final(hash); // Finalize the hash 

	fwrite(buff, 1, bufferSize, f);
	fwrite(hash, 1, sizeof(hash), f);
}