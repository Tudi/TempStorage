#include <iostream>
#include "ResourceManager/DataSourceManager.h"
#include "ResourceManager/AlertLifeCycleManager.h"
#include "MysqlManager.h"
#include "LogManager.h"
#include "Allocator.h"
#include "Network/VSSPacketDefines.h"

#include "soci.h"
#include "mysql/soci-mysql.h"

#define GENERIC_QUERY_FUNCTION_START 	ConnAutoRelease car(GetFreeConnection()); \
										if (car.Get() == NULL) \
										{ \
											return false; \
										} \
										soci::session* sql = (soci::session*)car.Get()->pDBConn; \
										try {

#define GENERIC_QUERY_FUNCTION_END		} \
										catch (const soci::mysql_soci_error& e) \
										{ \
											AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceDatabaseManager, 0, 0, \
												"MysqlManager:Failed to execute %s. Error %s", sql->get_last_query().c_str(), e.what()); \
										}\
										catch (const std::exception& e) \
										{ \
											AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceDatabaseManager, 0, 0, \
												"MysqlManager:Failed to execute %s. Error %s", sql->get_last_query().c_str(), e.what()); \
										} \
										return false;


VSSDBManager::VSSDBManager()
{
}

VSSDBManager::~VSSDBManager()
{
	DestructorCheckMemLeaks();
}

void VSSDBManager::DestructorCheckMemLeaks()
{
	StopServer();
}

bool VSSDBManager::StartServer(size_t ConnCount, const char* host, const char* port, const char* user, const char* pasw, const char* db)
{
	std::unique_lock<std::mutex> lock(m_ConListLock);
	if (ConnCount > MAX_DB_CONNECTIONS)
	{
		ConnCount = MAX_DB_CONNECTIONS;
	}
	if (ConnCount <= 0)
	{
		ConnCount = 1;
	}
	m_Connections.reserve(ConnCount);

	std::string connectString;
	connectString = connectString + " host=" + host;
	connectString = connectString + " port=" + port;
	connectString = connectString + " user=" + user;
	connectString = connectString + " password=" + pasw;
	connectString = connectString + " dbname=" + db;

	try {
		for (size_t i = 0; i < ConnCount; i++)
		{
			soci::session* sql;
			InternalNew(sql, soci::session, soci::mysql, connectString);

			AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceDatabaseManager, 0, 0,
				"MysqlManager:Added connection %lld, to %s, type %s", 
				i, connectString.c_str(), sql->get_backend_name().c_str());

			m_Connections.emplace_back();
			VSSDBManager::ConnectionStatusHolder& ch = m_Connections.back();

			ch.pDBConn = sql;
		}
		return true;
	}
	catch (soci::soci_error const& e)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityCritical, LogSourceGroups::LogSourceDatabaseManager, 0, 0,
			"MysqlManager:Failed to connect to %s. Error %s", connectString.c_str(), e.what());
	}
	catch (std::runtime_error const& e)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityCritical, LogSourceGroups::LogSourceDatabaseManager, 0, 0,
			"MysqlManager:Unexpected standard exception occurred while connecting to %s. Error %s", 
			connectString.c_str(), e.what());
	}
	catch (...)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityCritical, LogSourceGroups::LogSourceDatabaseManager, 0, 0,
			"MysqlManager:Unexpected unknown exception occurred while connecting to %s.",
			connectString.c_str());
	}

	return true;
}

VSSDBManager::ConnectionStatusHolder* VSSDBManager::GetFreeConnection(bool bBlockUntilFree)
{
	std::unique_lock<std::mutex> lock(m_ConListLock);
	if (m_Connections.empty())
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceDatabaseManager, 0, 0,
			"MysqlManager:Have 0 connections !");
		return NULL;
	}

	size_t retryCount = 0;
	do {
		for (auto itr = m_Connections.begin(); itr != m_Connections.end(); itr++)
		{
			if (itr->bIsBusy == false)
			{
				itr->Aquire();
				return &(*itr);
			}
		}

		retryCount++;
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceDatabaseManager, 0, 0,
			"MysqlManager:Failed to aquire free connection. Retry %d", retryCount);
		Sleep(DB_SLEEP_RETRY_FREE_CONN);

	} while (bBlockUntilFree == true);

	return NULL;
}

bool VSSDBManager::StopServer()
{
	bool FoundBusyConnections = true;
	while (FoundBusyConnections == true)
	{
		FoundBusyConnections = false;
		for (auto itr = m_Connections.begin(); itr != m_Connections.end(); itr++)
		{
			if (itr->bIsBusy == true)
			{
				FoundBusyConnections = true;
				continue;
			}
			if (itr->pDBConn != NULL)
			{
				soci::session* sql = (soci::session*)itr->pDBConn;
				itr->pDBConn = NULL;
				sql->close();
				InternalDelete(sql);
			}
			if (FoundBusyConnections == true)
			{
				Sleep(DB_SLEEP_RETRY_FREE_CONN);
			}
		}
	}

	return true;
}

bool VSSDBManager::Query_SessionValid(__int64 SessionId, __int64 UserId, __int64& out_salt)
{
	out_salt = -1;

	GENERIC_QUERY_FUNCTION_START;

	soci::rowset<soci::row> result = (sql->prepare << 
		"SELECT SessionSalt FROM usersessions where "
		" SessionID=" << SessionId <<
		" AND UserID=" << UserId <<
		" AND (MFAType = 0 or MFAType = 1)" <<
		" limit 0,1"
		);

	for (auto it = result.begin(); it != result.end(); ++it) 
	{
		out_salt = it->get<int>(0);
	}

	if (out_salt != -1)
	{
		return true;
	}

	GENERIC_QUERY_FUNCTION_END;
}

bool VSSDBManager::Query_DPSModules(__int64 DPS_Id, std::vector<ModuleInstanceQueryData>& out_res)
{
	GENERIC_QUERY_FUNCTION_START;

	soci::rowset<soci::row> result = (sql->prepare <<
		"SELECT MI.ModuleInstanceID, MI.ModuleIP, MI.ModuleStatusID FROM ModuleInstances MI " <<
		" INNER JOIN DPS_Modules DPSM ON DPSM.ModuleInstanceID = MI.ModuleInstanceID" <<
		" where "
		" MI.IsDeleted=0" <<
		" AND DPSM.DPSID=" << DPS_Id <<
		"");

	out_res.reserve(10); // oh wow, there is no way to guess how many rows got returned. This is so random
	for (const auto& it : result)
	{
		ModuleInstanceQueryData& row = out_res.emplace_back();
		row.InstanceId = it.get<int>(0);
		soci::indicator ind = it.get_indicator(1);
		if (ind == soci::i_ok)
		{
			row.ConnectionURL = it.get<std::string>(1);
		}
		else
		{
			row.ConnectionURL = "";
		}
		row.Status = it.get<int>(2);
	}
	return true;

	GENERIC_QUERY_FUNCTION_END;
}

/*
bool VSSDBManager::Query_ModuleInstanceInfo(__int64 ModuleId, __int64& out_LocationId)
{
	GENERIC_QUERY_FUNCTION_START;

	soci::rowset<soci::row> result = (sql->prepare <<
		"SELECT ModuleLocationID FROM ModuleInstances " <<
		" where "
		" ModuleInstanceID=" << ModuleId
		);

	auto itr = result.begin();
	if ( itr != result.end())
	{
		out_LocationId = itr->get<int>(0);
	}

	return true;

	GENERIC_QUERY_FUNCTION_END;
} */

bool VSSDBManager::Create_AlertInstance(__int64 AlertDefinitionId, __int64 AlertTypeId, 
	unsigned __int64 TrigerStamp, __int64 LocationId, __int64 ModuleId, __int64 OrganizationId, 
	__int64 UserId, __int64& out_AlertId)
{
	GENERIC_QUERY_FUNCTION_START;

	sql->begin();

	// Prepare an SQL statement
	soci::statement st = (sql->prepare <<
		"INSERT INTO Alerts (AlertDefinitionId, AlertStatusTypeId, CreatedTimestamp, LocationId, ModuleId, OwnerOrganizationId, OwnerUserId) VALUES "
		"(" 
		<< AlertDefinitionId << ","
		<< AlertTypeId << ","
		<< "FROM_UNIXTIME(" << TrigerStamp << "),"
		<< LocationId << ","
		<< ModuleId << ","
		<< OrganizationId << ","
		<< UserId << 
		")");

	// Execute the SQL statement
	st.execute();

	// Retrieve the last insert ID
	*sql << "SELECT LAST_INSERT_ID()", soci::into(out_AlertId);

	// Commit the transaction
	sql->commit();

	return true;

	GENERIC_QUERY_FUNCTION_END;
}

bool VSSDBManager::Query_DPSAlerts(__int64 DPS_Id, std::vector<AlertQueryData>& out_res)
{
	GENERIC_QUERY_FUNCTION_START;

	soci::rowset<soci::row> result = (sql->prepare <<
		"SELECT AlertId, AL.AlertDefinitionId, AlertStatusTypeId, LocationId, ModuleId, "
		"AL.OwnerOrganizationId, AL.OwnerUserId, AD.AlertTypeId FROM Alerts AL" <<
		" INNER JOIN DPS_Modules DPSM ON DPSM.ModuleInstanceID = AL.ModuleId" <<
		" INNER JOIN AlertDefintion AD ON AL.AlertDefinitionId = AD.AlertDefinitionId" <<
		" where "
		" ((AL.AlertStatusTypeId & (2|4)) <> (2|4))" <<
		" AND DPSM.DPSID=" << DPS_Id <<
		" AND AD.Disabled=0");

	out_res.reserve(100); // oh wow, there is no way to guess how many rows got returned. This is so random
	for (const auto& it : result)
	{
		AlertQueryData& row = out_res.emplace_back();
		row.AlertId = it.get<int>(0);
		row.AlertDefinitionId = it.get<int>(1);
		row.AlertStatusTypeId = it.get<int>(2);
		row.LocationId = it.get<int>(3);
		row.ModuleId = it.get<int>(4);
		row.OrganizationId = it.get<int>(5);
		row.UserId = it.get<int>(6);
		row.AlertTypeId = it.get<int>(7);
	}
	return true;

	GENERIC_QUERY_FUNCTION_END;
}

bool VSSDBManager::Query_SoftwareAlerts(__int64 ModuleId, __int64 AlertTypeId, std::vector<DSModuleAlert>& out_res)
{
	GENERIC_QUERY_FUNCTION_START;

	soci::rowset<soci::row> result = (sql->prepare <<
		"SELECT ad.AlertDefinitionId, ad.OwnerOrganizationId, ad.OwnerUserId FROM alertdefintion ad" <<
		" JOIN organizationmodules om ON ad.OwnerOrganizationId = om.OrganizationID" <<
		" where "
		" ad.AlertTypeId = " << AlertTypeId <<
		" AND om.ModuleInstanceID = " << ModuleId <<
		" AND Disabled=0");

	out_res.reserve(100); // oh wow, there is no way to guess how many rows got returned. This is so random
	for (const auto& it : result)
	{
		DSModuleAlert& row = out_res.emplace_back();
		row.AlertDefinitionId = it.get<int>(0);
		row.OrganizationId = it.get<int>(1);
		row.UserId = it.get<int>(2);
	}
	return true;

	GENERIC_QUERY_FUNCTION_END;
}

bool VSSDBManager::Update_AlertStatus(__int64 AlertId, __int64 StatusFlag)
{
	GENERIC_QUERY_FUNCTION_START;

	soci::rowset<soci::row> result = (sql->prepare <<
		"UPDATE Alerts set AlertStatusTypeId=AlertStatusTypeId | " << StatusFlag <<
		" where "
		" AlertId = " << AlertId);

	return true;

	GENERIC_QUERY_FUNCTION_END;
}

bool VSSDBManager::RunCacheQuery(const char* query, __int64 MaxAcceptedId, unsigned int** out_buff, __int64* out_maxId)
{
	*out_buff = NULL;
	*out_maxId = 0;

	GENERIC_QUERY_FUNCTION_START;

	soci::rowset<soci::row> result = (sql->prepare << query);

	unsigned int* temp_res = (unsigned int*)InternalMalloc(MaxAcceptedId * sizeof(unsigned int));
	if (temp_res == NULL)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeveritySever, LogSourceGroups::LogSourceDatabaseManager, 0, 0,
			"MysqlManager:Failed to allocated %lld bytes for : %s", MaxAcceptedId * sizeof(unsigned int), query);
		return false;
	}
	memset(temp_res, 0, MaxAcceptedId * sizeof(unsigned int));
	unsigned int maxIndex = 0;
	for (const auto& it : result)
	{
		unsigned int index = it.get<int>(0);
		unsigned int val = it.get<int>(1);
		if (index >= MaxAcceptedId)
		{
			AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeveritySever, LogSourceGroups::LogSourceDatabaseManager, 0, 0,
				"MysqlManager:Id %d > %d. Table is not cachable : %s", index, MaxAcceptedId, query);
			continue;
		}
		temp_res[index] = val;
		if (index > maxIndex)
		{
			maxIndex = index;
		}
	}

	// ? no values to cache ?
	if (maxIndex == 0)
	{
		return false;
	}

	// include highest index also
	maxIndex += 1;

	*out_buff = (unsigned int*)InternalMalloc(maxIndex * sizeof(unsigned int));
	if (*out_buff == NULL)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeveritySever, LogSourceGroups::LogSourceDatabaseManager, 0, 0,
			"MysqlManager:Failed to allocated %lld bytes for : %s", maxIndex * sizeof(unsigned int), query);
		return false;
	}

	memcpy(*out_buff, temp_res, maxIndex * sizeof(unsigned int));
	*out_maxId = maxIndex;

	// free temp buff
	InternalFree(temp_res);

	return true;

	GENERIC_QUERY_FUNCTION_END;
}

bool VSSDBManager::RunCacheQuery1Val(const char* query, __int64 RowId, __int64* out_val)
{
	GENERIC_QUERY_FUNCTION_START;

	soci::rowset<soci::row> result = (sql->prepare << query << RowId);

	auto itr = result.begin();
	if (itr != result.end())
	{
		*out_val = itr->get<int>(0);
	}

	return true;

	GENERIC_QUERY_FUNCTION_END;
}


bool VSSDBManager::Update_ServerHealthReport(__int64 DPS_Id, __int64 activeThreads, float CPUUsage, float CPUUsagePeak,
	__int64 memUsage, __int64 BPMIn, __int64 BPMOut, __int64 PPMIn, __int64 PPMOut,
	unsigned __int64 startupStamp, int PID, int wsClients)
{
	GENERIC_QUERY_FUNCTION_START;

	soci::rowset<soci::row> result = (sql->prepare <<
		"UPDATE DPS_Instances set StartupStamp=FROM_UNIXTIME(" << startupStamp << ")" <<
		",LastHeartbeat=FROM_UNIXTIME(" << time(NULL) << ")" <<
		",CPUAvg=" << CPUUsage <<
		",CPUPeak=" << CPUUsagePeak <<
		",BytesPerMinIn=" << BPMIn <<
		",BytesPerMinOut=" << BPMOut <<
		",PacketsPerMinIn=" << PPMIn <<
		",PacketsPerMinOut=" << PPMOut <<
		",BytesIn=BytesIn+" << BPMIn <<
		",BytesOut=BytesOut+" << BPMOut <<
		",PacketsIn=PacketsIn+" << PPMIn <<
		",PacketsOut=PacketsOut+" << PPMOut <<
		",PID=" << PID <<
		",Threads=" << activeThreads <<
		",WSClients=" << wsClients <<
		",MemUsage=" << memUsage <<
 		" where "
		" DPSID=" << DPS_Id);

	return true;

	GENERIC_QUERY_FUNCTION_END;
}
