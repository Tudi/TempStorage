#pragma once

#define RESEND_HANDSHAKE_INTERVAL	5000
#define MAX_HANDSHAKE_TRIES			5
#define RECONNECT_COOLDOWN			500
#define MAX_RECONNECT_TIME			5*60*1000 // try for 5 minutes than give up

#include <set>
#include "Network/VSSPacketDefines.h"

// simple callback. No params
typedef void (*WS_SCB)(void *this_);

// Interface to the websocket library to hide it from the whole project
enum VSS_WS_SocketResults
{
	SR_NO_ERROR = 0,
	SR_DROPPED = 1,
};

class VSSWebSocketClient
{
public:
	VSSWebSocketClient();
	~VSSWebSocketClient();
	bool ConnectToServer(const char *Addr, unsigned short port);
	bool ConnectToServer(const char* ConnectionURL);
	bool Disconnect();
	bool IsConnected();
	VSS_WS_SocketResults Send(const char* data, size_t len, bool isBinary = true);
	void SetClientContext(void* ctx);
protected:
	std::string m_sConnectionUrl;
	void* m_pWebsocketClient;
};

// More specific WSClient : Handles packets
class VSSWebSocketClientEx : public VSSWebSocketClient
{
public:
	VSSWebSocketClientEx();
	// send init packet, handle init failures, periodic heartbeat ?
	void PeriodicUpdate();

	// after creating a WS connection, we should log in to the server
	void SendHandshakeToServer();
	// received as a reply after we sent the handshake
	void OnSessionInitReply(unsigned short sc);

	// when a new connection is opened, or the user buys a new module subscription
	// we will subscribe to the Alerts and maybe feed of the Module
	void SubscribeToModules(std::set<unsigned __int64> &ModuleIds, 
		VSS_N_SubscribeModules::SubscriptionType IdType);
	void UnSubscribeFromModules(std::set<unsigned __int64>& ModuleIds,
		VSS_N_SubscribeModules::SubscriptionType IdType);

	// Do something with this data
	void OnModuleDataPkt(const struct VSS_N_ModuleObjectState *mos);
	void OnModuleAlertPkt(const struct VSS_N_ModuleAlertState* mos);

	enum WSSessionInitState
	{
		InitNotSent = 0,
		InitWaitingConnect, // wait for WS library to do it's handshake
		InitSent,
		InitSuccess,
		InitFailed,
		InitDisconnected,
	};
	void SetSessionInitState(WSSessionInitState ns);
	// If failed to connect to the server, failed to handshake .. skip trying
	bool IsConsideredUnusable();
	bool CanSendPackets() {
		return (IsConsideredUnusable() == false) && (m_SessionInitState == WSSessionInitState::InitSuccess);
	}
protected:
	static void OnConnect(void* this_);
	static void OnDisconnect(void* this_);
	WSSessionInitState m_SessionInitState;
	unsigned __int64 m_InitSentStamp;
	unsigned __int64 m_NextReconnectStamp;
	unsigned __int64 m_ReconnectGiveUpStamp;
	__int64 m_ConnectRetries;
	__int64 m_HandshakeRetries;
	bool m_bShouldReconnect; // reconnection is not supported due to handshake + subscriptions + data rates
};