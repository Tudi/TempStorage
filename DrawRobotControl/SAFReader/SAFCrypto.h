#pragma once

void Decrypt1Block_CBC_AES_256_NOAUTH(const byte* iv, const byte* data, int dataLen, byte* out_data);
void GetDataBlockHash(byte* iv, byte* data, int dataSize, byte* hash);
int ReadGenericEncryptedBlock(FILE* f, size_t blockSize, byte* out_dec);
int ReadGenericBlock(FILE* f, size_t blockSize, byte* out_dec);