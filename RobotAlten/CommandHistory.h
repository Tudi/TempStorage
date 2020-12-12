#pragma once

#include <list>

//store executed commands so later we can undo or redo them
class CommandHistory
{
public:
	~CommandHistory();
	//after a successfull move or fill command, we add them to the history
	void StoreCommand(const char* cmd);
	//undo the last command. Also inserts it into the redo history
	void UndoLastCommand();
	//undo the last undo = redo
	void RedoLastCommand();
private:
	std::list<const char*> StoredCommands;
	std::list<const char*> StoredCommandsRedo;
};


//store temporary move command coordinates to be able to undo them 
class TempMoveCmd;
class TempCommandHistory
{
public:
	~TempCommandHistory();
	void ClearHistory();
	void StoreCommand(int srcx, int srcy, int dstx, int dsty);
	void UndoStoredCommands();
private:
	std::list<TempMoveCmd*> StoredCommands;
};

extern CommandHistory UniqueCommandHistory;

#define sCmdHistory UniqueCommandHistory

extern TempCommandHistory UniqueTempCommandHistory;

#define sTempCmdHistory UniqueTempCommandHistory