#include <QtGui/QApplication>
#include "server.h"
#include <iostream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Server SSLServer;

	SSLServer.LoadCertificates("../../Certificates/");
	SSLServer.ToggleStartStopListening(QHostAddress::Any, 8081);

	std::cout << "Waiting for new connections" << std::endl;

    return a.exec();
}
