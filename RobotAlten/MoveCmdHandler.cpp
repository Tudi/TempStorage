#include "MoveCmdHandler.h"
#include "WareHouseMap.h"
#include "CommandHistory.h"
#include "Box.h"

class MovePosition
{
public:
	MovePosition(char* p)
	{
		if (p)
			pos = _strdup(p);
		else
			pos = NULL;
	}
	~MovePosition()
	{
		if (pos)
		{
			free(pos);
			pos = NULL;
		}
	}
	char* GetPos() { return pos; }
	char* GetContainerName() { return pos; }
	//needs proper implementation to obtain coordinates based on input strings
	void GetCoordinate(int* x, int* y)
	{
		*x = 0;
		*y = 0;
	}
private:
	char* pos;
};

bool MoveCmd::MoveBox(char* Src, char* Dst)
{
	assert(Src != NULL);
	assert(Dst != NULL);
	int dx, dy;
	int sx, sy;
	Box tsrc(Src);
	Box tdst(Dst);
	tdst.GetPosition(&dx, &dy);
	tsrc.GetPosition(&sx, &sy);

	//check if destination is free
	if (sWarehouse.HasEmptySpace(dx, dy))
		return false;
	//no reason to temp move boxes if source is not present
	if (sWarehouse.IsBoxPresent(sx, sy, tsrc.GetName()))
		return false;

	//should always be empty at this point
	sTempCmdHistory.ClearHistory();

	//make sure we can access the box we want
	//move boxes until next box is the one we are looking for
	while (strcmp(sWarehouse.GetNextBoxName(sx, sy), tsrc.GetName()) != 0)
	{
		int tx, ty;
		sWarehouse.FindFreeLocation(&tx, &ty, sx, sy, dx, dy);
		if (tx == -1 || ty == -1)
			break;
		Box *tb = sWarehouse.RemoveBox(sx, sy);
		assert(tb != NULL);
		bool ret = sWarehouse.AddBox(tx, ty, tb);
		assert(ret == true);
		sTempCmdHistory.StoreCommand(sx, sy, tx, ty);
	}

	//make sure next box is indeed what we want
	if (strcmp(sWarehouse.GetNextBoxName(sx, sy), tsrc.GetName()) == 0)
	{
		Box* tb = sWarehouse.RemoveBox(sx, sy);
		assert(tb != NULL);
		bool ret = sWarehouse.AddBox(dx, dy, tb);
		assert(ret == true);
	}

	//revert temp moves
	sTempCmdHistory.UndoStoredCommands();

	return true;
}
