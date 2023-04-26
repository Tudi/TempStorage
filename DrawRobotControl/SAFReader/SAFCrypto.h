#pragma once

void Decrypt1Block_CBC_AES_256_NOAUTH(const byte* iv, const byte* data, size_t dataLen, byte* out_data);
void GetDataBlockHash(byte* iv, byte* data, size_t dataSize, byte* hash);
int ReadGenericEncryptedBlock(FILE* f, size_t blockSize, byte* out_dec);
int ReadGenericBlock(FILE* f, size_t blockSize, byte* out_dec);
int GetIVSize();
int GetHashSize();
void WriteGenericEncryptedBlock(FILE* f, byte* buff, size_t bufferSize);
void WriteGenericBlock(FILE* f, byte* buff, size_t bufferSize);
