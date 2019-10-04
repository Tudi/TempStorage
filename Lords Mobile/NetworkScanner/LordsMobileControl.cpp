#include <time.h>
#include "LordsMobileControl.h"
#include "HTTPSendData.h"
#include <windows.h>
#include <stdio.h>
#include "Tools.h"
#include "ParsePackets.h"
#include "CapturePackets.h"

int		KeepGameScanThreadsRunning = 1;
int		ThreadParamScanGameData = 0;
HANDLE	ScanGameProcessThreadHandle = 0;
int		InverseScanOrder = (time(NULL) / (60*24)) % 2;

void InvertGameScanDirection()
{
	InverseScanOrder = 1 - InverseScanOrder;
	printf("New scan direction is : %d\n", InverseScanOrder);
}

bool ClickPlayerPacketCatched = false;
u_char *ClickPlayerPacketRetSample = NULL;
int ClickPlayerPacketSize = 0;
int ClickPlayerPacketHeaderSize = 0;
bool ObjectListPacketReceived = false;

#define MinGameX	5
#define MinGameY	5
#define MaxGameX	1024
#define MaxGameY	512
#define ScanJumpSizeX	13
#define ScanJumpSizeY	9

enum ScanStatuses
{
	SS_WaitingForClickPlayerPacket,
	SS_ReceivedClickPlayerPacket,
	SS_ClickPlayerPacketSent,
	SS_ObjectListPacketReceived,
	SS_RestartScan,
	SS_WaitExternalParseStart,
	SS_WaitExternalParseEnd,
	SS_StartNewScanCycle,
	SS_UpdatePlayerInfo,
};

ScanStatuses ScanStatus = SS_WaitingForClickPlayerPacket;
int ScanX = MinGameX;
int ScanY = MinGameY;
#define ExternalParserPeriod 0
unsigned int WaitExternalParserTimeoutStamp = 0;

unsigned int *IngameMapUpdateStamp = NULL;
void InitIngameMap()
{
	int SafeMemorySize = sizeof(unsigned int) * (MaxGameX + 50) * (MinGameY + 50);
	IngameMapUpdateStamp = (unsigned int *)malloc(SafeMemorySize);
	memset(IngameMapUpdateStamp, 0, SafeMemorySize);
}

void GetLeastUpdatedLocation(int *OutX, int * OutY)
{
	if (IngameMapUpdateStamp == NULL)
		InitIngameMap();
	unsigned int MinStamp = 0;
	for (int y = MinGameY; y < MaxGameY; y++)
		for (int x = MinGameX; x < MaxGameX; x++)
			if (IngameMapUpdateStamp[y * MaxGameY + x] < MinStamp)
				MinStamp = IngameMapUpdateStamp[y * MaxGameY + x];
}

void HandleCastleClickPacket(const unsigned char *pkt_data, int len, const unsigned char *GameData, int GameLen)
{
	printf("Received castle click packet\n");
	//is this a guid data ?
	int x, y;
	if (GetXYFromGUID(*(unsigned int*)GameData[7], x, y) != 0)
		return;
	printf("Castle location at click is : %d %d\n", x, y);

	ClickPlayerPacketRetSample = (u_char*)malloc(len);
	ClickPlayerPacketSize = len;
	memcpy(ClickPlayerPacketRetSample, pkt_data, len);
	ClickPlayerPacketCatched = true;
	ClickPlayerPacketHeaderSize = GameData - pkt_data;
}

void OnLordsClientPacketReceived(const unsigned char *pkt_data, int len, const unsigned char *GameData, int GameLen)
{
	//first 2 bytes are packet len. Unless these confirm the size, the packet did not arrive completely
	//also check packet opcode
	if (GameLen == 11 && (GameData[0] == 0x00 || GameData[1] == 0x0B) && (GameData[0] != 0x9A || GameData[1] != 0x08))
		HandleCastleClickPacket(pkt_data, len, GameData, GameLen);
		return;
}

void ConstructClickPlayerPacket()
{
	int x, y;
	GetLeastUpdatedLocation(&x, &y);
	unsigned int GUID = GenerateIngameGUID(x,y);
	*(unsigned int*)ClickPlayerPacketRetSample[ClickPlayerPacketHeaderSize + 7] = GUID;
	printf("Constructing castle click packet for location : %d %d\n", x, y);
}

void SendClickPlayerPacket()
{
	if (ClickPlayerPacketCatched == false)
		return;
	SendPacket(ClickPlayerPacketRetSample, ClickPlayerPacketSize);
}

//this is already done in ParsePackets.cpp
void ParseObjectPacketAndGenerateOutput()
{
}

void OnCastlePopupPacketReceived(int x, int y)
{
	OnMapLocationUpdate(x, y);
}

void OnMapLocationUpdate(int x, int y)
{
	if (IngameMapUpdateStamp == NULL)
		InitIngameMap();

	#define UPDATE_DISTANCE 10

	int StartX = x - UPDATE_DISTANCE;
	int EndX = x + UPDATE_DISTANCE;
	StartX = MAX(StartX, 0);
	EndX = MIN(EndX, MaxGameX);

	int StartY = y - UPDATE_DISTANCE;
	int EndY = y + UPDATE_DISTANCE;
	StartY = MAX(StartY, 0);
	EndY = MIN(EndY, MaxGameX);

	for (int y = StartY; y < EndY; y++)
		for (int x = StartX; x < EndX; x++)
		{
			int xDist = abs(UPDATE_DISTANCE / 2 - x);
			int yDist = abs(UPDATE_DISTANCE / 2 - y);
			IngameMapUpdateStamp[y * MaxGameY + x] = MAX(IngameMapUpdateStamp[y * MaxGameY + x], GetTickCount() + xDist * yDist);
		}

	//presume we received an update for our last request 
	WaitExternalParserTimeoutStamp = GetTickCount();
}

DWORD WINAPI BackgroundProcessScanGame(LPVOID lpParam)
{
	int LoopCounter = 0;
	unsigned int WaitServerReplyTimout = 0;
	while (KeepGameScanThreadsRunning == 1)
	{
		if (ScanStatus == SS_WaitingForClickPlayerPacket)
		{
			if (ClickPlayerPacketCatched == true)
				ScanStatus = SS_ReceivedClickPlayerPacket;
		}
		if (ScanStatus == SS_ReceivedClickPlayerPacket || ScanStatus == SS_StartNewScanCycle)
		{
			ConstructClickPlayerPacket();
			SendClickPlayerPacket();
//			printf("Will try to scan ingame area at %d:%d\n", ScanX, ScanY);
			ScanStatus = SS_ClickPlayerPacketSent;
			WaitServerReplyTimout = GetTickCount() + 2000;
		}
		if (ScanStatus == SS_ClickPlayerPacketSent && (ObjectListPacketReceived == true || WaitServerReplyTimout < GetTickCount()))
		{
			ParseObjectPacketAndGenerateOutput();
			ObjectListPacketReceived = false;
			ScanX = (ScanX + ScanJumpSizeX);
			if (ScanX > MaxGameX)
			{
				ScanX = MinGameX;
				ScanY = ScanY + ScanJumpSizeY;
			}
			if (ScanY > MaxGameY)
				ScanStatus = SS_RestartScan;
			else
				ScanStatus = SS_WaitExternalParseStart;
		}
		if (ScanStatus == SS_RestartScan)
		{
			ScanX = MinGameX;
			ScanY = MinGameY;
			ScanStatus = SS_WaitExternalParseStart;
		}
		if (ScanStatus == SS_WaitExternalParseStart)
		{
//			HTTP_GenerateMaps();
			WaitExternalParserTimeoutStamp = ExternalParserPeriod + GetTickCount();
			ScanStatus = SS_WaitExternalParseEnd;
		}
		if (ScanStatus == SS_WaitExternalParseEnd && WaitExternalParserTimeoutStamp < GetTickCount())
		{
//			remove("KingdomScanStatus.txt");
			ScanStatus = SS_StartNewScanCycle;
		}
	}
	KeepGameScanThreadsRunning = 0;
	return 0;
}

void	LordsMobileControlStartup()
{
	//1 processing thread is enough
	if (ScanGameProcessThreadHandle != 0)
		return;

	//create the processing thread 
	DWORD   PacketProcessThreadId;
	ScanGameProcessThreadHandle = CreateThread(
		NULL,							// default security attributes
		0,								// use default stack size  
		BackgroundProcessScanGame,		// thread function name
		&ThreadParamScanGameData,		// argument to thread function 
		0,								// use default creation flags 
		&PacketProcessThreadId);		// returns the thread identifier 

	printf("Done creating background thread to scan ingame\n");
}

void	LordsMobileControlShutdown()
{
	if (ScanGameProcessThreadHandle == 0)
		return;

	//signal that we want to break the processing loop
	KeepGameScanThreadsRunning = 2;
	//wait for the processing thread to finish
	while (KeepGameScanThreadsRunning != 0)
		Sleep(10);
	//close the thread properly
	CloseHandle(ScanGameProcessThreadHandle);
	ScanGameProcessThreadHandle = 0;
}