#include "CurlInterface.h"
#include "curl\\curl.h"
#include "qdebug.h"
#include "curl/multi.h"

#include "SessionStore.h" //temp testing

#pragma comment(lib, "../DemoAPIConsume/curl/libcurl.a")
#pragma comment(lib, "../DemoAPIConsume/curl/libcurl.dll.a")

static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct* mem = (struct MemoryStruct*)userp;

    char* ptr = (char*)realloc(mem->memory, mem->size + realsize + 1);
    if (ptr == NULL)
    {
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;

}

MemoryStruct *GetAPIReply(const char *Url, const char *Body, int Flags)
{
    static CURL* curl = NULL;
    CURLcode res;
    MemoryStruct *ret = NULL;
    //static struct curl_slist *cookies;

    if(curl == NULL)
        curl = curl_easy_init();
    else
        curl_easy_reset(curl);
    if (curl)
    {
        //curl_version_info_data* vinfo = curl_version_info(CURLVERSION_NOW);
        ret =  (MemoryStruct*)malloc(sizeof(MemoryStruct));

        ret->memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */
        ret->memory[0] = 0; // make it a "valid" string
        ret->size = 0;    /* no data at this point */

#ifndef _DEBUG
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
#else
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif

        char APIUrl[500];
        sprintf_s(APIUrl, sizeof(APIUrl), "%s%s", API_SERVER_URL, Url);
        curl_easy_setopt(curl, CURLOPT_URL, APIUrl);
        qDebug() <<"curl-url:" << APIUrl;
        qDebug() <<"curl-body:" << Body;

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, Body);

        //curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1);
        //curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        if(Flags & CF_IS_LOGIN)
        {
            curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
            curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "cookies.txt");
        }
        else
        {
            curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "cookies.txt");

//            char cookietxt[5000];
//            sprintf_s(cookietxt,sizeof(cookietxt),"token=%s;",GetSession()->GetSessionToken().toStdString().c_str());
//            curl_easy_setopt(curl, CURLOPT_COOKIE, cookietxt);

            char auth[5000];
            sprintf_s(auth,sizeof(auth),"Authorization: Bearer %s;",GetSession()->GetSessionToken().toStdString().c_str());
            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, auth);
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            curl_easy_setopt(curl, CURLOPT_XOAUTH2_BEARER, GetSession()->GetSessionToken().toStdString().c_str());

//            curl_easy_setopt(curl, CURLOPT_COOKIELIST, cookies);
//            if(cookies && cookies->data)
//                qDebug() << "added :" << cookies->data;

//            struct curl_slist *cookies;
//            curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &cookies);
//            if(cookies && cookies->data)
//                qDebug() << "cookies set :" << cookies->data;
        }

        /* cert is stored PEM coded in file... */
        /* since PEM is default, we needn't set it for PEM */
        //curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "PEM");

        /* set the cert for client authentication */
        //curl_easy_setopt(curl, CURLOPT_SSLCERT, "curl-ca-bundle.crt");

        /* set the private key (file or ID in engine) */
//            curl_easy_setopt(curl, CURLOPT_SSLKEY, pKeyName);
        if(Flags & CF_IS_GET)
            curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);

          /* send all data to this function  */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

        /* we pass our 'chunk' struct to the callback function */
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)ret);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
#ifdef _DEBUG
            const char* ErrorStr = curl_easy_strerror(res);
            fprintf(stderr, "curl_easy_perform() failed: %s\n", ErrorStr);
#endif
        }
//        if(Flags & CF_IS_LOGIN)
//        {
//            curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &cookies);
//            if(cookies && cookies->data)
//                qDebug() << "saved :" << cookies->data;
//        }
//        curl_easy_cleanup(curl);
    }
    return ret;
}

char *EscapeString(const char *str)
{
    CURL* curl;
    curl = curl_easy_init();
    if (curl)
    {
        char *ret = curl_easy_escape( curl, str , (int)strlen(str) );
        char *ret_1 = _strdup(ret);
        curl_easy_cleanup(curl);
        curl_free(ret);
        return ret_1;
    }
    return NULL;
}
