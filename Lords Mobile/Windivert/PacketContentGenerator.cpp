#include <Windows.h>
#include "ParseServerToClient.h"

#define MaxGameX			1024
#define MaxGameY			512
#define TimoutScanMS		(10*60*100)
#define TimoutScanCastleMS	(2*60*100)

struct ScanStatusOnCoord
{
	unsigned int StatusTimout; // can rescan this location once the timout expires
	unsigned int ObjectType;
};

ScanStatusOnCoord GameMap[MaxGameY][MaxGameX];

int ContinueScanX = 0;
int ContinueScanY = 0;
int GeteneratePosToScan(int &px, int &py)
{
	unsigned int TickNow = GetTickCount();
	for (int y = ContinueScanY; y < MaxGameY; y++)
		for (int x = ContinueScanX; x < MaxGameX; x++)
		{
			if (GameMap[y][x].StatusTimout < TickNow)
			{
				px = x;
				py = y;
				ContinueScanY = y;
				ContinueScanX = x + 1;
				GameMap[y][x].StatusTimout = TickNow + TimoutScanMS;
				return 0; // no issues
			}
		}

	//after a rescan without timeouts, we should just start scanning what we can
	if (ContinueScanY == 0 && ContinueScanX == 0)
	{
		for (int y = 0; y < MaxGameY; y++)
			for (int x = 0; x < MaxGameX; x++)
				GameMap[y][x].StatusTimout = 0; // mark all cells timeout
	}

	ContinueScanY = 0;
	ContinueScanX = 0;

	//nothing to update yet
	return 1;
}

//we want to click all locations at first go
void InitContentGenerator()
{
	for (int y = 0; y < MaxGameY; y++)
		for (int x = 0; x < MaxGameX; x++)
			GameMap[y][x].StatusTimout = 0; // mark all cells timeout
}

//if we managed to click a castle, we want to monitor this location with bigger frequency
void OnCastlePopupPacketReceived(int x, int y)
{
	if (y <0 || y> MaxGameY)
		return;
	if (x <0 || x> MaxGameX)
		return;
	GameMap[y][x].StatusTimout = GetTickCount() + TimoutScanCastleMS;
	GameMap[y][x].ObjectType = OBJECT_TYPE_PLAYER;
}