#pragma once

#include "Util/ObjDescriptor.h"

// built in network version. Based on this number we might allowe outdated clients or not when user tries to create a session
#define NetworkModuleBuiltInVersion	1
#define MaxPacketStackAlloc 8000 // some packets will be allocated and constructed on the stack

// this is all work in progress. Might scrape it all.
// this is data serialization and deserialization over network
// Some of the packets could use buffer pools

enum VSS_WS_Opcodes : unsigned char
{
	VSSWSO_C_InitSession = 1, // C-S : send session ID the server can use for session init. Version ...
	VSSWSO_S_InitSession = 2, // S-C : reply to session init attemp : success ?
	VSSWSO_C_SubscribeModules = 3, // C-S : subscribe to a list of Modules for which to receive status updates
	VSSWSO_C_UnSubscribeModules = 4, // C-S : subscribe to a list of Modules for which to receive status updates
	VSSWSO_S_ModuleDataStatus = 5, // S-C : Module sees X points / tags ...
	VSSWSO_S_ModuleAlertStatus = 6, // S-C : update alert status : created, sent emai, sent SMS ...
	VSSWSO_MAX,

	VSSWSO_S_Disconnect = 7, // S-C : force client to drop this connection and fetch a new DPS list
};

// values are given so we do not make the mistake of adding inbetween values
enum VSS_WS_StatusCodes : unsigned char
{
	VSSWSS_UninitializedValue = 0,
	VSSWSS_ServerSessionInitialized = 1,
	VSSWSS_ServerSessionInitFailed = 2,
	VSSWSS_MaxValue
};

// this is required so cross project will produce the same network packets !
#pragma pack(push,1)

typedef struct VSS_N_PacketHeader
{
	unsigned short Size; // should confirm that the whole packet was received as expected
	VSS_WS_Opcodes Opcode;
}VSS_N_PacketHeader;

typedef struct VSS_N_InitSession
{
	unsigned short Version; // this is the modulo of the actual version number used when built
	__int64 UserId;	// needed to obtain DB session
	__int64 SessionId;	// needed to obtain DB session
	__int64 SessionSalt; // used to make API calls to backend
}VSS_N_InitSession;

// Action result reply
typedef struct VSS_N_StatusReply
{
	VSS_WS_StatusCodes StatusCode;
}VSS_N_StatusReply;

typedef struct VSS_N_SubscribeModules
{
	enum class SubscriptionType : int
	{
		SubscribeUninitialized = 0,
		SubscribeAlert = 1,
		SubscribeFeed = 2,
		UnsubscribeAlert = 3,
		UnsubscribeFeed = 4,
	};
	typedef struct ModuleSubscriptionDetail
	{
		__int64 ModuleId;
//		__int64 DataRate; // bytes / sec
	}ModuleSubscriptionDetail;
	SubscriptionType SubscribeType;
	unsigned int Counter; // Size of the list
	ModuleSubscriptionDetail ModuleDetails[1]; // placeholder for N Module IDs
}VSS_N_SubscribeModules;

typedef struct VSS_N_UnSubscribeModules
{
	VSS_N_SubscribeModules::SubscriptionType SubscribeType;
	unsigned int Counter; // Size of the list
	__int64 ModuleIds[1]; // placeholder for N Module IDs
}VSS_N_UnSubscribeModules;

typedef struct VSS_N_ModuleObjectState
{
	typedef struct ObjectStateDetails
	{
		unsigned short id; // until object tracking is implemented IDs will rotate
		float x, y; // absolute coordinates
		//		char tags[MAX_TAG_STRING_LENGTH]; // todo : might create separate packet or something for this
	}ObjectStateDetails;
	__int64 ModuleID; // This module got some new data
	unsigned __int64 Timestamp; // when data was captured. UDP data may come out of order
	unsigned int Counter; // Size of the list
	ObjectStateDetails ObjectStates[1];
}VSS_N_ModuleObjectState;

typedef struct VSS_N_ModuleAlertState
{
	enum class AlertTypes : unsigned char
	{
		MultiPersonDetected = 1,
		FastApproachingPerson = 2,
		DangerousObject = 3,
		PersonInRange = 4,
		PersonCaution = 5,
		PersonDanger = 6,
	};
	enum class AlertStateFlags : unsigned char
	{
		None = 0,
		Triggered = 1 << 0,
		EmailSent = 1 << 1,
		SMSSent = 1 << 2,
		EmailPending = 1 << 3, 
		SMSPending = 1 << 4,
		Confired = 1 << 5,
		ASF_EmailJobNotAvailable = EmailSent | EmailPending,
		ASF_SMSJobNotAvailable = SMSSent | SMSPending,
		ASF_HAS_ALL_JOBS_DONE = Triggered | SMSSent | EmailSent,
		ASF_HAS_PENDING_JOBS = SMSPending | EmailPending,
	};
	__int64 AlertId; // will be obtained by creating a DB row
	AlertTypes AlertType; // Multi person detected ? Fast approach ? dangerous object ?
	unsigned __int64 TriggerStamp; 
	AlertStateFlags StateFlags; // created, sent SMS, send Email, Confirmed
	__int64 LocationId; // will be fetched from DB ( or cache ) once an alert arrives from a Module
	__int64 ModuleID; // This module got some new data
}VSS_N_ModuleAlertState;

#pragma pack(pop)

class ClientNetworkContext
{
public:
	REFLECT_TYPE(ClientNetworkContext);
	ClientNetworkContext(class VSSWebSocketClientEx* wsClient_)
	{
		wsClient = wsClient_;
		InitTypeInfo();
	}
	class VSSWebSocketClientEx* wsClient;
};

class ServerNetworkContext
{
public:
	REFLECT_TYPE(ServerNetworkContext);
	ServerNetworkContext(class NetworkSession* NwSession_)
	{
		NwSession = NwSession_;
		InitTypeInfo();
	}
	class NetworkSession* NwSession;
};


#define GENERIC_PACKET_INIT(Opcode, PktType, Cnt) VSSPacketHandlerBase* pktHndl = sPacketHandlers[Opcode]; \
	char packetBuffStackAlloc[MaxPacketStackAlloc]; \
	char *packetBuff = packetBuffStackAlloc; \
	AutoFreeBuffer autoFreeeDynamicAlloc(&packetBuff, packetBuffStackAlloc); \
	size_t pktSize = pktHndl->GetSize(Cnt); \
	assert(MaxPacketStackAlloc > pktSize); \
	packetBuff = pktHndl->Init(packetBuffStackAlloc, sizeof(packetBuffStackAlloc), Cnt); \
	PktType* pktOut = (PktType*)pktHndl->GetPacketData(packetBuff); \
	assert(sizeof(PktType) == pktHndl->GetTSize());

class AutoFreeBuffer
{
public:
	AutoFreeBuffer(char** checkThisVar, char* deallocUnlessThisValue) {
		DeallocThisBuffer = checkThisVar;
		DeallocUnlessThisValue = deallocUnlessThisValue;
	}
	~AutoFreeBuffer();
private:
	char** DeallocThisBuffer;
	char* DeallocUnlessThisValue;
};

class NetworkSession;
class VSSPacketHandlerBase
{
public:
	VSSPacketHandlerBase(VSS_WS_Opcodes Opcode, const char * name):m_dOpcode(Opcode),
		m_sName(name),
		m_dSize(0){}
	virtual size_t GetSize(size_t cnt1 = 0) { cnt1; return 0; }
	virtual char* Allocate(size_t cnt1 = 0);
	// init buffer for a packet default opcode and size
	virtual char *Init(char** out_buff, size_t cnt1 = 0) 
	{ 
		*out_buff = Allocate(cnt1);
		Init(*out_buff, cnt1);
		return *out_buff;
	}
	virtual char *Init(char* out_buff, size_t buffMaxSize, size_t cnt1 = 0);

	virtual bool CheckValid(const VSS_N_PacketHeader* header, const size_t len) const
	{
		return (header->Opcode == m_dOpcode) && (header->Size == len) && (len >= m_dSize);
	}

	const char* GetName() const { return m_sName; };
	virtual bool ParserFunc(const char* buff, const size_t len, void* user_data1) { buff; len; user_data1; return false; };
	void* GetPacketData(char* buff) { return buff + sizeof(VSS_N_PacketHeader); }
	const void* GetPacketData(const char* buff) { return buff + sizeof(VSS_N_PacketHeader); }
	size_t GetTSize() { return m_dSize; }
protected:
	VSS_WS_Opcodes m_dOpcode;
	size_t m_dSize; // size of data structure without header or repeating blocks
	const char* m_sName;
};

extern VSSPacketHandlerBase *sPacketHandlers[VSS_WS_Opcodes::VSSWSO_MAX];
