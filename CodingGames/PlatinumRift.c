#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_RECURSION_DEPTH 60
#define MAX_ZONE_COUNT      200
#define POD_PRICE           20
#define MAX_PLAYER_COUNT    4
#define MAX_LINK_COUNT      400

int playerCount; // the amount of players (2 to 4)
int myId; // my player ID (0, 1, 2 or 3)
int myId2 = 5; // my player ID (0, 1, 2 or 3)
int zoneCount; // the amount of zones on the map
int linkCount; // the amount of links between all zones
int zoneId[MAX_ZONE_COUNT]; // this zone's ID (between 0 and zoneCount-1)
int platinumSource[MAX_ZONE_COUNT]; // the amount of Platinum this zone can provide per game turn
int ZoneOwner[MAX_ZONE_COUNT];
int ZonePlayerPods[MAX_ZONE_COUNT][MAX_PLAYER_COUNT];
int Links[MAX_LINK_COUNT][2];
int platinumSourceOrdered[MAX_ZONE_COUNT];
int NotOwnedPlatZoneCount = 0;
//float platinumSourceScore[MAX_ZONE_COUNT]; //this list should keep platinum investment worth
//float platinumSourceNormalized[MAX_ZONE_COUNT]; //this list should keep platinum investment worth
int ZoneDistToPlat[MAX_ZONE_COUNT];    //if we conquer this Zone, how far will we be to a plat Zone ?
int ZoneDistToEnemy[MAX_ZONE_COUNT];    //should only do this after we conquered all plat Zones
int ZoneDistToNeutral[MAX_ZONE_COUNT];    //should only do this after we conquered all plat Zones
int CurrentGameTurn = 0;
int PlatZoneDistCalcTurn = -1;
int EnemyZoneDistCalcTurn = -1;
int NeutZoneDistCalcTurn = -1;
int ZoneNeighbourMap[MAX_ZONE_COUNT][MAX_ZONE_COUNT];
int ZoneContinent[MAX_ZONE_COUNT];
//int RememberPlatinumSpawnStrategy[MAX_ZONE_COUNT];  //if others also spawn pods on neutral zone, than try to spawn more than them
int ContinentCount;
int ContinentPlatinum[MAX_ZONE_COUNT];
int ContinentSize[MAX_ZONE_COUNT];
int ContinentPlatinumNodes[MAX_ZONE_COUNT];
int ContinentMostIsolatedPlatinum[MAX_ZONE_COUNT];
int platinum; // my available Platinum
int AdvisedPodSpawnCount;
//int PlatDistToPlat[MAX_ZONE_COUNT];
int PlatObjectiveSheduled[MAX_ZONE_COUNT];              // reset at it at each turn, dispatch only a few pods to take over it
int ZoneDistToPlatSolo[MAX_ZONE_COUNT][MAX_ZONE_COUNT]; //we want to assign a single pod to capture this plat, other should try to focus on other plats
int PlatSourceEchoFromZone[MAX_ZONE_COUNT][MAX_ZONE_COUNT]; // try to estimate how worthy is a plat zone considering nearby plat sources. Prioritize spawns to wealthy zones
int PlatZonetoPlatDistCalcTurn = -1;
int ZoneScoreDistIncome[MAX_ZONE_COUNT];
int PlatZonetoPlatNotOwnedDistCalcTurn = -1;
int ZoneDistToPlatNotOwnedSolo[MAX_ZONE_COUNT][MAX_ZONE_COUNT]; //we want to assign a single pod to capture this plat, other should try to focus on other plats

#define DEBUG_MOVE(c,f,t,l) printf("%d %d %d ", c, f, t);
//#define DEBUG_MOVE(c,f,t,l) {if(zoneId[f]==78) fprintf(stderr, "func %s, moves 78\n", l); printf("%d %d %d ", c, f, t); }

int FindZoneIdIndex(int ZoneId)
{
    for (int i = 0; i < zoneCount; i++)
        if (zoneId[i] == ZoneId)
            return i; //probably i == zoneId[i]
    return 0;
}

tBiggestEnemyCount = 0;
tBiggestEnemyCountIndex = -1;
int GetEnemyPodCountOnZone(int ZoneIndex)
{
    tBiggestEnemyCount = 0;
    tBiggestEnemyCountIndex = -1;
    for (int i = 0; i<MAX_PLAYER_COUNT; i++)
        if (i != myId)
        {
            if (ZonePlayerPods[ZoneIndex][i] > tBiggestEnemyCount)
            {
                tBiggestEnemyCount = ZonePlayerPods[ZoneIndex][i];
                tBiggestEnemyCountIndex = i;
            }
        }
    return tBiggestEnemyCount;
}

int NeutralCount;
int EnemyCount;
int MyCount;
int BiggestEnemyCount;
int BiggestEnemyCountIndex;
void GetZoneNeighbourCount(int zoneIndex)
{
    NeutralCount = 0;
    EnemyCount = 0;
    MyCount = 0;
    BiggestEnemyCount = 0;
    BiggestEnemyCountIndex = -1;
    for (int i = 0; i < zoneCount; i++)
        if (CanReachZoneFromZoneInd(zoneIndex, i))
        {
            if (ZoneOwner[i] == -1)
                NeutralCount++;
            else if (ZoneOwner[i] != myId && ZoneOwner[i] != myId2)
            {
                EnemyCount++;
                GetEnemyPodCountOnZone(i);
                if (tBiggestEnemyCount > BiggestEnemyCount)
                {
                    BiggestEnemyCount = tBiggestEnemyCount;
                    BiggestEnemyCountIndex = i;
                }
            }
            else
                MyCount++;
        }
}

void BuildZoneNeighbourMap()
{
    memset(ZoneNeighbourMap, -1, sizeof(ZoneNeighbourMap));
    for (int i = 0; i < zoneCount; i++)
        ZoneNeighbourMap[i][MAX_ZONE_COUNT - 1] = 0;
    for (int i = 0; i < linkCount; i++)
    {
        int FromZoneIndex = FindZoneIdIndex(Links[i][0]);
        int ToZoneIndex = FindZoneIdIndex(Links[i][1]);
        int NextFreeIndex = ZoneNeighbourMap[FromZoneIndex][MAX_ZONE_COUNT - 1];
        ZoneNeighbourMap[FromZoneIndex][MAX_ZONE_COUNT - 1]++;
        ZoneNeighbourMap[FromZoneIndex][NextFreeIndex] = ToZoneIndex;

        NextFreeIndex = ZoneNeighbourMap[ToZoneIndex][MAX_ZONE_COUNT - 1];
        ZoneNeighbourMap[ToZoneIndex][MAX_ZONE_COUNT - 1]++;
        ZoneNeighbourMap[ToZoneIndex][NextFreeIndex] = FromZoneIndex;
    }
}

void MarkNeighbourZoneToSameContinentRec(int CurZoneIndex, int RecursionDepth)
{
    for (int i = 0; i < ZoneNeighbourMap[CurZoneIndex][MAX_ZONE_COUNT - 1]; i++)
    {
        int NeighbourZone = ZoneNeighbourMap[CurZoneIndex][i];
        if (ZoneContinent[NeighbourZone] == -1)
        {
            ZoneContinent[NeighbourZone] = ZoneContinent[CurZoneIndex];
            MarkNeighbourZoneToSameContinentRec(NeighbourZone, RecursionDepth + 1);
        }
    }
}

void CalcZoneContinent()
{
    for (int i = 0; i < zoneCount; i++)
        ZoneContinent[i] = -1;
    int UnmarkedContinentFound = 0;
    int MarkContinentAtIndex = 0;
    ContinentCount = 0;
    //mark all neighbours of this zone to same continent
    do {
        ZoneContinent[MarkContinentAtIndex] = ContinentCount;
        MarkNeighbourZoneToSameContinentRec(MarkContinentAtIndex, 0);
        ContinentCount++;
        UnmarkedContinentFound = 0;
        for (int i = 0; i < zoneCount; i++)
            if (ZoneContinent[i] == -1)
            {
                MarkContinentAtIndex = i;
                UnmarkedContinentFound = 1;
            }
    } while (UnmarkedContinentFound == 1);
    fprintf(stderr, "Found %d continents\n", ContinentCount);
}

void CalcContinentPlatinumIncome()
{
    for (int i = 0; i < ContinentCount; i++)
    {
        ContinentPlatinum[i] = 0;
        ContinentSize[i] = 0;
        ContinentPlatinumNodes[i] = 0;
    }
    for (int i = 0; i < zoneCount; i++)
    {
        if (platinumSource[i] > 0)
        {
            ContinentPlatinum[ZoneContinent[i]] += platinumSource[i];
            ContinentPlatinumNodes[ZoneContinent[i]]++;
        }
        ContinentSize[ZoneContinent[i]]++;
    }
    for (int i = 0; i < ContinentCount; i++)
    {
        int PlatIncomePerContinentSize = ContinentPlatinum[i] * 100 / ContinentSize[i];
        fprintf(stderr, "Continent %d platinum income is %d. Size %d. PlatNodes %d. Movement cost / plat node %d\n", i, ContinentPlatinum[i], ContinentSize[i], ContinentPlatinumNodes[i], PlatIncomePerContinentSize);
    }
}

int EchoModVal = 1;
void EchoZoneDistanceOnMapRec(int *Map, int CurZoneIndex, int RecursionDepth)
{
    if (RecursionDepth > MAX_RECURSION_DEPTH)
        return;
    int NeighbourCountForZone = ZoneNeighbourMap[CurZoneIndex][MAX_ZONE_COUNT - 1];
    for (int i = 0; i < NeighbourCountForZone; i++)
    {
        int NeighbourZone = ZoneNeighbourMap[CurZoneIndex][i];
        if (Map[NeighbourZone] > Map[CurZoneIndex] + EchoModVal)
        {
            Map[NeighbourZone] = Map[CurZoneIndex] + EchoModVal;
            EchoZoneDistanceOnMapRec(Map, NeighbourZone, RecursionDepth + 1);
        }
    }
}

void CalculateZoneDistToPlatNotOwned()
{
    if (PlatZoneDistCalcTurn == CurrentGameTurn)
        return;
    PlatZoneDistCalcTurn = CurrentGameTurn;
    for (int i = 0; i < zoneCount; i++)
        ZoneDistToPlat[i] = MAX_ZONE_COUNT;
    EchoModVal = 1;
    for (int i = 0; i < zoneCount; i++)
        if (platinumSource[i] > 0 && ZoneOwner[i] != myId)
        {
            ZoneDistToPlat[i] = 0;
            EchoZoneDistanceOnMapRec(ZoneDistToPlat, i, 0);
        }
}

void CalculateZoneDistToPlatNeutralSolo()
{
    if (PlatZonetoPlatDistCalcTurn == CurrentGameTurn)
        return;
    PlatZonetoPlatDistCalcTurn = CurrentGameTurn;
    NotOwnedPlatZoneCount = 0;
    int NotOwnedPlatIncomeTotal = 0;
    for (int i = 0; i < zoneCount; i++)
        for (int j = 0; j < zoneCount; j++)
            ZoneDistToPlatSolo[i][j] = MAX_ZONE_COUNT;
    EchoModVal = 1;
    for (int i = 0; i < zoneCount; i++)
        //        if (platinumSource[i] > 0 && ZoneOwner[i] != myId)
        if (platinumSource[i] > 0 && ZoneOwner[i] == -1)
        {
            NotOwnedPlatZoneCount++;
            NotOwnedPlatIncomeTotal += platinumSource[i];
            ZoneDistToPlatSolo[i][i] = 0;
            EchoZoneDistanceOnMapRec(&ZoneDistToPlatSolo[i][0], i, 0);
        }
    //sum the distances for each plutinum node. It will be continent based automatically
    for (int i = 0; i < zoneCount; i++)
        //        if (platinumSource[i] > 0 && ZoneOwner[i] != myId)
        if (platinumSource[i] > 0 && ZoneOwner[i] == -1)
        {
            int Count = 0;
            for (int j = 0; j < zoneCount; j++)
                //                if (platinumSource[j] > 0 && ZoneOwner[j] != myId)
                if (platinumSource[i] > 0 && ZoneOwner[i] == -1 && ZoneDistToPlatSolo[i][j] != MAX_ZONE_COUNT)
                {
                    ZoneDistToPlatSolo[i][i] += 2 * ZoneDistToPlatSolo[i][j];   //my distance to this other plat node          
                    Count++;
                }
            if (Count > 0)
                ZoneDistToPlatSolo[i][i] /= Count;
        }
    memset(ContinentMostIsolatedPlatinum, 0, sizeof(ContinentMostIsolatedPlatinum));
    for (int c = 0; c < ContinentCount; c++)
    {
        for (int i = 0; i < zoneCount; i++)
            //            if (platinumSource[i] > 0 && ZoneOwner[i] != myId && ContinentMostIsolatedPlatinum[c] < ZoneDistToPlatSolo[i][i])
            if (platinumSource[i] > 0 && ZoneOwner[i] == -1 && ContinentMostIsolatedPlatinum[c] < ZoneDistToPlatSolo[i][i])
                ContinentMostIsolatedPlatinum[c] = ZoneDistToPlatSolo[i][i];

    }
    memset(ZoneScoreDistIncome, 0, sizeof(ZoneScoreDistIncome));
    int totalScore = 0;
    for (int i = 0; i < zoneCount; i++)
        //        if (platinumSource[i] > 0 && ZoneOwner[i] != myId)
        if (platinumSource[i] > 0 && ZoneOwner[i] == -1)
        {
            ZoneDistToPlatSolo[i][i] = ContinentMostIsolatedPlatinum[ZoneContinent[i]] - ZoneDistToPlatSolo[i][i]; //inverse score
            int AvgWalkDistance = ZoneDistToPlatSolo[i][i] / ContinentPlatinumNodes[ZoneContinent[i]];
            int NormalizedWalkImportanceThisContinent = ZoneDistToPlatSolo[i][i] * 1000 / ContinentMostIsolatedPlatinum[ZoneContinent[i]] / 1;
            int NormalizedIncomeImportanceContinent = platinumSource[i] * 1000 / ContinentPlatinum[ZoneContinent[i]] * 1;
            int NormalizedIncomeImportanceGlobal = platinumSource[i] * 100 / NotOwnedPlatIncomeTotal * 1;
            int NormalizedContinentIncomeImportance = ContinentPlatinum[ZoneContinent[i]] * 100 / NotOwnedPlatIncomeTotal;
            int AVGScore = NormalizedWalkImportanceThisContinent * NormalizedIncomeImportanceContinent * NormalizedIncomeImportanceGlobal * NormalizedContinentIncomeImportance / (10 * 100 * 100);
            ZoneScoreDistIncome[i] = AVGScore;
            totalScore += AVGScore;
            //            if (ZoneContinent[i] == 1)
            //            fprintf(stderr, "plat zone %d - score %d - %d. Has %d neighbours score, plat score %d, glob plat score %d, neighbour count %d, income %d\n", zoneId[i], AVGScore, ZoneDistToPlatSolo[i][i], NormalizedWalkImportanceThisContinent, NormalizedIncomeImportanceContinent, NormalizedIncomeImportanceGlobal, ContinentPlatinumNodes[ZoneContinent[i]], platinumSource[i]);
        }
    if (totalScore>0)
    {
        //eliminate below avg
        int Count = 0;
        int Total = 0;
        for (int i = 0; i < zoneCount; i++)
        {
            ZoneScoreDistIncome[i] = ZoneScoreDistIncome[i] * 100 / totalScore;
            Total += ZoneScoreDistIncome[i];
            Count++;
            //            if( ZoneScoreDistIncome[i] )
            //                fprintf(stderr, "plat zone %d - %d\n",zoneId[i],ZoneScoreDistIncome[i]);
        }
        int Avg = Total / Count;
        Total = 0;
        for (int i = 0; i < zoneCount; i++)
        {
            if (ZoneScoreDistIncome[i] <= Avg)
                ZoneScoreDistIncome[i] = 0;
            else
                Total += ZoneScoreDistIncome[i];
        }
        if (Total > 0)
            for (int i = 0; i < zoneCount; i++)
            {
                ZoneScoreDistIncome[i] = ZoneScoreDistIncome[i] * 100 / Total;
//                if (ZoneScoreDistIncome[i])
//                    fprintf(stderr, "plat zone %d - %d\n", zoneId[i], ZoneScoreDistIncome[i]);
            }
    }
}

int FindNextBestScorePlatNodeFree()
{
    CalculateZoneDistToPlatNeutralSolo();
    int BestScore = 0;
    int BestNode = -1;
    for (int i = 0; i < zoneCount; i++)
        if (ZoneScoreDistIncome[i] >= BestScore && PlatObjectiveSheduled[i] == 0 && platinumSource[i] > 0 && ZoneOwner[i] == -1 )
        {
            BestScore = ZoneScoreDistIncome[i];
            BestNode = i;
        }
    if ( BestNode >= 0 )
    {
        PlatObjectiveSheduled[BestNode] = 1;
        if (BestScore >= 10)
            AdvisedPodSpawnCount = 2;

        GetZoneNeighbourCount(BestNode);
        int MyPodCount = ZonePlayerPods[BestNode][myId];
        if (BiggestEnemyCount > MyPodCount && BiggestEnemyCount > AdvisedPodSpawnCount )
            AdvisedPodSpawnCount = BiggestEnemyCount - MyPodCount;
    }
    return BestNode;
}

void CalculateZoneDistToEnemyTeritory()
{
    if (EnemyZoneDistCalcTurn == CurrentGameTurn)
        return;
    EnemyZoneDistCalcTurn = CurrentGameTurn;
    for (int i = 0; i < zoneCount; i++)
        ZoneDistToEnemy[i] = MAX_ZONE_COUNT;
    EchoModVal = 1;
    for (int i = 0; i < zoneCount; i++)
        if (ZoneOwner[i] != myId)
        {
            ZoneDistToEnemy[i] = 0;
            EchoZoneDistanceOnMapRec(ZoneDistToEnemy, i, 0);
        }
}

void CalculateZoneDistToNeutralTeritory()
{
    //fprintf(stderr, "CalculateZoneDistToNeutralTeritory %d - %d\n", NeutZoneDistCalcTurn, CurrentGameTurn );
    if (NeutZoneDistCalcTurn == CurrentGameTurn)
        return;
    NeutZoneDistCalcTurn = CurrentGameTurn;
    for (int i = 0; i < zoneCount; i++)
        ZoneDistToNeutral[i] = MAX_ZONE_COUNT;
    EchoModVal = 1;
    for (int i = 0; i < zoneCount; i++)
        if (ZoneOwner[i] == -1)
        {
            ZoneDistToNeutral[i] = 0;
            EchoZoneDistanceOnMapRec(ZoneDistToNeutral, i, 0);
        }
    int CurZoneIndex = FindZoneIdIndex(89);
    int DestZone = FindZoneIdIndex(78);
    //if( zoneId[CurZoneIndex]==89||zoneId[CurZoneIndex]==78)fprintf(stderr, "zone %d has %d neighbours %d\n",zoneId[CurZoneIndex],ZoneNeighbourMap[CurZoneIndex][MAX_ZONE_COUNT - 1], zoneId[DestZone]);        
}

int FindRichestFreeZoneInitialRestrictContinent()
{
    //get richest continent and get the richest nodes there
    int BestContinentIncome = 0;
    int BestContinentIndex = 1;
    float PlatZoneTotalIncomePCT[MAX_ZONE_COUNT];
    for (int i = 0; i < ContinentCount; i++)
        if (ContinentPlatinum[i]>BestContinentIncome)
        {
            BestContinentIncome = ContinentPlatinum[i];
            BestContinentIndex = i;
        }

    int PlatinumIncome = 0;
    int ZoneIndex = -1;
    int HaveNodesCount = 0;
    for (int i = 0; i < zoneCount; i++)
        if (platinumSource[i] > PlatinumIncome
            && ZoneOwner[i] == -1
            && ZonePlayerPods[i][0] == 0 && ZonePlayerPods[i][1] == 0 && ZonePlayerPods[i][2] == 0 && ZonePlayerPods[i][3] == 0
            )
        {
            HaveNodesCount++;
            PlatZoneTotalIncomePCT[i] = (float)platinumSource[i] / (float)BestContinentIncome;
            if (ZoneContinent[i] == BestContinentIndex)
            {
                ZoneIndex = i;
                PlatinumIncome = platinumSource[i];
            }
        }

    //normalize continent spawn importance
    if (HaveNodesCount && ZoneIndex != -1)
    {
        int CanSpawnCount = platinum / POD_PRICE;
        HaveNodesCount = ContinentPlatinumNodes[BestContinentIndex];
        //        float SpawnPods = (float)CanSpawnCount / ((float)HaveNodesCount / 2.0f);
        float SpawnPods = (float)PlatZoneTotalIncomePCT[ZoneIndex] * (float)CanSpawnCount;
        SpawnPods += 0.9f;
        AdvisedPodSpawnCount = (int)SpawnPods;
        if (AdvisedPodSpawnCount <= 0)
            AdvisedPodSpawnCount = 1;
        if (AdvisedPodSpawnCount > CanSpawnCount)
            AdvisedPodSpawnCount = CanSpawnCount;
        //        fprintf(stderr, "On Cont %d, Zone %d, Income %d, Plat %d, Advise %d pods. Can spawn %d\n", BestContinentIndex, ZoneIndex, platinumSource[ZoneIndex], platinum, AdvisedPodSpawnCount, CanSpawnCount);
    }
    return ZoneIndex;
}

int FindRichestFreeZone()
{
    if (CurrentGameTurn <= 1 && playerCount > 2)
    {
        FindRichestFreeZoneInitialRestrictContinent();
        return;
    }
    int PlatinumIncome = 0;
    int ZoneIndex = -1;
    for (int i = 0; i < zoneCount; i++)
        if (platinumSource[i] > PlatinumIncome
            && ZoneOwner[i] == -1
            && ZonePlayerPods[i][0] == 0 && ZonePlayerPods[i][1] == 0 && ZonePlayerPods[i][2] == 0 && ZonePlayerPods[i][3] == 0)
        {
            ZoneIndex = i;
            PlatinumIncome = platinumSource[i];
        }
    return ZoneIndex;
}

int FindFreeZone()
{
    int ZoneIndex = -1;
    for (int i = 0; i < zoneCount; i++)
        if (ZoneOwner[i] == -1
            && ZonePlayerPods[i][0] == 0 && ZonePlayerPods[i][1] == 0 && ZonePlayerPods[i][2] == 0 && ZonePlayerPods[i][3] == 0)
        {
            ZoneIndex = i;
            break;
        }
    return ZoneIndex;
}

int CanReachZoneFromZoneInd(int start, int end)
{
    for (int i = 0; i < ZoneNeighbourMap[start][MAX_ZONE_COUNT - 1]; i++)
        if (ZoneNeighbourMap[start][i] == end)
            return 1;
    return 0;
}

int FindClosestFreeRichZone(int AllowToBeMine)
{
    CalculateZoneDistToPlatNotOwned();
    int BestDistance = MAX_ZONE_COUNT;
    int BestDistanceIndex = -1;
    for (int i = 0; i < zoneCount; i++)
        if (ZoneDistToPlat[i] < BestDistance
            && (ZoneOwner[i] == -1 || (AllowToBeMine == 1 && ZoneOwner[i] == myId))
            && ZonePlayerPods[i][0] == 0 && ZonePlayerPods[i][1] == 0 && ZonePlayerPods[i][2] == 0 && ZonePlayerPods[i][3] == 0)
        {
            BestDistanceIndex = i;
            BestDistance = ZoneDistToPlat[i];
        }
    return BestDistanceIndex;
}

int FindRichestDefendZone()
{
    int PlatinumIncome = 0;
    int ZoneIndex = -1;
    for (int i = 0; i < zoneCount; i++)
        if (platinumSource[i] != 0 && ZoneOwner[i] == myId)
        {
            GetZoneNeighbourCount(i);
            int MyPodCount = ZonePlayerPods[i][myId];
            if (BiggestEnemyCount > MyPodCount)
            {
                ZoneIndex = i;
                AdvisedPodSpawnCount = BiggestEnemyCount - MyPodCount;
                break;
            }
        }
    return ZoneIndex;
}

int FindFreeZoneNearMe()
{
    int ZoneIndex = -1;
    for (int i = 0; i < zoneCount; i++)
        if (ZoneOwner[i] == -1
            && ZonePlayerPods[i][0] == 0 && ZonePlayerPods[i][1] == 0 && ZonePlayerPods[i][2] == 0 && ZonePlayerPods[i][3] == 0)
        {
            GetZoneNeighbourCount(i);
            if (MyCount > 0)
            {
                ZoneIndex = i;
                break;
            }
        }
    return ZoneIndex;
}

int FindZoneNearEnemy(int DoNotCareOwner)
{
    int ZoneIndex = -1;
    for (int i = 0; i < zoneCount; i++)
        if ((ZoneOwner[i] == -1 || DoNotCareOwner == 1)
            && ZonePlayerPods[i][0] == 0 && ZonePlayerPods[i][1] == 0 && ZonePlayerPods[i][2] == 0 && ZonePlayerPods[i][3] == 0)
        {
            GetZoneNeighbourCount(i);
            if (EnemyCount > 0)
            {
                ZoneIndex = i;
                break;
            }
        }
    return ZoneIndex;
}

int FindMyZoneNearEnemy()
{
    int ZoneIndex = -1;
    int ZoneEnemyNeighbours[MAX_ZONE_COUNT];
    for (int i = 0; i < zoneCount; i++)
    {
        ZoneEnemyNeighbours[i] = -1;
        if (ZoneOwner[i] == myId)
        {
            for (int j = 0; j < zoneCount; j++)
                if (ZoneOwner[j] != myId && ZoneOwner[j] != myId2 && CanReachZoneFromZoneInd(i, j))
                {
                    GetEnemyPodCountOnZone(j);
                    if (tBiggestEnemyCount <= 0)
                        tBiggestEnemyCount = 1;
                    ZoneEnemyNeighbours[i] += tBiggestEnemyCount;
                }
        }
    }
    int MostEnemyPopulatedCount = 0;
    int MostEnemyPopulatedIndex = -1;
    for (int i = 0; i < zoneCount; i++)
        if (ZoneEnemyNeighbours[i] > MostEnemyPopulatedCount)
        {
            MostEnemyPopulatedCount = ZoneEnemyNeighbours[i];
            MostEnemyPopulatedIndex = i;
        }

    return MostEnemyPopulatedIndex;
}

//check zones we have enemies nearby and try to outnumber them
int FindBiggestEnemyGroupNeighbourMyZone()
{
    int MyZoneCount = 0;
    int PlacedPods = 0;
    for (int i = 0; i < zoneCount; i++)
        if (ZoneOwner[i] == myId)
        {
            GetZoneNeighbourCount(i);
            if (EnemyCount > 0 && BiggestEnemyCount > 0)
            {
                MyZoneCount++;
                PlacedPods += ZonePlayerPods[i][myId];
            }
        }
    int PlacedPodsAvg = 0;
    if (MyZoneCount > 0)
        PlacedPodsAvg = PlacedPods / MyZoneCount;
    for (int i = 0; i < zoneCount; i++)
        if (ZoneOwner[i] == myId && ZonePlayerPods[i][myId] <= PlacedPodsAvg)
        {
            GetZoneNeighbourCount(i);
            if (EnemyCount > 0 && BiggestEnemyCount > 0)
                return i;
        }
    return -1;
}

void CalculateZoneDistToPlatNotOwnedSolo()
{
    if (PlatZonetoPlatNotOwnedDistCalcTurn == CurrentGameTurn)
        return;
    PlatZonetoPlatNotOwnedDistCalcTurn = CurrentGameTurn;
    for (int i = 0; i < zoneCount; i++)
        for (int j = 0; j < zoneCount; j++)
            ZoneDistToPlatNotOwnedSolo[i][j] = MAX_ZONE_COUNT;
    EchoModVal = 1;
    for (int i = 0; i < zoneCount; i++)
        if (platinumSource[i] > 0 && ZoneOwner[i] != myId)
        {
            ZoneDistToPlatNotOwnedSolo[i][i] = 0;
            EchoZoneDistanceOnMapRec(&ZoneDistToPlatNotOwnedSolo[i][0], i, 0);
        }
}

int FindPlatEchoSource(int AtIndex)
{
    CalculateZoneDistToPlatNotOwnedSolo();
    for (int i = 0; i < zoneCount; i++)
        if (ZoneDistToPlatNotOwnedSolo[i][AtIndex] == ZoneDistToPlat[AtIndex])
            return i;
    return -1;
}

int MoveCloserToUnOwnedPlatSource()
{
    CalculateZoneDistToPlatNotOwned();
    for (int i = 0; i < zoneCount; i++)
    {
        int MyPodCountOnZone = ZonePlayerPods[i][myId];
        while (MyPodCountOnZone > 0)
        {
            int BestIndex = -1;
            int BestDistance = 200;
            int MostEnemyPods2 = 1;
            int BestPlatSource = 0;
            for (int j = 0; j < zoneCount; j++)
                if (ZoneDistToPlat[j] <= ZoneDistToPlat[i] && ZoneDistToPlat[i] != MAX_ZONE_COUNT && CanReachZoneFromZoneInd(i, j))
                {
//                    int EchoSource = FindPlatEchoSource(j);
//                    fprintf(stderr, "Echo source for %d->%d\n", zoneId[j], zoneId[EchoSource]);
//                    if (EchoSource > -1 && PlatObjectiveSheduled[EchoSource] != 0)
//                        continue;
//                    if (PlatObjectiveSheduled[j] != 0)
//                        continue;
//                    if(zoneId[j] == 39|| zoneId[j] == 40)
//                        fprintf(stderr, "Target node %d, plat dist %d, plat owner %d\n", zoneId[j], zoneId[EchoSource]);
                    int ShouldAttack = 0;
                    int MostEnemyPods = 1;
                    for (int k = 0; k<MAX_PLAYER_COUNT; k++)
                        if (k != myId)
                        {
                            if (ZonePlayerPods[j][k] > MostEnemyPods)
                                MostEnemyPods = ZonePlayerPods[j][k] + 1;
                            if (ZonePlayerPods[j][k] <= MyPodCountOnZone)
                                ShouldAttack++;
                        }
                    if (ShouldAttack >= 3 && MyPodCountOnZone >= MostEnemyPods && ( ( BestDistance == ZoneDistToPlat[j] && platinumSource[j] > BestPlatSource )|| (BestDistance > ZoneDistToPlat[j] ) ))
                    {
                        BestPlatSource = platinumSource[j];
                        BestDistance = ZoneDistToPlat[j];
                        BestIndex = j;
                        if (MostEnemyPods > MostEnemyPods2)
                            MostEnemyPods2 = MostEnemyPods;
                    }
                    //                    else
                    //                        fprintf(stderr, "%d->%d %d %d\n", zoneId[i], zoneId[j], ZonePlayerPods[i][myId], ShouldAttack);
                }
            if (BestIndex != -1)
            {
                DEBUG_MOVE( MostEnemyPods2, zoneId[i], zoneId[BestIndex], "MoveCloserToUnOwnedPlatSource");
                //                ZoneOwner[BestIndex] = myId2;
                ZonePlayerPods[i][myId] -= MostEnemyPods2;
                MyPodCountOnZone -= MostEnemyPods2;
                PlatObjectiveSheduled[BestIndex] = 1;
//                int EchoSource = FindPlatEchoSource(BestIndex);
//                if(EchoSource > -1)
//                    PlatObjectiveSheduled[EchoSource] = 1;
//                PlatZonetoPlatNotOwnedDistCalcTurn = -1;
//                ZoneOwner[EchoSource] = myId2;
                
            }
            else
                break;
        }
    }
    return 0;
}

int MoveCloserToEnemyTeritory()
{
    CalculateZoneDistToEnemyTeritory();
    for (int i = 0; i < zoneCount; i++)
    {
        int MyPodCountOnZone = ZonePlayerPods[i][myId];
        if (MyPodCountOnZone > 0)
        {
            int BestIndex = -1;
            int BestDistance = 200;
            for (int j = 0; j < zoneCount; j++)
                if (ZoneDistToEnemy[j] < ZoneDistToEnemy[i]
                    && CanReachZoneFromZoneInd(i, j)
                    && BestDistance > ZoneDistToEnemy[j]
                    )
                {
                    BestDistance = ZoneDistToEnemy[j];
                    BestIndex = j;
                }
            if (BestIndex != -1)
            {
                DEBUG_MOVE( MyPodCountOnZone, zoneId[i], zoneId[BestIndex], "MoveCloserToEnemyTeritory");
                ZonePlayerPods[i][myId] -= MyPodCountOnZone;
                return 1;
            }
        }
    }
    return 0;
}

int MoveCloserToNeutralTeritory()
{
    CalculateZoneDistToNeutralTeritory();
    for (int i = 0; i < zoneCount; i++)
    {
        int MyPodCountOnZone = ZonePlayerPods[i][myId];
        if (MyPodCountOnZone > 0)
        {
            //if( zoneId[i]==89)fprintf(stderr, "zone89 has a pod. Zone dist to neutral = %d\n",ZoneDistToNeutral[i]);        
            int BestIndex = -1;
            int BestDistance = MAX_ZONE_COUNT;
            for (int j = 0; j < zoneCount; j++)
            {
                //if( zoneId[i]==89&& zoneId[j]==78)fprintf(stderr, "\tZone %d dist to neutral = %d, canreach = %d, bestd %d, 89 owner %d\n",zoneId[j],ZoneDistToNeutral[j],CanReachZoneFromZoneInd(i, j),BestDistance,ZoneOwner[i]);        
                if (ZoneDistToNeutral[j] <= ZoneDistToNeutral[i] && ZoneDistToNeutral[i] != MAX_ZONE_COUNT && CanReachZoneFromZoneInd(i, j) && BestDistance >= ZoneDistToNeutral[j])
                {
                    BestDistance = ZoneDistToNeutral[j];
                    BestIndex = j;
                }
            }
            if (BestIndex != -1)
            {
                DEBUG_MOVE( MyPodCountOnZone, zoneId[i], zoneId[BestIndex], "MoveCloserToNeutralTeritory");
                ZonePlayerPods[i][myId] -= MyPodCountOnZone;
                return 1;
            }
        }
    }
    return 0;
}

//these are non platinum zones. Try to get as many as possible
int AttackZoneWeCanConquer()
{
    for (int i = 0; i < zoneCount; i++)
    {
        int MyPodCountOnZone = ZonePlayerPods[i][myId];
        while (MyPodCountOnZone > 0)
        {
            int DidSomething = 0;
            for (int j = 0; j < zoneCount; j++)
                if (i != j && ZoneOwner[j] != myId && ZoneOwner[j] != myId2 && PlatObjectiveSheduled[j] == 0 && CanReachZoneFromZoneInd(i, j))
                {
                    int ShouldAttack = 0;
                    int MostEnemyPods = 0;
                    for (int k = 0; k<MAX_PLAYER_COUNT; k++)
                        if (k != myId)
                        {
                            if (ZonePlayerPods[j][k] > MostEnemyPods)
                                MostEnemyPods = ZonePlayerPods[j][k];
                            if (ZonePlayerPods[j][k] < MyPodCountOnZone)
                                ShouldAttack++;
                        }
                    MostEnemyPods++;
                    if (MostEnemyPods <= 0)
                        MostEnemyPods = 1;
                    if (ShouldAttack >= 3 && MyPodCountOnZone >= MostEnemyPods)
                    {
                        DEBUG_MOVE( MostEnemyPods, zoneId[i], zoneId[j], "AttackZoneWeCanConquer");
                        ZoneOwner[j] = myId2;
                        ZonePlayerPods[i][myId] -= MostEnemyPods;
                        MyPodCountOnZone -= MostEnemyPods;
                        if (MyPodCountOnZone == 0)
                            break;
                        PlatObjectiveSheduled[j] = 1;
                        DidSomething = 1;
                    }
                    //                    else
                    //                        fprintf(stderr, "%d->%d %d %d\n",zoneId[i],zoneId[j],ZonePlayerPods[i][myId],ShouldAttack);
                }
            if (DidSomething == 0)
                break;
        }
    }
    return 0;
}

int MoveDefendPlatSource()
{
    for (int i = 0; i < zoneCount; i++)
    {
        int MyPodCountOnZone = ZonePlayerPods[i][myId];
        if (MyPodCountOnZone > 0 && platinumSource[i] > 0)
        {
            for (int j = 0; j < zoneCount; j++)
                if (i != j && ZoneOwner[j] != myId && ZoneOwner[j] != myId2 && CanReachZoneFromZoneInd(i, j)
                    && platinumSource[i] <= 0   //aggressive behavior, we want to take over enemy platinum even if he is near us
                    )
                {
                    GetEnemyPodCountOnZone(j);
                    int MostEnemyPods = tBiggestEnemyCount;
                    if (MostEnemyPods>MyPodCountOnZone)
                        ZonePlayerPods[i][myId] = 0;    // do not move my pods from this plat source. Maybe Enemy will not attack us
                    else
                        ZonePlayerPods[i][myId] -= MostEnemyPods;    // we could move out a few pods. Maybe we can capture something nearby
                                                                          //                    else
                                                                          //                        fprintf(stderr, "%d->%d %d %d\n",zoneId[i],zoneId[j],ZonePlayerPods[i][myId],ShouldAttack);
                }
        }
    }
    return 0;
}

void DebugShowPlayerPlatIncome()
{
    int PlayerPlatIncome[MAX_PLAYER_COUNT];
    memset(PlayerPlatIncome, 0, sizeof(PlayerPlatIncome));
    for (int i = 0; i < zoneCount; i++)
        if (platinumSource[i] && ZoneOwner[i] != -1)
            PlayerPlatIncome[ZoneOwner[i]] += platinumSource[i];
    for (int i = 0; i < MAX_PLAYER_COUNT; i++)
        fprintf(stderr, "Player %d plat income %d\n", i, PlayerPlatIncome[i]);
}

//#define DEBUG_SPAWNREASON( r, at ) if (BestFreeZoneIndex != -1 && ShowDebug==1){fprintf(stderr, r, at);ShowDebug=0;}
#define DEBUG_SPAWNREASON( r, at ) if (0==1)fprintf(stderr, r);

int main()
{
    scanf("%d%d%d%d", &playerCount, &myId, &zoneCount, &linkCount);
    fprintf(stderr, "id=%d\n", myId);
    for (int i = 0; i < zoneCount; i++)
        scanf("%d%d", &zoneId[i], &platinumSource[i]);
    for (int i = 0; i < linkCount; i++)
        scanf("%d%d", &Links[i][0], &Links[i][1]);

    memset(ZoneOwner, -1, sizeof(ZoneOwner));
    memset(ZonePlayerPods, 0, sizeof(ZonePlayerPods));
    CurrentGameTurn = 0;

    BuildZoneNeighbourMap();
    CalcZoneContinent();
    CalcContinentPlatinumIncome();
    CalculateZoneDistToPlatNeutralSolo();

    // game loop
    while (1)
    {
        scanf("%d", &platinum);
        for (int i = 0; i < zoneCount; i++)
        {
            int zId; // this zone's ID
            scanf("%d%d%d%d%d%d", &zId, &ZoneOwner[i], &ZonePlayerPods[i][0], &ZonePlayerPods[i][1], &ZonePlayerPods[i][2], &ZonePlayerPods[i][3]);
            if (zId != zoneId[i])
                fprintf(stderr, "Init error\n");
        }

        DebugShowPlayerPlatIncome();
        memset(PlatObjectiveSheduled, 0, sizeof(PlatObjectiveSheduled));

        //zone importance : defend my platinum source
        //zone importance : enemy owned platinum source
        MoveDefendPlatSource();
        while (MoveCloserToUnOwnedPlatSource());
        while (AttackZoneWeCanConquer());
        while (MoveCloserToEnemyTeritory());
        while (MoveCloserToNeutralTeritory());
        printf("\n");

        //try to buy pods
        //strategy 1: find most worthy platinum source zones and direct place them
        //strategy 2: find worthy zone and also count nearby worthy zones
        //strategy 3: worthy zone, but keep in mind nearby enemy
        while (platinum >= POD_PRICE)
        {
            int BestFreeZoneIndex = -1;
            AdvisedPodSpawnCount = 1;
            int ShowDebug = 1;

            if (BestFreeZoneIndex == -1)
                BestFreeZoneIndex = FindRichestDefendZone();
            DEBUG_SPAWNREASON("SpawnReason %d FindRichestDefendZone\n", BestFreeZoneIndex);

            if (BestFreeZoneIndex == -1)
                BestFreeZoneIndex = FindNextBestScorePlatNodeFree();
            DEBUG_SPAWNREASON("SpawnReason %d FindNextBestScorePlatNodeFree\n", BestFreeZoneIndex);

            if (BestFreeZoneIndex == -1)
                BestFreeZoneIndex = FindRichestFreeZone();
            DEBUG_SPAWNREASON("SpawnReason %d FindRichestFreeZone\n", BestFreeZoneIndex);

            if (BestFreeZoneIndex == -1)
                BestFreeZoneIndex = FindClosestFreeRichZone(0);
            DEBUG_SPAWNREASON("SpawnReason %d FindClosestFreeRichZone(0)\n", BestFreeZoneIndex);

            if (BestFreeZoneIndex == -1)
                BestFreeZoneIndex = FindClosestFreeRichZone(1);
            DEBUG_SPAWNREASON("SpawnReason %d FindClosestFreeRichZone(1)\n", BestFreeZoneIndex);

            if (BestFreeZoneIndex == -1)
                BestFreeZoneIndex = FindZoneNearEnemy(0);
            DEBUG_SPAWNREASON("SpawnReason %d FindFreeZoneNearEnemy()\n", BestFreeZoneIndex);

            if (BestFreeZoneIndex == -1)
                BestFreeZoneIndex = FindFreeZoneNearMe();
            DEBUG_SPAWNREASON("SpawnReason %d FindFreeZoneNearMe()\n", BestFreeZoneIndex);

            if (BestFreeZoneIndex == -1)
                BestFreeZoneIndex = FindFreeZone();
            DEBUG_SPAWNREASON("SpawnReason %d FindFreeZone()\n", BestFreeZoneIndex);

            if (BestFreeZoneIndex == -1)
                BestFreeZoneIndex = FindZoneNearEnemy(1);
            DEBUG_SPAWNREASON("SpawnReason %d FindMyZoneNearEnemy()\n", BestFreeZoneIndex);

            //            if( BestFreeZoneIndex == -1 )
            //                BestFreeZoneIndex = FindNeutralNeighbourToRichZone();

            if (BestFreeZoneIndex == -1)
                BestFreeZoneIndex = FindBiggestEnemyGroupNeighbourMyZone();
            DEBUG_SPAWNREASON("SpawnReason %d FindBiggestEnemyGroupNeighbourMyZone()\n", BestFreeZoneIndex);

            int PodsCanSpawn = platinum / POD_PRICE;
            if (AdvisedPodSpawnCount > PodsCanSpawn)
                AdvisedPodSpawnCount = PodsCanSpawn;
            if (BestFreeZoneIndex != -1 && AdvisedPodSpawnCount > 0)
            {
                printf("%d %d ", AdvisedPodSpawnCount, zoneId[BestFreeZoneIndex]);
                platinum -= (POD_PRICE*AdvisedPodSpawnCount);
                ZonePlayerPods[BestFreeZoneIndex][myId] += AdvisedPodSpawnCount;
            }

            if (BestFreeZoneIndex == -1)
                break;
        }
        // Write an action using printf(). DON'T FORGET THE TRAILING \n
        // To debug: fprintf(stderr, "Debug messages...\n");
        printf("\n");
        CurrentGameTurn++;
    }

    return 0;
}