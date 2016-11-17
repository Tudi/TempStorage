#ifndef _WDRCRYPT_H
#define _WDRCRYPT_H

#include <openssl/sha.h>
#include <openssl/rc4.h>

// multiple channels to be able to support multiple data streams at once with a single crypt
// by default there are 2 crypt sessions. One is used when sending data from server, other one is when receiving data by the server
enum CRYPTED_COMM_CHANNELS
{
	CRYPT_CHANNEL_1,
	CRYPT_CHANNEL_2,
	CRYPT_CHANNEL_COUNT,
	CRYPT_CHANNEL_SERVER_SEND = CRYPT_CHANNEL_1,
	CRYPT_CHANNEL_CLIENT_RECV = CRYPT_CHANNEL_1,
	CRYPT_CHANNEL_SERVER_RECV = CRYPT_CHANNEL_2,
	CRYPT_CHANNEL_CLIENT_SEND = CRYPT_CHANNEL_2,
};

class WDRCrypt
{
public:

	// without initialization the input and the output will be the same = No encryption
	WDRCrypt()
	{
		m_initialized = false;
	}

	// mix the static and the dynamic salts and initialize RC4 state
	void Init(const void *K, size_t KeyLen);

	//crypt/decrypt communication
	void Process(const void* pInData, size_t len, void* pOutData, int Channel)
	{
		if (!m_initialized)
			return;
		if (Channel >= CRYPT_CHANNEL_COUNT)
			return;
		RC4(&m_CryptChannels[Channel], (unsigned long)len, (const unsigned char*)pInData, (unsigned char*)pOutData);
	}
private:
	RC4_KEY m_CryptChannels[CRYPT_CHANNEL_COUNT];
	bool	m_initialized;
};

#endif
