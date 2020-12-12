#include <string.h>
#include "MoveCmdHandler.h"
#include "CommandHistory.h"
#include "FillCmdHandler.h"

int ParseCommand(const char* cmd, int Inverse = 0)
{
	if (strncmp(cmd, "move ", strlen("move ")) == 0)
	{
		char mcmd[50];
		char Src[50];
		char Dst[50];
		int cnt = sscanf_s(cmd, "%s %s %s", mcmd, sizeof(mcmd), Src, sizeof(Src), Dst, sizeof(Dst));
		if (cnt != 3)
			return 1;
		if (Inverse == 0 && MoveCmd::MoveBox(Src, Dst))
			sCmdHistory.StoreCommand(cmd);
		else if (Inverse == 1)
			MoveCmd::MoveBox(Dst, Src);
		return 0;
	}
	if (strncmp(cmd, "fill ", strlen("fill ")) == 0)
	{
		char mcmd[50];
		char Src[50];
		char amt[50];
		int cnt = sscanf_s(cmd, "%s %s %s", mcmd, sizeof(mcmd), Src, sizeof(Src), amt, sizeof(amt));
		if (cnt != 3)
			return 1;
		int iamt = atoi(amt);
		if (Inverse)
			iamt = -iamt;
		if (FillCmd::FillBox(Src, iamt))
		{
			if (Inverse == 0)
				sCmdHistory.StoreCommand(cmd);
		}
		return 0;
	}
	if (strncmp(cmd, "undo", strlen("undo")) == 0)
	{
		sCmdHistory.UndoLastCommand();
		return 0;
	}
	if (strncmp(cmd, "redo", strlen("redo")) == 0)
	{
		sCmdHistory.RedoLastCommand();
		return 0;
	}
	return 1;
}