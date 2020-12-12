#include <assert.h>
#include "FillCmdHandler.h"
#include "WareHouseMap.h"
#include "CommandHistory.h"
#include "Box.h"


bool FillCmd::FillBox(char* Src, int amt)
{
	assert(Src != NULL);
	int sx, sy;
	Box tsrc(Src);
	tsrc.GetPosition(&sx, &sy);

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
		sWarehouse.FindFreeLocation(&tx, &ty, sx, sy, -1, -1);
		if (tx == -1 || ty == -1)
			break;
		Box* tb = sWarehouse.RemoveBox(sx, sy);
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
		tb->Fill(amt);
		bool ret = sWarehouse.AddBox(sx, sy, tb);
		assert(ret == true);
	}

	//revert temp moves
	sTempCmdHistory.UndoStoredCommands();

	return true;
}