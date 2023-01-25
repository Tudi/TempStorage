#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdio.h>

int getFileContents(FILE* f, unsigned char** contents, size_t* size);
int getFileContentsNormal(const char *fileName, unsigned char** contents, size_t* size, unsigned int expectedSize);
int getFileContentsFast(const char *fileName, unsigned char** contents, size_t* size, unsigned int expectedSize);

#endif // UTILS_H
