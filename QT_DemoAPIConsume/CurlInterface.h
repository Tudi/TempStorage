#ifndef CURLINTERFACE_H
#define CURLINTERFACE_H

//should end with / since we will append url parts to it
#define API_SERVER_URL "https://api.quwi.com/v2/"

struct MemoryStruct {
    char* memory;
    size_t size;
};

enum CURL_FLAGS
{
    CF_IS_GET = 1,
    CF_IS_LOGIN = 2, // save cookie that will be used later
};

MemoryStruct *GetAPIReply(const char *Url, const char *Body, int Flags = 0);

char *EscapeString(const char *);

#endif // CURLINTERFACE_H
