#include <Windows.h>
#include <random>
#include "LogManager.h"
#include "ApplicationSession.h"
#include "DummyRadar.h"
#include "Util/Allocator.h"
#include "ResourceManager/DataSourceManager.h"
#include "DB/MysqlManager.h"

#define RadarImageSizeHalf (700/2)
#define RadarAlertDistance (RadarImageSizeHalf/3)
#define MaxObjectSpeed (RadarImageSizeHalf/5) // about 5 seconds to traverse radar screen
#define MinObjectSpeed (MaxObjectSpeed/2)

float GetRandomPos()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(-RadarImageSizeHalf, RadarImageSizeHalf);
	float random_number = dis(gen);
	return random_number;
}

float GetRandomSpeed()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(-(MaxObjectSpeed - MinObjectSpeed), (MaxObjectSpeed - MinObjectSpeed));

	// Generate a random float number
	float random_number = dis(gen);

	if (random_number < 0)
	{
		random_number += -MinObjectSpeed;
	}
	else
	{
		random_number += MinObjectSpeed;
	}

	return random_number;
}

DummyRadar::DummyRadar(__int64 dModuleId, unsigned __int64 dUpdatePeriod, __int64 dObjectsMoving)
{
	m_dModuleId = dModuleId;
	m_dUpdatePeriod = dUpdatePeriod;
	m_dObjectsMoving = dObjectsMoving;
	m_bThreadRunning = true;
	m_dLastUpdatedStamp = GetTickCount64();
	m_dBlockAlertUntilStamp = GetTickCount64();
	m_pWorkerThread = NULL;

	m_Objects.reserve(m_dObjectsMoving);
	for (__int64 i = 0; i < dObjectsMoving; i++)
	{
		m_Objects.emplace_back();
		m_Objects[i].x = GetRandomPos();
		m_Objects[i].y = GetRandomPos();
		m_Objects[i].px = GetRandomSpeed();
		m_Objects[i].py = GetRandomSpeed();
	}

	// create watchdog data
	WatchdogThreadData* wtd;
	InternalNew(wtd, WatchdogThreadData);
	if (wtd == NULL)
	{
		return;
	}

	// start worker thread to push log messages to server
	std::thread* myThread = new std::thread([wtd, this]() { DummyRadar::AsyncExecuteThread(wtd); });
	char ThreadName[500];
	sprintf_s(ThreadName, "Dummy Radar with ModuleId %lld", dModuleId);
	wtd->Init(myThread, (int)m_dUpdatePeriod, ThreadName);

	// Make the application wait until this thread also exits
	sAppSession.AddModuleThread(wtd);

	m_pWorkerThread = wtd;
}

bool IsOutOfBounds(float &x, float &y)
{
	if (x < -RadarImageSizeHalf)
	{
		x = -RadarImageSizeHalf;
		return true;
	}
	if (x > RadarImageSizeHalf)
	{
		x = RadarImageSizeHalf;
		return true;
	}
	if (y < -RadarImageSizeHalf)
	{
		y = -RadarImageSizeHalf;
		return true;
	}
	if (y > RadarImageSizeHalf)
	{
		y = RadarImageSizeHalf;
		return true;
	}
	return false;
}

bool IsObjectAtAlertVicinity(float x, float y)
{
	float dx = 0.0f - x;
	float dy = 0.0f - y;
	float dist = sqrt(dx * dx + dy * dy);
	if (dist < RadarAlertDistance)
	{
		return true;
	}
	return false;
}

void DummyRadar::UpdateObjectPositions()
{
	// if object went out of screen than pick a new move direction
	for (auto itr = m_Objects.begin(); itr != m_Objects.end(); itr++)
	{
		// first time init ?
		if ((itr->px == 0.0f && itr->py == 0.0f) ||
			IsOutOfBounds(itr->x, itr->y))
		{
			itr->px = GetRandomSpeed();
			itr->py = GetRandomSpeed();
		}
	}
	// allow objects to move around as time passes
	float secondsPassed = (GetTickCount64() - m_dLastUpdatedStamp) / 1000.0f;
	m_dLastUpdatedStamp = GetTickCount64();
	for (auto itr = m_Objects.begin(); itr != m_Objects.end(); itr++)
	{
		itr->x += (itr->px * secondsPassed);
		itr->y += (itr->py * secondsPassed);
	}
}

void DummyRadar::EmulateRadarRead()
{
//#define DISABLE_DUMMY_RADAR_LOCATION_MODULE
#ifndef DISABLE_DUMMY_RADAR_LOCATION_MODULE
	DSModuleData md;
	md.ModuleID = m_dModuleId;
	md.Timestamp = GetTickCount64();
	md.reserve(m_dObjectsMoving);
	for (__int64 i = 0; i < m_dObjectsMoving; i++)
	{
		DSModuleData::ObjectDetails &od = md.emplace_back();
		od.ObjectId = (unsigned short)i;
		od.x = m_Objects[i].x;
		od.y = m_Objects[i].y;
	}
	sDSManager.OnModuleFeedArrived(m_dModuleId, &md);
#endif
//#define DUMP_DUMMY_RADAR_TO_FILE
#ifdef DUMP_DUMMY_RADAR_TO_FILE // will replay this using an UDP client
	{
		static FILE* f = NULL;
		if (f == NULL)
		{
			fopen_s(&f, "LocationModuleMessages.txt","at");
		}
		if (f)
		{
			fprintf(f, "%lld %f %f %f\n", GetTickCount64(), m_Objects[0].x, m_Objects[0].y, 1.0f);
			fflush(f);
		}
	}
#endif
}

void DummyRadar::RunAlertDetectScripts()
{
	if (GetTickCount64() < m_dBlockAlertUntilStamp)
	{
		return;
	}

	// maybe we triggered an allert : 2 persons near the center
	size_t dPersonsAtCenter = 0;
	for (auto itr = m_Objects.begin(); itr != m_Objects.end(); itr++)
	{
		if (itr->alertIsOn == true)
		{
			continue;
		}
		// first time init ?
		if (IsObjectAtAlertVicinity(itr->x, itr->y))
		{
			dPersonsAtCenter++;
		}
	}
	// more than 1 person in the vicinity
	if (dPersonsAtCenter > 0)
	{
		DSModuleAlert ma;
		ma.InitTypeInfo();
		ma.AlertType = VSS_N_ModuleAlertState::AlertTypes::PersonInRange;
		ma.TriggerStamp = time(NULL);
		ma.LocationId = 2;
		ma.ModuleID = m_dModuleId;
		ma.OrganizationId = 1;
		ma.UserId = 9;
		sDSManager.OnModuleAlertArrived(m_dModuleId, &ma);
		// add it to the DB ?
		// block spam
		m_dBlockAlertUntilStamp = GetTickCount64() + TRIGGER_ALERT_INTERVAL;

		// mark the objects triggering the alarm. So they would not spam it
		for (auto itr = m_Objects.begin(); itr != m_Objects.end(); itr++)
		{
			if (itr->alertIsOn == true)
			{
				continue;
			}
			// first time init ?
			if (IsObjectAtAlertVicinity(itr->x, itr->y))
			{
				itr->alertIsOn = true;
			}
		}
	}
	else
	{
		// clear the list of objects triggering the alarm
		for (auto itr = m_Objects.begin(); itr != m_Objects.end(); itr++)
		{
			if (IsObjectAtAlertVicinity(itr->x, itr->y) == false)
			{
				itr->alertIsOn = false;
			}
		}
	}
}

void DummyRadar::AsyncExecuteThread(WatchdogThreadData* wtd)
{
	while (m_bThreadRunning && sAppSession.IsApplicationRunning() && wtd->ShouldShutDown() == false)
	{
		// let watchdog know this thread is functioning as expected
		wtd->SignalHeartbeat();

		// actual periodic code. Rest is usual
		{
			// update our objects
			UpdateObjectPositions();

			// periodically spam feed
			EmulateRadarRead();

			// maybe radar feed would trigger an alert. Hardcoded for now
			RunAlertDetectScripts();
		}

		wtd->BlockThreadUntilNextCycle();
	}

	// Let watchdog know we exited
	wtd->MarkDead();
}

void DummyRadar::DisconnectRadar()
{
	m_bThreadRunning = false; // tell worker thread it should shut down
	if (m_pWorkerThread != NULL)
	{
		m_pWorkerThread->WakeupThread(); // make sure it's active to process the command
		m_pWorkerThread = NULL;
	}
}