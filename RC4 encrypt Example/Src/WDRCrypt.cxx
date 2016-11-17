#include "WDRCrypt.hxx"
#include <openssl/hmac.h>

//could be loaded / hardcoded... 
const	unsigned char Channel1HardcodedSalt[16] = { 0xC2, 0xB3, 0x72, 0x3C, 0xC6, 0xAE, 0xD9, 0xB5, 0x34, 0x3C, 0x53, 0xEE, 0x2F, 0x43, 0x67, 0xCE };
const	unsigned char Channel2HardcodedSalt[16] = { 0xCC, 0x98, 0xAE, 0x04, 0xE8, 0x97, 0xEA, 0xCA, 0x12, 0xDD, 0xC0, 0x93, 0x42, 0x91, 0x53, 0x57 };

void WDRCrypt::Init(const void *K, size_t KeyLen)
{
	unsigned char encryptHash[SHA_DIGEST_LENGTH];
	unsigned char decryptHash[SHA_DIGEST_LENGTH];
	unsigned int md_len;

	// insert salt when generating the RC4 key
	HMAC(EVP_sha1(), Channel1HardcodedSalt, 16, (unsigned char*)K, KeyLen, decryptHash, &md_len);
	if (md_len != SHA_DIGEST_LENGTH)
	{
		//report that we failed to initialize
		return;
	}

	// insert salt when generating the RC4 key
	HMAC(EVP_sha1(), Channel2HardcodedSalt, 16, (unsigned char*)K, KeyLen, encryptHash, &md_len);
	if (md_len != SHA_DIGEST_LENGTH)
	{
		//report that we failed to initialize
		return;
	}

	// initialize rc4 structs
	RC4_set_key(&m_CryptChannels[CRYPT_CHANNEL_1], SHA_DIGEST_LENGTH, decryptHash);
	RC4_set_key(&m_CryptChannels[CRYPT_CHANNEL_2], SHA_DIGEST_LENGTH, encryptHash);

	// RC4 has a weakness. We should throw away at least the first 256 bytes of encoded data
	// In our case the throw away depends on the encryption key. A bit of salt in the salt... 
	int ThrowAwaySize = (*(unsigned short*)&Channel1HardcodedSalt[0]) % 2048;
	if (ThrowAwaySize < 256)
		ThrowAwaySize += 256;
	unsigned char *pass = new unsigned char[ThrowAwaySize + 4];
	RC4(&m_CryptChannels[CRYPT_CHANNEL_1], ThrowAwaySize, pass, pass);
	RC4(&m_CryptChannels[CRYPT_CHANNEL_2], ThrowAwaySize, pass, pass);
	delete[] pass;

	//if we got here, we can assume all went well
	m_initialized = true;
}
