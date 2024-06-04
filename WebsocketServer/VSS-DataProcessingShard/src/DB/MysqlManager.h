#pragma once

#define MAX_DB_CONNECTIONS	20
#define DB_SLEEP_RETRY_FREE_CONN 1
#define MAX_URL_LEN	1024

#include <mutex>
#include <vector>

struct ModuleInstanceQueryData
{
	ModuleInstanceQueryData() {}; // no default constructor
	__int64 InstanceId;
	std::string ConnectionURL;
	int Status; // is it even worth fetching offline modules ?
};

struct AlertQueryData
{
	AlertQueryData() {}; // no default constructor
	__int64 AlertId;
	__int64 AlertDefinitionId;
	__int64 AlertTypeId;
	__int64 AlertStatusTypeId;
	__int64 LocationId;
	__int64 ModuleId;
	__int64 OrganizationId;
	__int64 UserId;
};

class VSSDBManager
{
public:
	inline static VSSDBManager& getInstance()
	{
		static VSSDBManager instance;
		return instance;
	}
	~VSSDBManager();
	void DestructorCheckMemLeaks();
	bool StartServer(size_t ConnCount, const char *host, const char *port, const char *user, const char *pasw, const char *db);
	bool StopServer();

	bool Query_SessionValid(__int64 SessionId, __int64 UserId, __int64 &out_salt);
	bool Query_DPSModules(__int64 DPS_Id, std::vector<ModuleInstanceQueryData> &out_res);
	bool Query_DPSAlerts(__int64 DPS_Id, std::vector<AlertQueryData> &out_res);
	bool Query_SoftwareAlerts(__int64 ModuleId, __int64 AlertTypeId, std::vector<DSModuleAlert>& out_res);
//	bool Query_ModuleInstanceInfo(__int64 ModuleId, __int64& out_LocationId);

	bool Create_AlertInstance(__int64 AlertDefinitionId, __int64 AlertTypeId, unsigned __int64 TrigerStamp, __int64 LocationId,
		__int64 ModuleId, __int64 OrganizationId, __int64 UserId, __int64 &out_AlertId);

	bool Update_AlertStatus(__int64 AlertId, __int64 StatusFlag);
	bool Update_ServerHealthReport(__int64 DPS_Id, __int64 activeThreads, float CPUUsage, float CPUUsagePeak, 
		__int64 memUsage, __int64 BPMIn, __int64 BPMOut, __int64 PPMIn, __int64 PPMOut,
		unsigned __int64 startupStamp, int PID, int wsClients);


	bool RunCacheQuery(const char* query, __int64 MaxAcceptedId, unsigned int** out_buff, __int64* out_maxId);
	bool RunCacheQuery1Val(const char* query, __int64 RowId, __int64* out_val);
private:
	VSSDBManager();
	VSSDBManager(const VSSDBManager&) = delete;
	VSSDBManager& operator=(const VSSDBManager&) = delete;

	// avoid corrupting the array of connections
	std::mutex m_ConListLock;

	// store connection related info
	struct ConnectionStatusHolder
	{
		ConnectionStatusHolder() { pDBConn = NULL; bIsBusy = false; }
		void Aquire() { bIsBusy = true; }
		void Release() { bIsBusy = false; }
		bool bIsBusy;
		void* pDBConn; // hide implementation of WebSocket Server
	};
	class ConnAutoRelease
	{
	public:
		ConnAutoRelease(ConnectionStatusHolder* pH) { ch = pH; }
		~ConnAutoRelease() { ch->Release(); }
		ConnectionStatusHolder* Get() { return ch; }
	private:
		ConnectionStatusHolder* ch;
	};
	std::vector<ConnectionStatusHolder> m_Connections;

	// return a non busy connection
	ConnectionStatusHolder* GetFreeConnection(bool bBlockUntilFree = true);
};

#define sDBManager VSSDBManager::getInstance()