#include <utils.h>
#include <string.h>
#include <ctype.h>

//
// External interface
//

int getFileContents(FILE* f, unsigned char** contents, size_t* size)
{
    if (fseek(f, 0, SEEK_END) != 0) 
    { 
        return 1; 
    }

    *size = ftell(f);

    if (fseek(f, 0, SEEK_SET) != 0) 
    { 
        return 1; 
    }

    *contents = malloc(*size);
    if (*contents == NULL) 
    { 
        return 1; 
    }

    size_t numBytesRead = fread(*contents, 1, *size, f);
    if (numBytesRead != *size)
    {
        free(*contents);
        return 1;
    }

    return 0;
}