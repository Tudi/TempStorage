#include "StdAfx.h"

#pragma pack(push,1) // support multi platform and setting builds
#define PERSISTENT_KPI_DATA_VER 3
typedef struct PersistentKPIData
{
	static void Init(PersistentKPIData* self)
	{
		memset(self, 0, sizeof(PersistentKPIData));
		self->ver = PERSISTENT_KPI_DATA_VER;
		self->size = sizeof(PersistentKPIData);
		self->FPSMin = 0x7FFF;
		self->FPSMax = 0x0000;
		self->LastServerSavedStamp = time(NULL);
	}
	unsigned int ver; // version of this info struct
	unsigned int size; // just in case version is not updated, this should help us guess corectness
	unsigned __int64 TotalTimeLoggedIn;
	unsigned int NumberOfLogins;
	unsigned __int64 SumLatenciesAPI;
	unsigned int CountLatenciesAPI;
	unsigned int NumerOfAPICalls;
	unsigned int FPSMin;
	unsigned int FPSAvg;
	unsigned int FPSMax;
	unsigned __int64 LastServerSavedStamp;
	unsigned __int64 AsyncAPIDurationSum;
	unsigned __int64 AsyncAPIDurationCount;
}PersistentKPIData;
#pragma pack(pop)

void SaveKPIDataToServer(PersistentKPIData* data)
{
	// not enough time has passed
	if (data->LastServerSavedStamp + KPI_PERIOD_SAVE_TO_SERVER_HOURS * 60 * 60 < (unsigned __int64)time(NULL))
	{
		return;
	}

	// Convert relevant data to JSON to be sent
	nlohmann::json jsonData;
	jsonData["VER"] = data->ver;

	jsonData["FPS_MIN"] = data->FPSMin;
	jsonData["FPS_AVG"] = data->FPSAvg;
	jsonData["FPS_MAX"] = data->FPSMax;

	if (data->CountLatenciesAPI > 0)
	{
		jsonData["LAG_AVG"] = data->SumLatenciesAPI / data->CountLatenciesAPI;
	}

	jsonData["OnlineTime"] = data->TotalTimeLoggedIn;

	if (data->AsyncAPIDurationCount > 0)
	{
		jsonData["APIDUR_AVG"] = data->AsyncAPIDurationSum / data->AsyncAPIDurationCount;
	}

	// Convert the JSON object to a string
	std::string jsonString = jsonData.dump();

	// call backend API to save the data to DB
	if (WebApi_SaveKPIAsync(jsonString.c_str(), NULL, NULL) == WebApiErrorCodes::WAE_NoError)
	{
		data->LastServerSavedStamp = time(NULL);
	}
}

void KPIManager_AsyncExecuteThread(WatchdogThreadData* wtd)
{
	while (sAppSession.IsApplicationRunning() && wtd->ShouldShutDown() == false)
	{
		ULONGLONG startStamp = GetTickCount64();

		// let watchdog know this thread is functioning as expected
		wtd->SignalHeartbeat();

		if (sAppSession.IsUserLoggedIn())
		{
			// check how much latency is there between this user and the connected backend
			// maybe load balancer can redirect us to a better edge server
			if (sKPI.m_ullLastRawLatencyCheckStamp < startStamp)
			{
				sKPI.m_ullLastRawLatencyCheckStamp = startStamp + KPI_PERIOD_MEASURE_LATENCY_RAW;
				int dLatency = -1;
				WebApiErrorCodes err = WebApi_GetRawLatency(&dLatency);
				if (err == WebApiErrorCodes::WAE_NoError)
				{
					if (sKPI.m_dLastRawLatency / 10 != dLatency / 10)
					{
						AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityKPI, LogSourceGroups::LogSourceKPIWorkerThread, 0, 0,
							"KPIAsync:Updating latest raw latency to %d ms", dLatency);
					}
					sKPI.m_dLastRawLatency = dLatency;
				}
			}

			// is the system overloaded right now ?
			// able to tell the user if what he sees is real time or lagging behind a lot
			if (sKPI.m_ullLastAPILatencyCheckStamp < startStamp)
			{
				sKPI.m_ullLastAPILatencyCheckStamp = startStamp + KPI_PERIOD_MEASURE_LATENCY_API;
				int dLatency = -1;
				WebApiErrorCodes err = WebApi_GetAPILatency(&dLatency);
				if (err == WebApiErrorCodes::WAE_NoError)
				{
					if (sKPI.m_dLastAPILatency / 10 != dLatency / 10)
					{
						AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityKPI, LogSourceGroups::LogSourceKPIWorkerThread, 0, 0,
							"KPIAsync:Updating latest api latency to %d ms", dLatency);
					}
					sKPI.m_dLastAPILatency = dLatency;
					sKPI.m_PersistentDataNow->SumLatenciesAPI += dLatency;
					sKPI.m_PersistentDataNow->CountLatenciesAPI++;
				}
			}

			// count the seconds the Application has been used
			static ULONGLONG UserLoggedInStamp = GetTickCount64();
			sKPI.m_PersistentDataNow->TotalTimeLoggedIn = GetTickCount64() - UserLoggedInStamp;
			sKPI.m_PersistentDataNow->NumberOfLogins = 1;

			// check if we should save persistent data to the server
			SaveKPIDataToServer(sKPI.m_PersistentData);
		}

		wtd->BlockThreadUntilNextCycle();
	}

	// Let watchdog know we exited
	wtd->MarkDead();
}

KPIManager::KPIManager()
{
	m_ullLastRawLatencyCheckStamp = 0;
	m_ullLastAPILatencyCheckStamp = 0;
	m_dLastRawLatency = -1;
	m_dLastAPILatency = -1;
	m_PersistentData = (PersistentKPIData*)InternalMalloc(sizeof(PersistentKPIData));
	m_PersistentDataNow = (PersistentKPIData*)InternalMalloc(sizeof(PersistentKPIData));
	if (m_PersistentData == NULL || m_PersistentDataNow == NULL)
	{
		sAppSession.SetApplicationRunning(false);
		return;
	}
	PersistentKPIData::Init(m_PersistentDataNow);
}

KPIManager::~KPIManager()
{
#ifndef _DEBUG
	DestructorCheckMemLeaks();
#endif
}

void KPIManager::DestructorCheckMemLeaks()
{
	if (m_PersistentData != NULL)
	{
		UpdatePersistentData();
		InternalFree(m_PersistentData);
		InternalFree(m_PersistentDataNow);
	}

#ifdef _DEBUG
	delete& sKPI;
#endif
}

void KPIManager::Init()
{
	LoadPersistentData();
	
	sAppSession.CreateWorkerThread(KPIManager_AsyncExecuteThread, "KPIManager", KPI_PERIOD_UPDATE_LATENCY);
}

void KPIManager::LoadPersistentData()
{
	FILE* fPersistentDataFile = NULL;
	fopen_s(&fPersistentDataFile, KPI_FILE_NAME, "rb");
	if (fPersistentDataFile != NULL)
	{
		size_t readCount = fread(m_PersistentData, 1, sizeof(PersistentKPIData), fPersistentDataFile);
		if (readCount != sizeof(PersistentKPIData))
		{
			if (readCount != 0)
			{
				AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityKPI, LogSourceGroups::LogSourceKPIWorkerThread, 0, 0,
					"KPI:Failed to load KPI file data. Got %lld. Expected %lld", readCount, sizeof(PersistentKPIData));
			}
			PersistentKPIData::Init(m_PersistentData);
		}
		if (m_PersistentData->ver != PERSISTENT_KPI_DATA_VER)
		{
			PersistentKPIData::Init(m_PersistentData);
		}
		fclose(fPersistentDataFile);
	}
}

void KPIManager::UpdatePersistentData()
{
	PersistentKPIData newData;
	//first time init ?
	if (m_PersistentData->NumerOfAPICalls == 0)
	{
		memcpy(&newData, m_PersistentDataNow, sizeof(newData));
	}
	else
	{
		PersistentKPIData::Init(&newData);
		newData.TotalTimeLoggedIn = m_PersistentData->TotalTimeLoggedIn + m_PersistentDataNow->TotalTimeLoggedIn;
		newData.NumberOfLogins = m_PersistentData->NumberOfLogins + m_PersistentDataNow->NumberOfLogins;
		newData.SumLatenciesAPI = m_PersistentData->SumLatenciesAPI + m_PersistentDataNow->SumLatenciesAPI;
		newData.CountLatenciesAPI = m_PersistentData->CountLatenciesAPI + m_PersistentDataNow->CountLatenciesAPI;
		newData.NumerOfAPICalls = m_PersistentData->NumerOfAPICalls + m_PersistentDataNow->NumerOfAPICalls;
		newData.FPSMin = MIN(m_PersistentData->FPSMin, m_PersistentDataNow->FPSMin);
		newData.FPSMax = MAX(m_PersistentData->FPSMax, m_PersistentDataNow->FPSMax);
		newData.FPSAvg = (m_PersistentData->FPSAvg + m_PersistentDataNow->FPSAvg) / 2;
		newData.LastServerSavedStamp = MIN(m_PersistentData->LastServerSavedStamp, m_PersistentDataNow->LastServerSavedStamp);
		newData.AsyncAPIDurationCount = m_PersistentData->AsyncAPIDurationCount + m_PersistentDataNow->AsyncAPIDurationCount;
		newData.AsyncAPIDurationSum = m_PersistentData->AsyncAPIDurationSum + m_PersistentDataNow->AsyncAPIDurationSum;
	}
	FILE* fPersistentDataFile = NULL;
	fopen_s(&fPersistentDataFile, KPI_FILE_NAME, "wb");
	if (fPersistentDataFile != NULL)
	{
		fwrite(&newData, 1, sizeof(PersistentKPIData), fPersistentDataFile);
		fclose(fPersistentDataFile);
	}
}

void KPIManager::IncreaseAPICallCount()
{
	m_PersistentDataNow->NumerOfAPICalls++;
}

void KPIManager::UpdateFPSStat(__int64 AvgFPSEveryMinute)
{
	bool bMadeChange = false;
	if (m_PersistentDataNow->FPSMin == 0 || m_PersistentDataNow->FPSMin > AvgFPSEveryMinute)
	{
		m_PersistentDataNow->FPSMin = (int)AvgFPSEveryMinute;
		bMadeChange = true;
	}
	m_PersistentDataNow->FPSAvg = (int)AvgFPSEveryMinute;
	if (m_PersistentDataNow->FPSMax == 0 || m_PersistentDataNow->FPSMax < AvgFPSEveryMinute)
	{
		m_PersistentDataNow->FPSMax = (int)AvgFPSEveryMinute;
		bMadeChange = true;
	}
	// for the sake of curiosity
	static uint32_t prevAvg = 0;
	if (bMadeChange || prevAvg != m_PersistentDataNow->FPSAvg)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityKPI, LogSourceGroups::LogSourceKPI, 0, 0,
			"KPI:Practical FPS update : %d - %d - %d", 1000 / m_PersistentDataNow->FPSMax, 1000 / m_PersistentDataNow->FPSAvg, 1000 / m_PersistentDataNow->FPSMin);
	}
	prevAvg = m_PersistentDataNow->FPSAvg;
}

void KPIManager::UpdateAsyncDurationStat(__int64 CurCallTime)
{
#ifdef _DEBUG
	if (CurCallTime > 1000)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceKPI, 0, 0,
			"KPI:API call duration %lld ms", CurCallTime);
	}
#endif
	m_PersistentDataNow->AsyncAPIDurationSum += CurCallTime;
	m_PersistentDataNow->AsyncAPIDurationCount++;
}