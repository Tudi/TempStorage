#include <stdio.h>
#include <vector>
#include <stdarg.h>
#include <inttypes.h>
#include <string.h>
#include <assert.h>

// would need to extend this enum for every return to make specific error codes
enum ErrCodes {
	ERR_NONE = 0,
	ERR_FUNC_PARAM_INVALID,
	ERR_GENERIC
};

enum BallDirections
{
	Dir_N = 0,
	Dir_NE = 1,
	Dir_E = 2,
	Dir_ES = 3,
	Dir_S = 4,
	Dir_SW = 5,
	Dir_W = 6,
	Dir_WN = 7,
	Dir_Invalid = 8, // must be last + 1 !
};

typedef struct PlayerStore
{
	int64_t X, Y;
	int64_t Index; // custom. Not given by input
	int IsPlaying; // for logging : already received a ball ?
	BallDirections DirReceived; // for logging
	BallDirections DirGiven; // for logging
}PlayerStore;

typedef struct BoardStore
{
	int64_t MinX,MaxX,MinY,MaxY; // custom. Not provided by input
	int64_t NumberOfPlayers;
	std::vector<PlayerStore> Players; // could be a list to remove players that received balls. Leave it here for debugging
	char sFirstBallDirection[3];
	BallDirections FirstBallDirection; // translated from string
	int64_t FirstPlayerIndex;
}BoardStore;

typedef struct OutputStore
{
	int64_t ThrowsMade;
	int64_t LastPlayerIndex;
}OutputStore;

#define abs(x) ((x<0?(-x):(x)))

#ifdef _MSC_VER
	#define fscanf(...) fscanf_s(__VA_ARGS__)
#endif

// Make it a whole module for real cases
void MyLogModuleFunc(const char* msgFormat, ...)
{
	#define MAX_LOG_MESSAGE_SIZE 16000
	char buffer[MAX_LOG_MESSAGE_SIZE];
	va_list args;
	va_start(args, msgFormat);

	int length = vsnprintf(buffer, sizeof(buffer), msgFormat, args);

	va_end(args);

	// !! message was too large to fit into max size. Truncate it
	if (!(length >= 0 && length < sizeof(buffer)))
	{
		buffer[sizeof(buffer) - 1] = 0;
	}

	printf("%s", buffer);
}

int ReadPlayer(FILE* pfInput, PlayerStore* pOutPlayer)
{
	if (pfInput == NULL)
	{
		MyLogModuleFunc("Missing pfInput param\n");
		return ERR_FUNC_PARAM_INVALID;
	}
	if (pOutPlayer == NULL)
	{
		MyLogModuleFunc("Missing pOutPlayer param\n");
		return ERR_FUNC_PARAM_INVALID;
	}
	if (fscanf(pfInput, "%lld %lld\n", &pOutPlayer->X, &pOutPlayer->Y) != 2)
	{
		MyLogModuleFunc("Failed to read player coords\n");
		return ERR_GENERIC;
	}

	return ERR_NONE;
}

int ReadTestCase(FILE* pfInput, BoardStore* pOutBoard)
{
	if (pfInput == NULL)
	{
		return ERR_FUNC_PARAM_INVALID;
	}
	if (pOutBoard == NULL)
	{
		return ERR_FUNC_PARAM_INVALID;
	}

	if (fscanf(pfInput, "%lld\n", &pOutBoard->NumberOfPlayers) == 0)
	{
		MyLogModuleFunc("Failed to read NumberOfPlayers\n");
		return ERR_GENERIC;
	}
	if (pOutBoard->NumberOfPlayers < 2 || pOutBoard->NumberOfPlayers > 1000)
	{
		MyLogModuleFunc("NumberOfPlayers 2 < %lld < 1000 out of bounds\n", pOutBoard->NumberOfPlayers);
		return ERR_GENERIC;
	}
	pOutBoard->Players.reserve(pOutBoard->NumberOfPlayers + 1);
	for (int64_t i = 0; i < pOutBoard->NumberOfPlayers; i++)
	{
		PlayerStore player;
		memset(&player, 0, sizeof(PlayerStore));
		player.Index = i;
		player.IsPlaying = 1;
		player.DirGiven = Dir_Invalid;
		player.DirReceived = Dir_Invalid;
		if (ReadPlayer(pfInput, &player))
		{
			return ERR_GENERIC;
		}
		pOutBoard->Players.push_back(player);
	}
#ifdef _MSC_VER
	if (fscanf_s(pfInput, "%s\n", pOutBoard->sFirstBallDirection, (unsigned int)sizeof(pOutBoard->sFirstBallDirection)) != 1)
#else
	if (fscanf(pfInput, "%s\n", pOutBoard->sFirstBallDirection) != 1)
#endif
	{
		MyLogModuleFunc("Failed to read sFirstBallDirection\n");
		return ERR_GENERIC;
	}
	pOutBoard->sFirstBallDirection[2] = 0;

	// convert string direction to numeric
	if (strcmp(pOutBoard->sFirstBallDirection, "N") == 0) 
	{
		pOutBoard->FirstBallDirection = Dir_N;
	}
	else if(strcmp(pOutBoard->sFirstBallDirection, "E") == 0)
	{
		pOutBoard->FirstBallDirection = Dir_E;
	}
	else if (strcmp(pOutBoard->sFirstBallDirection, "S") == 0)
	{
		pOutBoard->FirstBallDirection = Dir_S;
	}
	else if (strcmp(pOutBoard->sFirstBallDirection, "W") == 0)
	{
		pOutBoard->FirstBallDirection = Dir_W;
	}
	else if (strcmp(pOutBoard->sFirstBallDirection, "NE") == 0)
	{
		pOutBoard->FirstBallDirection = Dir_NE;
	}
	else if (strcmp(pOutBoard->sFirstBallDirection, "NW") == 0)
	{
		pOutBoard->FirstBallDirection = Dir_WN;
	}
	else if (strcmp(pOutBoard->sFirstBallDirection, "SW") == 0)
	{
		pOutBoard->FirstBallDirection = Dir_SW;
	}
	else if (strcmp(pOutBoard->sFirstBallDirection, "SE") == 0)
	{
		pOutBoard->FirstBallDirection = Dir_ES;
	}
	else
	{
		MyLogModuleFunc("Failed to convert sFirstBallDirection\n");
		return ERR_GENERIC;
	}
	if (fscanf(pfInput, "%lld\n", &pOutBoard->FirstPlayerIndex) == 0)
	{
		MyLogModuleFunc("Failed to read FirstPlayerIndex\n");
		return ERR_GENERIC;
	}
	pOutBoard->FirstPlayerIndex -= 1; // because first was indexed was 1
	if (pOutBoard->FirstPlayerIndex < 0 || pOutBoard->FirstPlayerIndex >= (int64_t)pOutBoard->Players.size())
	{
		MyLogModuleFunc("FirstPlayerIndex 0 < %lld < %llu out of bounds\n", pOutBoard->FirstPlayerIndex, pOutBoard->Players.size());
		return ERR_GENERIC;
	}

	return ERR_NONE;
}

template<int64_t XMul, int64_t YMul>
int64_t FindClosestPlayer(int64_t BallAtPlayer, BoardStore* pBoard)
{
	assert(!(XMul == 0 && YMul == 0));

	// closes player to the N, will have minY
	int64_t ChosenIndex = BallAtPlayer;
	int64_t ChoosenDist;
	for (size_t i = 0; i < pBoard->Players.size(); i++)
	{
		// this player is out
		if (pBoard->Players[i].IsPlaying == 0)
		{
			continue;
		}
		int64_t diffY = pBoard->Players[i].Y - pBoard->Players[BallAtPlayer].Y;
		//flip y because it was give that -y is at bottom 
		diffY = -diffY;

		if (YMul == -1)
		{
			if (diffY >= 0)
			{
				continue;
			}
		}
		if (YMul == 0)
		{
			if (diffY != 0)
			{
				continue;
			}
		}
		if (YMul == 1)
		{
			if (diffY <= 0)
			{
				continue;
			}
		}
		int64_t diffX = pBoard->Players[i].X - pBoard->Players[BallAtPlayer].X;
		if (XMul == -1)
		{
			if (diffX >= 0)
			{
				continue;
			}
		}
		if (XMul == 0)
		{
			if (diffX != 0)
			{
				continue;
			}
		}
		if (XMul == 1)
		{
			if (diffX <= 0)
			{
				continue;
			}
		}
		int64_t diff;
		if (XMul != 0 && YMul != 0)
		{
			diffY = abs(diffY);
			diffX = abs(diffX);
			if (diffY != diffX)
			{
				continue;
			}
			diff = diffX;
		}
		else if (XMul != 0)
		{
			diff = abs(diffX);
		}
		else
		{
			diff = abs(diffY);
		}
		// first time init
		if (ChosenIndex == BallAtPlayer)
		{
			ChoosenDist = diff;
			ChosenIndex = i;
			continue;
		}
		assert(ChoosenDist != diff);
		if (diff < ChoosenDist)
		{
			ChoosenDist = diff;
			ChosenIndex = i;
		}
	}
	return ChosenIndex;
}

int ProcessBoard(BoardStore* pBoard, OutputStore* outRes)
{
	if (pBoard == NULL)
	{
		MyLogModuleFunc("Missing pBoard param\n");
		return ERR_FUNC_PARAM_INVALID;
	}
	if (outRes == NULL)
	{
		MyLogModuleFunc("Missing outRes param\n");
		return ERR_FUNC_PARAM_INVALID;
	}

	int64_t BallAtPlayer = pBoard->FirstPlayerIndex;
	BallDirections BallDir = pBoard->FirstBallDirection;
	outRes->ThrowsMade = 0;
	do {
		pBoard->Players[BallAtPlayer].DirReceived = BallDir;
		pBoard->Players[BallAtPlayer].IsPlaying = 0;
		int64_t ChosenIndex = BallAtPlayer;
		BallDirections throwDirection = Dir_Invalid;
		BallDirections incomingDirection = Dir_Invalid;
		for (size_t NextDirection = (size_t)BallDir + 1; NextDirection <= (size_t)BallDir + Dir_Invalid; NextDirection++)
		{
			BallDirections NextDir = (BallDirections)(NextDirection % Dir_Invalid);
			// can we find a player on this line ?
			if (NextDir == Dir_N)
			{
				ChosenIndex = FindClosestPlayer<0,-1>(BallAtPlayer, pBoard);
			}
			else if (NextDir == Dir_S)
			{
				ChosenIndex = FindClosestPlayer<0, 1>(BallAtPlayer, pBoard);
			}
			else if (NextDir == Dir_E)
			{
				ChosenIndex = FindClosestPlayer<1, 0>(BallAtPlayer, pBoard);
			}
			else if (NextDir == Dir_W)
			{
				ChosenIndex = FindClosestPlayer<-1, 0>(BallAtPlayer, pBoard);
			}
			else if (NextDir == Dir_NE)
			{
				ChosenIndex = FindClosestPlayer<1, -1>(BallAtPlayer, pBoard);
			}
			else if (NextDir == Dir_ES)
			{
				ChosenIndex = FindClosestPlayer<1, 1>(BallAtPlayer, pBoard);
			}
			else if (NextDir == Dir_SW)
			{
				ChosenIndex = FindClosestPlayer<-1, 1>(BallAtPlayer, pBoard);
			}
			else if (NextDir == Dir_WN)
			{
				ChosenIndex = FindClosestPlayer<-1, -1>(BallAtPlayer, pBoard);
			}
			if (ChosenIndex != BallAtPlayer)
			{
				throwDirection = NextDir;
				// convert to incomming direction
				incomingDirection = (BallDirections)((NextDir + Dir_Invalid / 2) % Dir_Invalid);
				break;
			}
		}
		if (ChosenIndex == BallAtPlayer)
		{
			outRes->LastPlayerIndex = BallAtPlayer;
			return ERR_NONE;
		}
		pBoard->Players[BallAtPlayer].DirGiven = throwDirection;
		BallAtPlayer = ChosenIndex;
		BallDir = incomingDirection;
		outRes->ThrowsMade++;
	} while (1);

	return ERR_NONE;
}

int ProcessInputFile(const char* sFileName, std::vector<OutputStore> &vout_Results)
{
	// sanity check
	if (sFileName == NULL)
	{
		MyLogModuleFunc("Missing sFileName param\n");
		return ERR_FUNC_PARAM_INVALID;
	}

	// open input file
	FILE* fInputFile = NULL;
#ifdef _MSC_VER
	errno_t open_er = fopen_s(&fInputFile, sFileName, "rt");
	if (open_er != 0 || fInputFile == NULL)
#else 
	fInputFile = fopen(sFileName, "rt");
	if(fInputFile == NULL)
#endif
	{
		MyLogModuleFunc("Failed to open input file %s\n", sFileName);
		return ERR_GENERIC;
	}

	// fetch number of tests
	int nTestCases = 0;
	if (fscanf(fInputFile, "%d\n", &nTestCases) == 0)
	{
		MyLogModuleFunc("Failed to read nTestCases\n");
		fclose(fInputFile);
		return ERR_GENERIC;
	}
	if (nTestCases <= 0)
	{
		MyLogModuleFunc("0 < nTestCases out of bounds\n");
		fclose(fInputFile);
		return ERR_GENERIC;
	}
	vout_Results.reserve(nTestCases);
	vout_Results.resize(nTestCases);
	for (int i = 0; i < nTestCases; i++)
	{
		BoardStore board;
		if (ReadTestCase(fInputFile, &board) != ERR_NONE)
		{
			continue; // bad
		}
		if (ProcessBoard(&board, &vout_Results[i]) != ERR_NONE)
		{
			continue; // bad
		}
	}

	fclose(fInputFile);

	return ERR_NONE;
}

int PrintResults(std::vector<OutputStore>& vResults)
{
	// should never happen
	if (vResults.size() == 0)
	{
		MyLogModuleFunc("0 < ResultCount(0) out of bounds\n");
		return ERR_FUNC_PARAM_INVALID;
	}

	// format outputs
	for (size_t i = 0; i < vResults.size(); i++)
	{
		printf("%lld %lld\n", vResults[i].ThrowsMade, vResults[i].LastPlayerIndex + (int64_t)1);
	}

	return ERR_NONE;
}

int main(int argc, char* argv[])
{
	std::vector<OutputStore> vResults;
	int err = ERR_NONE;
	const char* sFileName = "test.in";
	
	// not expecting to be used, but we do accept input file name as a command line argument
	if (argc == 2)
	{
		sFileName = argv[1];
	}

	// read the inputs + play the game
	err = ProcessInputFile(sFileName, vResults);
	if (err != ERR_NONE)
	{
		return 1;
	}

	// print out results in a formatted way
	err = PrintResults(vResults);
	if (err != ERR_NONE)
	{
		return 1;
	}

	// looks like all went well
	return 0;
}