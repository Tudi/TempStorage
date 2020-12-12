#pragma once

//a container that will be placed in the warehouse
//container has a name that we will use to identify it
//container does not know it's own position. This
class Box
{
public:
	Box() {};
	//extract name and position from a command parameter
	Box(const char* InitFromString);
	~Box();
	//box needs to be identified before move or fill command
	const char* GetName() { return Name; }
	//this is only valid when constructed from a command parameter
	void GetPosition(int* x, int* y);
	//fill command to change the content
	void Fill(int amt)
	{
		ContentSize += amt;
	}
private:
	char* Name;
	int x, y;
	int ContentSize;
};