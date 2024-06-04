#ifdef _WIN32
	#include <winsock2.h>
	#include <Ws2tcpip.h>
	#pragma comment(lib, "ws2_32.lib")
#else
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <netinet/in.h>
	#include <sys/socket.h>
#endif
#include <sstream>
#include <thread>
#include <mutex>
#include "UDPServer.h"
#include "ApplicationSession.h"
#include "Util/Allocator.h"
#include "LogManager.h"
#include "VSSPacketDefines.h"
#include "ResourceManager/DataSourceManager.h"
#include "ResourceManager/ServerHealthReportingManager.h"

UDPServerManager::UDPServerManager()
{
	m_dSocket = 0;
}

UDPServerManager::~UDPServerManager()
{
	DestructorCheckMemLeaks();
}

void UDPServerManager::DestructorCheckMemLeaks()
{
	if (m_dSocket)
	{
#ifdef _WIN32
		closesocket(m_dSocket);
		m_dSocket = 0;
#else
		close(m_dSocket);
		m_dSocket = 0;
#endif
	}
}

void UDPServerManager::ShutDownServer()
{
	DestructorCheckMemLeaks();
}

static void ParseUDPPacket(const char* pkt, int size)
{
	size;
	sServerHealthReportingManager.ReportPacketRecv(size + 14 + 20 + 8); // Ethernet + IP + UDP header

	// ver 1 format is : [timestamp] [x] [y] [confidence]
	// ver X : [size] [opcode] [moduleid] [timestamp] [x] [y] [confidence] [crc]
	char* EndPtr = NULL;
	uint64_t timestamp = strtoull(pkt, &EndPtr, 10);
	float x = strtof(EndPtr + 1, &EndPtr);
	float y = strtof(EndPtr + 1, &EndPtr);
//	float confidence = strtof(EndPtr + 1, nullptr);

	DSModuleData md;
	md.ModuleID = 1;
	md.Timestamp = timestamp;
	DSModuleData::ObjectDetails &od = md.emplace_back();
	od.ObjectId = 0;
	od.x = x;
	od.y = y;

	// expect a couple of Queries to be run for alerts ( this is a feed ) and packet to be sent to multiple clients
	// Don't expect this to be an instant call
	sDSManager.OnModuleFeedArrived(1, &md);
}

void UDPServerManager::UDPServerManager_AsyncExecuteThread(WatchdogThreadData* wtd)
{
	while (sAppSession.IsApplicationRunning() && 
		wtd->ShouldShutDown() == false &&
		sUDPServerManager.m_dSocket != 0)
	{
		// let watchdog know this thread is functioning as expected
		wtd->SignalHeartbeat();

		{
			char buffer[MaxPacketStackAlloc];
			sockaddr_in clientAddr;
			socklen_t clientAddrLen = sizeof(clientAddr);

			unsigned __int64 dStartListen = GetTickCount64();
			int bytesReceived = recvfrom(sUDPServerManager.m_dSocket, buffer, MaxPacketStackAlloc, 0, (sockaddr*)&clientAddr, &clientAddrLen);

			if (bytesReceived >= MaxPacketStackAlloc - 1)
			{
				AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceUDPServer, 0, 0,
					"UDPServerManager:Async:UDP packet too large %d!!! from %s", bytesReceived, inet_ntoa(clientAddr.sin_addr));
			}
			else if (bytesReceived > 0) 
			{
				// TODO : remove this later
				buffer[bytesReceived] = '\0'; // Null-terminate the received data
				
				AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceUDPServer, 0, 0,
					"UDPServerManager:Async:Got UDP packet size %d from %s", bytesReceived, inet_ntoa(clientAddr.sin_addr));

				// if we have 0 latency to fetch network packets, this thread is probably busy
				unsigned __int64 dEndListen = GetTickCount64();
				wtd->GetThreadGroupInfo()->OnThreadStartSleep(dEndListen - dStartListen);

				// convert to binary data and pass it to DSManager
				ParseUDPPacket(buffer, bytesReceived);
			}
		}

//		wtd->BlockThreadUntilNextCycle();
	}

	// Let watchdog know we exited
	wtd->MarkDead();
}

void UDPServerManager::Init(size_t maxNetworkThreads, unsigned short listenPort)
{
	if (maxNetworkThreads > INVALID_LARGET_THREADCOUNT)
	{
		maxNetworkThreads = (int)INVALID_LARGET_THREADCOUNT;
	}

	// create an UDP socket
	// Create UDP socket
	m_dSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_dSocket < 0) {
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityCritical, LogSourceGroups::LogSourceUDPServer, 0, 0,
			"UDPServerManager:Failed to create UDP socket");
		return;
	}

	// Bind the socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(listenPort); // Change port number as needed

	if (bind(m_dSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityCritical, LogSourceGroups::LogSourceUDPServer, 0, 0,
			"UDPServerManager:Failed to listen on UDP port %d", listenPort);
#ifdef _WIN32
		closesocket(m_dSocket);
#else
		close(m_dSocket);
#endif
		m_dSocket = 0;
		return;
	}

	// create worker thread
	sAppSession.CreateWorkerThreadGroup(WTG_UDP_Packet_Parser, (int)maxNetworkThreads, "UDPParser", UDPServerManager_AsyncExecuteThread, UDP_WORKERTHREAD_SLEEP);
}
