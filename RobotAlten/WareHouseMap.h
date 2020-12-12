#pragma once

//this should not exist. Or maybe loaded from some config file
//added here as safety measure until swapped out to something more desired
#define MAX_MAP_SIZE 50

class Box;
class BoxStack;

//store the layout where the boxes will be stacked up
//right now this is a simple square map, could be replaced later with map
class Warehouse
{
public:
	Warehouse();
	//create a warehouse based on specifications
	void Init(int x, int y, int z);
	//add a box in the warehouse to a specific location at the top of the existing box stack
	bool AddBox(int x, int y, Box* b);
	//remove a box from the top of the stack
	Box* RemoveBox(int x, int y);
	//find a location where we could stack a box. Used for temp store
	void FindFreeLocation(int* ret_x, int* ret_y, int SourceX, int SourceY, int skipx2, int skipy2);
	//check if we could store a box at a specific location
	bool HasEmptySpace(int x, int y);
	//check if a box is present in a stack
	bool IsBoxPresent(int x, int y, const char* Name);
	//check if next box needs to be removed or not
	const char* GetNextBoxName(int x, int y);
private:
	//dimensions of this map
	int x, y, z;
	//locations where boxes will be stored
	BoxStack** map;
};

//right now this is a unique instance. Later it could be converted into a map manager
extern Warehouse MyUniqueWareHouse;
#define sWarehouse MyUniqueWareHouse