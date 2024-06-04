#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "ResourceManager/LogManager.h"
#include "VSSPacketDefines.h"
#include "Util/Allocator.h"

#ifdef BUILD_SERVER_PACKETS
	#include "Network/NetworkSession.h"
	#include "ResourceManager/DataSourceManager.h"
	#include "DB/MysqlManager.h"
	#include "Network/WSServer.h"
#endif

#ifdef BUILD_CLIENT_PACKETS
	#include "Web/WebsocketClient.h"
#endif

VSSPacketHandlerBase* sPacketHandlers[VSS_WS_Opcodes::VSSWSO_MAX];

AutoFreeBuffer::~AutoFreeBuffer()
{
	if (*DeallocThisBuffer != NULL && *DeallocThisBuffer != DeallocUnlessThisValue)
	{
		InternalFree(*DeallocThisBuffer);
	}
}

char *VSSPacketHandlerBase::Allocate(size_t cnt1)
{
	return (char*)InternalMalloc(GetSize(cnt1));
}

char *VSSPacketHandlerBase::Init(char* out_buff, size_t buffMaxSize, size_t cnt1)
{
	cnt1;
	char* ret = out_buff;
	size_t size = GetSize(cnt1);
	assert(size < buffMaxSize);
	VSS_N_PacketHeader* header;
	if (size >= buffMaxSize)
	{
		ret = (char*)InternalMalloc(size);
	}

	header = (VSS_N_PacketHeader*)ret;
	header->Opcode = m_dOpcode;
	header->Size = (unsigned short)size;

	return ret;
}

template <typename T>
class PH_C_InitSession : public VSSPacketHandlerBase
{
public:
	PH_C_InitSession(VSS_WS_Opcodes Opcode, const char* name):
		VSSPacketHandlerBase(Opcode, name){ m_dSize = sizeof(T); }
	size_t GetSize(size_t cnt1) { cnt1; return sizeof(VSS_N_PacketHeader) + sizeof(T); }
	T* GetPacketData(char* buff) { return (T*)(buff + sizeof(VSS_N_PacketHeader)); }
	const T* GetPacketData(const char* buff) { return (const T*)(buff + sizeof(VSS_N_PacketHeader)); }

	// Initialized by client
	char *Init(char* out_buff, size_t buffMaxSize, size_t cnt1)
	{
		out_buff; buffMaxSize; cnt1;
		char* ret = NULL;
#ifdef BUILD_CLIENT_PACKETS
		ret = VSSPacketHandlerBase::Init(out_buff, buffMaxSize, cnt1);

		T* pkt = GetPacketData(out_buff);
		pkt->Version = NetworkModuleBuiltInVersion;
#endif
		return ret;
	};

	// parsed by server
	bool ParserFunc(const char* buff, const size_t len, void* user_data1)
	{
		buff; len; user_data1;
		bool ErrorDetected = false;
#ifdef BUILD_SERVER_PACKETS
		ServerNetworkContext* session = typecheck_castL(ServerNetworkContext, user_data1);

		if (ErrorDetected == false && !VSSPacketHandlerBase::CheckValid((const VSS_N_PacketHeader *)buff, len))
		{
			AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourcePacketParser, 0, 0,
				"PacketParser:Integrity check failed for opcode %d", m_dOpcode);
			ErrorDetected = true;
		}

		const T* pkt = GetPacketData(buff);

		// we only accept connections with same version
		if (ErrorDetected == false && pkt->Version != NetworkModuleBuiltInVersion)
		{
			printf("Client - Server version mismatch %d - %d\n", pkt->Version, NetworkModuleBuiltInVersion);
			AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourcePacketParser, 0, 0,
				"PacketParser:Init handshake, Unexpected version %d, expected %d, for opcode %d", 
				pkt->Version, NetworkModuleBuiltInVersion, m_dOpcode);
			ErrorDetected = true;
		}

		// sanity checks to not waste time on bad data
		if (ErrorDetected == false && (pkt->SessionId == 0 || pkt->SessionSalt == 0 || pkt->UserId == 0))
		{
			AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourcePacketParser, 0, 0,
				"PacketParser:Handshake failed : SessionId %lld Salt %lld User Id %lld for opcode %d",
				pkt->SessionId, pkt->SessionSalt, pkt->UserId, m_dOpcode);
			ErrorDetected = true;
		}

		// check if provided values are valid
		__int64 salt;
		if (ErrorDetected == false && sDBManager.Query_SessionValid(pkt->SessionId, pkt->UserId, salt) == true)
		{
			session->NwSession->OnLoginSuccess(pkt->SessionId, pkt->UserId, salt);
			AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourcePacketParser, 0, 0,
				"PacketParser:User %d successfully logged in", pkt->UserId);
		}

		// reply to the client with login result
		GENERIC_PACKET_INIT(VSS_WS_Opcodes::VSSWSO_S_InitSession, VSS_N_StatusReply, 1);
		if (ErrorDetected == true)
		{
			pktOut->StatusCode = VSS_WS_StatusCodes::VSSWSS_ServerSessionInitFailed;
		}
		else
		{
			pktOut->StatusCode = VSS_WS_StatusCodes::VSSWSS_ServerSessionInitialized;
		}
		
		sWSServer.SendMsg(packetBuff, pktSize, session->NwSession->GetSessionId());
#endif
		return !ErrorDetected;
	};
};

template <typename T>
class PH_S_InitSession : public VSSPacketHandlerBase
{
public:
	PH_S_InitSession(VSS_WS_Opcodes Opcode, const char* name) :
		VSSPacketHandlerBase(Opcode, name) { m_dSize = sizeof(T); }
	size_t GetSize(size_t cnt1) { cnt1; return sizeof(VSS_N_PacketHeader) + sizeof(T); }
	T* GetPacketData(char* buff) { return (T*)(buff + sizeof(VSS_N_PacketHeader)); }
	const T* GetPacketData(const char* buff) { return (const T*)(buff + sizeof(VSS_N_PacketHeader)); }

	// Initialized by server
	char *Init(char* out_buff, size_t buffMaxSize, size_t cnt1)
	{
		out_buff; buffMaxSize; cnt1;
		char* ret = NULL;
#ifdef BUILD_SERVER_PACKETS
		ret = VSSPacketHandlerBase::Init(out_buff, buffMaxSize, cnt1);

		T* pkt = (T*)GetPacketData(out_buff);
		pkt->StatusCode = VSS_WS_StatusCodes::VSSWSS_UninitializedValue;
#endif
		return ret;
	};

	// parsed by client
	bool ParserFunc(const char* buff, const size_t len, void* user_data1)
	{
		buff; len; user_data1;
		bool ErrorDetected = false;
#ifdef BUILD_CLIENT_PACKETS
		if (ErrorDetected == false && !VSSPacketHandlerBase::CheckValid((const VSS_N_PacketHeader*)buff, len))
		{
			AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourcePacketParser, 0, 0,
				"PacketParser:Integrity check failed for opcode %d", m_dOpcode);
			ErrorDetected = true;
		}

		if (ErrorDetected == false)
		{
			const T* pkt = GetPacketData(buff);

			ClientNetworkContext* cctx = typecheck_castL(ClientNetworkContext, user_data1);
			cctx->wsClient->OnSessionInitReply(pkt->StatusCode);
		}
#endif
		return !ErrorDetected;
	};
};

template <typename T>
class PH_C_SubscribeToModule : public VSSPacketHandlerBase
{
public:
	PH_C_SubscribeToModule(VSS_WS_Opcodes Opcode, const char* name) :
		VSSPacketHandlerBase(Opcode, name) {m_dSize = sizeof(T);}
	size_t GetSize(size_t cnt1) { cnt1; return sizeof(VSS_N_PacketHeader) + sizeof(T) + (cnt1 - 1) * sizeof(T::ModuleSubscriptionDetail); }
	T* GetPacketData(char* buff) { return (T*)(buff + sizeof(VSS_N_PacketHeader)); }
	const T* GetPacketData(const char* buff) { return (const T*)(buff + sizeof(VSS_N_PacketHeader)); }

	// Initialized by client
	char* Init(char* out_buff, size_t buffMaxSize, size_t cnt1)
	{
		out_buff; buffMaxSize; cnt1;
		char* ret = NULL;
#ifdef BUILD_CLIENT_PACKETS
		ret = VSSPacketHandlerBase::Init(out_buff, buffMaxSize, cnt1);

		T* pkt = GetPacketData(out_buff);
		pkt->Counter = (unsigned int)cnt1;
#endif
		return ret;
	};

	bool CheckValid(const char* buff, const size_t len)
	{
		if (!VSSPacketHandlerBase::CheckValid((const VSS_N_PacketHeader*)buff, len))
		{
			return false;
		}
		T* pkt = GetPacketData(buff);
		size_t size = sizeof(VSS_N_PacketHeader) + sizeof(T) + pkt->Counter * sizeof(VSS_N_SubscribeModules::ModulesubscriptionDetail);
		return len == size;
	}

	// parsed by server
	bool ParserFunc(const char* buff, const size_t len, void* user_data1)
	{
		buff; len; user_data1;
		bool ErrorDetected = false;
#ifdef BUILD_SERVER_PACKETS
		ServerNetworkContext* session = typecheck_castL(ServerNetworkContext, user_data1);

		if (ErrorDetected == false && !VSSPacketHandlerBase::CheckValid((const VSS_N_PacketHeader*)buff, len))
		{
			AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourcePacketParser, 0, 0,
				"PacketParser:Integrity check failed for opcode %d", m_dOpcode);
			ErrorDetected = true;
		}

		const T* pkt = GetPacketData(buff);
		for (size_t i = 0; i < pkt->Counter; i++)
		{
			const VSS_N_SubscribeModules::ModuleSubscriptionDetail& sd = pkt->ModuleDetails[i];
			session->NwSession->OnSubscribeToModule(pkt->SubscribeType, sd);
		}
#endif
		return !ErrorDetected;
	}
};

template <typename T>
class PH_C_UnSubscribeFromModule : public VSSPacketHandlerBase
{
public:
	PH_C_UnSubscribeFromModule(VSS_WS_Opcodes Opcode, const char* name) :
		VSSPacketHandlerBase(Opcode, name) { m_dSize = sizeof(T); }
	size_t GetSize(size_t cnt1) { cnt1; return sizeof(VSS_N_PacketHeader) + sizeof(T) + (cnt1 - 1) * sizeof(T::ModuleIds); }
	T* GetPacketData(char* buff) { return (T*)(buff + sizeof(VSS_N_PacketHeader)); }
	const T* GetPacketData(const char* buff) { return (const T*)(buff + sizeof(VSS_N_PacketHeader)); }

	// Initialized by client
	char *Init(char* out_buff, size_t buffMaxSize, size_t cnt1)
	{
		out_buff; buffMaxSize; cnt1;
		char* ret = NULL;
#ifdef BUILD_CLIENT_PACKETS
		ret = VSSPacketHandlerBase::Init(out_buff, buffMaxSize, cnt1);

		T* pkt = GetPacketData(out_buff);
		pkt->Counter = (unsigned int)cnt1;
#endif
		return ret;
	};

	bool CheckValid(const char* buff, const size_t len)
	{
		if (!VSSPacketHandlerBase::CheckValid((const VSS_N_PacketHeader*)buff, len))
		{
			return false;
		}
		T* pkt = GetPacketData(buff);
		size_t size = sizeof(VSS_N_PacketHeader) + sizeof(T) + pkt->Counter * sizeof(VSS_N_SubscribeModules::ModuleIds);
		return len == size;
	}

	// parsed by server
	bool ParserFunc(const char* buff, const size_t len, void* user_data1)
	{
		buff; len; user_data1;
		bool ErrorDetected = false;
#ifdef BUILD_SERVER_PACKETS
		ServerNetworkContext* session = typecheck_castL(ServerNetworkContext, user_data1);

		if (ErrorDetected == false && !VSSPacketHandlerBase::CheckValid((const VSS_N_PacketHeader*)buff, len))
		{
			AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourcePacketParser, 0, 0,
				"PacketParser:Integrity check failed for opcode %d", m_dOpcode);
			ErrorDetected = true;
		}

		const T* pkt = GetPacketData(buff);
		for (size_t i = 0; i < pkt->Counter; i++)
		{
			session->NwSession->OnUnSubscribeFromModule(pkt->SubscribeType, pkt->ModuleIds[i]);
		}
#endif
		return !ErrorDetected;
	}
};

template <typename T>
class PH_S_ModuleDataStatus : public VSSPacketHandlerBase
{
public:
	PH_S_ModuleDataStatus(VSS_WS_Opcodes Opcode, const char* name) :
		VSSPacketHandlerBase(Opcode, name) { m_dSize = sizeof(T);	}
	size_t GetSize(size_t cnt1) { cnt1; return sizeof(VSS_N_PacketHeader) + sizeof(T) + (cnt1 - 1) * sizeof(T::ObjectStateDetails); }
	T* GetPacketData(char* buff) { return (T*)(buff + sizeof(VSS_N_PacketHeader)); }
	const T* GetPacketData(const char* buff) { return (const T*)(buff + sizeof(VSS_N_PacketHeader)); }

	// Initialized by server
	char *Init(char* out_buff, size_t buffMaxSize, size_t cnt1)
	{
		out_buff; buffMaxSize; cnt1;
		char* ret = NULL;
#ifdef BUILD_SERVER_PACKETS
		ret = VSSPacketHandlerBase::Init(out_buff, buffMaxSize, cnt1);

		T* pkt = GetPacketData(out_buff);
		pkt->Counter = (unsigned int)cnt1;
#endif
		return ret;
	};

	bool CheckValid(const char* buff, const size_t len)
	{
		if (!VSSPacketHandlerBase::CheckValid((const VSS_N_PacketHeader*)buff, len))
		{
			return false;
		}
		T* pkt = GetPacketData(buff);
		size_t size = sizeof(VSS_N_PacketHeader) + sizeof(T) + pkt->Counter * sizeof(VSS_N_ModuleObjectState::ObjectStateDetails);
		return len == size;
	}

	// parsed by client
	bool ParserFunc(const char* buff, const size_t len, void* user_data1)
	{
		buff; len; user_data1;
		bool ErrorDetected = false;
#ifdef BUILD_CLIENT_PACKETS
		if (ErrorDetected == false && !VSSPacketHandlerBase::CheckValid((const VSS_N_PacketHeader*)buff, len))
		{
			AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourcePacketParser, 0, 0,
				"PacketParser:Integrity check failed for opcode %d", m_dOpcode);
			ErrorDetected = true;
		}

		if (ErrorDetected == false)
		{
			const T* pkt = GetPacketData(buff);

			ClientNetworkContext* cctx = typecheck_castL(ClientNetworkContext, user_data1);
			cctx->wsClient->OnModuleDataPkt(pkt);
		}
#endif
		return !ErrorDetected;
	}
};

template <typename T>
class PH_S_ModuleAlertStatus : public VSSPacketHandlerBase
{
public:
	PH_S_ModuleAlertStatus(VSS_WS_Opcodes Opcode, const char* name) :
		VSSPacketHandlerBase(Opcode, name) {m_dSize = sizeof(T);}
	size_t GetSize(size_t cnt1) { cnt1; return sizeof(VSS_N_PacketHeader) + sizeof(T); }
	T* GetPacketData(char* buff) { return (T*)(buff + sizeof(VSS_N_PacketHeader)); }
	const T* GetPacketData(const char* buff) { return (const T*)(buff + sizeof(VSS_N_PacketHeader)); }

	// Initialized by server
	char* Init(char* out_buff, size_t buffMaxSize, size_t cnt1)
	{
		out_buff; buffMaxSize; cnt1;
		char* ret = NULL;
#ifdef BUILD_SERVER_PACKETS
		ret = VSSPacketHandlerBase::Init(out_buff, buffMaxSize, cnt1);
#endif
		return ret;
	};

	bool CheckValid(const char* buff, const size_t len)
	{
		if (!VSSPacketHandlerBase::CheckValid((const VSS_N_PacketHeader*)buff, len))
		{
			return false;
		}
		T* pkt = GetPacketData(buff);
		size_t size = sizeof(VSS_N_PacketHeader) + sizeof(T);
		return len == size;
	}

	// parsed by client
	bool ParserFunc(const char* buff, const size_t len, void* user_data1)
	{
		buff; len; user_data1;
		bool ErrorDetected = false;
#ifdef BUILD_CLIENT_PACKETS
		if (ErrorDetected == false && !VSSPacketHandlerBase::CheckValid((const VSS_N_PacketHeader*)buff, len))
		{
			AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourcePacketParser, 0, 0,
				"PacketParser:Integrity check failed for opcode %d", m_dOpcode);
			ErrorDetected = true;
		}

		if (ErrorDetected == false)
		{
			const T* pkt = GetPacketData(buff);

			ClientNetworkContext* cctx = typecheck_castL(ClientNetworkContext, user_data1);
			cctx->wsClient->OnModuleAlertPkt(pkt);
		}
#endif
		return !ErrorDetected;
	}
};

class PHInitializer
{
public:
	PHInitializer()
	{
		memset(sPacketHandlers, 0, sizeof(sPacketHandlers));

		sPacketHandlers[VSSWSO_C_InitSession] = new PH_C_InitSession<VSS_N_InitSession>(
			VSSWSO_C_InitSession, "C:Init Session");
		sPacketHandlers[VSSWSO_S_InitSession] = new PH_S_InitSession<VSS_N_StatusReply>(
			VSSWSO_S_InitSession, "S:Init Session");
		sPacketHandlers[VSSWSO_C_SubscribeModules] = new PH_C_SubscribeToModule<VSS_N_SubscribeModules>(
			VSSWSO_C_SubscribeModules, "C:Subscribe to Module");
		sPacketHandlers[VSSWSO_C_UnSubscribeModules] = new PH_C_UnSubscribeFromModule<VSS_N_UnSubscribeModules>(
			VSSWSO_C_UnSubscribeModules, "C:UnSubscribe to Module");
		sPacketHandlers[VSSWSO_S_ModuleDataStatus] = new PH_S_ModuleDataStatus<VSS_N_ModuleObjectState>(
			VSSWSO_S_ModuleDataStatus, "S:Module data update");
		sPacketHandlers[VSSWSO_S_ModuleAlertStatus] = new PH_S_ModuleAlertStatus<VSS_N_ModuleAlertState>(
			VSSWSO_S_ModuleAlertStatus, "S:Module data update");

		// make sure we initialized all handlers
		for (size_t i = 1; i < VSS_WS_Opcodes::VSSWSO_MAX; i++)
		{
			if (sPacketHandlers[i] == NULL)
			{
				AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourcePacketParser, 0, 0,
					"PacketParser:Packet handler %lld is not initialized !!", i);
			}
		}
	}
};

static PHInitializer sPHInit;