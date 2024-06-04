#pragma once

// This is just a wrapper over the actual implementation of a Web Socket Server
// It exists to hide the implementation of the server, so it could be swapped to something else if required
// Right now this exists in a singleton. Could be dynamic also

class VSSWSServer
{
public:
	inline static VSSWSServer& getInstance()
	{
		static VSSWSServer instance;
		return instance;
	}
	bool StartServer(unsigned short port);
	bool StopServer();
	bool SendMsg(const char* data, const size_t len, const __int64 ClientOnly = 0);
	size_t GetClientConnectedCount();
private:
	VSSWSServer();
	VSSWSServer(const VSSWSServer&) = delete;
	VSSWSServer& operator=(const VSSWSServer&) = delete;

	void* m_pWSServer; // hide implementation of WebSocket Server
};

#define sWSServer VSSWSServer::getInstance()
