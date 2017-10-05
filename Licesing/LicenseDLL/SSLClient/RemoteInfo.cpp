#include "RemoteInfo_API.h"
#include "SSLClient.h"
#include <QtCore/QCoreApplication>
#include "../Config/ConfigLoader.h"

#define SINGLE_CONNECTION_FAST_QUERY_SUPPORT

#ifndef SINGLE_CONNECTION_FAST_QUERY_SUPPORT
int GetRemoteUUID(char *RemoteIP, int RemotePort, char *ret_UUID, int UUIDSize)
{
	//avoid issues
	if (UUIDSize < 16)
		return 1;

	memset(ret_UUID, 0, UUIDSize);

	int argc2 = 0;
	char **argv2 = NULL;
	QCoreApplication *a = QCoreApplication::instance();
	bool NeedsDestroy = false;
	if (a == NULL)
	{
		a = new QCoreApplication(argc2, argv2);
		NeedsDestroy = true;
	}

	Client *SSLClient = new Client(a);
	int er = SSLClient->ToggleConnectDisconnect(QString(RemoteIP), RemotePort);
	if (er == ERROR_SSL_NOT_SUPPORTED)
	{
		if (NeedsDestroy == true)
		{
			delete a;
		}
		return ERROR_SSL_NOT_SUPPORTED;
	}

	int ReceivedMessage = -1;
	DWORD Timeout = GetTickCount() + SOCKET_SEARCH_TIMEOUT_MS;
	while (ReceivedMessage != 0 && GetTickCount() < Timeout )
	{
		a->processEvents();	//faster than UI event processing
		ReceivedMessage = SSLClient->GetRemoteUUID(ret_UUID, UUIDSize);
	}
	delete SSLClient;
	SSLClient = NULL;
	if (NeedsDestroy == true)
	{
		a->exit();
		delete a;
		a = NULL;
	}

	//timouted the connection
	if (GetTickCount() >= Timeout)
		return ReceivedMessage;

	//all went well
	return 0;
}
#else
class ConnectionDestructor
{
public:
	ConnectionDestructor()
	{
		a = NULL;
		SSLClient = NULL;
		NeedsDestroy = false;
	}
	~ConnectionDestructor()
	{
		if (SSLClient != NULL
			&& NeedsDestroy == true //sadly if we are incorporated into a GUI, our parent is already dead. We have to leak this object or else we will crash on exit
			)
		{
			delete SSLClient;
		}
		SSLClient = NULL;
		if (a != NULL && NeedsDestroy == true)
		{
			a->exit();
			delete a;
		}
		a = NULL;
	}
	QCoreApplication *a;
	Client *SSLClient;
	bool NeedsDestroy;
	std::mutex mtx;
};
//static data. The destructor will get called on application exit or DLL unload. Depends on implementation
ConnectionDestructor DLLSocketDestructor;
int GetRemoteUUID(char *RemoteIP, int RemotePort, char *ret_UUID, int UUIDSize)
{
	//avoid issues
	if (UUIDSize < 16)
		return 1;

	//in case of early exit, do not return junk
	memset(ret_UUID, 0, UUIDSize);

	//make sure there is no secondary thread setting / using values
	DLLSocketDestructor.mtx.lock();

	//first time init ? Are we running inside a QT UI ?
	if (DLLSocketDestructor.a == NULL)
	{
		DLLSocketDestructor.a = QCoreApplication::instance();
	}
	//create a new "application" that we will connect to for event loop
	if (DLLSocketDestructor.a == NULL)
	{
		int argc2 = 0;
		char **argv2 = NULL;
		DLLSocketDestructor.a = new QCoreApplication(argc2, argv2);
		DLLSocketDestructor.NeedsDestroy = true;
	}

	//first time init of the SSL client
	if (DLLSocketDestructor.SSLClient == NULL)
	{
		DLLSocketDestructor.SSLClient = new Client(DLLSocketDestructor.a);
	}
	//reconnect in case Fingerprint service went away
	int er = DLLSocketDestructor.SSLClient->Connect(QString(RemoteIP), RemotePort);
	if (er == ERROR_SSL_NOT_SUPPORTED)
	{
		DLLSocketDestructor.mtx.unlock();
		return ERROR_SSL_NOT_SUPPORTED;
	}
	//query service for data
	int ReceivedMessage = -1;
	DWORD Timeout = GetTickCount() + SOCKET_SEARCH_TIMEOUT_MS;
	while (ReceivedMessage != 0 && GetTickCount() < Timeout)
	{
		DLLSocketDestructor.a->processEvents();	//faster than UI event processing
		ReceivedMessage = DLLSocketDestructor.SSLClient->GetRemoteUUID(ret_UUID, UUIDSize);
	}
	//allow socket reuse
	DLLSocketDestructor.SSLClient->ResetMessageState();

	//release resource as we no longer use it
	DLLSocketDestructor.mtx.unlock();

	//timouted the connection
	if (GetTickCount() >= Timeout)
		return ReceivedMessage;

	//all went well
	return 0;
}
#endif