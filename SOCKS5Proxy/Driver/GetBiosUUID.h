#pragma once

int DumpBIOS();
int GetBiosUUID(unsigned char *buff, int MaxLen);
int GetMotherBoardSN(unsigned char *buff, int MaxLen);