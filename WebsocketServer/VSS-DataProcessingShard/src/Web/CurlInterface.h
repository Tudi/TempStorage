#pragma once

/// <summary>
/// This is how an async Web API callback should be declared
/// </summary>
typedef void (*CURL_AsyncCallback)(int err, char *response, void *userData);

// Async querie worker thread will sleep this amount of time to check if new work arrived
// If there is more work than it can handle, it will not sleep at all
#define CURL_ASYNC_LOOP_SLEEP_MS	1000

/// <summary>
/// Default code to initialize CURL library
/// </summary>
void CURL_Init();

/// <summary>
/// Default code to shut down CURL library
/// </summary>
void CURL_Shutdown();

/// <summary>
/// Main interface call to obtain the result body of a web API call
/// Communication will be done using TLS
/// </summary>
/// <param name="url"></param>
/// <param name="postData"></param>
/// <param name="out_res"></param>
/// <returns></returns>
char *GetURLResponse(const char *url, const char* postData = NULL, CURLcode* out_res = NULL);

enum CURL_ASYNC_Flags
{
	CAF_FORCE_THREAD0, // needed by calls that should execute in a serial order
};

/// <summary>
/// When you want to update a UI element, but do not wish to block the main rendering thread to do so
/// Callback function needs to be very specific for every situation. It must know what data is expected to arrive
/// </summary>
/// <param name="url">Url to access</param>
/// <param name="postData">CURL packed POST parameters</param>
/// <param name="cb">Callback function that will be called after response arrived. There is only 1 worker thread. Try to not block this thread</param>
/// <param name="userData">Callback parameters that will be passed on from caller function to the callback function</param>
void GetURLResponseAsync(const char* url, const char* postData, CURL_AsyncCallback cb, void *userData, int flags = 0);
