#include "CommandParser.h"
#include "WareHouseMap.h"

int main(int argc, char* argv[])
{
	//right now there is only 1 warehouse. Could convert to a manager ..
	sWarehouse.Init(10, 10, 10);

	//test move command
	ParseCommand("move b1 b2");

	//test fill command
	ParseCommand("fill b1 50");

	//test undo command
	ParseCommand("undo");

	//test reo command
	ParseCommand("redo");

	return 0;
}