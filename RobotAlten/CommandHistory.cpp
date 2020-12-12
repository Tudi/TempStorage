#include <assert.h>
#include "CommandHistory.h"
#include "WareHouseMap.h"
#include "CommandParser.h"

CommandHistory UniqueCommandHistory;
TempCommandHistory UniqueTempCommandHistory;

void CommandHistory::StoreCommand(const char* cmd)
{
	StoredCommands.push_front(cmd);
}

CommandHistory::~CommandHistory()
{
	for (auto itr = StoredCommands.begin(); itr != StoredCommands.end(); itr++)
		delete (*itr);
	StoredCommands.clear();
}

void CommandHistory::UndoLastCommand()
{
	//nothing to undo
	if (StoredCommands.empty())
		return;
	//undo previous command
	const char* t = StoredCommands.front();
	//parse as usual, but specify we want to reverse the action
	ParseCommand(t, 1);
	//remove from undo list
	StoredCommands.pop_front();	
	//add to redo list
	StoredCommandsRedo.push_front(t);
}

void CommandHistory::RedoLastCommand()
{
	if (StoredCommandsRedo.empty())
		return;
	const char* t = StoredCommandsRedo.front();
	ParseCommand(t);
	StoredCommandsRedo.pop_front();
	StoredCommands.push_front(t);
}

class TempMoveCmd
{
public:
	int sx, sy, dx, dy;
};

void TempCommandHistory::StoreCommand(int srcx, int srcy, int dstx, int dsty)
{
	TempMoveCmd* t = new TempMoveCmd();
	t->sx = srcx;
	t->sy = srcy;
	t->dx = dstx;
	t->dy = dsty;
	StoredCommands.push_front(t);
}

void TempCommandHistory::UndoStoredCommands()
{
	while (StoredCommands.empty() == false)
	{
		TempMoveCmd* t = StoredCommands.front();

		Box* tb = sWarehouse.RemoveBox(t->dx, t->dy);
		assert(tb != NULL);
		sWarehouse.AddBox(t->sx, t->sy, tb);

		StoredCommands.pop_front();
		delete t;
	}
}

TempCommandHistory::~TempCommandHistory()
{
	for (auto itr = StoredCommands.begin(); itr != StoredCommands.end(); itr++)
		delete (*itr);
	StoredCommands.clear();
}

void TempCommandHistory::ClearHistory()
{
	assert(StoredCommands.empty() == true);
}
