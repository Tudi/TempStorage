#pragma once

//Parse a string and see if we can interpret it as a valid command
//returns 0 for a valid command
int ParseCommand(const char* StringCmd, int Inverse = 0);