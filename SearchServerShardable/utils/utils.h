#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdio.h>

char* strdupLower(const char* str);

int crc32(int crc, const void* buf, size_t size);

#ifndef CHECK_AND_FREE
	#define CHECK_AND_FREE(val) { free(val); val = NULL; }
#endif

int getFileContents(FILE* f, unsigned char** contents, size_t* size);

#endif // UTILS_H
