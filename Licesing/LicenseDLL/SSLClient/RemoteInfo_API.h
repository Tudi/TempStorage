#pragma once

#ifndef LIBRARY_API
	#include "../stdafx.h"
#endif

#define ERROR_NETWORK_OPERATION_TIMEOUT	(-4)

//connect to a remote Fingerprint service and obtain the UUID
/*
param RemoteIP : remote host IP. Ex : 192.168.0.1
param RemotePort : remote host port he listnes to
param ret_UUID : buffer where to store the UUID obtained from the host
param UUIDSize : size of the buffer where we can store the result. Should be 16 bytes
return - error code. 0 on success
*/
extern "C"
{
	LIBRARY_API int StartFingerprintService(int ListenPort, char *SSLCertificatePath);
	LIBRARY_API int GetRemoteUUID(char *RemoteIP, int RemotePort, char *ret_UUID, int UUIDSize);
}