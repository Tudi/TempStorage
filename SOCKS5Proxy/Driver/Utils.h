#pragma once

/*********************************************
* Some functions that do not have a specific module to put into
*********************************************/

#define IPFromBytes(x1,x2,x3,x4) ((x4<<24)|(x3<<16)|(x2<<8)|x1)

void IPv4Tostr(unsigned int IP, char* str, int size);

class LockFunctionExecution
{
public:
	LockFunctionExecution(void* Lock);
	~LockFunctionExecution();
private:
	void* pLock;
};

// number of bytes we need to allocate to be able to load file content
int GetFileSize2(const char* FileName);
// convert a string to base64encoded version of it
char* base64_encode(const unsigned char* data, size_t input_length, size_t* output_length);
unsigned char* base64_decode(const char* data, size_t input_length, size_t* output_length);

