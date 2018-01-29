#include "../SSLClient/RemoteInfo_API.h"
#include "GenericServer.h"
#include <QtCore/QCoreApplication>
#include "../Config/ConfigLoader.h"
#include <iostream>

int StartFingerprintService(int ListenPort, char *ConfigCertificatesPath)
{
	if (ListenPort <= 0)
	{
		DEBUG_SSL_PRINT(std::cout << "Port is invalid " << ListenPort << ". Exiting." << std::endl;);
		return 1;
	}
	if (ConfigCertificatesPath == NULL)
	{
		DEBUG_SSL_PRINT(std::cout << "Ceritificate path is invalid " << ConfigCertificatesPath << ". Exiting." << std::endl;);
		return 1;
	}

	int argc2 = 0;
	char **argv2 = NULL;
	QCoreApplication a(argc2, argv2);

	Server SSLServer;
	if (SSLServer.LoadCertificates(QString(ConfigCertificatesPath)))
	{
		DEBUG_SSL_PRINT(std::cout << "Could not load certificates from " << ConfigCertificatesPath << ". Exiting." << std::endl;);
		return 1;
	}
	SSLServer.ToggleStartStopListening(QHostAddress::Any, ListenPort);
	a.exec();

	return 0;
}
