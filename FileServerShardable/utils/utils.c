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

int getFileContentsNormal(const char *fileName, unsigned char** contents, size_t* size, unsigned int expectedSize)
{
	(void)expectedSize;
	FILE *f = fopen(fileName,"rb");
	if(f==NULL)
	{
		*contents = NULL;
		*size = 0;
		return 1;
	}
	getFileContents(f,contents,size);
	fclose(f);
	return 0;
}

#define  READALL_CHUNK   (32 * 1024 * 1024) 

int getFileContentsFast(const char *fileName, unsigned char** contents, size_t* size)
{
	char  *data = NULL, *temp = NULL;
    size_t size = 0;
    size_t used = 0;
    
	int fd = open(fileName, O_RDONLY);
	if(fd<0)
	{
		*dataptr = NULL;
		*sizeptr = 0;
		return __LINE__;
	}

    while (1) 
    {
        if (used + READALL_CHUNK + 1 > size) 
        {
            size = used + READALL_CHUNK + 1;
            temp = realloc(data, size);
            if (temp == NULL) 
            {
                free(data);
				close(fd);
				*dataptr = NULL;
				*sizeptr = 0;
                return __LINE__;
            }
            data = temp;
        }

        size_t n = read(fd, data + used, READALL_CHUNK);

        used += n;
        if( n < READALL_CHUNK )
        {
            break;
        }
    }
	close(fd);

    *dataptr = data;
    *sizeptr = used;

    return 0; 
}
