#pragma once

void StartCounter();
double GetTimeTick();

class AsciiLoadingBar
{
public:
	AsciiLoadingBar() { State = 0; }
	char GetState() 
	{ 
		static const char LoadingBarChars[] = { '|', '\\', '-', '/' };
		State = (State + 1) % sizeof(LoadingBarChars);
		return LoadingBarChars[State];
	}
	//for people lazy to initialize a bar...
	static char GetAnyState()
	{
		static int StateG = 0;
		static const char LoadingBarChars[] = { '|', '\\', '-', '/' };
		StateG = (StateG + 1) % sizeof(LoadingBarChars);
		return LoadingBarChars[StateG];
	}
	// for people too lasy to even print out 1 char
	static void Print( int OnNewLine = 0 )
	{
		if (OnNewLine)
			printf("\r%c\b", GetAnyState());
		else
			printf("%c\b", GetAnyState());
	}
private:
	int State;
};
