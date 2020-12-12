#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "Box.h"

Box::Box(const char* InitFromString)
{
	assert(InitFromString != NULL);
	Name = _strdup(InitFromString);
	//we should be able to obtain this from the init string
	x = 0;
	y = 0;
	ContentSize = 0;
}

Box::~Box()
{
	if (Name != NULL)
	{
		free(Name);
		Name = NULL;
	}
}

void Box::GetPosition(int* px, int* py)
{
	assert(px != NULL);
	assert(py != NULL);
	*px = x;
	*py = y;
}
