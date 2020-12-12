#include <stdlib.h>
#include <assert.h>
#include <list>
#include "WareHouseMap.h"
#include "Box.h"

Warehouse MyUniqueWareHouse;

class BoxStack
{
public:
	BoxStack(int StackSize)
	{
		MaxSize = StackSize;
	}
	size_t GetBoxCount() { return Boxes.size(); }
	const char* GetTopBoxName()
	{
		if (Boxes.empty())
			return NULL;
		return Boxes.front()->GetName();
	}
	bool HasEmptySpace()
	{
		return (int)GetBoxCount() < MaxSize;
	}
	bool AddBox(Box *pb)	
	{
		if (HasEmptySpace() == false)
			return false;
		Boxes.push_front(pb);
		return true;
	}
	Box* RemoveBox()
	{
		if (Boxes.empty())
			return NULL;
		Box *ret = Boxes.front();
		Boxes.pop_front();
		return ret;
	}
	bool IsBoxPresent(const char* Name)
	{
		assert(Name != NULL);
		for (auto itr = Boxes.begin(); itr != Boxes.end(); itr++)
			if (strcmp((*itr)->GetName(), Name) == 0)
				return true;
		return false;
	}
private:
	int MaxSize;
	std::list< Box*> Boxes;
};

Warehouse::Warehouse()
{
	map = NULL;
	x = y = z = 0;
}

void Warehouse::Init(int px, int py, int pz)
{
	assert(px > 0 && px < MAX_MAP_SIZE);
	assert(py > 0 && py < MAX_MAP_SIZE);
	assert(pz > 0 && pz < MAX_MAP_SIZE);
	if (map)
	{
		for (int yy = 0; yy < y; yy++)
			for (int xx = 0; xx < x; xx++)
				delete map[yy * x + xx];
		free(map);
	}
	x = px;
	y = py;
	z = pz;
	map = (BoxStack**)malloc(x*y*sizeof(BoxStack*));
	for (int yy = 0; yy < y; yy++)
		for (int xx = 0; xx < x; xx++)
			map[yy * x + xx] = new BoxStack(z);
}

bool Warehouse::AddBox(int px, int py, Box* pb)
{
	assert(px >= 0 && px < x);
	assert(py >= 0 && py < y);
	assert(pb != NULL);
	assert(map != NULL);
	if (map[py * x + px]->AddBox(pb) == false)
		return false;
	return true;
}

Box* Warehouse::RemoveBox(int px, int py)
{
	assert(px >= 0 && px < x);
	assert(py >= 0 && py < y);
	assert(map != NULL);
	return map[py * x + px]->RemoveBox();
}

//this could be improved to find the closest location to a source location
void Warehouse::FindFreeLocation(int* ret_x, int* ret_y, int SourceX, int SourceY, int Skipx2, int Skipy2)
{
	*ret_x = -1;
	*ret_y = -1;
	assert(map != NULL);
	for (int yy = 0; yy < y; yy++)
		for (int xx = 0; xx < x; xx++)
		{
			if (xx == SourceX && yy == SourceY)
				continue;
			if (xx == Skipx2 && yy == Skipy2)
				continue;

			if (map[yy * x + xx]->HasEmptySpace())
			{
				*ret_x = xx;
				*ret_y = yy;
				return;
			}
		}
}

bool Warehouse::HasEmptySpace(int px, int py)
{
	assert(px >= 0 && px < x);
	assert(py >= 0 && py < y);
	assert(map != NULL);
	return map[py * x + px]->HasEmptySpace();
}

bool Warehouse::IsBoxPresent(int px, int py, const char* Name)
{
	assert(px >= 0 && px < x);
	assert(py >= 0 && py < y);
	assert(map != NULL);
	return map[py * x + px]->IsBoxPresent(Name);
}

const char* Warehouse::GetNextBoxName(int px, int py)
{
	assert(px >= 0 && px < x);
	assert(py >= 0 && py < y);
	assert(map != NULL);
	return map[py * x + px]->GetTopBoxName();
}