#pragma once

int EncryptBufferXORKey(unsigned char *buf, int BufLen, const unsigned char *Key, int KeyLen);
int EncryptBufferXORKeyRotate(unsigned char *buf, int BufLen, int XORKey);

//int EncryptWithFingerprint(const char *Filename, unsigned int Salt, unsigned char *buf, int BufLen);
//int DecryptWithFingerprint(const char *Filename, unsigned int Salt, unsigned char *buf, int BufLen);

int EncryptWithFingerprintContent(unsigned char *EncryptKey, int KeyLen, unsigned int Salt, unsigned char *buf, int BufLen);